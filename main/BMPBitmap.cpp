// https://ru.wikipedia.org/wiki/BMP

#include <cstring>
#include <cstdio>
#include "BMPBitmap.h"

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

#pragma pack(push, 1)
struct BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
struct BITMAPINFO {
    DWORD biSize;
    DWORD biWidth;
    DWORD biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD biXPelsPerMeter;
    DWORD biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};
#pragma pack(pop)

BMPBitmap::BMPBitmap(File &f) : OwnBitmap(0, 0) {
    f.open(File::R);
    BITMAPFILEHEADER h;
    f.read(h);
    if (h.bfType != 0x4d42) {
        printf("File is not bmp image\n");
        return;
    }
    BITMAPINFO info;
    f.read(info);
    if (info.biSize != 40) {
        printf("BITMAPINFO size is not 40 (%u)\n", info.biSize);
        return;
    }
    if (info.biCompression != 0) {
        printf("BITMAPINFO compression is unsupported (%u)\n", info.biCompression);
        return;
    }
    if (info.biPlanes != 1) {
        printf("BITMAPINFO biPlanes is not 1 (%u)\n", info.biPlanes);
        return;
    }
    if (info.biBitCount != 1) {
        printf("BITMAP INFO biBitCount must be 1\n");
        return;
    }
    f.seek(h.bfOffBits, 0);

    width = info.biWidth;
    height = info.biHeight;
    size_t s = countSize();
    uint8_t *nonc = new uint8_t[s];
    memset((void *) nonc, 0, s);
    uint8_t zpadd = (info.biWidth + 8 - 1) / 8;
    zpadd = (zpadd + 4 - 1) / 4 * 4 - zpadd;
    for (DWORD y = 0; y < info.biHeight; y++) {
        for (DWORD x = 0; x < info.biWidth;) {
            uint8_t b;
            f.read(b);
            for (uint8_t i = 0; i < 8; i++) {
                bool v = !static_cast<bool>(b & 128);
                if (v) {
                    uint16_t px = x + i;
                    uint16_t py = info.biHeight - y - 1;

                    uint16_t index = (uint16_t) (px + (py / 8) * info.biWidth);
                    if (index < s)
                        nonc[index] |= (1 << (py & 7));
                }
                b <<= 1;
            }
            x += 8;
        }
        for (uint8_t i = 0; i < zpadd; ++i) {
            uint8_t byte;
            f.read(byte);
        }
    }
    data = nonc;
    /*
    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            printf("%c", ((data[y / 8 * width + x] >> (y % 8)) & 1) ? '#' : '.');
        }
        printf("\n");
    }*/
}

BMPBitmap::BMPBitmap(const OwnBitmap &other) :
        OwnBitmap(other) {

}

void BMPBitmap::save(File &f) {
    f.open(File::W);

    int widthInBytes = (width + 8 - 1) / 8;


    uint32_t extraBytes = static_cast<uint32_t>(4 - (widthInBytes % 4));
    if (extraBytes == 4) {
        extraBytes = 0;
    }
    uint32_t sizeWithPadding = (widthInBytes + extraBytes) * height;

    const char msg[] = {0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 };

    {
        BITMAPFILEHEADER h;
        h.bfType = 0x4d42;
        h.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + sizeof(msg);
        h.bfReserved1 = 0;
        h.bfReserved2 = 0;
        h.bfSize = h.bfOffBits + sizeWithPadding + 2;
        f.write(h);
    }
    {
        BITMAPINFO info;
        info.biSize = 40;
        info.biWidth = width;
        info.biHeight = height;
        info.biPlanes = 1;
        info.biBitCount = 1;
        info.biCompression = 0;
        info.biSizeImage = sizeWithPadding;
        info.biXPelsPerMeter = 0x0b12;
        info.biYPelsPerMeter = 0x0b12;
        info.biClrUsed = 0;
        info.biClrImportant = 0;
        f.write(info);
    }
    f.write(msg);

    for (uint16_t y = 0; y < height; y++) {
        uint8_t byte = 0;
        uint8_t byteFill = 0;
        for (uint16_t x = 0; x < width; ++x) {
            if (!getPixelAt(x, static_cast<uint16_t>(height - y - 1)))
                byte |= 1 << (7 - byteFill);
            ++byteFill;
            if (byteFill >= 8) {
                f.write(byte);
                byte = 0;
                byteFill = 0;
            }
        }
        if (byteFill > 0) {
            f.write(byte);
        }

        for (uint8_t i = 0; i < extraBytes; ++i) {
            uint8_t byte = 0;
            f.write(byte);
        }
    }

    f.close();
}
