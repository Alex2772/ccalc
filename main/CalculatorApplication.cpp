//
// Created by alex2772 on 03.12.17.
//

#include "CalculatorApplication.h"
#include "calc.png.h"
#include "Window.h"
#include "CCOSCore.h"
#include "TextArea.h"
#include "tinyexpr.h"
#include "TextView.h"
#include "TaskHelper.h"
#include "ConditionVariable.h"
#include "SpinnerView.h"
#include "ContextMenu.h"
#include "Mutex.h"


bool cmp_eq(double x, double y) {
    return fabs(x - y) < 0.05;
}

bool cmp_neq(double x, double y) {
    return fabs(x - y) > 0.05;
}

bool cmp_le(double x, double y) {
    return x < y;
}

bool cmp_ge(double x, double y) {
    return x > y;
}

bool cmp_leq(double x, double y) {
    return x <= y;
}

bool cmp_geq(double x, double y) {
    return x >= y;
}

class IntersectContextMenu: public ContextMenu {
public:
    IntersectContextMenu(const std::string& s, const std::vector<Item>& items) : ContextMenu(s, items) {}

    virtual void keyDown(uint8_t key) {
        Window::keyDown(key);
        if (key == 14) {
            close();
        }
    }

};

class WindowGraph : public Window {
private:
    double mOffsetX = 0, mOffsetY = 0, mScale = 10.f;
    double mRenderingX, mRenderingY, mRenderingScale;
    std::vector<double> mZeroes;
    Framebuffer mFB;
    Framebuffer mFBdisplay;
    Mutex mDisplayMutex;
    ConditionVariable cv;
    int comparsion = 0;
    int varCount = 1;
    double __x = 0;
    double __y = 0;
    double scale = 10;

    double scrollX = 0;
    double scrollY = 0;

    te_variable vars[2];
    te_expr* te = nullptr;
    int error;

    std::string mExpression;

    SpinnerView* mSpinner = new SpinnerView(127 - 11, 64 - 7 - 11 - 1);


    void drawDelims(Framebuffer& fb, int16_t delay, int16_t s) {

        /*
        for (int16_t i = -64 / (scale * delay); i < 64 / (scale * delay); i++)
        {
            int16_t f = fmod(i * delay * scale - scrollX + 64, 128) + 64;
            fb.drawLine(f, (-scrollY + 32) - s, f, (-scrollY + 32) + s, OLED_COLOR_WHITE);
        }
        for (int16_t i = -32 / (scale * delay); i < 32 / (scale * delay); i++)
        {
            int16_t f = fmod(i * delay * scale - scrollY, 64) + 32;
            fb.drawLine((-scrollX + 64) - s, f, (-scrollX + 64) + s, f, OLED_COLOR_WHITE);
        }*/
        //fb.drawLine((-scrollX + 64), 0, (-scrollX + 64), 63, 1);
    }

    void redraw() {
        cv.notify();
    }

