//
// Created by alex2772 on 18.11.18.
//

#include <map>
#include <esp_log.h>
#include "VKApplication.h"
#include "vk.png.h"
#include "CCOSCore.h"
#include "TextArea.h"
#include "Http.h"
#include "json.h"
#include "TaskHelper.h"
#include "SpinnerView.h"
#include "MessageDialog.h"
#include "Wifi.h"
#include "TextView.h"
#include "ContextMenu.h"
#include <cstdio>
#include <sstream>

Bitmap vk_icon = {vk, 30, 30};

#define VK_API_VERSION "5.92"

// Windows Phone
//#define VK_CLIENT_ID "3697615"
//#define VK_CLIENT_SECRET "AlVXZFMUqyrnABp8ncuU"
#define VK_CLIENT_ID "3140623"
#define VK_CLIENT_SECRET "VeWdmVclDCtn6ihuP1nt"
#define VK_NVS "VK"

namespace VK {
    json request(const std::string &method, std::map<std::string, std::string> args,
                 const std::string &domain = "api.vk.com/method/") {
        args["client_id"] = VK_CLIENT_ID;
        args["client_secret"] = VK_CLIENT_SECRET;
        args["v"] = VK_API_VERSION;
        http::Document d = http::get(std::string("https://") + domain + method, args);
        printf("[VK] %s\n", d.mContent.c_str());
        return json::parse(d.mContent);
    }

    void login(int userId, const std::string &token);

    void logout();

    void logout() {
        NVS nvs(VK_NVS);
        NVS::Accessor a = nvs.open();
        a.erase("access_token");
        a.erase("user_id");
    }


}


class VKConversationItem : public View {
private:
    std::string mLastMessage;
public:
    int mId;
    std::string mTitle;

    VKConversationItem(int id, const std::string &title, const std::string &lastMessage) :

            mLastMessage(lastMessage),
            mId(id),
            mTitle(title) {
        height = 20;
    }

    virtual void render(Framebuffer &fb) {
        View::render(fb);
        if (isFocused()) {
            fb.drawRect(0, 0, width, height, OLED_COLOR_WHITE);
        }

        int16_t offset = 1;

        char *cur = &(mTitle[0]);
        for (int16_t i = 0; *cur; i++) {
            cur = fb.drawStringML(0, offset, cur, width - 4, FONT_FACE_BITOCRA_7X13,
                                  isFocused() ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);
            offset += 13;
        }
        cur = &(mLastMessage[0]);
        for (int16_t i = 0; *cur; i++) {
            cur = fb.drawStringML(0, offset, cur, width - 4, FONT_FACE_TERMINUS_6X12_ISO8859_1,
                                  isFocused() ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);
            offset += 12;
        }
        height = offset + 2;
    }
};

class VKMain;

class VKDialog;

class VKUser {
private:
    int mId;
    std::string mFirstName;
    std::string mLastName;
public:
    VKUser() :
            VKUser(-1, "", "") {

    }

    VKUser(int id, const std::string &firstName, const std::string &lastName) : mId(id), mFirstName(firstName),
                                                                                mLastName(lastName) {}

    std::string getFullName() const {
        return mFirstName + " " + mLastName;
    }

    int getId() const {
        return mId;
    }

    const std::string &getFirstName() const {
        return mFirstName;
    }

    const std::string &getLastName() const {
        return mLastName;
    }
};

class VKMessage {
private:
    int mIdFrom;
    std::string text;
public:
    VKMessage(int mIdFrom, const std::string &text) : mIdFrom(mIdFrom), text(text) {}

    int getIdFrom() const {
        return mIdFrom;
    }

    std::string &getText() {
        return text;
    }
};

class VKDialogMessages : public View {
private:
    VKDialog *mVkDialog;
    std::deque<VKMessage> mMessages;

    int scroll = 0;
    float smoothScroll = 0;
public:
    VKDialogMessages(VKDialog *mVkDialog) : mVkDialog(mVkDialog) {
        isInput = true;
    }

