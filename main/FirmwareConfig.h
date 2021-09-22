//
// Created by alex2772 on 29.11.18.
//


#pragma once


namespace FirmwareConfig {
    struct cfg {
        int mMajorModel;
        char mVendor[32];
        char mName[32];
        char mModelId[16];
    };
    cfg read();
};