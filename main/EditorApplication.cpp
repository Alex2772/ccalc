//
// Created by alex2772 on 15.04.18.
//

#include "EditorApplication.h"
#include "Window.h"
#include "CCOSCore.h"
#include "TextArea.h"

const uint8_t notepad[] = {
    0xfe,
    0xff,
    0xeb,
    0xef,
    0xe7,
    0xfb,
    0xfb,
    0xfb,
    0xf7,
    0xf7,
    0xf7,
    0xf7,
    0xef,
    0xfb,
    0xff,
    0xeb,
    0xff,
    0xff,
    0xf7,
    0xfb,
    0xfb,
    0xeb,
    0xf3,
    0xff,
    0xeb,
    0xef,
    0xe3,
    0xf7,
    0xff,
    0xfe,
    0xff,
    0xff,
    0x18,
    0xbe,
    0x59,
    0xff,
    0x39,
    0x5c,
    0x19,
    0xff,
    0xd8,
    0x1a,
    0xdf,
    0xfe,
    0x98,
    0x7e,
    0x9f,
    0xfb,
    0xfc,
    0x38,
    0x9f,
    0xbf,
    0x1f,
    0xfc,
    0x79,
    0xbc,
    0xbf,
    0x58,
    0xff,
    0xff,
    0xff,
    0xff,
    0xf3,
    0xf3,
    0x6f,
    0xff,
    0xff,
    0xff,
    0xfb,
    0x6f,
    0xf3,
    0xfb,
    0xf7,
    0xff,
    0x7f,
    0xf7,
    0xf3,
    0x7b,
    0xe7,
    0xff,
    0x3b,
    0x1f,
    0x6f,
    0x7,
    0x6f,
    0x1f,
    0x3f,
    0xff,
    0xff,
    0xff,
    0x1f,
    0x3f,
    0x3f,
    0x3e,
    0x3e,
    0x3d,
    0x3f,
    0x3f,
    0x3d,
    0x3d,
    0x3e,
    0x3c,
    0x3d,
    0x3e,
    0x3e,
    0x3f,
    0x3f,
    0x3c,
    0x3f,
    0x3f,
    0x0,
    0x3f,
    0x3e,
    0x0,
    0x2a,
    0x15,
    0x0,
    0x3f,
    0x3f,
    0x1f,
};

class Editor: public Window {
private:
    File mFile;
    TextArea* ta;
    int mBottomPanel = 0;
    int mBPD = 0;
public:
    Editor() : Window("Editor") {
        ta = new TextArea();
        ta->x = -1;
        ta->y = -1;
        ta->width = 129;
        ta->height = 67;
        ta->multiline = true;
        ta->focus();
        addView(ta);
    }
    Editor(const File& f):
            Editor()
    {

        mFile = f;
        std::string s = "";
        mFile.open(File::R);
        printf("Opening %s\n", mFile.getPath().c_str());
        if (mFile.seek(0L, SEEK_END) == 0) {
            long size = mFile.tellg();
            if (size) {
                mFile.seek(0L, SEEK_SET);
                printf("Size %ld\n", size);
                s.resize(size);
                mFile.read(&(s[0]), size);
            }
        }
        mFile.close();
        ta->setText(s);

    }

    bool isFullscreen() override {
        return true;
    }

    void renderWindow(Framebuffer &fb) override {
    if (ta->ci.y > 44)
        mBPD = 2;
    mBottomPanel+=mBPD;
    if (mBottomPanel < 0)
        mBottomPanel = 0;
    else if (mBottomPanel > 10)
        mBottomPanel = 10;

    fb.drawRect(0, 64-8+mBottomPanel, 128, 9, OLED_COLOR_BLACK);
    fb.drawRect(0, 64-8+mBottomPanel, 128, 1, OLED_COLOR_WHITE);
        char s[32];
        sprintf(s,"%lld:%lld", ta->column() + 1, ta->row() + 1);

        int16_t xOffset = strlen(s) * 4;

        fb.drawString(static_cast<int8_t>(128 - xOffset), 64 - 7, s, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);

    sprintf(s,"len %u", ta->getText().length());

        xOffset = strlen(s) * 4;

        fb.drawString(static_cast<int8_t>(98 - xOffset), 64 - 7 + mBottomPanel, s, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);

sprintf(s,"%d%%", ta->getText().empty() ? 0 : int(float(ta->getPosition())/ta->getText().length()*100));

        xOffset = strlen(s) * 4;

        fb.drawString(static_cast<int8_t>(58 - xOffset), 64 - 7 + mBottomPanel, s, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);
    }
    virtual void keyRelease(uint8_t key) {
        Window::keyRelease(key);
        mBPD = -1;
    }
    virtual void keyLongDown(uint8_t key) {
        Window::keyLongDown(key);
        if (key == 9) {
                mBPD = 2;
        }
    }
    virtual ~Editor() {
        if (!mFile.getPath().empty() && !ta->getText().empty()) {
            mFile.open(File::W);
            mFile.write(ta->getText().c_str(), ta->getText().size());
            mFile.close();
        }
    }

public:

};

void EditorApplication::launch() {
    CCOSCore::displayWindow(new Editor);
}

const std::string EditorApplication::getTitle() {
    return "Editor";
}

Bitmap EditorApplication::getIcon() {
    return {notepad, 30, 30};
}

bool EditorApplication::accepts(const std::string &f) {
    return true;
}

void EditorApplication::openFile(const File &f) {
    CCOSCore::displayWindow(new Editor(f));
}
