//
// Created by alex2772 on 18.11.18.
//


#pragma once


#include <cJSON.h>
#include <memory>
#include <vector>

class json {
private:
    std::shared_ptr<cJSON> mHandle;
public:
    json();
    json(cJSON* from, bool del = true);
    cJSON* get();

    json getObjectItem(const std::string& name);
    std::vector<json> asArray();
    bool isString();

    static json parse(const std::string& v);

    std::string asString();

    bool isNumber();

    int asInt();

    double asDouble();

    bool hasObjectItem(const std::string &key);

    void print();

    json detachObjectItem(const std::string &key);
};