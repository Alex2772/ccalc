//
// Created by alex2772 on 28.10.18.
//
#include "StreamIcon.h"
#include "Socket.h"
#include "Streaming.h"

const uint8_t tream[] = {
0x55,
0x15,
0x65,
0x9,
0x71,
0x1,
0x41,
0x41,
0x41,
0x41,
0x7f,
};

void StreamIcon::render(Framebuffer &framebuffer) {
    View::render(framebuffer);
    if (Streaming::instance.mSocket) {
        width = 11;
        static Bitmap b(tream, 11, 7);
        framebuffer.drawImageWithBorder(0, 0, b);
    } else {
        width = 0;
    }
}

StreamIcon::StreamIcon() {
    width = 11;
    height = 7;
}
