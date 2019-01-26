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

#include "Framebuffer.h"
#include "font.h"
#include <string.h>
#include <avr/io.h>
#include <math.h>
#include <stdlib.h>


Framebuffer::Framebuffer() {
    this->clear();
}

#ifndef SIMULATOR
void Framebuffer::drawBitmap(const uint8_t *progmem_bitmap, uint8_t height, uint8_t width, uint8_t pos_x, uint8_t pos_y) {
    uint8_t current_byte;
    uint8_t byte_width = (width + 7)/8;

    for (uint8_t current_y = 0; current_y < height; current_y++) {
        for (uint8_t current_x = 0; current_x < width; current_x++) {
            current_byte = pgm_read_byte(progmem_bitmap + current_y*byte_width + current_x/8);
            if (current_byte & (128 >> (current_x&7))) {
                this->drawPixel(current_x+pos_x,current_y+pos_y,1);
            } else {
                this->drawPixel(current_x+pos_x,current_y+pos_y,0);
            }
        }
    }
}

void Framebuffer::drawBuffer(const uint8_t *progmem_buffer) {
    uint8_t current_byte;

    for (uint8_t y_pos = 0; y_pos < 64; y_pos++) {
        for (uint8_t x_pos = 0; x_pos < 128; x_pos++) {
            current_byte = pgm_read_byte(progmem_buffer + y_pos*16 + x_pos/8);
            if (current_byte & (128 >> (x_pos&7))) {
                this->drawPixel(x_pos,y_pos,1);
            } else {
                this->drawPixel(x_pos,y_pos,0);
            }
        }
    }
}
#endif

void Framebuffer::drawPixel(uint8_t pos_x, uint8_t pos_y, uint8_t pixel_status) {
    if (pos_x >= SSD1306_WIDTH || pos_y >= SSD1306_HEIGHT) {
        return;
        }

    if (pixel_status) {
        this->buffer[pos_x+(pos_y/8)*SSD1306_WIDTH] |= (1 << (pos_y&7));
    } else {
        this->buffer[pos_x+(pos_y/8)*SSD1306_WIDTH] &= ~(1 << (pos_y&7));
    }
}

void Framebuffer::drawPixel(uint8_t pos_x, uint8_t pos_y) {
    if (pos_x >= SSD1306_WIDTH || pos_y >= SSD1306_HEIGHT) {
        return;
    }

    this->buffer[pos_x+(pos_y/8)*SSD1306_WIDTH] |= (1 << (pos_y&7));
}

void Framebuffer::drawVLine(uint8_t x, uint8_t y, uint8_t length) {
    for (uint8_t i = 0; i < length; ++i)
{
        this->drawPixel(x,i+y);
    }
}

void Framebuffer::drawHLine(uint8_t x, uint8_t y, uint8_t length) {
    for (uint8_t i = 0; i < length; ++i) {
        this->drawPixel(i+x,y);
    }
}

void Framebuffer::drawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    uint8_t length = x2 - x1 + 1;
    uint8_t height = y2 - y1;

    this->drawHLine(x1,y1,length);
    this->drawHLine(x1,y2,length);
    this->drawVLine(x1,y1,height);
    this->drawVLine(x2,y1,height);
}

void Framebuffer::drawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t fill) {
    if (!fill) {
        this->drawRectangle(x1,y1,x2,y2);
    } else {
        uint8_t length = x2 - x1 + 1;
        uint8_t height = y2 - y1;

        for (int x = 0; x < length; ++x) {
            for (int y = 0; y <= height; ++y) {
                this->drawPixel(x1+x,y+y1);
            }
        }
    }
}

void Framebuffer::clear() {
    for (uint16_t buffer_location = 0; buffer_location < SSD1306_BUFFERSIZE; buffer_location++) {
        this->buffer[buffer_location] = 0x00;
    }
}

void Framebuffer::invert(uint8_t status) {
    this->oled.invert(status);
}

void Framebuffer::show() {
    this->oled.sendFramebuffer(this->buffer);
}
///////////////////////////////////////////////////////////////////////777
//Moje funkcije

