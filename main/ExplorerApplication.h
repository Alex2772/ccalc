//
// Created by alex2772 on 14.04.18.
//

#pragma once

#include "Application.h"
#include "WindowList.h"
#include "ContextMenu.h"
#include "dir.png.h"
#include "MessageDialog.h"
#include "TaskHelper.h"
#include "SpinnerView.h"
#include "TextView.h"
#include "ProgressBar.h"
#include "file.png.h"
#include <functional>

class Explorer: public Application {
public:
    virtual Bitmap getIcon();
    virtual void launch();
    virtual const std::string getTitle();
};


class ExplorerWindow;

class WindowCopy : public Dialog {
public:
    TextView *tv;
    ProgressBar *pB;
    TaskHelper *mHelper;

    WindowCopy(ExplorerWindow *e, std::string src, std::string dst);

    void copy(File &src, const std::string &dst);

    ~WindowCopy() {
        delete mHelper;
    }
};


class ExplorerWindow : public WindowList {
private:
    std::function<void(File*)> mCallback;
    File *toMC = nullptr;
    enum {
        COPY,
        MOVE,
        NONE
    } a = NONE;

    short horizontalScroll = 0;
    struct ExplorerItem {
        Bitmap ico = Bitmap(file, 10, 10);
        std::string mFile;
    };
    std::deque<std::string> breadboard;

    std::vector<ExplorerItem> mItems;
    std::shared_ptr<SpinnerView> mSpin;
    TaskHelper *mTask = nullptr;

    void openDir(std::string path);

    std::string compileBreadboard();

    std::string currentDir();

public:
    ExplorerWindow();
    ExplorerWindow(const std::function<void(File*)>& callback);
    virtual ~ExplorerWindow();
    virtual void itemSelected(size_t index);
    void keyRelease(uint8_t key) override;
    void refresh();
    ContextMenu::Item i_new;

    virtual void keyLongDown(uint8_t index);
    virtual void render(Framebuffer &fb);
    std::string currentFile();
    virtual void itemLongSelected(size_t index);

};
