//
// Created by alex2772 on 13.12.18.
//


#pragma once


#include "Window.h"

#include <glm/glm.hpp>

class D3Window: public Window {
private:
    glm::mat4 mProjection;
    glm::vec4 mViewport;

    glm::vec3 transform(const glm::mat4& trans, const glm::vec3& v);

    float mRotX = 0;
    float mRotY = 0;
    float mRotZ = 0;
    float mScale = 1.f;
public:
    D3Window();

    virtual void render(Framebuffer &framebuffer);

    void drawLine(Framebuffer &framebuffer, glm::vec3 p1, glm::vec3 p2);
};