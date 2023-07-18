#include "Settings.h"
#include "WindowList.h"
#include "SpinBox.h"
#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <algorithm>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "spiffs_vfs.h"
}

#include "TextView.h"
#include "Mutex.h"
#include "SpinnerView.h"
#include "wifi.png.h"
#include "lock.png.h"
#include "TextArea.h"
#include "Wifi.h"
#include "ListItem.h"
#include "config.h"
#include  "wifi.png.h"
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_wifi_types.h>
#include <driver/adc.h>
#include "lua.hpp"
#include "sol.hpp"
#include "ProgressBar.h"
#include "Utils.h"
#include "Build.h"
#include "Socket.h"
#include "TaskHelper.h"
#include "Http.h"
#include "MessageDialog.h"
#include "FirmwareConfig.h"
#include "D3Window.h"
#include "KeytestWindow.h"
#include "PItestWindow.h"
#include "WindowConnections.h"
#include "Indicator.h"
#include "Streaming.h"


struct wifi_info {
    std::string name;
    int8_t signal;
    wifi_auth_mode_t authmode;
};


Mutex _wifi_mutex;


std::vector<wifi_info> wifi_data;

//static void scan_done_cb(void *arg, sdk_scan_status_t status);

class SettingsWifi;


class SettingsWifiConnect : public Window {
private:
    wifi_info &wifi;
    TextArea *pwd = nullptr;
public:
    SettingsWifiConnect(wifi_info &wi) : Window(wi.name), wifi(wi) {
        if (wifi.authmode != WIFI_AUTH_OPEN) {
            addView(new TextView("Password", 0, 10));
            addView(pwd = new TextArea());
            pwd->x = 1;
            pwd->y = 23;
            pwd->width = 126;
        }

        char buf[32];
        char val[16];

        switch (wifi.authmode) {
            case WIFI_AUTH_OPEN:
                sprintf(val, "None");
                break;
            case WIFI_AUTH_WEP:
                sprintf(val, "WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                sprintf(val, "WPA (PSK)");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
            case WIFI_AUTH_WPA2_PSK:
                sprintf(val, "WPA2 (PSK)");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                sprintf(val, "WPA/WPA2 (PSK)");
                break;
            default:
                sprintf(val, "Unknown");
                break;
        }
        sprintf(buf, "Auth: %s", val);
        addView(new TextView(buf, 0, 52));
    }

    virtual void render(Framebuffer &framebuffer) override {
        Window::render(framebuffer);
        if (pwd)
            pwd->focus();
    }

    virtual void keyRelease(uint8_t key) override {
        Window::keyRelease(key);
        switch (key) {
            case 15: {
                NVS s("ccos_wifi");
                auto a = s.open();
                a.writeString("ssid", wifi.name);
                std::string pwd_s = pwd ? pwd->getText() : "";
                a.writeString("pwd", pwd_s);
            }
                Wifi::connect(wifi.name, pwd ? pwd->getText() : "");
                CCOSCore::removeWindow(this);
                break;
        }
    }
};



class WifiItem : public View {
private:
    wifi_info wifi;
    uint8_t signal;
public:
    WifiItem(int16_t _x, int16_t _y, wifi_info &wf) :
            wifi(wf) {
        x = _x;
        y = _y;
        width = 128;
        height = 15;
        signal = static_cast<uint8_t>(pow(double(wifi.signal - 120) / 60.0, 2) * 4);
    }

    virtual void render(Framebuffer &fb) {
        View::render(fb);

        bool col = Wifi::getWifiSsid() == wifi.name;

        height = col ? 26 : 15;

        char *cur = &(wifi.name[0]);
        if (isFocused()) {
            fb.drawRect(11, 0, width - 9, height, OLED_COLOR_WHITE);
        }
        for (uint8_t i = 0; i <= uint8_t(ceilf(float(50.f - signal) / 5.f)) && i < 4; i++) {
            Bitmap b(wifis[i], 9, 7);
            fb.drawImage(0, 3, b, OLED_COLOR_WHITE);
        }
        if (wifi.authmode != WIFI_AUTH_OPEN) {
            static Bitmap lck(lock, 6, 7);
            fb.drawImageWithBorder(7, 7, lck);
        }
        for (int16_t i = 0; *cur; i++)
            cur = fb.drawStringML(10, i * 12 + 1, cur, width, FONT_FACE_TERMINUS_6X12_ISO8859_1,
                                  isFocused() ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);

        if (col) {
            char buf[32];

            switch (Wifi::getStatus()) {
                case Wifi::CONNECTING:
                    sprintf(buf, "Connecting");
                    break;
                case Wifi::CONNECTED:
                    strcpy(buf, "Receiving ip...");
                    break;
                case Wifi::CONNECTED_GOT_IP: {
                    Wifi::ip_address ip = Wifi::getIpAddress();
                    sprintf(buf, "%u.%u.%u.%u", ip.b1, ip.b2, ip.b3, ip.b4);
                    break;
                }
                default:
                    sprintf(buf, "???");
            }
            fb.drawString(16, 14, buf, isFocused() ? OLED_COLOR_BLACK : OLED_COLOR_WHITE, FONT_FACE_BITOCRA_6X11);
        }
    }

