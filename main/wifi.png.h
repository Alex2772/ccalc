#pragma once

#include <cstdint>

const uint8_t wifi1[] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x40,
    0x0,
    0x0,
    0x0,
    0x0,
};
const uint8_t wifi2[] = {
        0x0,
        0x0,
        0x0,
        0x10,
        0x10,
        0x10,
        0x0,
        0x0,
        0x0,
};
const uint8_t wifi3[] = {
        0x0,
        0x8,
        0x4,
        0x4,
        0x4,
        0x4,
        0x4,
        0x8,
        0x0,
};
const uint8_t wifi4[] = {
        0x2,
        0x1,
        0x1,
        0x1,
        0x1,
        0x1,
        0x1,
        0x1,
        0x2,
};
static const uint8_t* wifis[] = {
        wifi1,
        wifi2,
        wifi3,
        wifi4
};