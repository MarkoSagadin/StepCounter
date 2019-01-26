#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h> 

#include "petitfs/pff.h"
#include "petitfs/diskio.h"
#include "uart.h"
#include "oled/Framebuffer.h"
#include "accelometer/accel.h"

#define EXAMPLE_FILENAME "example.txt"


#define LED_IS_ON() !(PINB & (1<<PORTB0))

#define BUFFER_SIZE 300
#define update 65535 - 2500		//Updating accelometer values every 20 ms
#define gumbPool 255-125		//Checking buttons every 1 ms

// Global variables for writing to SD card
FATFS file_system;
char write_buffer[BUFFER_SIZE] = "";
UINT byte_counter = 0;

void init_sd_card(void);
void timer_init();

// Object used to write to OLED disply
Framebuffer fb;

int xVal;		//x - axis vector
int yVal;		//y - axis vector
int zVal;		//z - axis vector
int gVal;		//General acceleration vector

// Used to turn on and of the screen
bool onOffScreen = 1;

char buf[5];

ISR (TIMER0_OVF_vect)          //Timer interrupt for button debounce
{
	static uint8_t pushDownStart =  0;
	static uint8_t pushDownSelect =  0;

	if(PIND & 1<<PORTD7)
	{
		pushDownStart++;
		if(pushDownStart == 20)
		{
			pushDownStart = 0;			
			///////////////////
			if(onOffScreen)
			step_count = 0;	
			///////////////////
			while(PIND & 1<<PORTD7);
		}
	}
	
	if(PIND & 1<<PORTD6)
	{
		pushDownSelect++;
		if(pushDownSelect == 20)
		{
			pushDownSelect = 0;
			///////////////////
			onOffScreen ^= 1;
			///////////////////
			while(PIND & 1<<PORTD6);
		}
	}			
}

ISR(TIMER1_OVF_vect)			
{	
	static int i = 0;			//Counter for uploading
	static int j = 0;			//Counter for multiplying
	
	get_adcvalues(&xVal, &yVal, &zVal);		//Read accelometer values 
	
	if(j % 50 == 0)
	{
		PORTD ^= (1<<PORTD3);
		_delay_ms(100);
		PORTD ^= (1<<PORTD3);
	}	
	
	gVal = get_gvector(xVal, yVal, zVal);   //General acceleration vector
	gVal = moving_average(gVal);			//Filter vector with moving average 
	
	
	update_values_rutine(&maximum_value, &minimum_value, &threshold_value, sample_window, gVal);
	 
	step_count = figure_out_step(gVal); 
	
	i++;
	
	// Following commented sections saves several variables to write_buffer nad then writes it to SD card.
	// It was used during the development to check if the algorithm works, in final application is disabled as is not needed. 
	
	//sprintf(buf, "%d", gVal);
	//strcat(write_buffer, buf);
	//strcat(write_buffer, " ");
	//
	//sprintf(buf, "%d", maximum_value);
	//strcat(write_buffer, buf);
	//strcat(write_buffer, " ");
	//
	//sprintf(buf, "%d", minimum_value);
	//strcat(write_buffer, buf);
	//strcat(write_buffer, " ");	
	//
	//sprintf(buf, "%d", threshold_value);
	//strcat(write_buffer, buf);
	//strcat(write_buffer, " ");
	//
	//sprintf(buf, "%d", step_count);
	//strcat(write_buffer, buf);
	//strcat(write_buffer, "\n");
	//
	//
	//
	//i++;
	//if(i == 10)
	//{
	//
		///* Write buffer */
		//pf_write(write_buffer, BUFFER_SIZE, &byte_counter);
		//if (byte_counter < BUFFER_SIZE) {
			///* End of file */
		//}
		//write_buffer[0]= '\0';			//Clean up buffer
		//j++;
		//i = 0;
	//}
	//
	//if( j == 50)
	//{
		//TIMSK1 &= ~(1 << TOIE1);		//Turn off timer
		///* Finalize write */
		//pf_write(0, 0, &byte_counter);
		//
		//PORTD |= (1<<PORTD3);
		//_delay_ms(100);
		//PORTD ^= (1<<PORTD3);
		//_delay_ms(100);
		//PORTD ^= (1<<PORTD3);
		//_delay_ms(100);
		//PORTD ^= (1<<PORTD3);
		//
		//fb.clear();
		//fb.setCursor(0, 0);
		//fb.drawString("DONE");
		//fb.show();
	//}
	
	
	TCNT1 = update;
}
	
