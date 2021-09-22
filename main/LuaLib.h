//
// Created by alex2772 on 20.04.18.
//

#pragma once

#include "lua.hpp"
#include <string>
#include "sol.hpp"

namespace LuaLib {
    uint8_t battery();
    void _sleep(uint32_t millis);
    void message(std::string m);
    std::string input(std::string title);
    void open(sol::state& L);

    std::string file();
}