    virtual void onClick() {
        View::onClick();
        if (Wifi::getStatus() == Wifi::CONNECTED_GOT_IP && Wifi::getWifiSsid() == wifi.name) {
            Wifi::disconnect();
        } else {
            CCOSCore::displayWindow(new SettingsWifiConnect(wifi));
        }
    }
};

SettingsWifi *_wifi_inst = nullptr;

void scan_callback(const std::vector<wifi_ap_record_t> &rc);

class SettingsWifi : public WindowConnections {
private:
    int8_t st = 2;
    bool scanDone = true;
public:
    SpinnerView spinner;

    SettingsWifi() :
            WindowConnections("WiFi"),
            spinner(48, 30) {
        if (isEnabled()) {
            scan();
        }
        _wifi_inst = this;
    }

    bool isEnabled() override {
        return Wifi::isEnabled();
    }

    void scan() {
        Wifi::scan(scan_callback);
    }

    void setEnabled(bool v) override {
        if (v) {
            Wifi::enable();
            scan();
        } else {
            Wifi::disable();
        }
    }

    void render(Framebuffer &fb) override {
        WindowConnections::render(fb);
        if (isEnabled()) {
            Mutex::L lock(_wifi_mutex);
            switch (st) {
                case 0: {
                    const char *s = "Scan failed.";
                    fb.drawString(64 - strlen(s) * 3, 30, s);
                    break;
                }
                case 1: {
                    if (views.empty()) {
                        const char *s = "No WiFi AP found.";
                        fb.drawString(64 - strlen(s) * 3, 30, s);
                        spinner.x = 51 - strlen(s) * 3;
                        spinner.render(fb);
                    }
                    break;
                }
                case 2:
                    spinner.x = 59;
                    spinner.render(fb);
                    break;
            }
            fb.setCoordsOffset(0, 0);
        }
    }

    ~SettingsWifi() {
        Mutex::L lock(_wifi_mutex);
        _wifi_inst = nullptr;
        if (!scanDone) {
            esp_wifi_scan_stop();
        }
    }

    void status(int8_t s) {
        st = s;

        if (st == 1) {
            views.clear();
            std::sort(wifi_data.begin(), wifi_data.end(), [](const wifi_info &l1, const wifi_info &l2) {
                return l1.signal > l2.signal;
            });
            for (size_t i = 0; i < wifi_data.size(); i++) {
                addView(new WifiItem(0, 0, wifi_data[i]));
            }
            wifi_data.clear();

            NVS x("ccos_wifi");
            auto a = x.open();
            std::string ssid, pwd;
            if (a.readString("ssid", ssid) && a.readString("pwd", pwd)) {
                Wifi::connect(ssid, pwd);
            }
        }
    }
};

void scan_callback(const std::vector<wifi_ap_record_t> &rc) {
    Mutex::L lock(_wifi_mutex);
    if (_wifi_inst) {
        for (size_t i = 0; i < rc.size(); i++) {
            const wifi_ap_record_t &r = rc[i];
            wifi_info t;
            t.name = reinterpret_cast<const char *>(r.ssid);
            t.authmode = r.authmode;
            t.signal = r.rssi;
            wifi_data.push_back(t);
        }
        _wifi_inst->status(1);
    }
}

/*
static void scan_done_cb(void *arg, sdk_scan_status_t status) {
    Mutex::L lock(_wifi_mutex);
    if (status != SCAN_OK) {

        //taskENTER_CRITICAL();
        {
           if (_stng) {
               _stng->status(0);
           }
        }
        //taskEXIT_CRITICAL();

        return;
    }

    struct sdk_bss_info *bss = (struct sdk_bss_info *) arg;
    bss = bss->next.stqe_next;
    // first one is invalid
    {
        wifi_data.clear();
        while (NULL != bss) {
            wifi_info wi;
            wi.name = (char *) bss->ssid;
            wi.signal = bss->rssi;
            wi.authmode = bss->authmode;
            wifi_data.push_back(wi);
            bss = bss->next.stqe_next;
        }
    }

    //taskENTER_CRITICAL();
    {
        if (_stng) {
            _stng->status(1);
        }
    }
    //taskEXIT_CRITICAL();

}*/
class SettingsDaT : public Window {
public:
    SpinBox<uint8_t> *hour;
    SpinBox<uint8_t> *min;
    SpinBox<uint8_t> *day;
    SpinBox<uint8_t> *month;
    SpinBox<unsigned short> *year;

