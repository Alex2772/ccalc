//
// Created by alex2772 on 03.12.17.
//

#include "TextArea.h"

bool TextArea::mode = true;
bool TextArea::shift = false;

int64_t TextArea::getPosition() {
	return position;
}
void TextArea::render(Framebuffer &fb) {
    View::render(fb);
    fb.drawRect(0, 0, width, height, OLED_COLOR_BLACK);
    fb.drawLine(1, 0, static_cast<int16_t>(width - 1), 0, OLED_COLOR_WHITE);
    fb.drawLine(0, 1, 0, static_cast<int16_t>(height - 1), OLED_COLOR_WHITE);
    fb.drawLine(width, 1, width, static_cast<int16_t>(height - 1), OLED_COLOR_WHITE);
    fb.drawLine(1, height, static_cast<int16_t>(width - 1), height, OLED_COLOR_WHITE);

    if (moving != 0)
    {
        if (moveTicker == 0) {
            switch (abs(moving)) {
                case 2: {
                    if (vertMovX == -1) {
                        vertMovX = column();
                    }
                    if (moving > 1) {
                        setPosition(row() + 1, vertMovX);
                    } else {
                        setPosition(row() - 1, vertMovX);
                    }
                    CCOSCore::vibrate(10);
                    break;
                }
                case 1: {
                    position += sgn(moving);
                    if (position < 0) {
                        position = 0;
                    } else if (position > mText.length()) {
                        position = mText.length();
                    } else {
                        CCOSCore::vibrate(8);
                    }
                    break;
                }
            }
        }
        switch (abs(moving)) {
            case 2: {
                moveTicker += (moving > 0 ? CCOSCore::getKeyState(9) : CCOSCore::getKeyState(1));
                if (moveTicker > 20)
                    moveTicker = 0;
                break;
            }
            case 1: {
                moveTicker += (moving > 0 ? CCOSCore::getKeyState(6) : CCOSCore::getKeyState(4));
                if (moveTicker > 10)
                    moveTicker = 0;
                break;
            }
        }
        blink = 0;
    }
    else
    {
        moveTicker = 0;
    }

    if (offs < 0)
        offs = 0;
    if (offsY < 0)
        offsY = 0;
    
    ci.index = position;
    fb.drawStringWithCharIndex(2 - offs, 2 - offsY, mText.c_str(), FONT, OLED_COLOR_WHITE, ci);
    //fb.drawString(0, 0, mText.c_str());
    //ci.x += 2;

    if (isFocused()) {
        blink++;
        blink %= 50;
        if (letterChoose) {
            fb.drawRect(ci.x - FONT_WIDTH, ci.y - (shift ? 1 : 2), FONT_WIDTH, shift ? (FONT_HEIGHT + 4) : (FONT_HEIGHT + 2), OLED_COLOR_WHITE);
            fb.drawString(ci.x - FONT_WIDTH, ci.y, std::string(1, curLetter).c_str(), OLED_COLOR_BLACK, FONT);
            letterChoose--;
            if (!letterChoose)
                setShift(false);
        }
        else
        {
            if (blink < 25) {
                int16_t fy = static_cast<int16_t>(ci.y + (shift ? 1 : 2));
                fb.drawLine(ci.x, fy, ci.x,
                            fy + static_cast<int16_t>(shift ? (FONT_HEIGHT - 2) : (FONT_HEIGHT - 4)), OLED_COLOR_WHITE);
            }
        }
    }
    /*
    {
        char buffer[32];
        sprintf(buffer, "%d:%d", ci.x, ci.y);
        fb.drawString(1, 56, buffer, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);
    }*/
    {
        int16_t curOffs = ci.x + offs;

        if (curOffs - offs > 118)
            offs += (curOffs - offs - 118) / 2;
        else if (curOffs - offs < 10)
            offs -= (10 - (curOffs - offs)) / 2;
    }
    {
        int16_t curOffs = ci.y + offsY;

        if (curOffs - offsY > 48)
            offsY += (curOffs - offsY - 48) / 2;
        else if (curOffs - offsY < 10)
            offsY -= (10 - (curOffs - offsY)) / 2;
    }
}

TextArea::TextArea() {
    width = 50;
    height = 15;
    isInput = true;

    switchMode();
}

int64_t TextArea::column() {
    for (int64_t i = position; i > 0; --i) {
        if (mText[i - 1] == '\n') {
            return position - i;
        }
    }
    return position;
}

int64_t TextArea::row() {
    int64_t counter = 0;
    for (int64_t i = position; i > 0; --i) {
        if (mText[i - 1] == '\n') {
            ++counter;
        }
    }
    return counter;
}

void TextArea::setText(const std::string &m) {
    mText = m;
    position = mText.length();
    blink = 0;
}

const std::string &TextArea::getText() {
    return mText;
}

void TextArea::keyLongDown(uint8_t key) {
    switch (key)
    {
        case 1:
            moving = -2;
            break;
        case 9:
            moving = 2;
            break;
        case 4:
            moving = -1;
            break;
        case 6:
            moving = 1;
            break;
    }
}

void TextArea::keyPressureChanged(uint8_t key) {
    if (moving != 0)
    {
        letterChoose = 0;
    }
}

void TextArea::switchMode() {
    setMode(!mode);
    CCOSCore::vibrate(3);
    updateIndicator();
    CCOSCore::showToast(mode ? "NUMBER" : "TEXT");
}

void TextArea::switchShift() {
    if (_skip_shift) {
        _skip_shift = false;
        return;
    }
    setShift(!shift);
    CCOSCore::vibrate(3);
    CCOSCore::showToast(shift ? "SHIFT" : "NO SHIFT");
}

