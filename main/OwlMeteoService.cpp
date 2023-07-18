//
// Created by alex2772 on 13.01.18.
//

#include <cmath>
#include <esp_log.h>
#include "OwlMeteoService.h"
#include "Socket.h"
#include "FirmwareConfig.h"


bool OwlMeteoService::fill(MeteoServiceProvider::meteo_data &data) {
    Socket s("alex2772.ru", 4545);
    if (s.state != Socket::STATE_OK)
        return false;
    s.write<int>(0x07);

    s.write<int>(0x01);
    FirmwareConfig::cfg cfg = FirmwareConfig::read();
    std::string name;
    {
        char buf[16];
        sprintf(buf, "%d", cfg.mMajorModel);
        name = std::string(cfg.mName) + " " + std::string(cfg.mModelId) + "-" + buf;
    }
    s.write<int>(name.length());
    s.write_bytes(name.c_str(), name.length());

    s.write<int>(0x01);
    owl_meteodata owl = s.read<owl_meteodata>();
    s.close();

    data.temp = static_cast<int8_t>(roundf(owl.temp));
    data.humidity = static_cast<uint8_t>(owl.humidity);
    //data.precipitation = owl.rain > 512;
    data.pressure = static_cast<uint16_t>(round(double(owl.pressure) / 133.3224));
    return true;
}
