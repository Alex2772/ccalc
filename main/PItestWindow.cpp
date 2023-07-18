//
// Created by alex2772 on 05.02.19.
//

#include "PItestWindow.h"
#include "TextView.h"

double factorial(double num) {
    double factorial = 1;
    double i = 1;
    while (i < num) {
        factorial = factorial * i;
        i += 1.0;
    }
    return factorial;
}

PItestWindow::PItestWindow() :
        Window("PI test"),
        mThread([&]() {
            for (;;) {
                mValue += 12 * pow(-1, mK) * factorial(6 * mK) * (13591409.0 + 545140134.0 * mK) / (factorial(3 * mK) * pow(factorial(mK), 3) * pow(640320, 3 * (mK + 0.5)));
                mK += 1;
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }) {

    addView(mLabel = new TextView());
    addView(mCounterLabel = new TextView());
    mLabel->x = 0;
    mLabel->y = 12;
    mLabel->width = 127;
    mLabel->height = 63 - 12 - 12;
    mCounterLabel->x = 0;
    mCounterLabel->y = 64-12;
    mCounterLabel->width = 127;
}

void PItestWindow::render(Framebuffer &framebuffer) {
    char buf[4096];
    sprintf(buf, "%0.32f", 1.0 / mValue);
    mLabel->setText(buf);
    sprintf(buf, "%u", mK);
    mCounterLabel->setText(buf);
    Window::render(framebuffer);
}
