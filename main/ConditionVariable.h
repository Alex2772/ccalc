//
// Created by alex2772 on 04.12.18.
//


#pragma once


#include <ffconf.h>

class ConditionVariable {
private:
    QueueHandle_t mQueue;
public:
    ConditionVariable();
    ~ConditionVariable();
    void notify();
    void wait();
};