    TaskHelper* mCalc;
    bool mWorking = true;
    bool mShowValue = false;
public:
    WindowGraph(const std::string& expression) :
            Window("Graph"),
            mExpression(expression) {
        addView(mSpinner);
        vars[0] = {"x", &__x};

        if (expression.find('y') != std::string::npos) {
            ++varCount;
            vars[1] = {"y", &__y};
        }

        te = te_compile(expression.c_str(), vars, varCount, &error);
        if (error) {
            //close();
        } else {
            redraw();
        }
        mFBdisplay.clear();
        mCalc = new TaskHelper([&]() {
            while (mWorking) {
                mSpinner->setVisibility(VISIBLE);
                mFB.clear();
                mZeroes.clear();
                mRenderingX = scrollX;
                mRenderingY = scrollY;
                mRenderingScale = scale;
                if (varCount == 2) {

                    bool (* func)(double, double) = nullptr;

                    switch (comparsion) {
                        case 0:
                            func = &cmp_eq;
                            break;
                        case 5:
                            func = &cmp_neq;
                            break;
                        case 4:
                            func = &cmp_geq;
                            break;
                        case 3:
                            func = &cmp_ge;
                            break;
                        case 2:
                            func = &cmp_leq;
                            break;
                        case 1:
                            func = &cmp_le;
                            break;
                        default:
                            break;
                    }
                    for (int16_t x = 0; x < 128; ++x)
                        for (int16_t y = 0; y < 64; ++y) {
                            double val = pixel(x, y);
                            if (func(val, 0)) {
                                mFB.setPixel(x, y, OLED_COLOR_WHITE);
                            }
                            if (y == 32 && cmp_eq(val, 0)) {
                                mZeroes.push_back(__x);
                            }
                        }
                } else {

                    double prev = pixel(0);

                    for (int16_t x = 1; x < 128; x++) {
                        double cur = pixel(x);
                        if ((cur >= 0 && cur < 64) || (prev >= 0 && cur < 64)) {
                            mFB.drawLine(x - 1, prev, x, cur, OLED_COLOR_WHITE);
                        }
                        if ((prev >= 32 && cur <= 32) || (prev <= 32 && cur >= 32)) {
                            mZeroes.push_back(__x);
                        }
                        prev = cur;
                    }
                }
                {
                    Mutex::L lock(mDisplayMutex);
                    mOffsetX = mRenderingX;
                    mOffsetY = mRenderingY;
                    mScale = mRenderingScale;
                    mFBdisplay.clear();
                    mFBdisplay.drawFB(0, 0, 128, 64, &mFB);
                }
                mSpinner->setVisibility(GONE);
                cv.wait();
            }
        });
    }

    virtual bool isFullscreen() {
        mDrawTitle = error != 0;
        return !mDrawTitle;
    }

    double pixel(int16_t x) {
        __x = (x - 64) / mRenderingScale + mRenderingX / 10;
        double val = -te_eval(te);
        val *= mRenderingScale;
        val -= mRenderingY * mRenderingScale / 10 - 32;
        return val;
    }

    double pixel(int16_t x, int16_t y) {
        __x = (x - 64) / mRenderingScale + mRenderingX / 10;
        __y = (y - 32) / -mRenderingScale - mRenderingY / 10;
        double val = -te_eval(te);
        return val;
    }

