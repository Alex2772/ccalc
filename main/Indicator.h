//
// Created by alex2772 on 14.12.18.
//


#pragma once


class Indicator {
private:
    float mBrightness;
public:
    void setBrightness(float brightness);

    Indicator();

    Indicator(float brightness);

    void set(float r, float g, float b);
    ~Indicator();
};