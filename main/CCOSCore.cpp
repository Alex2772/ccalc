#include "CCOSCore.h"
#include "SpinnerView.h"
#include "CalculatorApplication.h"
#include "catnoir.png.h"
#include <cmath>
#include <ctime>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include "Timberman.h"
#include "Settings.h"
#include "WifiIcon.h"
#include "BatteryIcon.h"
#include "Mutex.h"
#include "Wifi.h"
#include "MemoryIcon.h"
#include "SyncIcon.h"
#include "Cloud.h"
#include "LuaApplication.h"
#include "phys.png.h"
#include "xmas.png.h"
#include "ImageViewerApplication.h"
#include "ssd1306.h"
#include <netdb.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <sys/time.h>
#include <driver/gpio.h>
#include <esp_event_loop.h>
#include <esp_event_legacy.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <cstdint>
#include "nvs_flash.h"
#include "MeteoServiceProvider.h"
#include "Today.h"
#include "lb.h"
#include <string>
#include <glm/gtc/matrix_transform.hpp>

extern "C" {
#include <driver/uart.h>
#include <driver/adc.h>
/* Change this according to you schematics and display size */
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64

#define PROTOCOL SSD1306_PROTO_I2C
#define ADDR     SSD1306_I2C_ADDR_0

#define I2C_BUS  0
#define SCL_PIN  5
#define SDA_PIN  4
}


#include "ExplorerApplication.h"
#include "Scheduler.h"
#include "ContextMenu.h"
#include "EditorApplication.h"

#include "LuaLib.h"
#include "MessageDialog.h"
#include "sol.hpp"
#include "Process.h"
#include "BMPBitmap.h"
#include "Event.h"
#include "TaskHelper.h"
#include "Socket.h"
#include "StreamIcon.h"
#include "Encryption.h"
#include "Http.h"
#include "VKApplication.h"
#include "FirmwareConfig.h"
#include "BrowserApplication.h"
#include "ChooserWindow.h"
#include "calc.png.h"
#include "SowlonyaWindow.h"
#include "Smoother.h"
#include "Streaming.h"

static const char* LCC = "CCOS";

std::vector<Application*> gApplications;
std::vector<Window*> gWindows;
std::vector<ViewIcon*> gIcons;
std::vector<Picture*> gPictures;
std::map<size_t, std::shared_ptr<Framebuffer>> gWindowsFB;
Mutex uiThread;
std::deque<std::function<void()>> gUiThread;
uint8_t gWindowsHideAnimation = 0;

QueueHandle_t event_bus;
QueueHandle_t vibrator_queue;
QueueHandle_t blink_queue;

std::string gToastText;
uint8_t gToastLive = 0;
uint8_t gLastKey = 0;
uint8_t gLastKeyCounter = 0;
ssd1306_t display = {};

static bool ready = false;

struct {
    float mAnimation = 1;
    Window* mCurrent = nullptr;
    float mSelected;
    int8_t mDir = -1;
    bool mRemoveAll;
    uint8_t mRemoveCounter;
} gWindowSelect;

void removeWindowImmediately(Window* w);

uint32_t gUptime = 0;
enum State {
    LOCKSCREEN = 0,
    NORMAL = 1,
    SLEEP = 2,
    WINDOW_SELECT
};
State gCurrentState = LOCKSCREEN;

#define SLEEP_TIMEOUT 6000

static uint16_t gMainSelectedApp = 2;
uint16_t gSleepTimeout = SLEEP_TIMEOUT;


uint8_t gUnlockState = 0;
uint8_t gUnlockTimeout = 0;

float gUnlockAnimation = 0;

/**
 * Bugfix #28
 */
bool gSkipInputAfterSleep = false;

uint8_t gButtonStates[18];

void vibrator_task(void* vp) {
    while (true) {
        uint16_t ms;
        xQueueReceive(vibrator_queue, (void*) &ms, portMAX_DELAY);
        if (FirmwareConfig::read().mMajorModel < 4) {
            gpio_set_level(PIN_MOTOR, 0);
        } else {
            gpio_set_level(PIN_MOTOR, 1);
        }
        vTaskDelay(ms / portTICK_PERIOD_MS);
        if (FirmwareConfig::read().mMajorModel < 4) {
            gpio_set_level(PIN_MOTOR, 1);
        } else {
            gpio_set_level(PIN_MOTOR, 0);
        }
    }
}

void blink_task(void* p) {
    while (true) {
        uint16_t ms;
        xQueueReceive(blink_queue, (void*) &ms, portMAX_DELAY);
        CCOSCore::setIndicatorValue(0, 0, 0.5f);
        vTaskDelay(ms / portTICK_PERIOD_MS);
        CCOSCore::setIndicatorValue(0, 0, 0);
    }
}

