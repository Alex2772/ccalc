//
// Created by alex2772 on 18.11.18.
//


#pragma once

#include <string>
#include <map>
#include <sol/function.hpp>

namespace http {
    struct Document {
        int mStatus;
        std::string mContent;
        std::map<std::string,std::string> mHeaders;
    };
    Document get(const std::string& url);

    Document get(const std::string& url, const std::map<std::string, std::string>& args);

    Document get(const std::string &url, const std::map<std::string, std::string> &args,
            const std::function<void(char *, int, int)> &callback);
}