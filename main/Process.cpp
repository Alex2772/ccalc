//
// Created by alex2772 on 25.04.18.
//

#include "Process.h"
#include "CCOSCore.h"

void Process::ropt(std::function<void()> f) {
    Mutex::L lock(m);
    rpt.push_back(f);
}