void indicator_loading_task(void* p) {
    const float brightness = 0.05f;
    while (!ready) {
        CCOSCore::setIndicatorValue(0, brightness * (0.5f + sinf(float(esp_timer_get_time()) / 400000.f) * 0.5f),
                                    brightness);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    CCOSCore::setIndicatorValue(0, 0, 0);
    vTaskDelete(0);
}

void showIcons(int16_t& xOffset, Framebuffer& fb) {
    for (size_t i = 0; i < gIcons.size(); i++) {
        ViewIcon* icon = gIcons[i];
        if (icon->getVisibility() == View::VISIBLE) {
            xOffset += icon->width + 1;
            icon->x = 127 - xOffset;
            icon->y = 0;
            icon->render(fb);
        }
    }
    fb.setCoordsOffset(0, 0);
}

uint16_t _fps = 0;
uint16_t _ffps = 0;

extern MeteoServiceProvider::meteo_data* __meteo_data;

Mutex _battery;
std::deque<uint16_t> _battery_d;

void battery_task(void* t) {
    while (1) {
        {
            int sum = 0;
            for (int i = 0; i < 10; i++) {
                sum += adc1_get_raw(ADC_BATTERY);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            sum /= 10;
            Mutex::L lock(_battery);
            _battery_d.push_back(uint16_t(sum));
            while (60 < _battery_d.size()) {
                _battery_d.pop_front();
            }
        }
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(nullptr);
}

void fps_timer(TimerHandle_t t) {
    _fps = _ffps;
    _ffps = 0;
}

// bool __bug27_fix = false;

/**
 * Bugfix #27
 */
bool gSkipKeyUp = false;
Framebuffer fb;

Window* gCurrentWindow = nullptr;


void renderStatusBar() {
    if (CCOSCore::getKeyState(12) || !gCurrentWindow || !gCurrentWindow->isFullscreen() || gCurrentState == WINDOW_SELECT) {
        time_t rtime = ::time(0);
        struct tm* time = localtime(&rtime);
        fb.setCoordsOffset(0, 0);
        // Clock
        char min[6];
        strftime(min, sizeof(min), "%M", time);

        char s[6];
        sprintf(s, "%d:%s", time->tm_hour, min);

        int16_t xOffset = strlen(s) * 4;

        fb.drawString(static_cast<int8_t>(128 - xOffset), 0, s, OLED_COLOR_INVERT,
                      FONT_FACE_BITOCRA_4X7);

        xOffset += 1;

        showIcons(xOffset, fb);
    }
}

void renderWindow(size_t& i, float currentSelected, Window* window) {
    int16_t y;
    auto it = gWindowsFB.find(i);
    if (gWindows.size() < 4 || gWindows.size() - 4 <= i) {
        float scrollY = powf(float(i) - currentSelected, 5.f);
        if (scrollY < 0) {
            scrollY /= glm::max(-scrollY, 1.f) * 10.f;
        }
        float smoothY = 46.f * scrollY;
        float scale = powf(glm::max(smoothY / 10.f, 0.1f), 0.2f) * 0.7f;

        if (gWindowSelect.mCurrent == window) {
            float animation = glm::sin(gWindowSelect.mAnimation * glm::pi<float>() / 2);
            scale = glm::mix(scale, 1.f, animation);

            float tmpY = (1.f - scale) * 16.f;
            y = static_cast<int16_t>(glm::mix(tmpY + smoothY, tmpY, animation));
        } else {
            y = static_cast<int16_t>((1.f - scale) * 16.f + smoothY);
        }
        if (y < 64 && y > -48) {
            int16_t x = static_cast<int16_t>((1.f - scale) * 32);
            float sc = scale * 0.5f + 0.5f;

            std::shared_ptr<Framebuffer> target;
            if (it == gWindowsFB.end()) {
                target = std::make_shared<Framebuffer>();
                target->clear();
                window->render(*target);
                gWindowsFB[i] = target;
            } else {
                target = it->second;
            }

            auto p = static_cast<int16_t>(pow(window->_sa, 2));
            fb.drawRect(x + p, y, static_cast<uint8_t>(roundf(128.f * sc)),
                        static_cast<uint8_t>(roundf(64.f * sc)), OLED_COLOR_BLACK);
            fb.drawFBWithScale(x + p, y, 128, 64, target.get(), sc);
        }
    }

    if (window->_sadir) {
        ++window->_sa;
    } else if (gWindows[i]->_sa) {
        --window->_sa;
    }
    if (window->_sa == 10 && window->_sadir == 1) {
        if (it == gWindowsFB.end())
            gWindowsFB.erase(gWindowsFB.find(i));
        else
            gWindowsFB.erase(it);
        removeWindowImmediately(window);
    }
}


void render_task(void* v) {
    /*
    Wifi::enable();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Wifi::connect("ASUS", "asus-059-duu");
*/
    Framebuffer tmp;
    static File f("/wlp.bmp");
    static BMPBitmap* wp = nullptr;
    if (f.isFile()) {
        wp = new BMPBitmap(f);
        if (!wp->data) {
            delete wp;
            wp = nullptr;
        }
    }
    while (true) {
        time_t rtime = ::time(0);
        struct tm* time = localtime(&rtime);

        int _wlp_index = 0;

        if ((time->tm_mday >= 15 && time->tm_mon == 11) || (time->tm_year != 70 && time->tm_yday <= 15))
            _wlp_index = 3;
        Picture* wallpaper = CCOSCore::getPictures()[_wlp_index];

/*
        {
            static int prevInetState = 0;
            if (prevInetState != STATION_GOT_IP && Wifi::getConnectionStatus() == STATION_GOT_IP) {
                Cloud::onInternetConnection();
            }
            prevInetState = Wifi::getConnectionStatus();
        }*/

        {
            Mutex::L lock(uiThread);
            while (!gUiThread.empty()) {
                gUiThread.front()();
                gUiThread.pop_front();
            }
        }

        uint8_t b;

#ifdef KEY_EMU
        while (uart_read_bytes(UART_NUM_1, &b, 1, 0)) {
            uint8_t k;
            switch ((char)b) {
                case '1':
                    k = 0;
                    break;
                case '2':
                    k = 1;
                    break;
                case '3':
                    k = 2;
                    break;
                case '4':
                    k = 3;
                    break;
                case 'q':
                    k = 4;
                    break;
                case 'w':
                    k = 5;
                    break;
                case 'e':
                    k = 6;
                    break;
                case 'r':
                    k = 7;
                    break;
                case 'a':
                    k = 8;
                    break;
                case 's':
                    k = 9;
                    break;
                case 'd':
                    k = 10;
                    break;
                case 'f':
                    k = 11;
                    break;
                case 'z':
                    k = 12;
                    break;
                case 'x':
                    k = 13;
                    break;
                case 'c':
                    k = 14;
                    break;
                case 'v':
                    k = 15;
                    break;
                default:
                    continue;
            }
            CCOSCore::keyInput(k, 1);
            CCOSCore::keyInput(k, 0);
        }
#else
        while (uart_read_bytes(UART_NUM_1, &b, 1, 0)) {
            CCOSCore::keyInput(b >> 4, b & 0x0f);
        }
#endif
        {
            CCOSCore::keyInput(16, gpio_get_level(PIN_POWER_BUTTON) == 0);
        }
        {
            Event e;
            while (xQueueReceive(event_bus, &e, 0)) {
                //printf("Event %s, %ld\n", e.name, e.param);
                if (gCurrentWindow) {
                    gCurrentWindow->event(e);
                }
            }
        }
        if (gLastKeyCounter) {
            gLastKeyCounter--;
            if (gLastKeyCounter == 0) {
                CCOSCore::keyLongDown(gLastKey);
            }
        }
        fb.clear();

        if (!gUnlockTimeout) {
            if (gUnlockState == 4) {
                gCurrentState = State::NORMAL;
            }
            gUnlockState = 0;
        }
        if (gCurrentState == State::WINDOW_SELECT) {
            gWindowSelect.mAnimation += 0.09f * gWindowSelect.mDir;
            if (gWindowSelect.mAnimation < 0) {
                gWindowSelect.mAnimation = 0;
            } else if (gWindowSelect.mAnimation > 1) {
                gCurrentWindow = gWindowSelect.mCurrent;
                if (gCurrentWindow) {
                    gWindows.erase(std::find(gWindows.begin(), gWindows.end(), gCurrentWindow));
                    gWindows.push_back(gCurrentWindow);
                    gCurrentWindow->onResume();
                }
                gCurrentState = State::NORMAL;
                gWindowsFB.clear();
            }
        }
        switch (gCurrentState) {
            case State::WINDOW_SELECT: {
                if (wp) {
                    fb.drawImage(0, 0, *wp);
                } else {
                    wallpaper->draw(fb, 0, 0);
                }
                fb.shade();

                gWindowSelect.mSelected += (int8_t(CCOSCore::getKeyState(9)) - int8_t(CCOSCore::getKeyState(1))) * 0.1f;
                static Smoother selected(.4f);
                float currentSelected = selected.nextValue(gWindowSelect.mSelected);
                static int prev = int(roundf(currentSelected));
                if (prev != int(ceilf(currentSelected))) {
                    prev = int(ceilf(currentSelected));
                    CCOSCore::vibrate(10);
                }
                gWindowSelect.mSelected = glm::clamp(gWindowSelect.mSelected, 0.f, float(gWindows.size() - 1));
                size_t currentWindowIndex = 0;
                if (gWindowSelect.mRemoveAll) {
                    if (gWindowSelect.mRemoveCounter == 0) {
                        gWindowSelect.mRemoveCounter = 3;

                        for (size_t i = gWindows.size(); i > 0; --i) {
                            Window* w = gWindows[i - 1];
                            if (w->_sadir != 1) {
                                CCOSCore::removeWindow(w);
                                break;
                            }
                        }

                    } else {
                        --gWindowSelect.mRemoveCounter;
                    }
                }

                for (size_t i = 0; i < gWindows.size(); ++i) {
                    Window* window = gWindows[i];
                    if (window != gWindowSelect.mCurrent)
                        renderWindow(i, currentSelected, window);
                    else
                        currentWindowIndex = i;
                }
                if (gWindowSelect.mCurrent)
                    renderWindow(currentWindowIndex, currentSelected, gWindowSelect.mCurrent);

                if (gWindows.empty()) {
                    if (gWindowSelect.mRemoveAll) {
                        gCurrentState = State::NORMAL;
                    } else {
                        fb.drawString(1, 30, "No recently used apps", OLED_COLOR_WHITE, 0, true);
                    }
                }
                renderStatusBar();
                break;
            }
            case State::SLEEP: {
                if (!gSkipInputAfterSleep) {
                    for (size_t i = 0; i < 300; ++i) {
                        uint8_t s = gpio_get_level(PIN_POWER_BUTTON) == 0;
                        if (s) {
                            CCOSCore::wakeUp();
                            break;
                        }
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                    }
                } else {
                    gSkipInputAfterSleep = false;
                }
                int x = rand() % 70;
                int y = rand() % 30;

                char s[10];
                strftime(s, sizeof(s), "%H:%M", time);


                fb.drawString(x, y, s, OLED_COLOR_WHITE, FONT_FACE_TERMINUS_12X24_ISO8859_1, true);
                int len = strlen(s) * 12;
                strftime(s, sizeof(s), "%d/%m/%y", time);
                fb.drawString(x + len / 2 - strlen(s) * 6 / 2, y + 21, s, OLED_COLOR_WHITE,
                              FONT_FACE_TERMINUS_6X12_ISO8859_1, true);
                int16_t offset = 0;
                showIcons(offset, fb);

                break;
            }
            case State::NORMAL:
                // Top bar
            {
                int i;
                for (i = static_cast<int>(gWindows.size() - 1); i >= 0; i--) {
                    if (!gWindows[i]->isTransparent()) {
                        break;
                    }
                }
                if (i < 0)
                    i = 0;
                if (!gCurrentWindow || gWindows.empty() || (i == 0 && gWindows[0]->isTransparent()) || gWindowsHideAnimation) {
                    int16_t act = 10 - (int16_t(gMainSelectedApp) - 1) * 40;
                    static float smooth = 0;
                    smooth += (act - smooth) * 0.3f;

                    if (wp) {
                        float a = ((-(smooth - 10) / 40.f) + 1) / (gApplications.size() - 1);
                        fb.drawImage((int(wp->width) - 128) * -(a), 0, *wp);
                    } else {
                        wallpaper->draw(fb, 0, 0);
                    }

                    for (int16_t i = 0; i < int16_t(gApplications.size()); i++) {
                        if (smooth + i * 40 > 128) {
                            break;
                        }
                        Bitmap bitmap = gApplications[i]->getIcon();
                        fb.drawImage(smooth + i * 40 - 1, 15, bitmap, false, OLED_COLOR_BLACK);
                        fb.drawImage(smooth + i * 40 + 1, 15, bitmap, false, OLED_COLOR_BLACK);
                        fb.drawImage(smooth + i * 40, 15 - 1, bitmap, false, OLED_COLOR_BLACK);
                        fb.drawImage(smooth + i * 40, 15 + 1, bitmap, false, OLED_COLOR_BLACK);
                        fb.drawImage(smooth + i * 40, 15, bitmap);
                    }
                    fb.drawString(64 - gApplications[gMainSelectedApp]->getTitle().length() * 3, 50,
                                  gApplications[gMainSelectedApp]->getTitle().c_str(), OLED_COLOR_WHITE, 255, true);
                }
                if (gCurrentWindow) {
                    int j = i;
                    for (; i < gWindows.size(); i++) {
                        if ((gWindows[i]->_sa && gWindows[i]->_sadir == 0) || (gWindows[i]->_sadir == 1)) {
                            if (gWindows[i]->_sa == 10 && gWindows[i]->_sadir == 1) {
                                Window* w = gWindows[i];
                                removeWindowImmediately(w);
                            }
                        }
                    }
                    i = j;
                    for (; i < gWindows.size(); i++) {
                        gCurrentWindow = gWindows[i];
                        if ((gWindows[i]->_sa && gWindows[i]->_sadir == 0) || (gWindows[i]->_sadir == 1)) {
                            tmp.clear();
                            tmp.setCoordsOffset(0, 0);
                            gWindows[i]->render(tmp);
                            if (!gCurrentWindow) {
                                break;
                            }
                            uint8_t buffer[1024];
                            tmp.snapshot(128, 64, buffer);
                            Bitmap buf(buffer, 128, 64);
                            /*
                            uint16_t y = static_cast<uint16_t>(gWindows[i]->_sa * gWindows[i]->_sa *
                                                               gWindows[i]->_sa *
                                                               gWindows[i]->_sa / 156);
                            fb.drawRect(0, y, 128, 64, OLED_COLOR_BLACK);
                            fb.drawImage(0, y, buf);*/

                            if (gWindows[i]->isAnimationHorizontal()) {
                                uint16_t y = static_cast<uint16_t>(gWindows[i]->_sa * gWindows[i]->_sa *
                                                                   gWindows[i]->_sa *
                                                                   gWindows[i]->_sa / 78);
                                //printf("Drawing ebat\n");
                                //if (!gWindows[i]->hasTransparency())
                                //    fb.drawRect(y + gWindows[i]->x, gWindows[i]->y, 127, 64, OLED_COLOR_BLACK);
                                fb.drawImage(y, 0, buf);
                            } else {
                                uint16_t y = static_cast<uint16_t>(gWindows[i]->_sa * gWindows[i]->_sa *
                                                                   gWindows[i]->_sa *
                                                                   gWindows[i]->_sa / 156);

                                if (!gWindows[i]->hasTransparency())
                                    fb.drawRect(gWindows[i]->x, y + gWindows[i]->y, 128, 64, OLED_COLOR_BLACK);
                                fb.drawImage(0, y, buf);
                            }

                            if (gWindows[i]->_sadir) {
                                ++gWindows[i]->_sa;
                            } else {
                                --gWindows[i]->_sa;
                            }
                        } else {
                            /*
                            Framebuffer temp;
                            temp.clear();
                            gWindows[i]->render(temp);
                            glm::mat4 transform = glm::translate(glm::mat4(1), {0, 0, -4});
                            fb.drawTextureWithTranform(transform, &temp);
                            */
                            if (gWindowsHideAnimation) {
                                Framebuffer temp;
                                temp.clear();
                                gWindows[i]->render(temp);
                                float scale = float(gWindowsHideAnimation) / 128.f;
                                fb.drawRect(int16_t(scale * 64.f), int16_t(scale * 32.f), uint8_t(scale * 128.f), uint8_t(scale * 64.f), OLED_COLOR_WHITE);
                                fb.drawFBWithScale(int16_t(scale * 64.f), int16_t(scale * 32.f), 128, 64, &temp, 1.f - scale);
                            } else {
                                gWindows[i]->render(fb);
                            }
                            if (!gCurrentWindow) {
                                break;
                            }
                        }
                        if (i + 2 == gWindows.size()) {
                            fb.shade();
                        }
                    }
                }
                if (gWindowsHideAnimation) {
                    gWindowsHideAnimation += gWindowsHideAnimation;
                    if (gWindowsHideAnimation >= 128) {
                        gWindowsHideAnimation = 0;
                        gCurrentState = NORMAL;
                        if (gCurrentWindow) {
                            gCurrentWindow->onPause();
                        }
                        gCurrentWindow = nullptr;
                        gWindowsFB.clear();
                    }
                }

                //gCurrentWindow = nullptr;
                renderStatusBar();

                fb.setCoordsOffset(0, 0);

                // Toast
                if (gToastLive) {
                    gToastLive--;

                    int16_t len = gToastText.length() * 6 + 4;
                    int16_t x = (127 - len) / 2;
                    fb.drawRect(x, 50, len, 16, OLED_COLOR_WHITE);
                    fb.drawRect(x + 1, 51, len - 2, 14, OLED_COLOR_BLACK);
                    fb.drawString(x + 3, 53, gToastText.c_str(), OLED_COLOR_WHITE,
                                  FONT_FACE_TERMINUS_6X12_ISO8859_1);
                }
            }
                break;
            case State::LOCKSCREEN: {
                wallpaper->draw(fb, 0, 0);

                // Clock
                char s[10];

                gUnlockAnimation += (gUnlockState * 16 - gUnlockAnimation) * 0.3f;


                fb.setCoordsOffset(0, 0);
                strftime(s, sizeof(s), "%d/%m/%y", time);
                fb.drawString(127 - strlen(s) * 7 + gUnlockAnimation, 21, s, OLED_COLOR_WHITE,
                              FONT_FACE_BITOCRA_7X13, true);

                strftime(s, sizeof(s), "%H:%M", time);
                fb.drawString(127 - strlen(s) * 12 + gUnlockAnimation, 0, s, OLED_COLOR_WHITE,
                              FONT_FACE_TERMINUS_12X24_ISO8859_1, true);


                if (__meteo_data) {
                    char buf[16];
                    int len = sprintf(buf, "%d oC", __meteo_data->temp);
                    fb.drawString(static_cast<int8_t>(128 - len * 4 + gUnlockAnimation), 35, buf, OLED_COLOR_INVERT,
                                  FONT_FACE_BITOCRA_4X7);
                }


                if (!gUnlockTimeout) {
                    gUnlockState = 0;
                    // Slide to unlock
                    fb.drawString(15, 52, "slide to unlock", OLED_COLOR_WHITE, FONT_FACE_BITOCRA_6X11, true);
                    static uint8_t stu_anim = 0;
                    uint8_t stu_a = stu_anim / 4;
                    if (stu_a > 1) {
                        fb.drawString(98 + stu_a * 6, 52, ">", OLED_COLOR_WHITE, FONT_FACE_BITOCRA_6X11, true);
                    }
                    stu_anim %= 24;
                    stu_anim++;
                } else {
                    gUnlockTimeout--;
                }
                int16_t xOffset = strlen(s) * 12 - gUnlockAnimation + 1;
                showIcons(xOffset, fb);
                break;
            }
        }
        if (gSleepTimeout) {
            gSleepTimeout--;
            if (!gSleepTimeout) {
                if (gWindows.empty()) {
                    CCOSCore::shutdown();
                } else {
                    for (Window* w : gWindows) {
                        if (w->sleepLock()) {
                            gSleepTimeout = SLEEP_TIMEOUT;
                            goto ololo;
                        }
                    }
                    CCOSCore::sleep();
                }
            }
        }

        ololo:

/*
        _ffps++;

        char buf[16];
        sprintf(buf, "%d", hall_sensor_read());
        fb.drawString(0, 0, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_7X13, true);
        sprintf(buf, "%d", adc1_get_voltage(ADC1_CHANNEL_6));
        fb.drawString(0, 13, buf, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_7X13, true);
*/

        if (Streaming::instance.mSocket) {
            if (Streaming::instance.mStreamDst.sin_addr.s_addr == 0) {
                int status = Socket::resolvehost(Streaming::instance.mAddress, 4530, Streaming::instance.mStreamDst);
                if (status != 0) {
                    Streaming::instance.stop(strerror(status));

                }
            }
            if (Streaming::instance.mSocket) {
                if (Wifi::getStatus() != Wifi::CONNECTED_GOT_IP) {
                    Streaming::instance.stop("Network unreachable");
                } else if (Streaming::instance.mSocket->write_bytes(reinterpret_cast<const char*>(fb.buffer), 1024, (sockaddr*) &Streaming::instance.mStreamDst) < 0) {
                    if (errno != 12) {
                        char buf[128];
                        sprintf(buf, "errno %d: %s", errno, strerror(errno));
                        Streaming::instance.stop(buf);
                    }
                }
            }

        }

        ssd1306_load_frame_buffer(&display, fb.buffer);
    }

    vTaskDelete(NULL);
}

void removeWindowImmediately(Window* w) {
    w->lua_close();
    gWindows.erase(std::find(gWindows.begin(), gWindows.end(), w));
    if (!w->mLuaState) // Window will be removed automatically by Lua
        delete w;
    if (w == gWindowSelect.mCurrent) {
        gWindowSelect.mCurrent = nullptr;
    }
    Window* prevBack = gWindows.back();
    if (prevBack != gWindows.back())
        gCurrentWindow->onResume();
    if (gCurrentWindow) {
        if (gWindows.empty()) {
            gCurrentWindow = nullptr;
        } else {
            gCurrentWindow = gWindows.back();
        }
    }
}

extern "C" {
#include "esp_log.h"
#include "spiffs_vfs.h"
#include <errno.h>
#include <sys/fcntl.h>
};

static void list(char* path, char* match) {

    DIR* dir = NULL;
    struct dirent* ent;
    char type;
    char size[9];
    char tpath[255];
    char tbuffer[80];
    struct stat sb;
    struct tm* tm_info;
    char* lpath = NULL;
    int statok;

    printf("LIST of DIR [%s]\r\n", path);
    // Open directory
    dir = opendir(path);
    if (!dir) {
        printf("Error opening directory\r\n");
        return;
    }

    // Read directory entries
    uint64_t total = 0;
    int nfiles = 0;
    printf("T  Size      Date/Time         Name\r\n");
    printf("-----------------------------------\r\n");
    while ((ent = readdir(dir)) != NULL) {
        sprintf(tpath, path);
        if (path[strlen(path) - 1] != '/') strcat(tpath, "/");
        strcat(tpath, ent->d_name);
        tbuffer[0] = '\0';

        //if ((match == NULL) || (fnmatch(match, tpath, (FNM_PERIOD)) == 0))
        {
            // Get file stat
            statok = stat(tpath, &sb);

            if (statok == 0) {
                tm_info = localtime(&sb.st_mtime);
                strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
            } else sprintf(tbuffer, "                ");

            if (ent->d_type == DT_REG) {
                type = 'f';
                nfiles++;
                if (statok) strcpy(size, "       ?");
                else {
                    total += sb.st_size;
                    if (sb.st_size < (1024 * 1024)) sprintf(size, "%8d", (int) sb.st_size);
                    else if ((sb.st_size / 1024) < (1024 * 1024)) sprintf(size, "%6dKB", (int) (sb.st_size / 1024));
                    else sprintf(size, "%6dMB", (int) (sb.st_size / (1024 * 1024)));
                }
            } else {
                type = 'd';
                strcpy(size, "       -");
            }

            printf("%c  %s  %s  %s  %s\r\n",
                   type,
                   size,
                   tbuffer,
                   ent->d_name,
                   tpath
            );
        }
    }
    if (total) {
        printf("-----------------------------------\r\n");
        if (total < (1024 * 1024)) printf("   %8d", (int) total);
        else if ((total / 1024) < (1024 * 1024)) printf("   %6dKB", (int) (total / 1024));
        else printf("   %6dMB", (int) (total / (1024 * 1024)));
        printf(" in %d file(s)\r\n", nfiles);
    }
    printf("-----------------------------------\r\n");

    closedir(dir);

    free(lpath);

    uint32_t tot, used;
    spiffs_fs_stat(&tot, &used);
    printf("SPIFFS: free %d KB of %d KB\r\n", (tot - used) / 1024, tot / 1024);
}

void loading_task(void* v) {
    /* Disable buffering on stdin and stdout */
    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);


    float animation = 0;

    Framebuffer framebuffer;
    framebuffer.clear();

    framebuffer.drawString(static_cast<int8_t>(20), 16, "CCOS", OLED_COLOR_WHITE,
                           FONT_FACE_TERMINUS_16X32_ISO8859_1);

    ssd1306_load_frame_buffer(&display, framebuffer.buffer);
    ssd1306_set_whole_display_lighting(&display, false);


    vfs_spiffs_register_k();

    mkdir("/spiffs/ccos", 0777);
    list("/spiffs", NULL);
    list("/spiffs/ccos", NULL);

    /*
    Dir d("/");
    std::vector<File> files;
    d.list(files);
    for (auto& f : files) {
        printf("%s\n", f.getFilename().c_str());
    }
    */
    SpinnerView spinner(10, 20);
    spinner.radius = 12;
    spinner.androidStyleAnimation = 0;

    bool firstTime = true;

    while (!ready) {
        if (animation > 1) {
            animation = 1;
        } else if (animation < 1) {
            animation += 0.05f;
        }
        framebuffer.clear();

        float anim = static_cast<float>(1 - (cos(animation / 3.14 * 10) + 1) / 2);
        spinner.x = static_cast<int16_t>(28 - 18 * anim);
        if (!firstTime) {
            spinner.render(framebuffer);
            framebuffer.setCoordsOffset(0, 0);
            framebuffer.drawRect(static_cast<int8_t>(20 + 18 * anim), 10, 40, 40, OLED_COLOR_BLACK); // overlay
        }
        framebuffer.drawString(static_cast<int8_t>(20 + 18 * anim), 16, "CCOS", OLED_COLOR_WHITE,
                               FONT_FACE_TERMINUS_16X32_ISO8859_1);

        ssd1306_load_frame_buffer(&display, framebuffer.buffer);
        if (firstTime) {
            vTaskDelay(250 / portTICK_PERIOD_MS);
            firstTime = false;
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }


    xTaskCreate(render_task, "RenderTask", 8192, NULL, 2, NULL);
    //TimerHandle_t t = xTimerCreate("fps_timer", 1000 / portTICK_PERIOD_MS, true, 0, fps_timer);
    //xTimerStart(t, portMAX_DELAY);
    //sdk_wifi_set_opmode_current(STATION_MODE);
    //sdk_wifi_set_sleep_type(WIFI_SLEEP_MODEM);


    vTaskDelete(NULL);
}

void timer_x(TimerHandle_t t) {
    gUptime++;
    /*
    if (__time % 10 == 0) {
        int fd = open("time", O_WRONLY|O_CREAT, 0);
        if (fd < 0) {
            return;
        }

        write(fd, &__time, sizeof(__time));
        close(fd);
    }*/
}

/*
void sync_cloud() {
    addrinfo hints;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    //hostent* h = gethostbyname("alex2772.ru");

    int err = getaddrinfo("alex2772.ru", "4322", &hints, &res);
    if (err != 0 || res == NULL) {
        if(res)
            freeaddrinfo(res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }

    int s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        freeaddrinfo(res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }

    int c;
    if((c = connect(s, res->ai_addr, res->ai_addrlen)) != 0) {
        lwip_close(s);
        freeaddrinfo(res);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        return;
    }
    freeaddrinfo(res);

    vTaskDelay(1000 / portTICK_PERIOD_MS);


    lwip_read(s, (char*)&__time, sizeof(time_t));
    __time += TIME_OFFSET * 3600;

    lwip_close(s);
}

void wifi_task(void* mp) {
sdk_station_config config = {};

sdk_wifi_set_opmode(STATION_MODE);

memset(&config, 0, sizeof(sdk_station_config));
sprintf((char *)(config.ssid), WIFI_SSID);
sprintf((char *)(config.password), WIFI_PASS);
sdk_wifi_station_set_config(&config);
sdk_wifi_station_connect();

int c;
while (1) {
    c = sdk_wifi_station_get_connect_status();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    switch (c) {
        case STATION_GOT_IP:
            goto sync;
        default:
            goto ready;
    }
}

sync:

sync_cloud();
ready:
//ready = true;
vTaskDelete( NULL );
}*/
#include <tcpip_adapter.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <driver/ledc.h>

static esp_err_t event_handler(void* ctx, system_event_t* event) {
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            break;
        case SYSTEM_EVENT_STA_GOT_IP: {
            Wifi::setIp(uint32_t(event->event_info.got_ip.ip_info.ip.addr));
            Wifi::setState(Wifi::CONNECTED_GOT_IP);


            break;
        }
        case SYSTEM_EVENT_STA_CONNECTED:
            Wifi::setState(Wifi::CONNECTED);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Wifi::setState(Wifi::IDLE);
            break;
        default:
            break;
        case SYSTEM_EVENT_WIFI_READY:
            break;
        case SYSTEM_EVENT_SCAN_DONE:
            break;
        case SYSTEM_EVENT_STA_STOP:
            break;
        case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
            break;
        case SYSTEM_EVENT_AP_START:
            break;
        case SYSTEM_EVENT_AP_STOP:
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            break;
        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            break;
        case SYSTEM_EVENT_AP_PROBEREQRECVED:
            break;
        case SYSTEM_EVENT_GOT_IP6:
            break;
        case SYSTEM_EVENT_ETH_START:
            break;
        case SYSTEM_EVENT_ETH_STOP:
            break;
        case SYSTEM_EVENT_ETH_CONNECTED:
            break;
        case SYSTEM_EVENT_ETH_DISCONNECTED:
            break;
        case SYSTEM_EVENT_ETH_GOT_IP:
            break;
        case SYSTEM_EVENT_MAX:
            break;
    }
    return ESP_OK;
}


static int64_t prev = 0;

void _wup_programmable_handler(void*) {
    static const char* l = "34_NEGEDGE";

    Event e;
    e.name = l;
    int64_t cur = esp_timer_get_time();
    e.param = cur - prev;
    if (e.param > 25000) {
        xQueueSendFromISR(event_bus, &e, NULL);
        prev = cur;
    }

}

void CCOSCore::init() {
    setlocale(LC_ALL, "");
    ESP_LOGI(LCC, "Starting CCOS");

    //Blink
    {
        ledc_timer_config_t ledc_timer = {
                LEDC_HIGH_SPEED_MODE,
                LEDC_TIMER_13_BIT,
                //LEDC_TIMER_13_BIT,
                LEDC_TIMER_0,
                5000
        };
        ledc_timer_config(&ledc_timer);
        ledc_timer.timer_num = LEDC_TIMER_1;
        ledc_timer_config(&ledc_timer);
        ledc_timer.timer_num = LEDC_TIMER_2;
        ledc_timer_config(&ledc_timer);

        ledc_channel_config_t ledc_channel[] = {
                {
                        PIN_LED_B,
                        LEDC_HIGH_SPEED_MODE,
                        LEDC_CHANNEL_0,
                        LEDC_INTR_DISABLE,
                        LEDC_TIMER_0,
                        8192
                        /*
                        .channel    = LEDC_CHANNEL_0,
                        .duty       = 4000,
                        .gpio_num   = PIN_LED_B,
                        .speed_mode = LEDC_HIGH_SPEED_MODE,
                        .timer_sel  = LEDC_TIMER_0*/
                },
                {
                        PIN_LED_R,
                        LEDC_HIGH_SPEED_MODE,
                        LEDC_CHANNEL_1,
                        LEDC_INTR_DISABLE,
                        LEDC_TIMER_1,
                        8192
                        /*
                        .channel    = LEDC_CHANNEL_0,
                        .duty       = 4000,
                        .gpio_num   = PIN_LED_B,
                        .speed_mode = LEDC_HIGH_SPEED_MODE,
                        .timer_sel  = LEDC_TIMER_0*/
                },
                {
                        PIN_LED_G,
                        LEDC_HIGH_SPEED_MODE,
                        LEDC_CHANNEL_2,
                        LEDC_INTR_DISABLE,
                        LEDC_TIMER_2,
                        8192
                        /*
                        .channel    = LEDC_CHANNEL_0,
                        .duty       = 4000,
                        .gpio_num   = PIN_LED_B,
                        .speed_mode = LEDC_HIGH_SPEED_MODE,
                        .timer_sel  = LEDC_TIMER_0*/
                },
        };
        for (size_t i = 0; i < sizeof(ledc_channel) / sizeof(ledc_channel_config_t); ++i) {
            ledc_channel_config(&ledc_channel[i]);
        }
    }
    xTaskCreate(indicator_loading_task, "Indicator", 2048, nullptr, 2, nullptr);

    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, PIN_INPUT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 256, 256, 10, 0, 0));

    esp_event_loop_init(event_handler, NULL);
    ESP_LOGI(LCC, "Set up input device");

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_PERIPHERAL) | (1ULL << PIN_MOTOR);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    if (FirmwareConfig::read().mMajorModel < 4) {
        gpio_set_level(PIN_MOTOR, 1);
    } else {
        gpio_set_level(PIN_MOTOR, 0);
    }
    gpio_set_level(PIN_PERIPHERAL, 1);


    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_POWER_BUTTON);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);


    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_PROGRAMMABLE);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    //gpio_intr_enable(PIN_POWER_BUTTON);
    gpio_intr_enable(PIN_PROGRAMMABLE);

    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    //gpio_isr_handler_add(PIN_POWER_BUTTON, _wup_handler, 0);
    gpio_isr_handler_add(PIN_PROGRAMMABLE, _wup_programmable_handler, 0);


    display.protocol = PROTOCOL;
    display.screen = SSD1306_SCREEN;
    display.addr = ADDR;
    display.width = DISPLAY_WIDTH;
    display.height = DISPLAY_HEIGHT;

    vTaskDelay(350 / portTICK_PERIOD_MS);
    if (ssd1306_init(&display) != 0) {
        gpio_set_level(PIN_PERIPHERAL, 0);
        esp_restart();
    }
    ssd1306_set_segment_remapping_enabled(&display, true);
    ssd1306_set_scan_direction_fwd(&display, false);
    xTaskCreate(loading_task, "LoadingTask", 65536, NULL, 3, NULL);

    {
        TimerHandle_t t = xTimerCreate("Timer", 1000 / portTICK_PERIOD_MS, pdTRUE, 0, timer_x);

        xTimerStart(t, portMAX_DELAY);
    }

    memset(gButtonStates, 0, 18);

    vibrator_queue = xQueueCreate(20, sizeof(uint16_t));
    blink_queue = xQueueCreate(20, sizeof(uint16_t));
    event_bus = xQueueCreate(40, sizeof(Event));
    xTaskCreate(vibrator_task, "VibratorTask", 2048, NULL, 2, NULL);
    xTaskCreate(blink_task, "BlinkTask", 2048, NULL, 2, NULL);
    xTaskCreate(battery_task, "BatteryTask", 2048, NULL, 2, NULL);

    FirmwareConfig::read();


    //CCOSCore::registerIcon(new MemoryIcon);
    CCOSCore::registerIcon(new BatteryIcon);
    CCOSCore::registerIcon(new WifiIcon);
    CCOSCore::registerIcon(new SyncIcon);
    CCOSCore::registerIcon(new StreamIcon);

    CCOSCore::registerApplication(new VKApplication);
    //CCOSCore::registerApplication(new BrowserApplication);
    CCOSCore::registerApplication(new Settings);
    //CCOSCore::registerApplication(new Scheduler);
    CCOSCore::registerApplication(new CalculatorApplication);
    CCOSCore::registerApplication(new Today);
    CCOSCore::registerApplication(new ImageViewerApplication);
    CCOSCore::registerApplication(new Explorer);
    CCOSCore::registerApplication(new EditorApplication);
    CCOSCore::registerApplication(new Timberman);

    gPictures.push_back(new Picture(catnoir, 128, 64));
    gPictures.push_back(new Picture(lb, 128, 64));
    gPictures.push_back(new Picture(phys, 128, 64));
    gPictures.push_back(new LivePicture(xmas, 128, 64, [](Framebuffer& fb) {
        struct _particle {
            float x = 0, y = 0;
            float dx = 0, dy = 0;
            uint8_t life = 80;
        };

        static std::deque<_particle> particles;

        int _cnt = rand() % 4;
        for (int i = 0; i < _cnt; i++) {
            _particle p;
            p.x = rand() % 170;
            p.y = -rand() % 10 - 5;
            p.dx = float(rand() % 1000) / -10000.f;
            p.dy = float(rand() % 1000) / 10000.f + 0.01f;
            particles.push_back(p);
        }
        for (std::deque<_particle>::iterator i = particles.begin(); i != particles.end();) {
            _particle& p = *i;
            p.dy += (float(rand() % 1000) / 5000.f - 0.07f) * 0.1f;
            p.dx += (float(rand() % 1000) / 5000.f - 0.16f) * 0.09f;

            float x = p.x, y = p.y;

            p.x += p.dx * 12;
            p.y += p.dy * 10;

            fb.drawLine(x, y, p.x - p.dx * 5, p.y - p.dy * 5, OLED_COLOR_WHITE);


            p.life--;
            if (!p.life) {
                i = particles.erase(i);
            } else {
                i++;
            }
        }
    }));

    CCOSCore::blink(100);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_BATTERY, ADC_ATTEN_DB_0);
    //adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    Wifi::init();
    //gewd();

    vTaskDelay(50 / portTICK_PERIOD_MS);

    ESP_LOGI(LCC, "Ready");
    ready = true;

    /*
    NVS::Accessor t = getNVSHandle().open();
    bool sowl;
    if (!t.read("sowl", sowl)) {
        t.write("sowl", sowl = true);
    }
    t.write("sowl", sowl = true);
    if (sowl)
        CCOSCore::displayWindow(new SowlonyaWindow());*/
}

