#pragma once

#include "Bitmap.h"
#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <vector>

extern "C" {
#include <cstdint>
#include "ssd1306.h"
}

#include "ITexture.h"
class Framebuffer: public ITexture {
public:
    struct ci_info_struct
    {
        int16_t x;
        int16_t y;
        size_t index;
    };

    uint8_t* buffer;

    typedef ssd1306_color_t Color;

    Framebuffer();
    Framebuffer(const Framebuffer& fb);
    Framebuffer(int16_t w);
    ~Framebuffer();

    void setCoordsOffset(int16_t x, int16_t y);

    void clear();
    void setPixel(int16_t x, int16_t y, Color color);
    int drawChar(int16_t x, int16_t y, const char c, Color color, int font);
    void drawString(int16_t x, int16_t y, std::string s, Color color = OLED_COLOR_WHITE, int font = 255,
                    bool border = false);
    void drawStringWithCharIndex(int16_t x, int16_t y, const char* string, int font, Color color, ci_info_struct& ci);
    void drawLine(int16_t x, int16_t y, int16_t x1, int16_t y1, Color color);

    void drawRect(int16_t x, int16_t y, uint8_t width, uint8_t height, Framebuffer::Color color);

    void drawImage(int16_t x, int16_t y, Bitmap &bitmap, bool mirrorX = false, Color color = OLED_COLOR_WHITE);
    void drawImageWithBorder(int16_t x, int16_t y, Bitmap &bitmap, bool mirrorX = false, Color color = OLED_COLOR_WHITE);
    char *drawStringML(int16_t x, int16_t y, char *str, int16_t width, int font, Color color);

    void roundedRect(int16_t x, int16_t y, uint16_t width, uint16_t height, Color color = OLED_COLOR_WHITE);
    void drawCircle(int16_t x, int16_t y, uint8_t radius, Color color = OLED_COLOR_WHITE);
    void snapshot(int16_t width, int16_t height, uint8_t* dst);
    void snapshot(int16_t width, int16_t height, uint8_t* dst, float scale);
    void shade();
    static int16_t length(font_face_t font, const char *str);
    struct Vertex {
        glm::vec3 mPos;
        glm::vec2 mUV;
    };
    void drawArrays(const glm::mat4& model, const std::vector<Vertex> &vertices,
                        const std::function<int(const glm::vec2&)> &frag);

    void drawBorder(int16_t x, int16_t y, uint8_t width, uint8_t height, Color color);
    void drawFB(int16_t x, int16_t y, uint16_t w, uint16_t h, Framebuffer* fb);
    void drawFBWithScale(int16_t x, int16_t y, uint16_t w, uint16_t h, Framebuffer* fb, float scale);

    Color getPixel(int16_t x, int16_t y);

    void drawTextureWithTranform(const glm::mat4& model, ITexture* texture);
private:
    virtual uint8_t getTextureValue(const glm::vec2 &uv);

private:
    int16_t offsetX = 0, offsetY = 0;
    void displayImagePart(int16_t x, int16_t y, int16_t width, int16_t u, int16_t v, const uint8_t *&data, Color);

};

