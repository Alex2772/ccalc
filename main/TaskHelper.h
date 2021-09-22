//
// Created by alex2772 on 21.04.18.
//


#pragma once


#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class TaskHelper {
private:
    TaskHandle_t mTask = 0;
public:
    TaskHelper();
    TaskHelper(std::function<void()> callable);
    TaskHelper(const TaskHelper&) = delete;
    ~TaskHelper();

    std::function<void()> mCallable;
};