void CCOSCore::keyInput(int8_t key, int8_t str) {
    static uint8_t _cd = 0;
    if (str == 0) {
        if (key < 16) {
            _cd = 7;
        } else if (_cd) {
            _cd--;
            return;
        }
    }
    if (key != 16)
        gSleepTimeout = SLEEP_TIMEOUT;
    key %= 18;
#ifndef KEY_EMU

    if (FirmwareConfig::read().mMajorModel < 4) {
        static uint8_t keymap[] = {
                12,
                13,
                1,
                5,
                8,
                0,
                4,
                3,
                2,
                6,
                10,
                14,
                7,
                9,
                11,
                15
        };
        if (key < 16)
            key = keymap[key];
        if (key == 12 || key == 10)
            str--;
    }
#endif
    if (key != 16)
        ESP_LOGI("INPUT", "Key %d %d", key, str);
    uint8_t prev = gButtonStates[key];
    if (gButtonStates[key] != str) {
        if (gCurrentWindow) {
            gCurrentWindow->keyPressureChanged(key);
        }
    }
    gButtonStates[key] = str;
    if (prev == 0 && str) {
        keyDown(key);
    } else if (prev && str == 0) {
        keyUp(key);
    }
}

void ::CCOSCore::keyDown(uint8_t key) {
    switch (gCurrentState) {
        case State::WINDOW_SELECT: {
            vibrate(12);
            switch (key) {
                case 5:
                    if (gWindows.empty()) {
                        gCurrentState = State::NORMAL;
                        gSkipKeyUp = true;
                    } else {
                        gWindowSelect.mDir = 1;
                        gWindowSelect.mCurrent = gWindows[glm::min(size_t(ceilf(gWindowSelect.mSelected)), gWindows.size())];
                    }
                    break;
                case 13:
                    gCurrentWindow = nullptr;
                    gCurrentState = State::NORMAL;
                    break;
                case 3: {
                    removeWindow(gWindows[glm::min(size_t(ceilf(gWindowSelect.mSelected)), gWindows.size())]);
                    break;
                }

            }
            break;
        }
        case State::LOCKSCREEN:
            vibrate(12);
            if (key < 4) {
                if (gUnlockState == key) {
                    gUnlockState++;
                    gUnlockTimeout = 15;
                }
            }

            if (key == 16)
                sleep();
            break;
        case State::NORMAL:
            vibrate(12);
            if (!gCurrentWindow) {
                switch (key) {
                    // DEBUG
/*
                    case 14:
                        CCOSCore::displayWindow(new MessageDialog("Upper", "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"));
                        break;
                    case 15:
                        CCOSCore::displayWindow(new MessageDialog("Lower", "абвгдеёжзийклмнопрстуфхцчшщъыьэюя"));
                        break;
*/

                    case 4:
                        if (gMainSelectedApp) {
                            gMainSelectedApp--;
                        } else {
                            gMainSelectedApp = gApplications.size() - 1;
                        }
                        break;
                    case 6:
                        gMainSelectedApp++;
                        break;

                }
                gMainSelectedApp %= gApplications.size();
            } else {
                gCurrentWindow->keyDown(key);
                switch (key) {
                    case 15:
                        gCurrentWindow->focusNext();
                        break;
                }
            }
            break;
        case SLEEP:
            break;
    }
    gLastKey = key;
    gLastKeyCounter = 15;
}