int main(void)
{
	
	DDRD |= (1<<PORTD3);	//Led
	PORTD &= ~(1<<PORTD3);
	
	DDRD &= ~(1<<PORTD7);	//Button Start
	DDRD &= ~(1<<PORTD6);	//Button Select
	
	_delay_ms(20);
		
	
	usb_fix();				//Needed to remove USB priority
		
	timer_init();
	
	//* Initialize card */
	//init_sd_card();		//Commented out, not needed in final application
	
	while (1)
	{
		// functions for drawing shapes and string to OLED display 
		if(onOffScreen)	
		{
			//Screen is on
			fb.drawRectangle(0,0,127,63);
			fb.drawRectangle(2,2,125,61);
		
			fb.setCursor(7,8);
			fb.drawString("Steps ");
			fb.setCursor(7,16);
			fb.drawString("taken ");
			fb.setCursor(43,12);
			fb.drawString("=");
			fb.setCursor(55,8);
			fb.setFont(true);
			fb.drawNumber(step_count);		//Write step count
			fb.drawHLine(3, 30, 128-5);

			fb.setFont(false);
			fb.setCursor(4,34);
			fb.drawString("Press STR to reset");
			fb.setCursor(4,42);
			fb.drawString("Press SEL to turn");
			fb.setCursor(82,50);
			fb.drawString("off");
			fb.show();
			_delay_ms(100);
			fb.clear();
		}
		else
		{
			//Screen is off
			_delay_ms(100);
			fb.clear();
			fb.show();
		}
	}
}

void init_sd_card(void)
{
	DSTATUS status;
	FRESULT result;
	bool flag = false; 
	
	/* Initialize physical drive */
	do {
		status = disk_initialize();
		if (status != 0) {
			//uartSenddata("status je 1 ");
			//PORTB |= (1<<PORTB0);
			
			fb.setCursor(0,0);
			fb.drawString("Status je 1");
			fb.show();
			
			flag = false;
		} else {
			//uartSenddata("status je 0 ");
			//PORTB &= ~(1<<PORTB0);
			/* Double SPI clock after initialization */
			SPCR |= (1<<SPE) | (1<<MSTR) | (1<<SPR1);
			SPSR |= (1<<SPI2X);
					
			fb.setCursor(0,0);
			fb.drawString("Status je 0");
			fb.show();
			_delay_ms(500);
			
			flag = true;						//Prescale is now fcpu/32
			
		}
		/* The application will continue to try and initialize the card.
		 * If the LED is on, try taking out the SD card and putting
		 * it back in again.  After an operation has been interrupted this is
		 * sometimes necessary.
		 */
	} while (flag == false);
	
	fb.clear();
	fb.setCursor(0,0);
	fb.drawString("Prisel ven iz zanke");
	fb.show();
	_delay_ms(1000);

	
	/* Mount volume */
	result = pf_mount(&file_system);
	
	
	fb.clear();
	fb.setCursor(0,0);
	fb.drawString("Mountal");
	fb.show();
	_delay_ms(1000);
	
	if (result == FR_OK)
	{
		//uartSenddata("MOUNT OK    ");	
		//
		fb.clear();
		fb.setCursor(0,0);
		fb.drawString("MOUNT OK");
		fb.show();
		_delay_ms(1000);
		
	}
	else
	{
		//uartSenddata("MOUNT NOT OK    ");
		fb.clear();
		fb.setCursor(0,0);
		fb.drawString("MOUNT NOT OK");
		fb.show();
		_delay_ms(1000);
	}
	/* Open file */
	result = pf_open(EXAMPLE_FILENAME);
	//if (result != FR_OK)
		//PORTB |= (1<<PORTB0);
	if (result == FR_OK)
	{
		uartSenddata("OPENING OK");
		fb.clear();
		fb.setCursor(0,0);
		fb.drawString("OPENING OK");
		fb.show();
		_delay_ms(1000);
	}
	else
	{
		uartSenddata("OPENING NOT OK");
		fb.clear();
		fb.setCursor(0,0);
		fb.drawString("OPENING NOT OK");
		fb.show();
		_delay_ms(1000);
	}
}

void timer_init()
{
	//////////////////////////////////
	//Timer setup for counting steps//
	//////////////////////////////////
	TCNT1 = update;   
	TCCR1A = 0x00;
	TCCR1B = (1<<CS10) | (1<<CS11);  
	TIMSK1 = (1 << TOIE1) ;   
	
	//////////////////////////////////////
	//Timer setup for button debouncing //
	//////////////////////////////////////
	TCNT0 = gumbPool;   
	TCCR0A = 0x00;
	TCCR0B |= (1<<CS00) | (1<<CS01);  
	TIMSK0 = (1 << TOIE1) ; 
	sei();        
}


