//
// Created by alex2772 on 17.11.18.
//

#include <sstream>
#include "Encryption.h"
#include <cstdio>

std::string enc::sha256(const std::string& value) {
    uint8_t res[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *) value.c_str(), value.length());
    mbedtls_md_finish(&ctx, res);
    mbedtls_md_free(&ctx);
    std::stringstream ss;
    for (size_t i = 0; i < sizeof(res); ++i) {
        char str[3];
        sprintf(str, "%02x", res[i]);
        ss << str;
    }
    return ss.str();
}