bool ignoreSleep = false;

void ::CCOSCore::keyUp(uint8_t key) {
    if (gSkipKeyUp) {
        gSkipKeyUp = false;
        return;
    }

    gLastKeyCounter = 0;

    switch (gCurrentState) {
        case State::NORMAL:
            if (key == 16) {
                if (ignoreSleep) {
                    ignoreSleep = false;
                } else {
                    sleep();
                }
            }
            if (gCurrentWindow) {
                if (key == 13 && getKeyState(12) && !gCurrentWindow->isTransparent()) {
                    CCOSCore::vibrate(10);
                    gWindowsHideAnimation = 1;

                    return;
                }
                gCurrentWindow->keyRelease(key);
            } else {
                if (key == 5) {
                    gApplications[gMainSelectedApp]->launch();
                }
            }
            break;
        case SLEEP:
            break;
        case LOCKSCREEN:
            break;
        case WINDOW_SELECT:
            break;
    }
}

uint8_t CCOSCore::getBatteryLevel() {
    Mutex::L lock(_battery);
    uint32_t sum = 0;
    for (auto& f : _battery_d) {
        sum += f;
    }
    if (_battery_d.empty()) {
        return 0;
    } else {
        sum /= _battery_d.size();

        if (sum > BATT_UP) {
            return 100;
        } else if (sum < BATT_DOWN) {
            return 0;
        }
        sum -= BATT_DOWN;
        return static_cast<uint8_t>(powf(float(sum) / float(BATT_UP - BATT_DOWN), 2) * 100);
    }
}

