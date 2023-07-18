//
// Created by alex2772 on 02.12.17.
//

#include <string.h>
#include <cstdlib>
#include <iconv.h>
#include "Framebuffer.h"
#include "CCOSCore.h"
#include "ssd1306.h"
#include <glm/glm.hpp>

Framebuffer::Framebuffer() {
    buffer = new uint8_t[1024];
}

Framebuffer::~Framebuffer() {
    delete[] buffer;
}

Framebuffer::Color invertColor(Framebuffer::Color color) {
    switch (color) {
        case OLED_COLOR_BLACK:
            return OLED_COLOR_WHITE;
        case OLED_COLOR_WHITE:
            return OLED_COLOR_BLACK;
        default:
            return color;
    }
}

void Framebuffer::drawString(int16_t x, int16_t y, std::string s, Color color, int font, bool border) {
    if (font == 255)
        font = FONT_FACE_TERMINUS_6X12_ISO8859_1;
    if (border) {
        for (int8_t i = 0; i < 9; i++) {
            if (i == 4) {
                continue;
            }
            int16_t _x = i % 3 - 1;
            int16_t _y = i / 3 - 1;
            ssd1306_draw_string(&display, buffer, font_builtin_fonts[font], x + offsetX + _x, y + offsetY + _y,
                                s.c_str(),
                                invertColor(color), OLED_COLOR_TRANSPARENT);
        }
    }

    ssd1306_draw_string(&display, buffer, font_builtin_fonts[font], x + offsetX, y + offsetY, s.c_str(), color,
                        OLED_COLOR_TRANSPARENT);
}

void Framebuffer::setPixel(int16_t x, int16_t y, Color color) {

    ssd1306_draw_pixel(&display, buffer, x + offsetX, y + offsetY, color);
}

Framebuffer::Color Framebuffer::getPixel(int16_t x, int16_t y) {
    return (buffer[(uint16_t) (x + (y / 8) * display.width)] & (1 << (y & 7))) > 0 ? OLED_COLOR_WHITE : OLED_COLOR_BLACK;
}

void Framebuffer::clear() {
    memset(buffer, 0, 1024);
}

void Framebuffer::drawLine(int16_t x, int16_t y, int16_t x1, int16_t y1, Framebuffer::Color color) {
    ssd1306_draw_line(&display, buffer, x + offsetX, y + offsetY, x1 + offsetX, y1 + offsetY, color);
}

void Framebuffer::setCoordsOffset(int16_t _x, int16_t _y) {
    offsetX = _x;
    offsetY = _y;
}

void Framebuffer::drawRect(int16_t x, int16_t y, uint8_t width, uint8_t height, Framebuffer::Color color) {
    //printf("drawRect %d %d %d %d\n", x, y, width, height);
    ssd1306_fill_rectangle(&display, buffer, x + offsetX, y + offsetY, width, height, color);
}

void Framebuffer::drawBorder(int16_t x, int16_t y, uint8_t width, uint8_t height, Framebuffer::Color color) {
    drawLine(x, y, x + width, y, color);
    drawLine(x + width, y, x + width, y + height, color);
    drawLine(x, y + height, x + width, y + height, color);
    drawLine(x, y, x, y + height, color);
}