    virtual void render(Framebuffer &fb);

    void push_front(VKMessage m) {
        mMessages.push_front(m);
    }

    void push_back(VKMessage m) {
        mMessages.push_back(m);
    }

    void clear();
};

class VKDialog : public Window {
public:
    VKMain *mVkMain;
    int mId;
    VKDialogMessages *mVkMessages;
private:
    SpinnerView *mSpinner = new SpinnerView(64 - 8, 32 - 8);
    TextArea *mText = new TextArea();

    void update();

public:
    VKDialog(VKMain *m, std::shared_ptr<VKConversationItem> p);

    ~VKDialog();

    virtual void render(Framebuffer &framebuffer);

    virtual void keyLongDown(uint8_t key);

};

class VKMain : public WindowList {
public:
    int mUserId;
    TaskHelper* mLongPoll = nullptr;

    VKDialog* mVkDialog = nullptr;
private:
    std::string mToken;
    SpinnerView *mSpinner = new SpinnerView(64 - 8, 32 - 8);
    TaskHelper *mHelper = nullptr;
    std::map<int, VKUser> mKeptUsers;
public:

    VKMain(int mUserId, const std::string &mToken) : WindowList("VK"), mUserId(mUserId),
                                                     mToken(mToken) {
        addView(mSpinner);

        request("messages.getConversations", {
                {"count",    "10"},
                {"extended", "1"}
        }, [&](json &j) {
            printf("Lambda\n");
            auto items = j.getObjectItem("items").asArray();
            for (json &item : items) {
                std::string title;
                json conv = item.getObjectItem("conversation");
                int id = conv.getObjectItem("peer").getObjectItem("id").asInt();;
                if (conv.hasObjectItem("chat_settings")) {
                    title = conv.getObjectItem("chat_settings").getObjectItem("title").asString();
                } else if (isUserKnown(id)) {
                    title = getUserById(id).getFullName();
                } else {
                    std::stringstream ss;
                    ss << "@id" << id;
                    title = ss.str();
                }
                std::string last = item.getObjectItem("last_message").getObjectItem("text").asString();
                printf("%s, %s\n", title.c_str(), last.c_str());
                CCOSCore::runOnUiThread([&, id, title, last]() {
                    addView(new VKConversationItem(id, title, last));
                });
            }
        });

        /*
        mLongPoll = new TaskHelper([&]() {
            json res;
            if (request("messages.getLongPollServer", {
                    {"lp_version", "3"},
                    {"need_ptr",   "0"}
            }, res)) {
                res.print();

                std::string address = res.getObjectItem("server").asString();
                std::string key = res.getObjectItem("key").asString();
                int ts = res.getObjectItem("ts").asInt();

                printf("[VK] Got Long poll server: %s; key = %s; ts = %d\n", address.c_str(), key.c_str(), ts);
                while (mLongPoll) {
                    std::stringstream fts;
                    fts << ts;
                    json s = VK::request("", {
                            {"act",     "a_check"},
                            {"key",     key},
                            {"ts",      fts.str()},
                            {"wait",    "25"},
                            {"mode",    "2"},
                            {"version", "3"}
                    }, address);
                    ts = s.getObjectItem("ts").asInt();

                    for (json& item : s.getObjectItem("updates").asArray()) {
                        std::vector<json> cur = item.asArray();
                        int id = cur[0].asInt();
                        switch (id) {
                            case 4: // MESSAGE
                            {
                                int peerId = cur[3].asInt();
                                if (mVkDialog->mId == peerId) {
                                    int from = peerId;
                                    if (cur[6].hasObjectItem("from")) {
                                        std::string cs = cur[6].getObjectItem("from").asString();
                                        sscanf(cs.c_str(), "%d", &peerId);
                                    }
                                    mVkDialog->mVkMessages->push_back({from, cur[5].asString()});
                                }
                                break;
                            }
                        }
                    }
                }

            }
        });*/
    }