void ::CCOSCore::vibrate(uint16_t ms) {

    static uint8_t strength = 255;
    CCOSCore::getNVSHandle().open().read("vibrator", strength);
    if (strength) {
        ms = uint16_t(floorf(float(ms) * float(strength) / 255.f));

        xQueueSend(vibrator_queue, (void*) &ms, portMAX_DELAY);
    }
}

void ::CCOSCore::blink(uint16_t ms) {
    xQueueSend(blink_queue, (void*) &ms, portMAX_DELAY);
}

uint8_t CCOSCore::getKeyState(uint8_t key) {
    return gButtonStates[key % 18];
}

void ::CCOSCore::registerApplication(Application* pApplication) {
    gApplications.push_back(pApplication);
}

void ::CCOSCore::displayWindow(Window* window) {
    runOnUiThread([&, window]() {
        for (uint8_t i = 0; i < 15; i++) {
            if (getKeyState(i)) {
                gSkipKeyUp = true;
            }
        }
        if (gCurrentWindow) {
            gCurrentWindow->onPause();
        }
        gWindows.push_back(window);
        gCurrentWindow = window;
        if (!window->views.empty())
            window->views[0]->focus();
    });
}

void ::CCOSCore::showToast(const std::string text) {
    gToastText = text;
    gToastLive = 30;
}