void Framebuffer::displayImagePart(int16_t x, int16_t y, int16_t width, int16_t u, int16_t v, const uint8_t*& data,
                                   Color color) {
    int16_t k = (v / 8) * width + u;
    int16_t c = (y / 8) * display.width + x;
    if (c < 1024 && c >= 0) {
        if (y < 0) {
            switch (color) {
                case OLED_COLOR_INVERT:
                case OLED_COLOR_WHITE:
                    buffer[c] |= data[k] >> (-y % 8);
                    break;
                case OLED_COLOR_TRANSPARENT:
                case OLED_COLOR_BLACK:
                    buffer[c] &= ~(data[k] >> (-y % 8));
                    break;
            }
        } else {
            switch (color) {
                case OLED_COLOR_INVERT:
                case OLED_COLOR_WHITE:
                    buffer[c] |= data[k] << (y % 8);
                    break;
                case OLED_COLOR_TRANSPARENT:
                case OLED_COLOR_BLACK:
                    buffer[c] &= ~(data[k] << (y % 8));
                    break;
            }
        }
        if (y < 0)
            return;
        c = (y / 8 + 1) * display.width + x;
        if (y % 8 && c < 1024 && c >= 0) {
            if (y < 0) {
                switch (color) {
                    case OLED_COLOR_INVERT:
                    case OLED_COLOR_WHITE:
                        buffer[c] |= data[k] << (8 - (-y % 8));
                        break;
                    case OLED_COLOR_TRANSPARENT:
                    case OLED_COLOR_BLACK:
                        buffer[c] &= ~(data[k] << (8 - (-y % 8)));
                        break;
                }
            } else {
                switch (color) {
                    case OLED_COLOR_INVERT:
                    case OLED_COLOR_WHITE:
                        buffer[c] |= data[k] >> (8 - (y % 8));
                        break;
                    case OLED_COLOR_TRANSPARENT:
                    case OLED_COLOR_BLACK:
                        buffer[c] &= ~(data[k] >> (8 - (y % 8)));
                        break;
                }
            }
        }
    }
}

void Framebuffer::drawImage(int16_t x, int16_t y, Bitmap& bitmap, bool mirrorX, Color color) {
    x += offsetX;
    y += offsetY;
    if (x + bitmap.width < 0 || y + bitmap.height < 0 || x >= display.width || y >= display.height) {
        return;
    }
    int16_t cnt = (bitmap.height) / 8 + 1;
    //if (!(y % 8 == 0 && height % 8 == 0))
    //	cnt++;
    for (int16_t i = 0; i < cnt; i++) {
        for (int16_t j = x; j < x + bitmap.width; j++) {
            if (j >= 0 && j < 128) {
                displayImagePart(j, y + i * 8, bitmap.width, mirrorX ? (bitmap.width - (j - x)) : j - x, i * 8,
                                 bitmap.data, color);
            }
        }
    }
}

extern "C" {
char toKOI8(char c);
};

void Framebuffer::drawStringWithCharIndex(int16_t x, int16_t y, const char* string, int font, Framebuffer::Color color,
                                          Framebuffer::ci_info_struct& ci) {

    const font_info_t* f = font_builtin_fonts[font];
    x += offsetX;
    y += offsetY;
    ci.x = x;
    ci.y = y;

    size_t curIndex = 0;
    while (y < -f->height && (string[curIndex] == '\n' || string[curIndex] == '\0')) {
        curIndex++;
    }
    bool ciSet = false;

    while (string[curIndex] != '\0') {
        int16_t currentX = x;
        int16_t currentY = y;
        while (string[curIndex] != '\0') {
            if (!ciSet && curIndex == ci.index) {
                ciSet = true;
                ci.x = currentX;
                ci.y = currentY;
            }
            if (string[curIndex] == '\n') {
                y += f->height;
                if (!ciSet && curIndex + 1 == ci.index) {
                    ciSet = true;
                    ci.x = 0;
                    ci.y = y;
                }
                curIndex++;
                break;
            }
            int d;
            if ((d = ssd1306_draw_char(&display, buffer, f, currentX, currentY, string[curIndex], color,
                                       OLED_COLOR_TRANSPARENT)) == 0) {
                //break;
            }
            currentX += d;
            curIndex++;
        }
        if ((curIndex) == ci.index) {
            if (!ciSet) {
                ciSet = true;
                ci.x = currentX;
                ci.y = y;
            }
        }
        /*
        if (y >= 64)
        {
            goto end;
        }*/
        if (!ciSet) {
            ci.x = currentX;
            ci.y = currentY;
        }

    }
    //end:
    ci.x -= offsetX;
    ci.y -= offsetY;
}

