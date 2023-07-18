//
// Created by alex2772 on 25.04.18.
//


#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <deque>
#include "lua.hpp"
#include "sol.hpp"
#include "Mutex.h"
#include <functional>


class Process {
public:
    Mutex m;
    std::vector<sol::object> alive;
    sol::state lua;
    TaskHandle_t task;
    void ropt(std::function<void()> f);

    std::deque<std::function<void()>> rpt;
};