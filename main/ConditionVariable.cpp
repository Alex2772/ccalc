//
// Created by alex2772 on 04.12.18.
//

#include "ConditionVariable.h"

ConditionVariable::ConditionVariable() {
    mQueue = xQueueCreate(1, 1);
}



ConditionVariable::~ConditionVariable() {
    vQueueDelete(mQueue);
}

void ConditionVariable::notify() {
    char t = 0;
    xQueueSend(mQueue, &t, 0);
}

void ConditionVariable::wait() {
    char t = 0;
    xQueueReceive(mQueue, &t, portMAX_DELAY);
}
