//
// Created by alex2772 on 13.01.18.
//


#pragma once


#include "MeteoServiceProvider.h"

class OwlMeteoService: public MeteoServiceProvider {
private:
    struct owl_meteodata {
        int status; // 1 - ok
        float temp; // cels
        float humidity; // 0-100%
        //int rain; // 0 - 1024
        int pressure; // pascals
    };

public:
    bool fill(meteo_data &data) override;
};