    virtual void render(Framebuffer& fb) {
        if (isFocused()) {
            if (CCOSCore::getKeyState(1) ||
                CCOSCore::getKeyState(9) ||
                CCOSCore::getKeyState(4) ||
                CCOSCore::getKeyState(6) ||
                CCOSCore::getKeyState(7) ||
                CCOSCore::getKeyState(11)) {
                redraw();
            }
        }

        if (error)
            return;

        if (isFocused()) {
            scrollX -= double(CCOSCore::getKeyState(4)) / 2.0 / scale * 10;
            scrollX += double(CCOSCore::getKeyState(6)) / 2.0 / scale * 10;
            scrollY -= double(CCOSCore::getKeyState(1)) / 2.0 / scale * 10;
            scrollY += double(CCOSCore::getKeyState(9)) / 2.0 / scale * 10;


            scale += double(CCOSCore::getKeyState(7)) / 10.0;
            scale -= double(CCOSCore::getKeyState(11)) / 10.0;

            scale = glm::clamp(scale, 0.01, 30.0);
        }
        fb.drawRect(0, 0, 127, 63, OLED_COLOR_BLACK);

        fb.drawLine(0, static_cast<int16_t>(-scrollY * scale / 10 + 32), 127, static_cast<int16_t>(-scrollY * scale / 10 + 32), OLED_COLOR_WHITE);
        fb.drawLine(static_cast<int16_t>(-scrollX * scale / 10 + 64), 0, static_cast<int16_t>(-scrollX * scale / 10 + 64), 63, OLED_COLOR_WHITE);

        if (scale > 0.2)
            drawDelims(fb, 10, 2);

        if (scale > 2) {
            drawDelims(fb, 1, 1);
        }

        std::string fstr = mExpression;

        if (varCount == 2) {
            char c;

            bool (* func)(double, double) = nullptr;

            switch (comparsion) {
                case 0:
                    c = '=';
                    break;
                case 1:
                    c = '>';
                    break;
                case 2:
                    c = static_cast<char>('\x82'); // GEQUAL
                    break;
                case 3:
                    c = '<';
                    break;
                case 4:
                    c = static_cast<char>('\x80'); // LEQUAL
                    break;
                case 5:
                    c = static_cast<char>('\x81'); // NOT EQUAL
                    break;
                default:
                    c = '?';
                    break;
            }


            fstr += std::string(" ") + c + " 0";
        }
        {
            Mutex::L lock(mDisplayMutex);
            double sc = (scale - mScale) / 10.f;
            fb.drawFBWithScale(int16_t((mOffsetX - scrollX) * scale / 10 + (- 64) * sc), int16_t((mOffsetY - scrollY) * scale / 10 + (- 32) * sc), 128, 64, &(WindowGraph::mFBdisplay), glm::clamp(float(sc) + 1.f, 0.01f, 50.f));
            //fb.drawFBWithScale(int16_t(mOffsetX - scrollX + (scrollX - 64) * sc), int16_t(mOffsetY - scrollY + (scrollY - 32) * sc), 128, 64, &(WindowGraph::mFBdisplay), glm::clamp(sc + 1, 0.01f, 50.f));
        }
        Window::render(fb);
        if (mShowValue) {
            fb.drawLine(64 - 1, 0, 64 - 1, 64, OLED_COLOR_BLACK);
            fb.drawLine(64 + 1, 0, 64 + 1, 64, OLED_COLOR_BLACK);
            if (varCount == 2) {
                fb.drawLine(0, 32 - 1, 128, 32 - 1, OLED_COLOR_BLACK);
                fb.drawLine(0, 32 + 1, 128, 32 + 1, OLED_COLOR_BLACK);
            }

            fb.drawLine(64, 0, 64, 64, OLED_COLOR_WHITE);
            if (varCount == 2)
                fb.drawLine(0, 32, 128, 32, OLED_COLOR_WHITE);

            if (mSpinner->getVisibility() != VISIBLE) {
                char buf[128];
                double ev = -pixel(64, 32);
                switch (varCount) {
                    case 2:
                        sprintf(buf, "f(%g,%g)=%g", __x, __y, ev);
                        break;
                    default:
                        sprintf(buf, "f(%g)=%g", __x, ev);
                }

                fb.drawString(int16_t(127) - int16_t(strlen(buf)) * int16_t(4), 63 - 7 - 8, buf, OLED_COLOR_WHITE,
                              FONT_FACE_BITOCRA_4X7, true);
            }
        }

        fb.drawString(int16_t(127) - int16_t(fstr.length()) * int16_t(4), 63 - 7, fstr, OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);
    }

    ~WindowGraph() {
        mWorking = false;
        while (mSpinner->getVisibility() != GONE) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            cv.notify();
        }
        if (te)
            te_free(te);
        delete mCalc;
    }

    virtual void keyDown(uint8_t key) {
        Window::keyDown(key);
        switch (key) {
            case 10:
                mShowValue = !mShowValue;
                break;
            case 14:
                if (mSpinner->getVisibility() == VISIBLE) {
                    CCOSCore::showToast("Please wait");
                } else {
                    std::vector<ContextMenu::Item> items;

                    char buf[128];
                    for (auto& val : mZeroes) {
                        sprintf(buf, "~%g", val);
                        items.push_back({buf, []() {}});
                    }
                    CCOSCore::displayWindow(new IntersectContextMenu("INTERSECT", items));
                }
                break;
            case 15:
                ++comparsion;
                comparsion %= 6;
                redraw();
                break;
        }
    }
};


