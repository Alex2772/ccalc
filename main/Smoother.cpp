//
// Created by alex2772 on 19.03.19.
//

#include "Smoother.h"

Smoother::Smoother(float speed): mSpeed(speed) {

}

float Smoother::nextValue(float value) {
    mCurrent += (value - mCurrent) * mSpeed;
    return mCurrent;
}
