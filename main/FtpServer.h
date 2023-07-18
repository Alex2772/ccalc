//
// Created by alex2772 on 21.04.18.
//


#pragma once


#include <vector>
#include "TaskHelper.h"

class FtpServer {
private:
    TaskHelper* acceptor;
    TaskHelper* debugger;
    std::vector<TaskHelper*> pool;
public:
    FtpServer();
    ~FtpServer();
};