class CalcEditText : public TextArea {
private:
    bool updateNeed = false;
    bool mBinary = false;
    bool mErrorFlag = false;
    int error = 0;

    void clear() {
        setText("");
        mTextView->setText("=");
    }

    bool isContainsX() {
        return getText().find('x') != std::string::npos;
    }

    bool isContainsY() {
        return isContainsX() && getText().find('y') != std::string::npos;
    }

public:
    TextView* mTextView;

    virtual void onTextChanged() {
        if (mText.empty()) {
            mTextView->setText("="); // Sowlonya)) Owls can do impossible)
        }
        updateNeed = true;
    }

    CalcEditText(TextView* res) :
            mTextView(res) {
        x = 0;
        width = 128;

    }

    std::string getExpression() {
        std::string s = mText;
        std::replace(s.begin(), s.end(), '\x7f', 'p');
        return s;
    }

    virtual void render(Framebuffer& fb) {
        if (updateNeed) {
            double zero = 0;
            te_variable vars[] = {
                    {"x", &zero},
                    {"y", &zero}
            };

            te_expr* te = te_compile(getExpression().c_str(), vars, 2, &error);

            mErrorFlag = error != 0;
            if (isContainsX() && isContainsY()) {
                mTextView->setText("CTRL+ENTER expr graph");
            } else if (isContainsX()) {
                mTextView->setText("CTRL+ENTER graph");
            } else {
                if (error == 0) {
                    double value = te_eval(te);
                    if (mBinary) {
                        short num = static_cast<short>(value);
                        char buffer[sizeof(num) * 8 + 1];
                        memset(buffer, 0, sizeof(buffer));

                        for (size_t i = 0; i < sizeof(buffer) - 1; ++i) {
                            char c = num & (1 << (sizeof(buffer) - 2 - i)) ? '1' : '0';
                            buffer[i] = c;
                        }
                        char buf[64];
                        sprintf(buf, "BIN %s", buffer);
                        mTextView->setText(buf);
                    } else {
                        char buffer[32];
                        snprintf(buffer, sizeof(buffer), "= %g", value);
                        mTextView->setText(buffer);
                    }
                }
            }
            te_free(te);
        }
        if (moving != 0) {

            if (moveTicker == 0) {
                position += ((moving > 0) ? 1 : -1);
                if (position < 0) {
                    position = 0;
                } else if (position > mText.length()) {
                    position = mText.length();
                } else {
                    CCOSCore::vibrate(10);
                }
            }

            moveTicker += (moving > 0 ? CCOSCore::getKeyState(6) : CCOSCore::getKeyState(4));
            moveTicker %= 20;
            blink = 0;
        } else {
            moveTicker = 0;
        }
        fb.drawString(126 - mText.length() * 6 + offs, y + 2, mText.c_str());
        int16_t bx = 126 - mText.length() * 6 + position * 6 + offs;
        if (isFocused() && bx - 1 < x + width) {
            blink++;
            blink %= 50;
            fb.drawLine(bx, y + 2, bx, y + 10, blink < 25 ? OLED_COLOR_WHITE : OLED_COLOR_BLACK);
        }

        if (mErrorFlag && !getText().empty()) {
            fb.drawString(102, 34, "ERR", OLED_COLOR_WHITE, FONT_FACE_TERMINUS_8X14_ISO8859_1);
        }


        {
            int16_t curOffs = bx;

            if (curOffs > 126)
                offs -= (curOffs - 126) / 2;
            else if (curOffs < 10)
                offs += (10 - (curOffs)) / 2;
        }
    }

    bool mSkipNext = false;

    virtual void keyLongDown(uint8_t key) {
        if (moving == 0 && CCOSCore::getKeyState(12)) {
            switch (key) {
                case 0:
                    clear();
                    mSkipNext = true;
                    return;
                case 7:
                    mBinary = !mBinary;
                    updateNeed = true;
                    break;
                case 5:
                    insert("atan()");
                    --position;
                    mSkipNext = true;
                    return;
                case 10:
                    insert("log()");
                    position--;
                    mSkipNext = true;
                    return;

            }
        }
        TextArea::keyLongDown(key);
    }

