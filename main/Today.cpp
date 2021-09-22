//
// Created by alex2772 on 14.01.18.
//

#include <sys/time.h>
#include "Today.h"
#include "Framebuffer.h"
#include "today.png.h"
#include "Window.h"
#include "MeteoServiceProvider.h"

class TodayWindow: public Window {
public:
    TodayWindow():
            Window("Today") {

    }

    void render(Framebuffer &framebuffer) override {
        Window::render(framebuffer);
        char buf[64];
        timeval tv;
        gettimeofday(&tv, 0);
        tm* time = localtime(&tv.tv_sec);
        strftime(buf, sizeof(buf), "%B %e", time);
        framebuffer.drawString(0, 0, buf, OLED_COLOR_INVERT, FONT_FACE_BITOCRA_4X7);
        strftime(buf, sizeof(buf), "%a", time);
        framebuffer.drawString(0, 7, buf, OLED_COLOR_INVERT, FONT_FACE_BITOCRA_4X7);
    }
};

void Today::launch() {

}
extern Framebuffer fb;
extern MeteoServiceProvider::meteo_data* __meteo_data;
const std::string Today::getTitle() {
    timeval tv;
    gettimeofday(&tv, 0);
    tm* t = localtime(&tv.tv_sec);
    char buf[32];

    strftime(buf, sizeof(buf), "%d/%m/%y", t);
    fb.setCoordsOffset(0, 0);
    fb.drawString(0, 0, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);

    if (__meteo_data) {
        sprintf(buf, "%d oC", __meteo_data->temp);

        fb.drawString(127-Framebuffer::length(FONT_FACE_BITOCRA_4X7, buf), 56, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);
        sprintf(buf, "%d mmHg", __meteo_data->pressure);
        fb.drawString(127-Framebuffer::length(FONT_FACE_BITOCRA_4X7, buf), 49, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);

        sprintf(buf, "%d%% h", __meteo_data->humidity);
        fb.drawString(0, 56, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);
    }
    strftime(buf, sizeof(buf), "%A", t);
    return {buf};
}


Bitmap Today::getIcon() {
    timeval tv;
    gettimeofday(&tv, 0);
    if (tv.tv_sec - 3600 > time || !gen) {
        gen = true;
        static Bitmap b(today, 30, 30);
        Framebuffer framebuffer;
        framebuffer.clear();
        framebuffer.setCoordsOffset(0, 0);
        framebuffer.drawImage(0, 0, b);
        char buf[6];
        tm* t = localtime(&tv.tv_sec);
        strftime(buf, sizeof(buf), "%d", t);
        int16_t w = Framebuffer::length(FONT_FACE_BITOCRA_7X13, buf);
        framebuffer.drawString(15 - w / 2, 11, buf, OLED_COLOR_BLACK, FONT_FACE_BITOCRA_7X13);
        strftime(buf, sizeof(buf), "%b", t);
        w = Framebuffer::length(FONT_FACE_BITOCRA_4X7, buf);
        framebuffer.drawString(15 - w / 2, 3, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7);

        framebuffer.snapshot(30, 30, _bitmap);
    }
    return {_bitmap, 30, 30};
}
