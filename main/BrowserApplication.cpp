//
// Created by alex2772 on 15.04.18.
//

#include <sstream>
#include <esp_log.h>
#include "BrowserApplication.h"
#include "Window.h"
#include "CCOSCore.h"
#include "TextArea.h"
#include "SpinnerView.h"
#include "TaskHelper.h"
#include "Http.h"
#include "MessageDialog.h"

static const char* B_TAG = "CCOS-BROWSER";

const uint8_t browser[] = {
        0xfe,
        0xff,
        0xff,
        0x3f,
        0x1f,
        0xf,
        0x8f,
        0xc7,
        0xc7,
        0xc7,
        0xc7,
        0xc7,
        0xc7,
        0x8f,
        0xf,
        0x1f,
        0x3f,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xfe,
        0xff,
        0xff,
        0xc0,
        0x0,
        0x0,
        0x3f,
        0x7f,
        0xfc,
        0xfe,
        0xff,
        0xff,
        0xff,
        0xff,
        0x7f,
        0x3f,
        0x0,
        0x0,
        0x40,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xff,
        0xfe,
        0xfc,
        0xfc,
        0xf8,
        0xf8,
        0xf8,
        0xf8,
        0xf8,
        0xf8,
        0xfc,
        0xfe,
        0xfc,
        0xf8,
        0xf0,
        0xe0,
        0xc1,
        0x83,
        0x7,
        0xf,
        0x1f,
        0x3f,
        0x7f,
        0xff,
        0xff,
        0xff,
        0xff,
        0x1f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3f,
        0x3e,
        0x3c,
        0x38,
        0x30,
        0x30,
        0x39,
        0x3f,
        0x1f,
};

class BrowserFrame: public View {
public:
    std::stringstream mContent;
    int sX = 0, sY = 0;

    BrowserFrame() {
        isInput = true;
    }

    virtual void render(Framebuffer &framebuffer) {
        View::render(framebuffer);
        if (isFocused()) {
            sX += int(CCOSCore::getKeyState(4)) - int(CCOSCore::getKeyState(6));
            sY += int(CCOSCore::getKeyState(1)) - int(CCOSCore::getKeyState(9));
        }
        std::string s = mContent.str();
        char *cur = &(s[0]);
        for (int16_t i = 0; *cur; i++)
            cur = framebuffer.drawStringML(sX, i * 12 + 1 + sY, cur, 127, FONT_FACE_TERMINUS_6X12_ISO8859_1, OLED_COLOR_WHITE);
    }

};

class Browser : public Window {
private:
    BrowserFrame* mFrame = new BrowserFrame();
    TextArea *mUrl = new TextArea();
    TaskHelper *mHelper = nullptr;
    float mLoad = 0.f;
    std::string mLastURL;
public:
    Browser() : Window("Browser") {
        mUrl->x = 1;
        mUrl->width = 127 - mUrl->x * 2;
        mUrl->y = 9;
        addView(mFrame);
        addView(mUrl);

        openURL("https://www.google.com");
    }

    void openURL(const std::string &url) {
        mLastURL = url;
        mUrl->setText(url);
        mLoad = 0;
        mHelper = new TaskHelper([&]() {
            bool first = true;
            size_t sumLength = 0;
            bool lock = true;
            int cntLength = 0;
            auto doc = http::get(mUrl->getText(), {}, [&](char *buf, int len, int contentLength) {
                cntLength = contentLength;
                char *temp = new char[len];
                memcpy(temp, buf, len);
                CCOSCore::runOnUiThread([&, temp, len, contentLength]() {
                    sumLength += len;
                    mLoad = float(sumLength) / float(contentLength);
                    if (mLoad != mLoad) {
                        mLoad = 0.f;
                    }
                    ESP_LOGI(B_TAG, "Loading %d/%d (%f)", len, contentLength, mLoad);
                    if (first) {
                        first = false;
                        mFrame->mContent.clear();
                    }

                    mFrame->mContent.write(temp, len);
                    delete[] temp;
                });
            });
            CCOSCore::runOnUiThread([&]() {
                lock = false;
            });
            while (lock) {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            for (auto& d : doc.mHeaders) {
                printf("%s : %s\n", d.first.c_str(), d.second.c_str());
            }
            switch (doc.mStatus) {
                case 200:
                case 302:
                case 301: {
                    auto it = doc.mHeaders.find("Location");
                    if (it != doc.mHeaders.end()) {
                        std::string dst = it->second;
                        CCOSCore::runOnUiThread([&, dst] {
                            openURL(dst);
                        });
                    }

                    break;
                }
                default: {
                    char buf[64];
                    sprintf(buf, "%d", doc.mStatus);
                    CCOSCore::displayWindow(new MessageDialog("Error", buf));
                }
            }
        });
    }

    virtual void renderPre(Framebuffer &fb) {
        Window::renderPre(fb);
        fb.drawRect(0, 0, 128, mUrl->y + mUrl->height, OLED_COLOR_WHITE);


    }

    virtual void keyLongDown(uint8_t key) {
        Window::keyLongDown(key);
        switch (key) {
            case 7:
                openURL(mLastURL);
                break;
            case 15:
                openURL(mUrl->getText());
                break;
        }
    }

    virtual void renderPost(Framebuffer &fb) {
        Window::renderPost(fb);

        fb.drawRect(2, 10, int(124.f * mLoad), 14, OLED_COLOR_INVERT);
    }

    ~Browser() {
        delete mHelper;
    }
/*
    bool isFullscreen() override {
        return true;
    }*/
};

void BrowserApplication::launch() {
    CCOSCore::displayWindow(new Browser);
}

const std::string BrowserApplication::getTitle() {
    return "Browser";
}

Bitmap BrowserApplication::getIcon() {
    return {browser, 30, 30};
}
