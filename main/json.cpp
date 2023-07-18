//
// Created by alex2772 on 18.11.18.
//

#include "json.h"
#include <cstdio>

json::json():
    json(cJSON_CreateObject())
{
}

cJSON *json::get() {
    return mHandle.get();
}

json::json(cJSON *from, bool del) {
    mHandle = std::shared_ptr<cJSON>(from, [del](cJSON* c) {
        if (del)
            cJSON_Delete(c);
    });
}

json json::parse(const std::string &v) {
    cJSON* x = cJSON_Parse(v.c_str());
    if (!x) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            printf("JSON PARSE ERROR\n%s\nError before: %s\n", v.c_str(), error_ptr);
        }
    }
    return json(x);
;}

json json::getObjectItem(const std::string &name) {
    cJSON* s = cJSON_GetObjectItem(get(), name.c_str());
/*
    char* d = cJSON_Print(s);
    printf("%s\n", d);
    delete[] d;
*/
    return {s, false};
}

bool json::isString() {
    return bool(cJSON_IsString(get()));
}

std::string json::asString() {
    return cJSON_GetStringValue(get());
}

bool json::isNumber() {
    return bool(cJSON_IsNumber(get()));
}

int json::asInt() {
    return get()->valueint;
}
double json::asDouble() {
    return get()->valuedouble;
}

bool json::hasObjectItem(const std::string &key) {
    return static_cast<bool>(cJSON_HasObjectItem(get(), key.c_str()));
}

std::vector<json> json::asArray() {
    std::vector<json> v;
    int size = cJSON_GetArraySize(get());
    v.reserve(size);
    for (int i = 0; i < size; ++i)
        v.push_back({cJSON_GetArrayItem(get(), i), false});
    return v;
}

void json::print() {
    char* d = cJSON_Print(get());
    printf("%s\n", d);
    delete[] d;
}

json json::detachObjectItem(const std::string &key) {
    return json(cJSON_DetachItemFromObject(get(), key.c_str()));
}