void Framebuffer::drawChar(char c)
{
	uint8_t vrstica;
	uint8_t st = c;
	
	if(pisava == 0)
	{
		for(int i = 0; i < 5; i++)						//z i-jem se premikaš skozi vrstico petih elemntov, ki predstavljajo en znak
		{
			vrstica = pgm_read_byte(&font[st*5+i]);		//Premika se skozi font spremenljivko in shrani vrednost v vrstico. 
														//Ker je font shranjen v FLASH pomnilniku je potrebno dostopati do vrednosti z pgm_read_byte() ukazom.
			for(int j = 0; j < 8; j++)					//Premikaš se skozi vrstico
			{
				if(vrstica & (1<<j))					//Za vsaki bit preverjaš vrednost in postavljaš piksel na doloèeno lokacijo
				{
					this->drawPixel(i + cursor_x, j + cursor_y, 1);
				}
				else
				{
					this->drawPixel(i + cursor_x, j + cursor_y, 0);
				}	
			}	
		}
	}
	else
	{
		st = st - 48;						//Da zaradi ascii tabele izenaèim z zamikom
		for(int i = 0; i < 17; i++) 
		{	
			for (int j = 0; j < 2; j++)
			{
				vrstica = pgm_read_byte(&arial[st*34+i*2+j]);
				
				for(int k = 0; k < 8; k++)
				{
					if(vrstica & (1<<k))	//Za vsaki bit preverjaš vrednost in postavljaš piksel na doloèeno lokacijo
					{
						this->drawPixel(cursor_x + 7 - k + j*8, cursor_y + i, 1);
					}
					else
					{
						this->drawPixel(cursor_x + 7 - k + j*8, cursor_y + i, 0);
					}
				}
			}
		}
		
	}	
}
void Framebuffer::drawString(char *text)
{	
	uint8_t sirina;
	
	if(pisava == 0)
	{
		sirina = 6;
	}
	else
	{
		sirina = 13;
	}
	
	for(uint8_t i=0; i<strlen(text); i++)
	{
		if (i == 0)
			this->setCursor(cursor_x, cursor_y);
		else
			this->setCursor(cursor_x + sirina, cursor_y);
		
		this->drawChar(text[i]);
	}
}

void Framebuffer::setCursor(uint8_t x, uint8_t y) 
{
	cursor_x = x;
	cursor_y = y;
}

void Framebuffer::setFont(bool p)
{
	pisava = p;
}

void Framebuffer::drawNumber(long num)
{
	int lenght = this->getDigit(num);
	int stevke_array[lenght]; //Prilagodim velikost stevke_array na število števk iz danega argumenta
	uint8_t sirina;
	//razclenitev vhodne spremenljivke na vsako stevko

	for(int i=lenght-1; i>=0; i--)
	{
		
		int digit = num % 10;
		stevke_array[i] = digit;
		num = floor(num/10);
	}
	
	if(pisava == 0)
	{
		sirina = 6;
	}
	else
	{
		sirina = 13;
	}
	
	for(int i=0; i<lenght; i++)
	{
		char stevka = '0' + stevke_array[i]; //potreben offset
		if (i == 0)
			this->setCursor(cursor_x, cursor_y);
		else
			this->setCursor(cursor_x+sirina, cursor_y);
		
		this->drawChar(stevka);
	}
	setCursor(0,0);
}
uint8_t Framebuffer::getDigit(long num)
{
	if ( num < 10 )
		return 1;
	if ( num < 100 )
		return 2;
	if ( num < 1000 )
		return 3;
	if ( num < 10000 )
		return 4;
	if ( num < 100000 )
		return 5;
	if ( num < 1000000 )
		return 6;
	if ( num < 10000000 )
		return 7;
	else
		return 1;	
	
}

void Framebuffer::drawFloat(float float_num)
{
	int num = floor(float_num*1000);	//prikazujem samo do treh decimalk
	int lenght = 4;						//Samo do štiri števke
	int stevke_array[lenght];			//Prilagodim velikost stevke_array na število števk iz danega argumenta
	uint8_t sirina;
	
	if (float_num < 0)
	{
		this->setCursor(cursor_x, cursor_y);
		this->drawChar('-');
	}
	else
	{
		this->setCursor(cursor_x, cursor_y);
		this->drawChar('+');
	}

	num = abs(num);
	
	//razclenitev vhodne spremenljivke na vsako stevko
	for(int i=lenght-1; i>=0; i--)
	{
		
		int digit = num % 10;
		stevke_array[i] = digit;
		num = floor(num/10);
	}
	
	if(pisava == 0)
	{
		sirina = 6;
	}
	else
	{
		sirina = 13;
	}
	
	this->setCursor(cursor_x+7, cursor_y);
	for(int i=0; i<lenght; i++)
	{
		char stevka = '0' + stevke_array[i];  //potreben offset
		if (i == 0)
		{
			this->setCursor(cursor_x, cursor_y);
			this->drawChar(stevka);
			this->setCursor(cursor_x + 6, cursor_y);
			this->drawChar('.');
		}
		else
		{
			this->setCursor(cursor_x + sirina, cursor_y);
			this->drawChar(stevka);
		}
	}
	setCursor(0,0);
}