    virtual void itemSelected(size_t index) {
        WindowList::itemSelected(index);
        CCOSCore::displayWindow(new VKDialog(this, std::dynamic_pointer_cast<VKConversationItem>(views[index])));
    }

    bool request(const std::string &method, std::map<std::string, std::string> args, json &response) {
        args["access_token"] = mToken;
        json j = VK::request(method, args);
        if (j.hasObjectItem("response")) {
            json resp = j.detachObjectItem("response");
            if (resp.hasObjectItem("profiles")) {
                keepUsers(resp.getObjectItem("profiles"));
            }
            response = resp;
            return true;
        } else if (j.hasObjectItem("error")) {
            printf("Have error object\n");
            CCOSCore::displayWindow(
                    new MessageDialog("Error", j.getObjectItem("error").getObjectItem("error_msg").asString()));
        } else {
            printf("Unknown error\n");
        }
        return false;
    }

    void
    request(const std::string &method, std::map<std::string, std::string> args, std::function<void(json &)> response) {
        run([&, method, args, response]() {
            json res;
            if (request(method, args, res)) {
                response(res);
            }
        });

    }

    void run(const std::function<void()> &func) {
        delete mHelper;
        mHelper = new TaskHelper(func);
    }

    virtual ~VKMain() {
        delete mHelper;
        delete mLongPoll;
    }

    virtual void keyLongDown(uint8_t key) {
        Window::keyLongDown(key);
        switch (key) {
            case 7: {
                CCOSCore::displayWindow(new ContextMenu("Menu", {
                        {"Logout", [&]() {
                            VK::logout();
                            close();
                        }}
                }));
                break;
            }
        }
    }

    void keepUsers(json j) {
        for (json &item : j.asArray()) {
            keepUser({item.getObjectItem("id").asInt(), item.getObjectItem("first_name").asString(),
                      item.getObjectItem("last_name").asString()});
        }
    }

    void keepUser(VKUser user) {
        mKeptUsers[user.getId()] = user;
    }

    const VKUser &getUserById(int id) {
        return mKeptUsers[id];
    }

    bool isUserKnown(int id) {
        return mKeptUsers.find(id) != mKeptUsers.end();
    }
};


void VKDialog::keyLongDown(uint8_t key) {
    Window::keyLongDown(key);
    switch (key) {
        case 7: {
            update();
            break;
        }
        case 15: {
            std::string text = mText->getText();
            std::stringstream ss;
            ss << esp_random();
            std::stringstream peer;
            peer << mId;

            mVkMain->request("messages.send", {
                    {"random_id", ss.str()},
                    {"peer_id",   peer.str()},
                    {"message",   text}
            }, [](json &j) {

            });
            mVkMessages->push_back({mVkMain->mUserId, text});
            mText->setText("");
            //mText->focus();
            break;
        }
    }
}

void VKDialog::update() {
    std::stringstream sId;
    sId << mId;
    mVkMain->request("messages.getHistory", {
            {"count",    "30"},
            {"peer_id",  sId.str()},
            {"extended", "1"}
    }, [&](json &j) {
        CCOSCore::runOnUiThread([&]() {
            mVkMessages->clear();
        });
        std::vector<json> items = j.getObjectItem("items").asArray();
        for (json &item : items) {
            int idFrom = item.getObjectItem("from_id").asInt();
            std::string text = item.getObjectItem("text").asString();
            for (json &attachment : item.getObjectItem("attachments").asArray()) {
                text += "\n";
                text += attachment.getObjectItem("type").asString();
            }
            CCOSCore::runOnUiThread([&, idFrom, text]() {
                mVkMessages->push_front({idFrom, text});
            });
        }
        CCOSCore::runOnUiThread([&]() {
            mSpinner->setVisibility(Visibility::GONE);
        });
    });
}

void VKDialog::render(Framebuffer &framebuffer) {
    Window::render(framebuffer);
    mText->setVisibility(mText->isFocused() ? VISIBLE : INVISIBLE);
}

