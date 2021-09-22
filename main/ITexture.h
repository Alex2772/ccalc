//
// Created by alex2772 on 16.03.19.
//

#pragma once


class ITexture {
public:
    virtual uint8_t getTextureValue(const glm::vec2& uv) = 0;
};