//
// Created by alex2772 on 12.12.17.
//

#pragma once

#include <cstdint>

namespace Cloud {
    bool isSyncing();
    void sync();
    void onInternetConnection();
}