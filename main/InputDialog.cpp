//
// Created by alex2772 on 15.04.18.
//

#include "InputDialog.h"
#include "TextArea.h"

InputDialog::InputDialog(std::string title, std::string text, std::function<void(const std::string &)> c): Dialog(title), callback(c) {
    t = new TextArea();
    t->setText(text);
    addView(t);
    t->x = 15;
    t->y = 23;
    t->width = 88;
    t->focus();
}

InputDialog::~InputDialog() {
}

void InputDialog::keyDown(uint8_t key) {
    View::keyDown(key);
    switch (key) {
        case 15:
            callback(t->getText());
            CCOSCore::removeWindow(this);
            break;
    }
}
