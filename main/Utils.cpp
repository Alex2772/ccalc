//
// Created by alex2772 on 21.04.18.
//

#include "Utils.h"
#include <cstdio>

std::string Utils::sizeFormat(uint32_t size) {
    char f[] = {
            'K',
            'M',
            'G',
            'P'
    };
    char buf[32];
    int index = -1;
    uint32_t orig = size;
    while (size > 1200 && index + 1 < 4) {
        size /= 1024;
        ++index;
    }
    if (index >= 0) {
        sprintf(buf, "%u%cB (%u bytes)", size, f[index], orig);
    } else {
        sprintf(buf, "%uB", size);
    }
    return {buf};
}
