//
// Created by alex2772 on 15.04.18.
//


#pragma once


#include <functional>
#include "Dialog.h"
#include "TextArea.h"

class InputDialog: public Dialog {
private:
    std::function<void(const std::string&)> callback;
    TextArea* t = nullptr;
public:
    InputDialog(std::string title, std::string text, std::function<void(const std::string&)> c);

    void keyDown(uint8_t key) override;

    ~InputDialog() override;
};