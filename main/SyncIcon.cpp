//
// Created by alex2772 on 12.12.17.
//

#include "SyncIcon.h"
#include "cloud.png.h"
#include "Cloud.h"

void SyncIcon::render(Framebuffer &framebuffer) {
    View::render(framebuffer);
    static Bitmap cld(cloud, 10, 7);
    if (Cloud::isSyncing()) {
        width = 10;
        framebuffer.drawImage(0, 0, cld);
    } else {
        width = 0;
    }
}
