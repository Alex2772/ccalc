#include "ExplorerApplication.h"
#include <algorithm>
#include "CCOSCore.h"
#include "WindowList.h"
#include "file.png.h"
#include "explorer.png.h"
#include "dir.png.h"
#include "File.h"
#include "ContextMenu.h"
#include "TextView.h"
#include "InputDialog.h"
#include "ListItem.h"
#include "MessageDialog.h"
#include "TaskHelper.h"
#include "ProgressBar.h"
#include "pct.png.h"
#include "SpinBox.h"
#include "SpinnerView.h"
#include <sys/stat.h>

void _waf_helper(void *p);

class WindowAboutFile : public WindowList {
public:
    TaskHandle_t helper = 0;
    ListItem *size;
    File file;

    WindowAboutFile(std::string f) : WindowList(f), file(f) {
        if (file.isFile() || file.isDir()) {
            mTitle = file.getFilename();
            addView(new ListItem(file.isDir() ? "Dir name" : "File name", mTitle));
            addView(new ListItem("Full path", file.convert()));
            addView(size = new ListItem("Size", "Counting..."));
            struct stat s;
            file.stat(s);
            char buf[80];

            tm *tm_info = localtime(&s.st_mtime);
            strftime(buf, 80, "%d/%m/%Y %R", tm_info);
            addView(new ListItem("Timestamp", buf));
            xTaskCreate(_waf_helper, "WAFHelper", 3 * 2048, this, 2, &helper);
        } else {
            addView(new ListItem("Error", "Couldn't read file."));
        }
    }

    virtual ~WindowAboutFile() {
        if (helper)
            vTaskDelete(helper);
    }
};

void _waf_helper(void *p) {
    WindowAboutFile *h = reinterpret_cast<WindowAboutFile *>(p);
    char buf[80];
    sprintf(buf, "%u", h->file.size());
    std::string s(buf);
    CCOSCore::runOnUiThread([h, s]() {
        h->size->setText(s);
    });
    while (1)
        vTaskDelay(portMAX_DELAY);
    vTaskDelete(0);
}