void CCOSCore::keyLongDown(uint8_t key) {
    if (key == 16 && gCurrentState == State::NORMAL) {
        if (getKeyState(12)) {
            screenshot();
        } else {
            //shutdown();

            bool k = true;
            for (auto& p : gWindows) {
                if (p->getTitle() == "Shutdown") {
                    k = false;
                    break;
                }
            }
            if (k) {
                ignoreSleep = true;
                CCOSCore::displayWindow(new ChooserWindow("Shutdown", {
                        {"Shutdown",   {calc, 30, 30}, []() {
                            CCOSCore::shutdown();
                        }},
                        {"Reboot",     {calc, 30, 30}, []() {
                            CCOSCore::reboot(0);
                        }},
                        {"Recovery",   {calc, 30, 30}, []() {
                            CCOSCore::reboot(1);
                        }},
                        {"Screenshot", {calc, 30, 30}, []() {
                            CCOSCore::runOnUiThread([]() {
                                CCOSCore::screenshot();
                            });
                        }}
                }));
            }
        }
    }
    if (gCurrentState == WINDOW_SELECT && key == 3) {
        gWindowSelect.mRemoveAll = true;
        gWindowSelect.mRemoveCounter = 0;
        if (gWindows.empty()) {
            gWindowSelect.mSelected = 0;
        } else {
            gWindowSelect.mSelected = gWindows.size() - 1;
        }
    }
    if (getKeyState(12) && key == 13 && (!gCurrentWindow|| !gCurrentWindow->isTransparent())) {
        CCOSCore::vibrate(20);
        gCurrentState = WINDOW_SELECT;
        gWindowSelect.mAnimation = 1;
        gWindowSelect.mRemoveAll = false;
        gWindowSelect.mCurrent = gCurrentWindow;
        if (gCurrentWindow) {
            gCurrentWindow->onPause();
        }
        if (gWindows.empty()) {
            gWindowSelect.mSelected = 0;
        } else {
            gWindowSelect.mSelected = gWindows.size();
        }
        gWindowSelect.mDir = -1;
    }
    if (gCurrentWindow) {
        gCurrentWindow->keyLongDown(key);
        switch (key) {
            case 3:
                if (getKeyState(12)) {
                    removeWindow(gCurrentWindow);
                }
                break;
        }
    }
    switch (key) {
        case 2:
            if (getKeyState(12) && !Cloud::isSyncing() && Wifi::isInetAvailable()) {
                Cloud::sync();
            }
            break;
    }
}

