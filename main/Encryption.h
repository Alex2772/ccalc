#pragma once

#include "mbedtls/md.h"
#include <string>

namespace enc {
    std::string sha256(const std::string& value);
}