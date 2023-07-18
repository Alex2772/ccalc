//
// Created by alex2772 on 18.04.18.
//

#include "LuaLib.h"
#include "CCOSCore.h"
#include "MessageDialog.h"
#include "InputDialog.h"
#include "Process.h"
#include "TextView.h"
#include "Mutex.h"
#include <map>
#include <driver/adc.h>
#include "TextArea.h"
#include "SpinBox.h"
#include "WindowList.h"
#include "ContextMenu.h"
#include "ListItem.h"
#include "ExplorerApplication.h"
#include <freertos/timers.h>
#include <map>

sol::table cp;
extern std::map<lua_State*, Process*> processes;

template <class T>
std::shared_ptr<T> fromSol(sol::object g) {
    return {g.as<T*>(), [g](View*) {

    }};
}
std::map<TimerHandle_t, sol::function> gt;
void generic_timer(TimerHandle_t t) {
    sol::function x = gt[t];
    processes[x.lua_state()]->ropt([x]() {
        if (x != sol::nil)
            x();
    });
}
template <class Window>
void registerWindow(std::string name) {
    class WindowWrapper: public Window {
    private:
        sol::this_state mState;
        std::deque<TimerHandle_t > timers;
    public:
        sol::function _render;
        sol::function _keydown;
        sol::function _keyup;
        sol::function _focus;
        sol::function _keylongdown;
        sol::function _keypressurechanged;
        sol::function _onclick;
        sol::function _istr;
        sol::function _focuslost;
        sol::function _isfullscreen;
        sol::function _itemselected;
        sol::function _sleeplock;
        sol::function _close;
        sol::function _event;
        WindowWrapper(sol::this_state L, std::string s) : Window(s),
                                                          mState(L)
        {
            Window::mLuaState = mState.lua_state();
        }
        virtual ~WindowWrapper() {
            printf("WindowWrapper delete\n");
            for (TimerHandle_t t : timers) {
                std::map<TimerHandle_t, sol::function>::iterator it = gt.find(t);
                if (it != gt.end()) {
                    gt.erase(it);
                }
                xTimerDelete(t, portMAX_DELAY);
            }
        }
        virtual void lua_close() {
            printf("Lua close called\n");
            if (_close != sol::nil) {
                Mutex::L lock(processes[mState.lua_state()]->m);
                _close();
            }
        }
        virtual void addViewS(sol::object x) {

            std::shared_ptr<View> v = fromSol<View>(x);
            CCOSCore::runOnUiThread([&, v]() {
                Window::addViewS(v);
            });
        }

        virtual void render(Framebuffer &framebuffer) {

            if (_render != sol::nil) {
                Mutex::L lock(processes[mState.lua_state()]->m);
                _render(framebuffer);
            }
            Window::render(framebuffer);
        }

        virtual void focus() {
            Window::focus();
            processes[mState.lua_state()]->ropt([&]() {
                if (_focus != sol::nil) {
                    _focus();
                }
            });
        }
        virtual void event(Event e) {
            Window::event(e);

            processes[mState.lua_state()]->ropt([&, e]() {
                if (_event != sol::nil) {
                    _event(e);
                }
            });
        }

        virtual bool isFullscreen() {
            if (_isfullscreen != sol::nil) {
                Mutex::L lock(processes[mState.lua_state()]->m);
                return _isfullscreen();
            }
            return Window::isFullscreen();
        }

        virtual void keyLongDown(uint8_t key) {
            Window::keyLongDown(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keylongdown != sol::nil)
                    _keylongdown(key);
            });
        }

        virtual void keyDown(uint8_t key) {
            Window::keyDown(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keydown != sol::nil)
                    _keydown(key);
            });
        }

        virtual void keyPressureChanged(uint8_t key) {
            Window::keyPressureChanged(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keypressurechanged != sol::nil)
                    _keypressurechanged(key);
            });
        }

        virtual void keyRelease(uint8_t key) {
            Window::keyRelease(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keyup != sol::nil)
                    _keyup(key);
            });
        }

        virtual void onClick() {
            Window::onClick();
            processes[mState.lua_state()]->ropt([&]() {
                if (_onclick != sol::nil)
                    _onclick();
            });
        }

        virtual bool isTransparent() {
            return Window::isTransparent();
        }

        virtual void focusLost() {
            Window::focusLost();
            processes[mState.lua_state()]->ropt([&]() {
                if (_focuslost != sol::nil)
                    _focuslost();
            });
        }
        virtual void itemSelected(size_t index) {
            processes[mState.lua_state()]->ropt([&, index]() {
                if (_itemselected != sol::nil)
                    _itemselected(index);
            });
        }
        virtual bool sleepLock() {
            if (_sleeplock != sol::nil)
                return _sleeplock();
            return Window::sleepLock();
        }
        void registerTimer(sol::function f, size_t delay) {
            TimerHandle_t x = xTimerCreate("GenericTimer", delay / portTICK_PERIOD_MS, true, 0, generic_timer);
            timers.push_back(x);
            gt[x] = f;
            xTimerStart(x, portMAX_DELAY);
        }

    };
    if (std::is_same<Window, ::Window>::value) {
        cp.new_usertype<WindowWrapper>(name,
                                       sol::constructors<WindowWrapper(sol::this_state, std::string)>(),
                                       "addView", &WindowWrapper::addViewS,
                                       "render", &WindowWrapper::_render,
                                       "focus", &WindowWrapper::focus,
                                       "keyLongDown", &WindowWrapper::_keylongdown,
                                       "keyDown", &WindowWrapper::_keydown,
                                       "keyUp", &WindowWrapper::_keyup,
                                       "keyPressureChanged", &WindowWrapper::_keylongdown,
                                       "onClick", &WindowWrapper::_onclick,
                                       "isTransparent", &WindowWrapper::isTransparent,
                                       "focusLost", &WindowWrapper::_focuslost,
                                       "isFullscreen", &WindowWrapper::_isfullscreen,
                                       "itemSelected", &WindowWrapper::_itemselected,
                                       "onClose", &WindowWrapper::_close,
                                       "close", &WindowWrapper::close,
                                       "sleepLock", &WindowWrapper::_sleeplock,
                                       "timer", &WindowWrapper::registerTimer,
                                       "event", &WindowWrapper::_event,
                                       "mDrawTitle", &WindowWrapper::mDrawTitle,
                                       sol::base_classes, sol::bases<::View>()
        );
    } else {
        cp.new_usertype<WindowWrapper>(name,
                                       sol::constructors<WindowWrapper(sol::this_state, std::string)>(),
                                       "addView", &WindowWrapper::addViewS,
                                       "render", &WindowWrapper::_render,
                                       "focus", &WindowWrapper::focus,
                                       "keyLongDown", &WindowWrapper::_keylongdown,
                                       "keyDown", &WindowWrapper::_keydown,
                                       "keyUp", &WindowWrapper::_keyup,
                                       "keyPressureChanged", &WindowWrapper::_keylongdown,
                                       "onClick", &WindowWrapper::_onclick,
                                       "isTransparent", &WindowWrapper::isTransparent,
                                       "isFullscreen", &WindowWrapper::_isfullscreen,
                                       "focusLost", &WindowWrapper::_focuslost,
                                       "itemSelected", &WindowWrapper::_itemselected,
                                       "onClose", &WindowWrapper::_close,
                                       "close", &WindowWrapper::close,
                                       "sleepLock", &WindowWrapper::_sleeplock,
                                       "event", &WindowWrapper::_event,
                                       "timer", &WindowWrapper::registerTimer,
                                       "mDrawTitle", &WindowWrapper::mDrawTitle,
                                       sol::base_classes, sol::bases<Window>()
        );
    }
}
template <class View>
void registerView(std::string name) {
    class ViewWrapper: public View {
    private:
        sol::this_state mState;
    public:
        sol::function _render;
        sol::function _keydown;
        sol::function _keyup;
        sol::function _focus;
        sol::function _keylongdown;
        sol::function _keypressurechanged;
        sol::function _onclick;
        sol::function _istr;
        sol::function _focuslost;
        ViewWrapper(sol::this_state L) : View(), mState(L)
                                                                  {

                                                                  }


        virtual void render(Framebuffer &framebuffer) {
            View::render(framebuffer);
            if (_render != sol::nil) {
                Mutex::L lock(processes[mState.lua_state()]->m);
                _render(framebuffer);
            }
        }

        virtual void focus() {
            View::focus();
            processes[mState.lua_state()]->ropt([&]() {
                if (_focus != sol::nil)
                    _focus();
            });
        }


        virtual void keyLongDown(uint8_t key) {
            View::keyLongDown(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keylongdown != sol::nil)
                    _keylongdown(key);
            });
        }

        virtual void keyDown(uint8_t key) {
            View::keyDown(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keydown != sol::nil)
                    _keydown(key);
            });
        }

        virtual void keyPressureChanged(uint8_t key) {
            View::keyPressureChanged(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keypressurechanged != sol::nil)
                    _keypressurechanged(key);
            });
        }

        virtual void keyRelease(uint8_t key) {
            View::keyRelease(key);
            processes[mState.lua_state()]->ropt([&,key]() {
                if (_keyup != sol::nil)
                    _keyup(key);
            });
        }

        virtual void onClick() {
            View::onClick();
            processes[mState.lua_state()]->ropt([&]() {
                if (_onclick != sol::nil)
                    _onclick();
            });
        }

        virtual bool isTransparent() {
            return View::isTransparent();
        }

        virtual void focusLost() {
            View::focusLost();
            processes[mState.lua_state()]->ropt([&]() {
                if (_focuslost != sol::nil)
                    _focuslost();
            });
        }

    };
    if (std::is_same<View, ::View>::value) {
        cp.new_usertype<ViewWrapper>(name,
                                     sol::constructors<ViewWrapper(sol::this_state)>(),
                                     "render", &ViewWrapper::_render,
                                     "focus", &ViewWrapper::focus,
                                     "keyLongDown", &ViewWrapper::_keylongdown,
                                     "keyDown", &ViewWrapper::_keydown,
                                     "keyUp", &ViewWrapper::_keyup,
                                     "keyPressureChanged", &ViewWrapper::_keylongdown,
                                     "onClick", &ViewWrapper::_onclick,
                                     "isTransparent", &ViewWrapper::isTransparent,
                                     "focusLost", &ViewWrapper::_focuslost,
                                     "x", &ViewWrapper::x,
                                     "y", &ViewWrapper::y,
                                     "w", &ViewWrapper::width,
                                     "h", &ViewWrapper::height,
                                     "isInput", &ViewWrapper::isInput,
                                     "isFocused", &ViewWrapper::isFocused
        );
    } else {
        cp.new_usertype<ViewWrapper>(name,
                                     sol::constructors<ViewWrapper(sol::this_state)>(),
                                     "render", &ViewWrapper::_render,
                                     "focus", &ViewWrapper::focus,
                                     "keyLongDown", &ViewWrapper::_keylongdown,
                                     "keyDown", &ViewWrapper::_keydown,
                                     "keyUp", &ViewWrapper::_keyup,
                                     "keyPressureChanged", &ViewWrapper::_keylongdown,
                                     "onClick", &ViewWrapper::_onclick,
                                     "isTransparent", &ViewWrapper::isTransparent,
                                     "focusLost", &ViewWrapper::_focuslost,
                                     "x", &ViewWrapper::x,
                                     "y", &ViewWrapper::y,
                                     "w", &ViewWrapper::width,
                                     "h", &ViewWrapper::height,
                                     "isInput", &ViewWrapper::isInput,
                                     "isFocused", &ViewWrapper::isFocused,
                                     sol::base_classes, sol::bases<::View>()
        );
    }
}
extern Mutex uiThread;
void displayWindow(sol::object x) {
    {
        Mutex::L lock(uiThread);
        processes[x.lua_state()]->alive.push_back(x);
    };

    CCOSCore::displayWindow(&x.as<Window>());
}

