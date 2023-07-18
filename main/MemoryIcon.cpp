//
// Created by alex2772 on 12.12.17.
//

#include <esp_system.h>
#include <cstring>
#include "MemoryIcon.h"

void MemoryIcon::render(Framebuffer &framebuffer) {
    View::render(framebuffer);

    uint32_t free = esp_get_free_heap_size();
    free /= 1024;

    char buf[8];
    sprintf(buf, "%uKB", free);
    framebuffer.drawString(1, 0, buf, OLED_COLOR_INVERT, FONT_FACE_BITOCRA_4X7);
    width = static_cast<int16_t>(strlen(buf) * 4);
}
