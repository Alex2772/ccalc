//
// Created by alex2772 on 13.01.18.
//


#pragma once


#include <cstdint>

class MeteoServiceProvider {
public:
    struct meteo_data {
        int8_t temp;
        uint8_t humidity;
        uint16_t pressure;
        bool precipitation;
    };
    virtual bool fill(meteo_data&) = 0;
};