WindowCopy::WindowCopy(ExplorerWindow *e, std::string src, std::string dst) : Dialog("Copy") {
    addView(pB = new ProgressBar(2, 22, 125));
    addView(tv = new TextView("Preparing...", 2, 30));
    mHelper = new TaskHelper([&, e, src, dst]() {
        File s = src;
        File d = dst;
        size_t counter = 1;
        char buf[256];
        while (d.isDir() || d.isFile()) {
            sprintf(buf, "%s-%u", dst.c_str(), ++counter);
            d = File(buf);
        }
        if (!s.isDir() && !s.isFile()) {
            CCOSCore::removeWindow(this);
            CCOSCore::displayWindow(new MessageDialog("Error", "Src file doesn't exist"));
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
        copy(s, d.getPath());
        vTaskDelay(100 / portTICK_PERIOD_MS);
        e->refresh();
        CCOSCore::removeWindow(this);
    });
}

void WindowCopy::copy(File &s, const std::string &dst) {
    File d = dst;
    CCOSCore::runOnUiThread([&]() {
        tv->setText(std::string("Copying from ") + s.getPath() + " to " + d.getPath());
        pB->max = s.size();
        pB->current = 0;
    });
    char buf[1024];
    if (s.isDir()) {
        Dir jj(dst);
        jj.mkdir(0777);
        Dir d(s.getPath());
        std::vector<File> files;
        d.list(files);
        for (auto &f : files) {
            copy(f, dst + "/" + f.getFilename());
        }
    } else {
        s.open(File::R);
        d.open(File::W);
        while (size_t read = s.read(buf, 1024)) {
            d.write(buf, read);
            CCOSCore::runOnUiThread([&]() {
                pB->current += read;
            });
        }
    }
}

void Explorer::launch() {
    CCOSCore::displayWindow(new ExplorerWindow);
}

Bitmap Explorer::getIcon() {
    return {explorer, 30, 30};
}

const std::string Explorer::getTitle() {
    return "Explorer";
}

void ExplorerWindow::itemLongSelected(size_t index) {
    WindowList::itemLongSelected(index);
    if (!mCallback) {
        CCOSCore::vibrate(10);
        CCOSCore::displayWindow(new ContextMenu(mItems[index].mFile, {
                {"Open", [&, index]() {
                    itemSelected(index);
                }},
                i_new,
                {"Copy", [&, index]() {
                    toMC = new File(currentFile());
                    a = COPY;
                }},
                {"Move", [&, index]() {
                    toMC = new File(currentFile());
                    a = MOVE;
                }},
                {"Rename", [&, index]() {
                    CCOSCore::displayWindow(new InputDialog("Rename", mItems[index].mFile, [&](const std::string &n) {
                        std::string t = n;
                        for (size_t i = 0; i < t.length(); ++i) {
                            if (t[i] == '/') {
                                t.insert(t.begin() + i, '\\');
                                ++i;
                            }
                        }
                        rename(File(currentDir() + mItems[index].mFile).convert().c_str(),
                               File(currentDir() + n.c_str()).convert().c_str());
                        refresh();
                    }));
                }},
                {"Delete", [&, index]() {
                    std::string path = currentDir();
                    path += mItems[index].mFile;
                    File f(path);
                    f.remove();
                    refresh();
                }},
                {"Properties", [&, index]() {
                    CCOSCore::displayWindow(new WindowAboutFile(currentFile()));
                }}
        }));
    }
}

std::string ExplorerWindow::currentFile() {
    return currentDir() + mItems[currentIndex].mFile;
}

void ExplorerWindow::render(Framebuffer &fb) {
    WindowList::render(fb);
    mHighlight = toMC != nullptr || mCallback;
}

void ExplorerWindow::keyLongDown(uint8_t index) {
    WindowList::keyLongDown(index);
    switch (index) {
        case 5:
            if (views.empty()) {
                i_new.callable();
            }
            break;
    }

}

void ExplorerWindow::refresh() {
    std::string s = currentDir();

    openDir(compileBreadboard());
}

void ExplorerWindow::keyRelease(uint8_t key) {
    WindowList::keyRelease(key);
    switch (key) {
        case 3:
            if (!breadboard.empty()) {
                breadboard.pop_back();
                openDir(compileBreadboard());
            }
            break;
        case 15:
            if (toMC) {
                std::string cd = currentDir();
                if (cd.find(toMC->getPath()) == 0) {
                    CCOSCore::displayWindow(
                            new MessageDialog("Error", "Dest dir is child of the source directory."));
                } else {
                    switch (a) {
                        case MOVE: {
                            std::string n = File(currentDir()).convert() + toMC->getFilename();
                            std::string t = toMC->convert().c_str();
                            if (rename(t.c_str(), n.c_str()) == 0) {
                                refresh();
                            } else {
                                char *buf = new char[32 + n.length() + t.length()];
                                sprintf(buf, "Couldn't move.\nFrom %s\nTo %s", t.c_str(), n.c_str());
                                CCOSCore::displayWindow(new MessageDialog("Error", buf));
                                delete[] buf;
                            }
                            break;
                        }
                        case COPY:
                            CCOSCore::displayWindow(
                                    new WindowCopy(this, toMC->getPath(), currentDir() + toMC->getFilename()));
                            break;
                        default:
                            break;
                    }
                    a = NONE;
                    delete toMC;
                    toMC = nullptr;
                }
            }
            break;
    }
}

void ExplorerWindow::itemSelected(size_t index) {
    if (!mItems.empty()) {
        ExplorerItem &f = mItems[index];
        if (f.ico.data == dir) {
            breadboard.push_back(f.mFile);
            openDir(compileBreadboard());
        } else {
            std::string path = currentDir();
            path += f.mFile;
            if (mCallback) {
                mCallback(new File(path));
                mCallback = 0;
                close();
            } else {
                CCOSCore::openFile(path);
            }
        }
    }
}


ExplorerWindow::ExplorerWindow()
        : WindowList("Explorer"), i_new({
        "New", [&]() {
            CCOSCore::displayWindow(new ContextMenu("New", {
                    {"File",      [&]() {
                        CCOSCore::displayWindow(new InputDialog("File name", "", [&](const std::string &n) {
                            File f(currentDir() + n);
                            if (!f.isFile()) {
                                f.open(File::W);
                                f.close();
                            }
                            refresh();
                        }));
                    }},
                    {"Directory", [&]() {
                        CCOSCore::displayWindow(new InputDialog("Dir name", "", [&](const std::string &n) {
                            Dir f(currentDir() + n);
                            if (!f.isDir()) {
                                f.mkdir(0777);
                            }
                            refresh();
                        }));
                    }}
            }));
        }
})
{
    mSpin = std::make_shared<SpinnerView>(60, 28);
    openDir("/");
}


ExplorerWindow::~ExplorerWindow() {
    if (mCallback) {
        mCallback(nullptr);
    }
    delete toMC;
    delete mTask;

}

std::string ExplorerWindow::currentDir() {
    std::string path = compileBreadboard();
    if (path == "/") {
        path = "";
    } else {
        path += "/";
    };
    return path;
}

std::string ExplorerWindow::compileBreadboard() {
    std::string res;
    for (std::deque<std::string>::iterator i = breadboard.begin(); i != breadboard.end(); i++) {
        res += "/" + *i;
    }
    if (res.empty())
        res = "/";
    return res;
}

void ExplorerWindow::openDir(std::string path) {
    if (breadboard.empty())
        mTitle = "/";
    else mTitle = breadboard.back();
    currentIndex = 0;
    mItems.clear();
    views.clear();
    addViewS(mSpin);
    delete mTask;
    mTask = new TaskHelper([&, path]() {
        Dir dir(path);
        std::vector<File> files;
        dir.list(files);
        std::vector<ExplorerItem> items;
        for (size_t i = 0; i < files.size(); i++) {
            ExplorerItem item;
            item.mFile = files[i].getFilename();

            if (files[i].isDir()) {
                item.ico.data = ::dir;
            } else {
                size_t t = std::string::npos;
                while (1) {
                    size_t temp;
                    if (t == std::string::npos)
                        temp = item.mFile.find(".");
                    else
                        temp = item.mFile.find(".", t);
                    if (temp == std::string::npos) {
                        break;
                    }
                    t = temp + 1;
                }
                if (t != std::string::npos) {
                    std::string extension = item.mFile.substr(t);
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    if (extension == "txt") {
                        item.ico.data = ::txt;
                    } else if (extension == "bmp") {
                        item.ico.data = ::pct;
                    }
                }
            }
            items.push_back(item);
        }
        if (!items.empty()) {
            std::sort(items.begin(),
                      items.end(),
                      [](const ExplorerItem &l, const ExplorerItem &r) {
                          if ((l.ico.data == ::dir && r.ico.data == ::dir) || r.ico.data == l.ico.data) {
                              return l.mFile < r.mFile;
                          } else if (l.ico.data == ::dir) {
                              return true;
                          }
                          return false;
                      });
        }
        CCOSCore::runOnUiThread([&, items]() {
            views.clear();
            mItems = std::move(items);
            for (auto &item : mItems) {
                TextView *t = new TextView(item.mFile, 0, 0);
                t->icon = item.ico;
                addView(t);
            }
            if (!views.empty()) {
                views[0]->focus();
            }
        });
    });
}

ExplorerWindow::ExplorerWindow(const std::function<void(File*)> &callback):
    ExplorerWindow()
{
    mCallback = callback;
}