std::string LuaLib::input(std::string title) {
    std::string text;
    bool locked = true;
    CCOSCore::displayWindow(new InputDialog(title, "", [&](const std::string& t) {
        text = t;
        locked = false;
    }));
    while (locked) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    return text;
}
std::string LuaLib::file() {
    File* file;
    bool locked = true;
    CCOSCore::displayWindow(new ExplorerWindow([&](File* f) {
        file = f;
        locked = false;
    }));
    while (locked) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    if (file) {
        std::string path = file->convert();
        delete file;
        return path;
    }
    return {};
}

uint8_t LuaLib::battery() {
    return CCOSCore::getBatteryLevel();
}

void LuaLib::_sleep(uint32_t millis) {
    vTaskDelay(millis / portTICK_PERIOD_MS);
}
void LuaLib::message(std::string m) {
    CCOSCore::displayWindow(new MessageDialog("", m));
}
int get_adc() {
    return adc1_get_raw(ADC1_CHANNEL_6);
}
void LuaLib::open(sol::state& lua) {
    cp = lua.create_named_table("cp");
    cp.set_function("indicator", &CCOSCore::setIndicatorValue);
    cp.set_function("message", &message);
    cp.set_function("hall", &hall_sensor_read);
    cp.set_function("input", &input);
    cp.set_function("file", &file);
    cp.set_function("random", &esp_random);
    cp.set_function("battery", &battery);
    cp.set_function("sleep", &_sleep);
    cp.set_function("keyState", &CCOSCore::getKeyState);
    cp.set_function("vibrator", &CCOSCore::vibrate);
    cp.set_function("blink", &CCOSCore::blink);
    cp.set_function("displayWindow", &displayWindow);
    cp.set_function("round", &roundf);
    cp.set_function("toast", &CCOSCore::showToast);
    cp.set_function("adc", get_adc);
    cp.set_function("millis", esp_timer_get_time);

    cp.new_usertype<Event>("Event",
                           "name", &Event::name,
                           "param", &Event::param
    );

    registerView<View>("View");
    registerView<ListItem>("ListItem");
    registerView<TextView>("TextView");
    registerView<TextArea>("TextArea");
    registerView<SpinBox<int>>("SpinBox");
    registerWindow<Window>("Window");
    registerWindow<WindowList>("WindowList");
    registerWindow<ContextMenu>("WindowCM");
    cp.new_usertype<Framebuffer>("Framebuffer",
                            "rect", &Framebuffer::drawRect,
                                 "string", &Framebuffer::drawString,
                                 "stringML", &Framebuffer::drawStringML,
                                 "line", &Framebuffer::drawLine,
                                 "circle", &Framebuffer::drawCircle,
                                 "shade", &Framebuffer::shade,
                                 "fb", &Framebuffer::drawFB,
                                 "clear", &Framebuffer::clear
    );
    cp["ListItem"]["setText"] = &ListItem::setText;
    cp["ListItem"]["setTitle"] = &ListItem::setTitle;

    cp["TextArea"]["setText"] = &TextArea::setText;
    cp["TextArea"]["getText"] = &TextArea::getText;

    cp["TextView"]["setText"] = &TextView::setText;

    cp["SpinBox"]["getValue"] = &SpinBox<int>::getValue;
    cp["SpinBox"]["setValue"] = &SpinBox<int>::setValue;
    cp["SpinBox"]["min"] = &SpinBox<int>::kmin;
    cp["SpinBox"]["max"] = &SpinBox<int>::kmax;
    {
        sol::table font = lua.create_named_table("Font");
        font["B4x7"] = FONT_FACE_BITOCRA_4X7;
        font["R10"] = FONT_FACE_ROBOTO_10PT;
    }
    cp = sol::table();
}