char* Framebuffer::drawStringML(int16_t x, int16_t y, char* str, int16_t width, int font, Color color) {
    x += offsetX;
    y += offsetY;
    /* Write characters */
    const font_info_t* f = font_builtin_fonts[font];


    int16_t length = 2;// = f->char_descriptors->width;
    bool fs = false;

    while (*str && length <= width && *str != '\n') {
        /* Write character by character */
        int16_t len = 0;
        if (fs) {
            fs = false;
            len = ssd1306_draw_char(&display, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_6X12_KOI8_R], x + length, y,
                                    toKOI8(*str), color, OLED_COLOR_TRANSPARENT);
            length += len;
        } else if (*str != (0b11010000) && *str != (0b11010001)) {
            if (*str != '\r')
                if ((len = ssd1306_draw_char(&display, buffer, f, x + length, y, *str, color,
                                             OLED_COLOR_TRANSPARENT)) == 0) {
                    /* Return error */
                    return str++;
                }
            length += len;
        } else {
            fs = true;
        }
        /* Increase string pointer */
        ++str;
    }

    if (*str == '\n')
        str++;

    /* Everything OK, zero should be returned */
    return str;
}

void Framebuffer::roundedRect(int16_t x, int16_t y, uint16_t width, uint16_t height, Framebuffer::Color color) {
    drawLine(x + 1, y, x + width - 1, y, color);
    drawLine(x + width, y + 1, x + width, y + height - 1, color);
    drawLine(x + 1, y + height, x + width - 1, y + height, color);
    drawLine(x, y + 1, x, y + height - 1, color);
}

void Framebuffer::drawImageWithBorder(int16_t x, int16_t y, Bitmap& bitmap, bool mirrorX, Framebuffer::Color color) {
    drawImage(x - 1, y, bitmap, mirrorX, invertColor(color));
    drawImage(x + 1, y, bitmap, mirrorX, invertColor(color));
    drawImage(x, y - 1, bitmap, mirrorX, invertColor(color));
    drawImage(x, y + 1, bitmap, mirrorX, invertColor(color));
    drawImage(x, y, bitmap, mirrorX, color);
}

int Framebuffer::drawChar(int16_t x, int16_t y, const char c, Framebuffer::Color color, int font) {
    const font_info_t* fnt = font_builtin_fonts[font];
    return ssd1306_draw_char(&display, buffer, fnt, x, y, c, color, OLED_COLOR_TRANSPARENT);
}

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

void Framebuffer::snapshot(int16_t width, int16_t height, uint8_t* dst) {
    for (int16_t y = 0; y < int16_t(ceilf(height / 8.f)); y++)
        for (int16_t x = 0; x < width; x++) {
            dst[y * width + x] = buffer[y * display.width + x];
        }
}
void Framebuffer::snapshot(int16_t width, int16_t height, uint8_t* dst, float scale) {
    memset(dst, 0, width * height);
    for (int16_t y = 0; y < height; y++)
        for (int16_t x = 0; x < width; x++) {
            dst[y / 8 * width + x] |= getPixel(int16_t(roundf(x * scale)), int16_t(roundf(y * scale))) << (y % 8);
        }
}

int16_t Framebuffer::length(font_face_t font, const char* str) {
    int16_t x = 0;
    for (; *str; str++) {
        x += font_get_char_desc(font_builtin_fonts[font], *str)->width;
    }
    return x;
}

void Framebuffer::shade() {
    static const uint8_t mask[] = {85, 170};
    for (size_t i = 0; i < 1024; ++i) {
        buffer[i] &= mask[i % 2];
    }
}

Framebuffer::Framebuffer(const Framebuffer& fb) {
    buffer = new uint8_t[1024];
    memcpy(buffer, fb.buffer, 1024);
}

void Framebuffer::drawCircle(int16_t x, int16_t y, uint8_t radius, Framebuffer::Color color) {
    ssd1306_draw_circle(&display, buffer, x, y, radius, color);
}

