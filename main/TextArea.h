#pragma once
#include "View.h"
#include "CCOSCore.h"
#include <string>
#include <algorithm>
#include <cstring>
#include "fonts.h"
#include "Indicator.h"

#define FONT_WIDTH 6
#define FONT_HEIGHT 12
#define FONT FONT_FACE_TERMINUS_6X12_ISO8859_1


class TextArea: public View
{
protected:
	Indicator mIndicator;
	std::string mText;
	unsigned char blink = 0;
	int64_t position = 0;
	short moving = 0;
	short moveTicker = 0;
    int64_t vertMovX = -1;
	uint8_t letterChoose = 0; // тикер
	uint8_t letter = 0; // индекс текущей буквы
	char curLetter = 0; // текущая буква
	uint8_t gpn = 0; // кнопка
	static bool shift;
	static bool mode; // TEXT; NUMBER
	int16_t offs = 0;
	int16_t offsY = 0;
	bool _skip_shift = false;

	virtual bool onSymbol(char& c);
	virtual void onTextChanged() {}
	void enterSymbol(char c);
	void insert(std::string c);
    template <typename T> int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
public:
Framebuffer::ci_info_struct ci;
	bool multiline = false;
	TextArea();
	~TextArea();
    int64_t column();
    int64_t row();
    void setPosition(int row, int column);
	virtual void render(Framebuffer& fb);
	void setText(const std::string& m);
	const std::string& getText();

	virtual void keyLongDown(uint8_t key);
	virtual void keyPressureChanged(uint8_t key);

	void switchMode();

	void switchShift();
	const char* getLettersForButton(uint8_t b);

	void iterateLetters();

	virtual void keyRelease(uint8_t key);
	virtual void focusLost();
	int64_t getPosition();
	void setPosition(short i);

    virtual void keyDown(uint8_t key);

    void updateIndicator();
	void setShift(bool shift);
	void setMode(bool mode);
};
