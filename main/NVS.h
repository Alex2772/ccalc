//
// Created by alex2772 on 12.01.18.
//


#pragma once
#include <nvs_flash.h>
#include <string>

class NVS {
private:
    nvs_handle _nvs;
public:
    class Accessor {
    private:
        bool _changed = false;
        NVS* nvs;
    public:
        Accessor(NVS* _nvs);
        ~Accessor();

        template<typename T>
        bool write(const char* key, T& value) {
            _changed = true;
            return nvs_set_blob(nvs->_nvs, key, (void*)&value, sizeof(T)) == ESP_OK;
        }
        template<typename T>
        bool read(const char* key, T& value) {
            size_t len = sizeof(T);
            return nvs_get_blob(nvs->_nvs, key, (void*)&value, &len) == ESP_OK;
        }
        bool writeString(const char *key, const std::string &value) {
            _changed = true;
            return nvs_set_str(nvs->_nvs, key, value.c_str()) == ESP_OK;
        }
        bool readString(const char *key, std::string &value) {
            value.resize(1024);
            size_t len = value.length();
            esp_err_t r = nvs_get_str(nvs->_nvs, key, &(value[0]), &len);
            value.resize(len - 1);
            return r == ESP_OK;
        }
        bool write(const char* key, const char* val, size_t len);
        bool read(const char* key, char* val, size_t len);
        bool erase(const char* key);
    };
    NVS(const char* name);
    ~NVS();
    Accessor open();
};