void Framebuffer::drawFB(int16_t x, int16_t y, uint16_t w, uint16_t h, Framebuffer* fb) {
    uint8_t* buf = new uint8_t[w * h];
    fb->snapshot(w, h, buf);
    Bitmap b(buf, w, h);
    drawImage(x, y, b);
    delete[] buf;
}

void Framebuffer::drawFBWithScale(int16_t x, int16_t y, uint16_t w, uint16_t h, Framebuffer* fb, float scale) {
    w = static_cast<uint16_t>(roundf(w * scale));
    h = static_cast<uint16_t>(roundf(h * scale));
    auto* buf = new uint8_t[w * h];
    fb->snapshot(w, h, buf, 1.f / scale);
    Bitmap b(buf, w, h);
    drawImage(x, y, b);
    delete[] buf;
}

template<typename T>
glm::vec2 tovec2(const T& t) {
    return {t.x, t.y};
}

glm::vec3 barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& point) {
    glm::vec3 s[2];
    for (int i = 2; i--;) {
        s[i][0] = c[i] - a[i];
        s[i][1] = b[i] - a[i];
        s[i][2] = a[i] - point[i];
    }
    glm::vec3 u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2)
        return {1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z};
    return {-1, 1, 1};
}

void Framebuffer::drawArrays(const glm::mat4& model, const std::vector<Vertex>& vertices,
                             const std::function<int(const glm::vec2&)>& frag) {

    static auto projection = glm::perspective(45.f, 128.f / 64.f, 0.01f, 100.f);
    static auto viewport = glm::vec4(0, 0, 128.f, 64.f);

    for (size_t i = 0; i < vertices.size() / 3; ++i) {
        auto v0 = (glm::project(vertices[i * 3].mPos, model, projection, viewport));
        auto v1 = (glm::project(vertices[i * 3 + 1].mPos, model, projection, viewport));
        auto v2 = (glm::project(vertices[i * 3 + 2].mPos, model, projection, viewport));


        //drawLine(int16_t(v0.x),int16_t(v0.y),int16_t(v1.x),int16_t(v1.y), OLED_COLOR_WHITE);
        //drawLine(int16_t(v2.x),int16_t(v2.y),int16_t(v1.x),int16_t(v1.y), OLED_COLOR_WHITE);
        //drawLine(int16_t(v0.x),int16_t(v0.y),int16_t(v2.x),int16_t(v2.y), OLED_COLOR_WHITE);

        // Barycentric Algorithm

        glm::ivec2 min = glm::min(glm::min(tovec2(v0), tovec2(v1)), tovec2(v2));
        glm::ivec2 max = glm::max(glm::max(tovec2(v0), tovec2(v1)), tovec2(v2));
        for (int x = min.x; x <= max.x; x += 1) {
            for (int y = min.y; y <= max.y; y += 1) {
                glm::vec2 p(x, y);
                glm::vec3 bc = barycentric(tovec2(v0), tovec2(v1), tovec2(v2), p);
                if (!(bc.x < 0.f || bc.y < 0.f || bc.z < 0.f)) {
                    //printf("%f, %f, %f : %f, %f\n", w0, w1, w2, uv.x, uv.y);
                    glm::vec3 vertex = {p.x, p.y, v0.z * bc.x + v1.z * bc.y + v2.z * bc.z};
                    glm::vec3 bc_clip = glm::unProject(vertex, model, projection, viewport);
                    //bc_clip = bc_clip/(bc_clip.x + bc_clip.y + bc_clip.z);
                    glm::vec3 bc_attr = barycentric(tovec2(vertices[i * 3].mPos), tovec2(vertices[i * 3 + 1].mPos),
                                                    tovec2(vertices[i * 3 + 2].mPos), tovec2(bc_clip));
                    setPixel(int16_t(roundf(x)), int16_t(roundf(y)), frag(
                            vertices[i * 3].mUV * bc_attr.x +
                            vertices[i * 3 + 1].mUV * bc_attr.y +
                            vertices[i * 3 + 2].mUV * bc_attr.z
                    ) >= 128 ? OLED_COLOR_WHITE : OLED_COLOR_BLACK);
                }
            }
        }
    }
}