    SettingsDaT() :
            Window("Clock") {
        time_t rtime = time(0);
        tm *time = localtime(&rtime);
        addView(year = new SpinBox<unsigned short>(0, 13, 1970, 2038, time->tm_year + 1900));
        addView(day = new SpinBox<uint8_t>(35, 13, 1, 31, time->tm_mday));
        addView(month = new SpinBox<uint8_t>(56, 13, 1, 12, time->tm_mon + 1));
        addView(hour = new SpinBox<uint8_t>(86, 13, 0, 23, time->tm_hour));
        addView(min = new SpinBox<uint8_t>(110, 13, 0, 59, time->tm_min));
    }

    void keyDown(uint8_t key) override {
        Window::keyDown(key);
        if (key == 6) {
            focusNext();
        }
    }

    virtual void render(Framebuffer &fb) {
        Window::render(fb);
        //fb.mems(85);
        fb.drawString(103, 24, ":");
        fb.drawString(1, 50, "YYYY");
        fb.drawString(37, 50, "DD");
        fb.drawString(57, 50, "MM");
        fb.drawString(87, 50, "hh");
        fb.drawString(103, 50, ":");
        fb.drawString(112, 50, "mm");
    }

    virtual ~SettingsDaT() {
        tm time;
        memset(&time, 0, sizeof(tm));
        time.tm_year = year->getValue() - 1900;
        time.tm_mday = day->getValue();
        time.tm_mon = month->getValue() - 1;
        time.tm_hour = hour->getValue();
        time.tm_min = min->getValue();
        CCOSCore::setTime(mktime(&time));

        //CCOSCore::setTime(mktime(&time));
    }
};
#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif
class SettingsAboutWindow : public WindowList {
private:
    ListItem* date;
    ListItem* cpu;
    ListItem* mem;
    ListItem* uptime;
    ListItem* bat;
    ListItem* bat2;
    ListItem* hall;
    ListItem* adc;
    ListItem* temp;
public:
    SettingsAboutWindow() : WindowList("About") {
        addView(date = new ListItem("Date"));
        addView(uptime = new ListItem("Uptime"));
        uint8_t mac[6];
        esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
        char buf[32];
        sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        addView(new ListItem("Mac", buf));
        addView(cpu = new ListItem("CPU frequency"));
        addView(temp = new ListItem("CPU temperature"));
        addView(mem = new ListItem("Available memory"));
        addView(bat = new ListItem("Battery level"));
        addView(bat2 = new ListItem("Raw battery level"));
        addView(hall = new ListItem("Hall sensor"));
        addView(adc = new ListItem("ADC"));
        addView(new ListItem("Processor mode", "real"));
        addView(new ListItem("CCOS version", CCOS_VERSION));
        sprintf(buf, "%u", Build::getBuildNumber());
        addView(new ListItem("CCOS build", buf));
        FirmwareConfig::cfg c = FirmwareConfig::read();
        addView(new ListItem("Vendor", c.mVendor));
        addView(new ListItem("Model", c.mName));
        addView(new ListItem("Model ID", c.mModelId));
        addView(new ListItem("SDK version", system_get_sdk_version()));
        addView(new ListItem("IDF version", esp_get_idf_version()));


        {
            sol::state state;
            state.open_libraries(sol::lib::base);
            addView(new ListItem("Lua version", state["_VERSION"]));
        }

#ifdef KEY_EMU
        addView(new ListItem("keyboard", "Emulated"));
#else
        addView(new ListItem("keyboard", "Hardware"));
#endif

    }

