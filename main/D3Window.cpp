//
// Created by alex2772 on 13.12.18.
//

#include "D3Window.h"
#include "CCOSCore.h"
#include "calc.png.h"

//#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

D3Window::D3Window() : Window("3D") {
    mProjection = glm::perspective(45.f, 128.f / 64.f, 0.01f, 100.f);
    mViewport = glm::vec4(0, 0, 128.f, 64.f);
}

void printVec(Framebuffer &framebuffer, const glm::vec3 &v, int16_t x, int16_t y) {
    char buf[64];
    sprintf(buf, "%1.3f, %1.3f, %1.3f", v.x, v.y, v.z);
    framebuffer.drawString(x, y, buf, OLED_COLOR_WHITE, FONT_FACE_ROBOTO_8PT);
}

void D3Window::render(Framebuffer &framebuffer) {
    Window::render(framebuffer);
    mRotX += float(CCOSCore::getKeyState(6)) - float(CCOSCore::getKeyState(4));
    mRotY += float(CCOSCore::getKeyState(1)) - float(CCOSCore::getKeyState(9));
    mRotZ += float(CCOSCore::getKeyState(2)) - float(CCOSCore::getKeyState(0));
    mScale += (float(CCOSCore::getKeyState(8)) - float(CCOSCore::getKeyState(12))) / 20.f;
    glm::mat4 trans = glm::scale(glm::rotate(glm::rotate(
            glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -1.f)), glm::radians(mRotX),
                        glm::vec3(0, 1, 0)), glm::radians(mRotY), glm::vec3(1, 0, 0)), glm::radians(mRotZ),
                                             glm::vec3(0, 0, 1)), glm::vec3(mScale));

    mScale = glm::clamp(mScale, 0.1f, 2.5f);

/*
    auto p1 = transform(trans, {-1, -1, 1});
    auto p2 = transform(trans, {1, -1, 1});
    auto p3 = transform(trans, {-1, 1, 1});
    auto p4 = transform(trans, {1, 1, 1});
    auto p5 = transform(trans, {-1, -1, -1});
    auto p6 = transform(trans, {1, -1, -1});
    auto p7 = transform(trans, {-1, 1, -1});
    auto p8 = transform(trans, {1, 1, -1});

    printVec(framebuffer, p1, 0, 12);
    printVec(framebuffer, p2, 0, 12 + 8 * 1);
    printVec(framebuffer, p3, 0, 12 + 8 * 2);
    printVec(framebuffer, p4, 0, 12 + 8 * 3);

    drawLine(framebuffer, p1, p2);
    drawLine(framebuffer, p2, p4);
    drawLine(framebuffer, p3, p4);
    drawLine(framebuffer, p1, p3);
    drawLine(framebuffer, p1, p5);
    drawLine(framebuffer, p2, p6);
    drawLine(framebuffer, p3, p7);
    drawLine(framebuffer, p4, p8);
    drawLine(framebuffer, p5, p6);
    drawLine(framebuffer, p6, p8);
    drawLine(framebuffer, p7, p8);
    drawLine(framebuffer, p5, p7);
*/

    /*
    glm::mat4 trans = glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -10.f)),
                                  glm::radians(esp_timer_get_time() / 10000.f), glm::vec3(0, 1, 0));

    auto p1 = transform(trans, {-1, -1, 1});
    auto p2 = transform(trans, {1, -1, 1});
    auto p3 = transform(trans, {-1, 1, 1});
    auto p4 = transform(trans, {1, 1, 1});

    printVec(framebuffer, p1, 0, 12);
    printVec(framebuffer, p2, 0, 12 + 8 * 1);
    printVec(framebuffer, p3, 0, 12 + 8 * 2);
    printVec(framebuffer, p4, 0, 12 + 8 * 3);

    drawLine(framebuffer, p1, p2);
    drawLine(framebuffer, p2, p4);
    drawLine(framebuffer, p3, p4);
    drawLine(framebuffer, p1, p3);

    */
    framebuffer.drawArrays(trans, {
            {{-1.f, 1.f,  0.f}, {0, 1}},
            {{-1.f, -1.f, 0.f}, {0, 0}},
            {{1.f,  1.f,  0.f}, {1, 1}},

            {{1.f,  1.f,  0.f}, {1, 1}},
            {{-1.f, -1.f, 0.f}, {0, 0}},
            {{1.f,  -1.f, 0.f}, {1, 0}}
    }, [](const glm::vec2 &uv) {
        static Bitmap bitmap = {calc, 30, 30};
        return bitmap.getPixelAt(uint16_t(roundf(uv.x * bitmap.width)),
                                                              uint16_t(roundf(uv.y * bitmap.height))) * 128;
    });
}

glm::vec3 D3Window::transform(const glm::mat4 &trans, const glm::vec3 &v) {
    return glm::project(v, trans, mProjection, mViewport);
}

void D3Window::drawLine(Framebuffer &framebuffer, glm::vec3 p1, glm::vec3 p2) {
    framebuffer.drawLine(int16_t(p1.x), int16_t(p1.y), int16_t(p2.x), int16_t(p2.y), OLED_COLOR_WHITE);
}