void Framebuffer::drawTextureWithTranform(const glm::mat4& model, ITexture* texture) {
    static std::vector<Vertex> vertices = {
            {{-2.f, 1.f,  0.f}, {0, 1}},
            {{-2.f, -1.f, 0.f}, {0, 0}},
            {{2.f,  1.f,  0.f}, {1, 1}},

            {{2.f,  1.f,  0.f}, {1, 1}},
            {{-2.f, -1.f, 0.f}, {0, 0}},
            {{2.f,  -1.f, 0.f}, {1, 0}}
    };
    static glm::mat4 projection = glm::perspective(45.f, 128.f / 64.f, 0.01f, 100.f);
    static glm::vec4  viewport = {0, 0, 128.f, 64.f};

    for (size_t i = 0; i < vertices.size() / 3; ++i) {
        auto v0 = (glm::project(vertices[i * 3].mPos, model, projection, viewport));
        auto v1 = (glm::project(vertices[i * 3 + 1].mPos, model, projection, viewport));
        auto v2 = (glm::project(vertices[i * 3 + 2].mPos, model, projection, viewport));


        //drawLine(int16_t(v0.x),int16_t(v0.y),int16_t(v1.x),int16_t(v1.y), OLED_COLOR_WHITE);
        //drawLine(int16_t(v2.x),int16_t(v2.y),int16_t(v1.x),int16_t(v1.y), OLED_COLOR_WHITE);
        //drawLine(int16_t(v0.x),int16_t(v0.y),int16_t(v2.x),int16_t(v2.y), OLED_COLOR_WHITE);

        // Barycentric Algorithm

        glm::ivec2 min = glm::min(glm::min(tovec2(v0), tovec2(v1)), tovec2(v2));
        glm::ivec2 max = glm::max(glm::max(tovec2(v0), tovec2(v1)), tovec2(v2));
        for (int x = min.x; x <= max.x; x += 1) {
            for (int y = min.y; y <= max.y; y += 1) {
                glm::vec2 p(x, y);
                glm::vec3 bc = barycentric(tovec2(v0), tovec2(v1), tovec2(v2), p);
                if (!(bc.x < 0.f || bc.y < 0.f || bc.z < 0.f)) {
                    //printf("%f, %f, %f : %f, %f\n", w0, w1, w2, uv.x, uv.y);
                    glm::vec3 vertex = {p.x, p.y, v0.z * bc.x + v1.z * bc.y + v2.z * bc.z};
                    glm::vec3 bc_clip = glm::unProject(vertex, model, projection, viewport);
                    //bc_clip = bc_clip/(bc_clip.x + bc_clip.y + bc_clip.z);
                    glm::vec3 bc_attr = barycentric(tovec2(vertices[i * 3].mPos), tovec2(vertices[i * 3 + 1].mPos),
                                                    tovec2(vertices[i * 3 + 2].mPos), tovec2(bc_clip));
                    glm::vec2 uv = vertices[i * 3].mUV * bc_attr.x +
                                   vertices[i * 3 + 1].mUV * bc_attr.y +
                                   vertices[i * 3 + 2].mUV * bc_attr.z;
                    setPixel(int16_t(roundf(x)), int16_t(roundf(y)), texture->getTextureValue(uv) >= 128 ? OLED_COLOR_WHITE : OLED_COLOR_BLACK);
                }
            }
        }
    }
}

uint8_t Framebuffer::getTextureValue(const glm::vec2& uv) {
    return getPixel(int16_t(roundf(uv.x * 128)), int16_t(roundf(uv.y * 64))) == OLED_COLOR_WHITE ? uint8_t(255)
                                                                                                 : uint8_t(0);
}
