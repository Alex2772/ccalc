#pragma once

#include <cstdint>
#include <functional>
#include <nvs.h>
#include "Framebuffer.h"
#include "Application.h"
#include "Window.h"
#include "ViewIcon.h"
#include "Picture.h"
#include "NVS.h"

extern "C" {
#include "ssd1306.h"
#include <time.h>
}

extern ssd1306_t display;


namespace CCOSCore {
    void init();
    void keyInput(int8_t key, int8_t str);
    void keyDown(uint8_t key);
    void keyUp(uint8_t key);


    uint8_t getKeyState(uint8_t key);

    void vibrate(uint16_t ms);

    void registerApplication(Application *pApplication);

    void displayWindow(Window *window);

    void showToast(const std::string string);

    void keyLongDown(uint8_t key);

    void wakeUp();

    void sleep();
    void killMePlz(std::string m, lua_State* L = nullptr);

    void setTime(time_t t);

    std::vector<Window *> &getWindows();

    void registerIcon(ViewIcon *icon);

    uint8_t getBatteryLevel();
    uint32_t getUptime();

    void shutdown();

    NVS& getNVSHandle();

    void removeWindow(Window *window);

    void runOnUiThread(std::function<void()> func);
    std::vector<Picture*>& getPictures();

    void openFile(const File &f);

    void execute(const File &file);

    void screenshot();

    void blink(uint16_t ms);
    void setIndicatorValue(float r, float g = 0, float b = 0);

    void reboot(int type);
};