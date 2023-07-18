//
// Created by alex2772 on 29.11.18.
//

#include <cstring>
#include <esp_partition.h>
#include <esp_log.h>
#include "FirmwareConfig.h"

FirmwareConfig::cfg FirmwareConfig::read() {
    static cfg c = {0};
    static const char* TAG = "CCOS-FIRMWARE";
    if (c.mMajorModel == 0) {
        const esp_partition_t *t = esp_partition_find_first(ESP_PARTITION_TYPE_EX, ESP_PARTITION_SUBTYPE_ANY,
                                                            "firmware_cfg");
        if (t) {
            esp_partition_read(t, 0, &c, sizeof(c));

            ESP_LOGI(TAG, "Model Major: %d", c.mMajorModel);
            ESP_LOGI(TAG, "Vendor: %s", c.mVendor);
            ESP_LOGI(TAG, "Calculator Name: %s", c.mName);
            ESP_LOGI(TAG, "Model ID: %s", c.mModelId);
        } else {
            ESP_LOGE(TAG, "FIRMWARE DATA NOT FOUND");
            abort();
        }
    }
    return c;
}