    void render(Framebuffer &fb) override {
        char buf[32];
        {
            time_t rtime = time(0);
            struct tm *time = localtime(&rtime);
            strftime(buf, sizeof(buf), "%H:%M:%S\n%B %e\n%A, %y", time);
            date->setText(std::string(buf));
        }
        {
            sprintf(buf, "%u", ets_get_cpu_frequency());
            cpu->setText(std::string(buf));
        }
        {
            sprintf(buf, "%.1f oC", (temprature_sens_read() - 32) / 1.8f);
            temp->setText(buf);
        }
        {
            mem->setText(std::string(Utils::sizeFormat(esp_get_free_heap_size())));
        }
        {
            sprintf(buf, "%u%%", CCOSCore::getBatteryLevel());
            bat->setText(std::string(buf));
        }
        {
            uint32_t hours = CCOSCore::getUptime() / 3600;
            uint32_t mins = (CCOSCore::getUptime() / 60) % 60;
            uint32_t secs = CCOSCore::getUptime() % 60;
            sprintf(buf, "%02u:%02u:%02u", hours, mins, secs);
            uptime->setText(std::string(buf));
        }
        {
            sprintf(buf, "%d", adc1_get_raw(ADC1_CHANNEL_5));
            bat2->setText(std::string(buf));
        }
        {
            sprintf(buf, "%d", hall_sensor_read());
            hall->setText(std::string(buf));
        }
        {
            sprintf(buf, "%d", adc1_get_raw(ADC1_CHANNEL_6));
            adc->setText(std::string(buf));
        }
        WindowList::render(fb);
    }

    virtual void keyLongDown(uint8_t index) {
        WindowList::keyLongDown(index);
        switch (index) {
            case 15:
                CCOSCore::displayWindow(new D3Window);
                break;
            case 11:
                CCOSCore::displayWindow(new KeytestWindow);
                break;
            case 14:
                CCOSCore::displayWindow(new PItestWindow);
                break;

        }
    }
};

class SettingsStorage : public WindowList {
public:
    SettingsStorage() : WindowList("Storage") {
        uint32_t tot, used;
        spiffs_fs_stat(&tot, &used);

        ProgressBar *pb = new ProgressBar(0, 2, 127);
        pb->max = tot;
        pb->current = used;
        addView(pb);

        addView(new ListItem("Used space", Utils::sizeFormat(used)));
        addView(new ListItem("Available space", Utils::sizeFormat(tot - used)));
        addView(new ListItem("Total space", Utils::sizeFormat(tot)));

    }

public:

};

class SettingsFirmwareUpgrade : public Window {
private:
    TextView *mStatus;
    SpinnerView *mSpinner;
    TaskHelper *mChecker = nullptr;

    void setStatus(const std::string &s) {
        CCOSCore::runOnUiThread([&, s]() {
            mStatus->setText(s);
            mSpinner->setVisibility(GONE);
        });
    }

public:


    SettingsFirmwareUpgrade() : Window("Firmware upgrade") {
        addView(mStatus = new TextView("Checking for updates...", 0, 12));
        addView(mSpinner = new SpinnerView(117, 54));

        NVS x("ccos_wifi");
        auto a = x.open();
        std::string ssid, pwd;
        if (a.readString("ssid", ssid) && a.readString("pwd", pwd)) {
            // Wi-Fi data exists
            if (Wifi::isConnected()) {
                // Everything is ok, check for updates
                mChecker = new TaskHelper([&]() {
                    http::Document d = http::get("http://alex2772.ru/ccalc/build.txt");
                    int number;
                    if (!d.mContent.empty() && sscanf(d.mContent.c_str(), "%d", &number) != EOF) {
                        if (number > Build::getBuildNumber()) {
                            setStatus("Update available");
                            CCOSCore::runOnUiThread([&]() {
                                addView(new TextView("Long press enter", 0, 24));
                            });
                        } else {
                            setStatus("Up to date");
                        }
                    } else {
                        setStatus("Failed");
                    }
                });
            }
        } else {
            setStatus("Connect to Wi-Fi first");
        }
    }

    virtual void keyLongDown(uint8_t key) {
        switch (key) {
            case 15:
                CCOSCore::displayWindow(new MessageDialog("Perform upgrade?", "Wi-Fi connection has to be available.", []() {
                    CCOSCore::reboot(1);
                }));
                break;
            default:
                break;
        }
    }