VKDialog::VKDialog(VKMain *m, std::shared_ptr<VKConversationItem> p) :
        Window(p->mTitle),
        mVkMain(m),
        mId(p->mId),
        mVkMessages(new VKDialogMessages(this)) {
    mText->x = 0;
    mText->y = 64 - 15;
    mText->width = 127;

    addView(mSpinner);
    addView(mVkMessages);
    addView(mText);
    mText->focus();
    update();
    mVkMain->mVkDialog = this;
}

VKDialog::~VKDialog() {
    mVkMain->mVkDialog = nullptr;
}


void VKDialogMessages::render(Framebuffer &fb) {
    View::render(fb);
    fb.drawRect(0, 64 - 17, 128, 17, OLED_COLOR_BLACK);
    if (isFocused()) {
        scroll += CCOSCore::getKeyState(1);
        scroll -= CCOSCore::getKeyState(9);
    }
    smoothScroll += (scroll - smoothScroll) * 0.2f;

    int offset = static_cast<int>(smoothScroll);

    for (VKMessage &m : mMessages) {
        if (offset < 64) {
            int msgHeight = 0;
            std::string title;
            if (mVkDialog->mVkMain->isUserKnown(m.getIdFrom())) {
                title = mVkDialog->mVkMain->getUserById(m.getIdFrom()).getFullName();
            } else {
                std::stringstream ss;
                ss << m.getIdFrom();
                title = ss.str();
            }
            char *cur = &(title[0]);
            for (int16_t i = 0; *cur; i++) {
                cur = fb.drawStringML(0, offset + msgHeight, cur, width - 2, FONT_FACE_TERMINUS_6X12_ISO8859_1,
                                      OLED_COLOR_WHITE);
                msgHeight += 12;
            }
            cur = &(m.getText()[0]);
            for (int16_t i = 0; *cur; i++) {
                cur = fb.drawStringML(2, offset + msgHeight, cur, width - 4, FONT_FACE_TERMINUS_6X12_ISO8859_1,
                                      OLED_COLOR_WHITE);
                msgHeight += 11;
            }

            fb.drawLine(0, offset + msgHeight + 2, 127, offset + msgHeight + 2, OLED_COLOR_WHITE);
            offset += msgHeight + 4;
        }
    }
    fb.drawRect(0, 0, 128, 13, OLED_COLOR_BLACK);
}

void VKDialogMessages::clear() {
    mMessages.clear();
}

void VK::login(int userId, const std::string &token) {
    NVS nvs(VK_NVS);
    NVS::Accessor a = nvs.open();
    a.erase("login");
    a.erase("password");
    a.write("user_id", userId);
    a.writeString("access_token", token.c_str());
    CCOSCore::displayWindow(new VKMain(userId, token));
}

class VKAuthWindow : public Window {
private:
    TextArea *mUsername = new TextArea();
    TextArea *mPassword = new TextArea();
    SpinnerView *mSpinner = new SpinnerView(32, 2);
    TaskHelper *mThread = nullptr;
    NVS nvs;

public:
    VKAuthWindow() : Window("Authorization"), nvs(VK_NVS) {
        mDrawTitle = false;
        mUsername->x = mPassword->x = 0;
        mUsername->width = mPassword->width = 127;
        mUsername->y = 12;
        mPassword->y = 12 + 16 + 12;

        mSpinner->setVisibility(GONE);

        addView(mUsername);
        addView(mPassword);
        addView(mSpinner);


        NVS::Accessor a = nvs.open();
        std::string v;
        if (a.readString("login", v)) {
            mUsername->setText(v);
        }
        if (a.readString("password", v)) {
            mPassword->setText(v);
        }
    }

    virtual ~VKAuthWindow() {
        delete mThread;
    }

    virtual void keyLongDown(uint8_t key) {
        Window::keyLongDown(key);
        if (mSpinner->getVisibility() != VISIBLE && key == 15) {
            auth();
        }
    }