void TextArea::setShift(bool shift) {
    TextArea::shift = shift;
    updateIndicator();
}

void TextArea::setMode(bool mode) {
    TextArea::mode = mode;
    updateIndicator();
}

const char *TextArea::getLettersForButton(uint8_t b) {
    const char* r[] = {
            "abc",
            "def",
            "ghi",
            "jkl",
            "mno",
            "pqrs",
            "tuv",
            "wxyz"
    };
    return r[b - b / 4 - 1];
}

void TextArea::iterateLetters() {
    letterChoose = 30;
    letter++;
    const char* lett = getLettersForButton(gpn);
    if (strlen(lett) <= letter)
    {
        letter = 0;
    }
    std::string toIns(1, lett[letter]);
    if (shift)
    {
        std::transform(toIns.begin(), toIns.end(), toIns.begin(), ::toupper);
    }
    mText[position - 1] = toIns[0];
    curLetter = toIns[0];
}

void TextArea::keyDown(uint8_t key) {
    View::keyDown(key);
    if (key != 12 && CCOSCore::getKeyState(12)) {
        _skip_shift = true;
    }
}

void TextArea::keyRelease(uint8_t key) {
    if (letterChoose) {
        if (gpn != key)
        {
            letterChoose = 0;
        }
        else
        {
            iterateLetters();
            return;
        }
    }
    if (moving == 0)
    {
        switch (key)
        {
            case 13:
            case 14:
            case 0:
            case 1:
            case 2:
            case 4:
            case 5:
            case 6:
            case 8:
            case 9:
            case 10:
                if (mode)
                {
                    if (CCOSCore::getKeyState(12)) {
                        char symbols[] = {
                                '\'',
                                '[',
                                ']',
                                '?',
                                '"',
                                '{',
                                '}',
                                '?',
                                '~',
                                '<',
                                '>',
                                ':',
                                '?',
                                '\\',
                                ',',
                        };
                        enterSymbol(symbols[key]);
                    } else {
                        if (shift) {
                            char symbols[] = {
                                    '!',
                                    '@',
                                    '#',
                                    '?',
                                    '$',
                                    '%',
                                    '^',
                                    '?',
                                    '&',
                                    '*',
                                    '(',
                                    '?',
                                    '?',
                                    ')',
                                    '?',
                            };
                            enterSymbol(symbols[key]);
                            setShift(false);
                        } else {
                            char symbols[] = {
                                    '1',
                                    '2',
                                    '3',
                                    '?',
                                    '4',
                                    '5',
                                    '6',
                                    '?',
                                    '7',
                                    '8',
                                    '9',
                                    '?',
                                    '?',
                                    '0',
                                    '.'
                            };
                            enterSymbol(symbols[key]);
                        }
                    }
                }
                else
                {
                    gpn = key;
                    if (gpn > 0 && gpn < 11) {
                        letter = -1;
                        enterSymbol('?');
                        iterateLetters();
                    }
                    else if (gpn == 13)
                    {
                        enterSymbol(' ');
                    }
                }
                break;
            case 7:
                switchMode();
                break;
            case 11:
                if (mode) {
                    if (CCOSCore::getKeyState(12)) {
                        enterSymbol(':');
                    } else {
                        if (shift) {
                            enterSymbol('+');
                        } else {
                            enterSymbol('-');
                        }
                    }
                } else
                {
                    if (shift) {
                        enterSymbol(';');
                    } else {
                        enterSymbol('=');
                    }
                }
                break;
            case 12:
                switchShift();
                break;
            case 3:
                if (CCOSCore::getKeyState(12)) {
                    enterSymbol('/');
                    setShift(false);
                } else {
                    if (!mText.empty() && position > 0) {
                        std::string res = mText.substr(0, position - 1);
                        if (position != mText.length())
                            res += mText.substr(position);
                        mText = res;
                        position--;
                        onTextChanged();
                    }
                }
                break;
            case 15:
                if (multiline)
                    enterSymbol('\n');
                break;
        }
    }
    else
    {
        moving = 0;
        vertMovX = -1;
    }
}

void TextArea::focusLost() {
    moving = 0;
    mIndicator.set(0, 0, 0);
}

void TextArea::setPosition(short i) {
    position = i % mText.length();
}

void TextArea::insert(std::string c) {
    blink = 0;
    mText.insert(position, c);
    position += c.length();
    onTextChanged();
}

void TextArea::enterSymbol(char c) {
    blink = 0;
    if (this->onSymbol(c)) {

        mText.insert(mText.begin() + position, c);
        position++;
        onTextChanged();
    }
}

bool TextArea::onSymbol(char &c) {
    return true;
}

void TextArea::setPosition(int row, int column) {
    position = 0;
    if (row >= 0 && !mText.empty()) {
        for (int r = 0; r < row; r++) {
            while (mText[position] != '\n') {
                ++position;
                if (position >= mText.length()) {
                    position = mText.length();
                    return;
                }
            }
            ++position;
        }
        if (column >= 0) {
            for (int i = 0; i < column; ++i) {
                if (mText[position] != '\n')
                    ++position;
            }
        }
        if (position >= mText.length()) {
            position = mText.length();
        }
    }
}

void TextArea::updateIndicator() {
    const float b = 0.06f;
    mIndicator.set(shift ? b : 0, mode ? b : 0, mode ? 0 : b);
}

TextArea::~TextArea() {
    mIndicator.set(0,0,0);

}
