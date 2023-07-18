//
// Created by alex2772 on 18.04.18.
//


#pragma once


#include "Dialog.h"

class MessageDialog: public Dialog {
private:
    std::string mText;
    std::function<void()> mCallback;
public:
    MessageDialog(const std::string &s, const std::string& msg);
    MessageDialog(const std::string &s, const std::string& msg, const std::function<void()> & callback);

    void render(Framebuffer &framebuffer) override;

    //void keyRelease(uint8_t key) override;

    void keyDown(uint8_t key);
};