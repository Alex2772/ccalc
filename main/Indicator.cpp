//
// Created by alex2772 on 14.12.18.
//

#include "Indicator.h"
#include "CCOSCore.h"

Indicator::Indicator()
{
    uint8_t brightness = 64;
    CCOSCore::getNVSHandle().open().read("indicator", brightness);
    mBrightness = float(brightness) / 255.f;
}


Indicator::~Indicator() {
    CCOSCore::setIndicatorValue(0, 0, 0);
}

void Indicator::set(float r, float g, float b) {
    CCOSCore::setIndicatorValue(glm::clamp(r * mBrightness, 0.f, 1.f), glm::clamp(g * mBrightness, 0.f, 1.f), glm::clamp(b * mBrightness, 0.f, 1.f));
}

void Indicator::setBrightness(float brightness) {
    mBrightness = brightness;
}

Indicator::Indicator(float brightness) : mBrightness(brightness) {}