void ::CCOSCore::wakeUp() {
    gCurrentState = LOCKSCREEN;
    ssd1306_set_contrast(&display, 255);
    gSleepTimeout = SLEEP_TIMEOUT;
    gSkipInputAfterSleep = false;
    uint8_t b;
    while (uart_read_bytes(UART_NUM_1, &b, 1, 0)) {

    }
}

void ::CCOSCore::sleep() {
    gSkipInputAfterSleep = true;
    gCurrentState = SLEEP;
    ssd1306_set_contrast(&display, 1);
}

void ::CCOSCore::setTime(time_t t) {
    timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec = t;
    settimeofday(&tv, NULL);
}

std::vector<Window*>& CCOSCore::getWindows() {
    return gWindows;
}

void ::CCOSCore::registerIcon(ViewIcon* icon) {
    gIcons.push_back(icon);
}

#include <algorithm>
#include <map>
#include <esp_ota_ops.h>

void ::CCOSCore::removeWindow(Window* window) {
    runOnUiThread([&, window]() {
        if (!gWindows.empty()) {
            window->_sadir = 1;
            CCOSCore::vibrate(10);
        }
    });
}

void ::CCOSCore::runOnUiThread(std::function<void()> func) {
    Mutex::L lock(uiThread);
    gUiThread.push_back(func);
}

uint32_t CCOSCore::getUptime() {
    return gUptime;
}

std::vector<Picture*>& CCOSCore::getPictures() {
    return gPictures;
}