    virtual void render(Framebuffer &framebuffer) {
        Window::render(framebuffer);

        framebuffer.drawString(0, 0, "Login");
        framebuffer.drawString(0, 12 + 16, "Password");
    }

    void auth(const std::string &code = "");
};

class VKAuth2FAWindow : public Window {
private:
    VKAuthWindow *mMain;
    TextArea *mInput = new TextArea();
public:
    VKAuth2FAWindow(VKAuthWindow *main, const std::string &phoneMask) : Window("Validation"), mMain(main) {
        mInput->x = 0;
        mInput->y = 13;
        mInput->width = 127;

        addView(mInput);
        if (phoneMask.empty()) {
            addView(new TextView(std::string("Please enter code from the app"), 0, 13 + 16));
        } else {
            addView(new TextView(std::string("Please enter code from the SMS sent to ") + phoneMask, 0, 13 + 16));
        }
    }

    virtual void keyLongDown(uint8_t key) {
        Window::keyLongDown(key);
        if (key == 15) {
            mMain->auth(mInput->getText());
            close();
        }
    }
};

void VKAuthWindow::auth(const std::string &code) {
    {
        NVS::Accessor a = nvs.open();
        a.writeString("login", mUsername->getText());
        a.writeString("password", mPassword->getText());
    }

    if (Wifi::isConnected()) {
        mSpinner->setVisibility(VISIBLE);
        delete mThread;
        mThread = new TaskHelper([&, code]() {
            std::map<std::string, std::string> v = {
                    {"grant_type",    "password"},
                    {"username",      mUsername->getText()},
                    {"password",      mPassword->getText()},
                    {"2fa_supported", "1"}
            };

            if (!code.empty()) {
                v["code"] = code;
            }
            json j = VK::request("token", v, "oauth.vk.com/");
            json error = j.getObjectItem("error");
            printf("error %p\n", error.get());
            if (j.get()) {
                if (error.isString()) {
                    std::string t = error.asString();
                    std::string title = "Auth error";
                    std::string d = j.getObjectItem("error_description").asString();
                    std::string desc = d;
                    if (t == "invalid_client") {
                        desc = "Login and/or password is incorrect";
                    } else if (t == "need_validation") {
                        title.clear();
                        CCOSCore::displayWindow(new VKAuth2FAWindow(this, j.getObjectItem("phone_mask").asString()));
                    } else {
                        title = t;
                    }
                    if (!t.empty()) {
                        CCOSCore::displayWindow(new MessageDialog(title, desc));
                        printf("[VK] Error %s %s\n", t.c_str(), d.c_str());
                    }
                } else {
                    json access_token = j.getObjectItem("access_token");
                    json user_id = j.getObjectItem("user_id");
                    if (access_token.isString() && user_id.isNumber()) {
                        int userId = user_id.asInt();
                        std::string token = access_token.asString();
                        CCOSCore::runOnUiThread([&, userId, token]() {
                            VK::login(userId, token);
                            close();
                        });
                        return;
                    }
                }
            } else {
                CCOSCore::runOnUiThread([&]() {
                    CCOSCore::displayWindow(
                            new MessageDialog("Network error", "Please check network connection and try again."));
                });
            }
            CCOSCore::runOnUiThread([&]() {
                mSpinner->setVisibility(GONE);
            });
        });
    }
}

void VKApplication::launch() {
    NVS nvs(VK_NVS);
    NVS::Accessor accessor = nvs.open();
    std::string token;
    int user_id;
    if (accessor.read("user_id", user_id) && accessor.readString("access_token", token)) {
        CCOSCore::displayWindow(new VKMain(user_id, token));
    } else {
        CCOSCore::displayWindow(new VKAuthWindow);
    }
}

const std::string VKApplication::getTitle() {
    return "VK";
}

Bitmap VKApplication::getIcon() {
    return vk_icon;
}
