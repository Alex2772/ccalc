//
// Created by alex2772 on 19.03.19.
//


#pragma once


class Smoother {
private:
    float mSpeed;
    float mCurrent;
public:
     explicit Smoother(float speed);

    float nextValue(float value);
};