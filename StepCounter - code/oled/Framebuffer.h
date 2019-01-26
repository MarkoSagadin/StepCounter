/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#include <stdint.h>
#ifndef SIMULATOR
#include <avr/pgmspace.h>
#endif
#include "SSD1306.h"


class Framebuffer {
public:
    Framebuffer();
    void drawBitmap(const uint8_t *bitmap, uint8_t height, uint8_t width, uint8_t pos_x, uint8_t pos_y);
    void drawBuffer(const uint8_t *buffer);
    void drawPixel(uint8_t pos_x, uint8_t pos_y, uint8_t pixel_status);
    void drawPixel(uint8_t pos_x, uint8_t pos_y);
    void drawVLine(uint8_t x, uint8_t y, uint8_t length);
    void drawHLine(uint8_t pos_y, uint8_t start, uint8_t length);
    void drawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
    void drawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fill);
    void invert(uint8_t status);
    void clear();
    void show();
	/////////////////////////////////////////////
	void drawChar(char c);
	void drawString(char *text);
	void setCursor(uint8_t x, uint8_t y);
	void setFont(bool p);
	void drawNumber(long num);
	uint8_t getDigit(long num);
	void drawFloat(float float_num);
private:
    uint8_t buffer[1024];
    SSD1306 oled;
	uint8_t cursor_x = 0;
	uint8_t cursor_y = 0;
	bool pisava = 0; //Èe je niè je majhna pisava, èe je 1 so na voljo velike številke
};