    virtual void keyRelease(uint8_t key) {
        if (moving == 0) {
            if (mSkipNext) {
                mSkipNext = false;
                return;
            }
            switch (key) {
                case 0:
                    if (CCOSCore::getKeyState(12)) {
                        enterSymbol('y');
                    } else
                        enterSymbol('1');
                    break;
                case 1:
                    if (CCOSCore::getKeyState(12)) {
                        enterSymbol('x');
                    } else
                        enterSymbol('2');
                    break;
                case 2:
                    if (CCOSCore::getKeyState(12)) {
                        enterSymbol(static_cast<char>('\x7f')); // PI
                    } else
                        enterSymbol('3');
                    break;
                case 4:
                    if (CCOSCore::getKeyState(12)) {
                        insert("asin()");
                        position--;
                    } else
                        enterSymbol('4');
                    break;
                case 5:
                    if (CCOSCore::getKeyState(12)) {
                        insert("tan()");
                        position--;
                    } else
                        enterSymbol('5');
                    break;
                case 6:
                    if (CCOSCore::getKeyState(12)) {
                        insert("sin()");
                        position--;
                    } else
                        enterSymbol('6');
                    break;
                case 8:
                    if (CCOSCore::getKeyState(12)) {
                        insert("acos()");
                        position--;
                    } else
                        enterSymbol('7');
                    break;
                case 9:
                    if (CCOSCore::getKeyState(12)) {
                        insert("ctg()");
                        position--;
                    } else
                        enterSymbol('8');
                    break;
                case 10:
                    if (CCOSCore::getKeyState(12)) {
                        insert("cos()");
                        position--;
                    } else
                        enterSymbol('9');
                    break;
                case 13:
                    enterSymbol('0');
                    break;
                case 14:
                    if (CCOSCore::getKeyState(12)) {
                        insert("sqrt()");
                        position--;
                    } else
                        enterSymbol('.');
                    break;
                case 3:
                    if (CCOSCore::getKeyState(12)) {
                        if (!mText.empty() && position > 0) {
                            std::string res = mText.substr(0, position - 1);
                            if (position != mText.length())
                                res += mText.substr(position);
                            mText = res;
                            --position;
                            onTextChanged();
                            //CCOSCore::vibrate(10);
                        }
                    } else {
                        enterSymbol('/');
                    }
                    break;
                case 7:
                    if (CCOSCore::getKeyState(12))
                        enterSymbol('^');
                    else
                        enterSymbol('*');
                    break;
                case 11:
                    if (CCOSCore::getKeyState(12)) {
                        insert("()");
                        --position;
                    } else {
                        enterSymbol('-');
                    }
                    break;
                case 15:
                    if (CCOSCore::getKeyState(12) && mTextView->getText().length() > 2) {
                        if (isContainsX()) {
                            CCOSCore::displayWindow(new WindowGraph(getExpression()));
                        } else {
                            setText(mTextView->getText().substr(2));
                        }
                        //CCOSCore::vibrate(10);
                    } else {
                        enterSymbol('+');
                    }
                    break;
            }
        } else {
            moving = 0;
        }
    }

};

class Calc_Window : public Window {
public:
    CalcEditText* editText;
    TextView* mText;

    Calc_Window() :
            Window("Calculator") {
        mText = new TextView("=", 0, 50);
        addView(editText = new CalcEditText(mText));
        editText->y = 20;
        addView(mText);
        editText->focus();
    }

    virtual void resume() {
        editText->focus();
    }

};


void CalculatorApplication::launch() {
    CCOSCore::displayWindow(new Calc_Window);
}

const std::string CalculatorApplication::getTitle() {
    return std::string("Calc");
}

Bitmap CalculatorApplication::getIcon() {
    return {calc, 30, 30};
}
