//
// Created by alex2772 on 06.03.19.
//
/*
#include "SowlonyaWindow.h"
#include "TextView.h"
#include "CCOSCore.h"

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>

SowlonyaWindow::SowlonyaWindow():
    WindowScroll("")
{
    mDrawTitle = false;
    TextView* text = new TextView();
    text->setText("Сонечка.. Поздравляю тебя с 8 марта и дарю тебе этот скромный подарок. Хехе.. я всю зиму на него потратил. Дарю этот калькулятор тебе. Пятeрка. Единственный экземпляр. Крайне сложный в производстве, из-за чего было убито 2 экрана, зато надeжный. Надеюсь, он прослужит тебе долго и не сломается спустя 2 дня эксплуатации. Мяу... Сонька... моя прелесть.. моя радость.. хорошая девочка... любимая.. ты не представляешь, как я тебя люблю.. Муррр.. с праздником тебя. <3 :-* :-3 ^^");
    addView(text);
}

void SowlonyaWindow::render(Framebuffer &fb) {
    WindowScroll::render(fb);
    mAnimation += 0.05f;
    glm::mod(mAnimation, glm::pi<float>() * 2.f);
    float scroll = getScroll();
    mIndicator.set(0.5f * (scroll), 0.5f * (1.f - scroll), 0.5f + glm::sin(mAnimation) * 0.5f);
}

void SowlonyaWindow::keyLongDown(uint8_t key) {
    Window::keyLongDown(key);
    if (key == 15) {
        bool s = false;
        CCOSCore::getNVSHandle().open().write("sowl", s);
        close();
    }
}
*/