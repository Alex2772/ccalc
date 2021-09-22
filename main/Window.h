//
// Created by alex2772 on 03.12.17.
//


#pragma once


#include <vector>
#include "View.h"
#include "lua.hpp"
#include "Event.h"
#include <string>
#include <memory>

class Window: public View {
public:
    bool mHighlight = false;
    bool mDrawTitle = true;
    lua_State* mLuaState = nullptr;
    uint8_t _sa = 10; // show animation
    uint8_t _sadir = 0; // 0 - appear, 1 - disappear
    std::vector<std::shared_ptr<View>> views;
    Window(std::string s);
    virtual ~Window() override;

    void render(Framebuffer &framebuffer) override;
    virtual void renderPre(Framebuffer& fb);
    virtual void renderPost(Framebuffer& fb);

    virtual bool isAnimationHorizontal();

    virtual void focusNext();

    virtual void onPause();

    virtual void onResume();

    virtual void addView(View* v);
    virtual void addViewS(std::shared_ptr<View> v);

    size_t focusIndex = 0;

    virtual void setFocus(View* focus);

    virtual bool isTransparent();
    virtual bool hasTransparency();
    virtual void renderWindow(Framebuffer &framebuffer);
    virtual bool isFullscreen();
    virtual bool sleepLock();
    virtual void event(Event e);
    virtual void lua_close();

    void keyLongDown(uint8_t key) override;

    void keyDown(uint8_t key) override;

    void keyPressureChanged(uint8_t key) override;

    void keyRelease(uint8_t key) override;

    virtual void close();

    virtual const std::string &getTitle();

    virtual bool isFocused();

    std::string mTitle;

};