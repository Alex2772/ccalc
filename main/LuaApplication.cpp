//
// Created by alex2772 on 16.12.17.
//
/*

#include "LuaApplication.h"
//#include "lua.png.h"
#include "Window.h"
#include "CCOSCore.h"
#include "lua.png.h"
#include <random>
#include "lua.hpp"
extern "C" {
extern int pmain(lua_State *L);
extern int report(lua_State *L, int status);
struct Smain {
    int argc;
    char **argv;
    int status;
};
};


class ConsoleWindow: public Window {
private:
    int16_t _k = 0;
public:
    ConsoleWindow(): Window("Console") {
        int status;
        struct Smain s;
        lua_State *L = lua_open();
        s.argc = 0;
        s.argv = 0;
        status = lua_cpcall(L, &pmain, &s);

        report(L, status);
        lua_close(L);
    }
    ~ConsoleWindow() {

    }

    void render(Framebuffer &framebuffer) override {
        Window::render(framebuffer);
        int16_t x = 0;
        int16_t y = 12 + _k;

        for (uint8_t i = 0; i < _ccos_io_inf; i++) {
            if (_ccos_io_buf[i] == '\n'){
                x = 0;
                y += 11;
                continue;
            }
            framebuffer.drawChar(x, y, _ccos_io_buf[i], OLED_COLOR_WHITE, FONT_FACE_BITOCRA_6X11);
            x += 6;
            if (x > 128 - 11 - 10) {
                x = 0;
                y += 11;
            }
            if (y >= 64) {
                _k -= 1;
                break;
            } else if (_k < 0) {
                _k++;
            }

        }
    }
};


void LuaApplication::launch() {
    CCOSCore::displayWindow(new ConsoleWindow);
}

const std::string LuaApplication::getTitle() {
    return "Lua Shell";
}

Bitmap LuaApplication::getIcon() {
    return {lua, 30, 30};
}
*/