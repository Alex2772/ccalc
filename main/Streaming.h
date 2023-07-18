//
// Created by alex2772 on 07.04.19.
//


#pragma once


#include "Socket.h"

class Streaming {
public:
    UDPSocket *mSocket = nullptr;
    std::string mAddress;
    std::string mStatus = "Ready";
    sockaddr_in mStreamDst;

    static Streaming instance;

    void stop(const std::string& err);
};
