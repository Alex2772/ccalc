//
// Created by alex2772 on 17.12.17.
//

#include <cstdint>
#include "ImageViewerApplication.h"
#include "iv.png.h"
#include "CCOSCore.h"
#include "BMPBitmap.h"

class IVWindow: public Window {
private:
    size_t index = 0;
    int16_t x = 0;
    int16_t y = 0;
    BMPBitmap* bmp = nullptr;
public:
    void keyDown(uint8_t key) override {
        View::keyDown(key);
        if (!bmp) {
            switch (key) {
                case 4:
                    if (index) {
                        index--;
                    } else {
                        index = CCOSCore::getPictures().size() - 1;
                    }
                    break;
                case 6:
                    index++;
                    if (index >= CCOSCore::getPictures().size()) {
                        index = 0;
                    }
                    break;
            }
        }
    }

    IVWindow() : Window("Image Viewer") {
    }

    void render(Framebuffer &fb) override {
        if (bmp) {
            x += CCOSCore::getKeyState(4);
            x -= CCOSCore::getKeyState(6);
            y += CCOSCore::getKeyState(1);
            y -= CCOSCore::getKeyState(9);
            fb.drawImage(x, y, *bmp);
        } else {
            Picture *p = CCOSCore::getPictures()[index];
            p->draw(fb, (128 - p->width) / 2, (64 - p->height) / 2);
        }
    }

    virtual ~IVWindow() {
        delete bmp;
    }

    virtual bool isFullscreen() {
        return true;
    }

    IVWindow(const File &file): IVWindow() {
        File f(file);
        bmp = new BMPBitmap(f);
        x = (128 - bmp->width) / 2;
        y = (64 - bmp->height) / 2;
    }
};

void ImageViewerApplication::launch() {
    CCOSCore::displayWindow(new IVWindow);
}

const std::string ImageViewerApplication::getTitle() {
    return "Image Viewer";
}

Bitmap ImageViewerApplication::getIcon() {
    return {iv, 30, 30};
}

bool ImageViewerApplication::accepts(const std::string &f) {
    return f.length() > 4 && f.substr(f.length() - 4, 4) == ".bmp";
}

void ImageViewerApplication::openFile(const File &f) {
    CCOSCore::displayWindow(new IVWindow(f));
}
