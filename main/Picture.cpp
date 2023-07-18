//
// Created by alex2772 on 17.12.17.
//

#include "Picture.h"

Picture::Picture(const uint8_t *data, uint16_t width, uint16_t height) : Bitmap(data, width, height) {

}

void Picture::draw(Framebuffer& fb, int16_t x, int16_t y) {
    fb.drawImage(x, y, *this);
}

LivePicture::LivePicture(const uint8_t *data, uint16_t width, uint16_t height, std::function<void(Framebuffer& fb)> dr) : Picture(data, width, height), _dr(dr) {

}

void LivePicture::draw(Framebuffer &fb, int16_t x, int16_t y) {
    Picture::draw(fb, x, y);
    fb.setCoordsOffset(x, y);
    _dr(fb);
    fb.setCoordsOffset(0, 0);
}
