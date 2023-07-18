//
// Created by alex2772 on 07.04.19.
//

#include "Streaming.h"

Streaming Streaming::instance;

void Streaming::stop(const std::string& status) {
    mStatus = status;
    delete mSocket;
    mSocket = nullptr;
}
