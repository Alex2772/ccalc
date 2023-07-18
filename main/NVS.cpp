//
// Created by alex2772 on 12.01.18.
//

#include "NVS.h"

NVS::Accessor::Accessor(NVS *_nvs):
    nvs(_nvs)
{

}

NVS::Accessor::~Accessor() {
    if (_changed)
        nvs_commit(nvs->_nvs);
}

bool NVS::Accessor::write(const char *key, const char *val, size_t len) {
    _changed = true;
    return nvs_set_blob(nvs->_nvs, key, val, len) == ESP_OK;
}

bool NVS::Accessor::read(const char *key, char *val, size_t len) {
    return nvs_get_blob(nvs->_nvs, key, val, &len) == ESP_OK;
}

bool NVS::Accessor::erase(const char *key) {
    return nvs_erase_key(nvs->_nvs, key) == ESP_OK;
}

NVS::NVS(const char *name) {
    nvs_open(name, NVS_READWRITE, &_nvs);
}

NVS::~NVS() {

    nvs_close(_nvs);
}

NVS::Accessor NVS::open() {
    return {this};
}