void CCOSCore::shutdown() {
    runOnUiThread([]() {
        if (FirmwareConfig::read().mMajorModel < 4) {
            gpio_set_level(PIN_MOTOR, 1);
        } else {
            gpio_set_level(PIN_MOTOR, 0);
        }
        float len = 0.5f;
        for (float i = 0; i < 32; i += len) {
            fb.drawRect(0, 0, 128, i, OLED_COLOR_BLACK);
            fb.drawRect(0, 64 - i, 128, i + 1, OLED_COLOR_BLACK);

            fb.drawLine(0, i, 128, i, Framebuffer::Color::OLED_COLOR_WHITE);
            fb.drawLine(0, 63 - i, 128, 63 - i, Framebuffer::Color::OLED_COLOR_WHITE);

            ssd1306_load_frame_buffer(&display, fb.buffer);
            len *= 1.1f;
        }
        fb.clear();
        fb.drawLine(0, 32, 127, 32, Framebuffer::Color::OLED_COLOR_WHITE);
        ssd1306_load_frame_buffer(&display, fb.buffer);
        len = 3.f;
        for (float i = 0; i < 64; i += len) {
            fb.drawLine(0, 32, i, 32, OLED_COLOR_BLACK);
            fb.drawLine(128, 32, 128 - i, 32, OLED_COLOR_BLACK);
            ssd1306_load_frame_buffer(&display, fb.buffer);
            len -= 0.05f;
        }
        ssd1306_display_on(&display, false);

        esp_sleep_enable_ext1_wakeup(1ULL << 0, ESP_EXT1_WAKEUP_ALL_LOW);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        esp_deep_sleep_start();
    });
}

NVS& CCOSCore::getNVSHandle() {
    static NVS _handle("ccos");
    return _handle;
}

void CCOSCore::openFile(const File& f) {
    std::vector<ContextMenu::Item> items;
    if (f.getFilename().length() > 4 && f.getFilename().substr(f.getFilename().length() - 4, 4) == ".lua") {
        items.push_back({
                                "Execute",
                                [&, f]() {
                                    execute(f);
                                }});
    }
    for (size_t i = 0; i < gApplications.size(); ++i) {
        if (gApplications[i]->accepts(f.getPath())) {
            items.push_back({
                                    gApplications[i]->getTitle(),
                                    [&, i, f]() {
                                        gApplications[i]->openFile(f);
                                    }});
        }
    }
    if (items.size() > 1) {
        CCOSCore::displayWindow(new ContextMenu("Action", items));
    } else if (items.size() == 1) {
        items[0].callable();
    } else {
        showToast("Couldn't open file");
    }
}

std::map<lua_State*, Process*> processes;

static int _at_panic(lua_State* L) {
    const char* s = lua_tostring(L, -1);
    CCOSCore::killMePlz(s, L);
    // Just for sure
    vTaskDelete(nullptr);
    return 0;
}

void __lua_run(void* p) {
    char* path = static_cast<char*>(p);
    printf("Executing %s\n", path);
    {
        Process* process = new Process;
        process->task = xTaskGetCurrentTaskHandle();
        lua_State* L = process->lua.lua_state();

        processes[L] = process;
        process->lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::string, sol::lib::math,
                                    sol::lib::table, sol::lib::io);
        LuaLib::open(process->lua);
        lua_atpanic(L, _at_panic);
        int error = luaL_loadfile(L, path);
        delete[] path;
        if (error) // if non-0, then an error
        {
            // the top of the stack should be the error string
            if (lua_isstring(L, lua_gettop(L))) {


                // get the top of the stack as the error and pop it off
                std::string str = lua_tostring(L, lua_gettop(L));
                lua_pop(L, 1);
                CCOSCore::displayWindow(new MessageDialog("Lua error", str));
            }
        } else {
            // if not an error, then the top of the stack will be the function to call to run the file
            if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
                printf("Lua Runtime Error: %s\n", lua_tostring(L, -1));
                CCOSCore::displayWindow(new MessageDialog("Runtime error", lua_tostring(L, -1)));
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        bool r = true;
        while (r) {
            r = false;
            for (Window* w : gWindows) {
                //printf("%p %p\n", L, w->mLuaState);
                if (L == w->mLuaState) {
                    r = true;
                    break;
                }
            }
            {
                Mutex::L lock(process->m);
                while (!process->rpt.empty()) {
                    process->rpt.front()();
                    process->rpt.pop_front();
                }
                if (esp_get_free_heap_size() / 1024 < 20)
                    lua_gc(L, LUA_GCCOLLECT, 0);
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        processes[L] = nullptr;
        process->alive.clear();
        delete process;
    }
    vTaskDelete(NULL);
}

void CCOSCore::execute(const File& file) {
    char* path = new char[file.convert().length() + 1];
    strcpy(path, file.convert().c_str());
    xTaskCreate(__lua_run, "User", 4 * 2048, path, 3, 0);
}

void CCOSCore::killMePlz(std::string m, lua_State* L) {
    printf("Kill Lua VM %p, %s\n", L, m.c_str());
    if (!L && gCurrentWindow) {
        L = gCurrentWindow->mLuaState;
    }

    if (L) {
        if (TaskHandle_t t = processes[L]->task) {
            int jd = 0;
            char buf[256];
            sprintf(buf, "%s has stopped", gCurrentWindow ? gCurrentWindow->getTitle().c_str() : "Application");
            printf("%d\n", jd++); // 0
            if (!m.empty())
                displayWindow(new MessageDialog(buf, m));
            if (t != xTaskGetCurrentTaskHandle())
                uiThread.lock();

            printf("%d\n", jd++); // 1
            for (std::vector<Window*>::iterator i = gWindows.begin(); i != gWindows.end();) {
                if ((*i)->mLuaState == L) {
                    i = gWindows.erase(i);
                    continue;
                }
                ++i;
            }

            printf("%d\n", jd++); // 2
            delete processes[L];
            printf("%d\n", jd++); // 3
            processes[L] = nullptr;
            printf("%d\n", jd++); // 4
            if (t != xTaskGetCurrentTaskHandle())
                uiThread.unlock();
            printf("%d\n", jd++); // 5
            vTaskDelete(t);
            return;
        }
    }

    displayWindow(new MessageDialog("Error", m));
    vTaskDelete(NULL);
}

void CCOSCore::screenshot() {
    // Animation
    ssd1306_set_whole_display_lighting(&display, true);

    OwnBitmap bmp(128, 64);

    fb.snapshot(128, 64, const_cast<uint8_t*>(bmp.data));

    BMPBitmap bit(bmp);
    Dir s("screenshots");
    if (!s.isDir())
        s.mkdir(0777);
    char name[128];
    std::time_t t = std::time(nullptr);
    std::tm tm;
    tm = *localtime(&t);
    std::strftime(name, sizeof(name), "screenshots/scr-%F_%H.%M.%S.bmp", &tm);
    File f(name);
    bit.save(f);
    vibrate(10);
    ssd1306_set_whole_display_lighting(&display, false);
}

/**!
 * Reboot
 * @param type 0 - normal, 1 - to recovery
 */
void CCOSCore::reboot(int type) {
    switch (type) {
        case 0: {
            break;
        }
        case 1: {
            esp_partition_t part = {
                    ESP_PARTITION_TYPE_APP,
                    ESP_PARTITION_SUBTYPE_APP_FACTORY,
                    0x10000,
                    0xaffff,
                    {'f', 'a', 'c', 't', 'o', 'r', 'y'},
                    false
            };
            esp_ota_set_boot_partition(&part);
            break;
        }
    }
    esp_restart();
}

/**
 * Use <src>Indicator::setIndicatorValue</src> instead
 * @param r red
 * @param g green
 * @param b blue
 */
void CCOSCore::setIndicatorValue(float r, float g, float b) {
    if (FirmwareConfig::read().mMajorModel < 4) {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, static_cast<uint32_t>(r * 8192));
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    } else {
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, static_cast<uint32_t>((1.f - b) * 8192));
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, static_cast<uint32_t>((1.f - r) * 8192));
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, static_cast<uint32_t>((1.f - g) * 8192));
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
    }
}

int system(const char* d) {
    return 0;
}

extern "C" {
void _init(void) {
    //xTaskCreate(_init_call, "MainTask", 3 * 2048, NULL, 2, NULL);
    CCOSCore::init();
}
}