    ~SettingsFirmwareUpgrade() {
        delete mChecker;
    }
};

class SettingsDevice: public Window {
private:
    Indicator mIndicator;
    SpinBox<uint8_t>* mBrightness;
    SpinBox<uint8_t>* mStrength;
public:
    SettingsDevice() : Window("Device") {
        uint8_t brightness = 64;
        CCOSCore::getNVSHandle().open().read("indicator", brightness);
        addView(mBrightness = new SpinBox<uint8_t>(10, 15, 0, 255, brightness));
        mBrightness->width = 30;

        uint8_t strength = 255;
        CCOSCore::getNVSHandle().open().read("vibrator", strength);
        addView(mStrength = new SpinBox<uint8_t>(74, 15, 0, 255, strength));
        mStrength->width = 30;


        mIndicator.set(1, 1, 1);
    }
    ~SettingsDevice() {
        uint8_t brightness = mBrightness->getValue();
        CCOSCore::getNVSHandle().open().write("indicator", brightness);
    }
    virtual void render(Framebuffer& framebuffer) {
        Window::render(framebuffer);
        framebuffer.drawString(0, 50, "Indicator");
        mIndicator.setBrightness(float(mBrightness->getValue()) / 255.f);
        mIndicator.set(1, 1, 1);
        framebuffer.drawString(70, 50, "Vibrator");

        {
            uint8_t strength = mStrength->getValue();
            static uint8_t prevStrength = strength;
            if (strength != prevStrength) {
                prevStrength = strength;
                CCOSCore::getNVSHandle().open().write("vibrator", strength);
            }
        }
    }

    virtual void keyDown(uint8_t key) override {
        Window::keyDown(key);
        if (key == 6) {
            focusNext();
        }
    }
};


class StreamingWindow: public Window {
private:
    static time_t time;

    TextArea* mIp;

    void save() {
        CCOSCore::getNVSHandle().open().writeString("streaming_url", mIp->getText());
        Streaming::instance.mAddress = mIp->getText();
    }
public:
    StreamingWindow() : Window("Streaming") {
        addView(mIp = new TextArea());
        std::string addr = Streaming::instance.mAddress;
        if (addr.empty()) {
            CCOSCore::getNVSHandle().open().readString("streaming_url", addr);
        }
        mIp->setText(addr);
        mIp->x = 2;
        mIp->y = 26;
        mIp->width = 124;
    }

    virtual void render(Framebuffer& framebuffer) {
        Window::render(framebuffer);
        framebuffer.drawString(2, 12, "Server IP");

        std::string status = Streaming::instance.mStatus;
        if (!Wifi::isConnected()) {
            status = "Network unavailable";
        }

        if (Streaming::instance.mSocket) {
            char buf[64];
            time_t curTime = std::time(nullptr) - time;
            tm* tm = localtime(&curTime);
            strftime(buf, sizeof(buf), "Streaming %H:%M:%S", tm);
            status = buf;
        }
        framebuffer.drawString(2, 64 - 12, status);
    }

    virtual void keyDown(uint8_t key) {
        Window::keyDown(key);
        if (key == 15) {
            if (Streaming::instance.mSocket) {
                Streaming::instance.stop("Streaming disabled");
            } else {
                if (mIp->getText().empty()) {
                    CCOSCore::showToast("IP is empty");
                } else if (Wifi::getStatus() != Wifi::CONNECTED_GOT_IP) {
                    CCOSCore::showToast("Network unavailable");
                } else {
                    time = std::time(nullptr);
                    save();
                    Streaming::instance.mSocket = new UDPSocket(4530);
                    Streaming::instance.mStatus = std::string();
                    Streaming::instance.mStreamDst = {0};
                }
            }
        }
    }


};
time_t StreamingWindow::time = 0;

class SettingsWindow : public WindowList {
public:
    SettingsWindow()
            : WindowList("Settings") {
        addView(new TextView("Wi-Fi", 0, 0));
        addView(new TextView("Storage", 0, 0));
        addView(new TextView("Device", 0, 0));
        addView(new TextView("Date and time", 0, 0));
        addView(new TextView("Streaming", 0, 0));
        addView(new TextView("Firmware upgrade", 0, 0));
        addView(new TextView("About system", 0, 0));
    }

    virtual void itemSelected(size_t index) {
        switch (index) {
            case 0:
                CCOSCore::displayWindow(new SettingsWifi);
                break;
            case 1:
                CCOSCore::displayWindow(new SettingsStorage);
                break;
            case 2:
                CCOSCore::displayWindow(new SettingsDevice);
                break;
            case 3:
                CCOSCore::displayWindow(new SettingsDaT);
                break;
            case 4:
                CCOSCore::displayWindow(new StreamingWindow);
                break;
            case 5:
                CCOSCore::displayWindow(new SettingsFirmwareUpgrade);
                break;
            case 6:
                CCOSCore::displayWindow(new SettingsAboutWindow);
                break;
        }
    }
};

void Settings::launch() {
    CCOSCore::displayWindow(new SettingsWindow);
}
