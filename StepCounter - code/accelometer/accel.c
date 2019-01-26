/*
 * accel.c
 *
 * Created: 31. 05. 2018 12:15:54
 *  Author: Marko
 */ 

#include "accel.h"

int sample_window[50];
int maximum_value = 0;
int minimum_value = 0;
int threshold_value = 0;
int step_count = 0;
bool previous_positive = false;

int adcRead(uint8_t channel)
{
	int adcValue;
	ADMUX &= 0xC0;					 //Clear  lower six bits, to prevent updating previous values
	ADMUX |= channel;				 // Select channel based on function input
	ADMUX &= ~(1 << REFS1) & ~(1 << REFS0);   		 // use ARef as the reference
	ADCSRB |= (1<<MUX5);			//You need this for channels from ADC8 and on
	
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale
	
	ADCSRA |= (1 << ADEN);    		 // Enable the ADC
	ADCSRA |= (1 << ADSC);   		 // Start the ADC conversion
	while(ADCSRA & (1 << ADSC));     // Wait for the ADC conversion to finish
	
	adcValue = ADCL;
	adcValue = (ADCH << 8) + adcValue; // Transfer data values into a variable

	return adcValue;
}

float rawtoForce(int raw)
{
	float f_raw = raw;
	float force = (f_raw/1023)*6-3;		//converting raw value to force
	return force;
}

float rawtoForceCalibrated(int raw, char axis)
{
	float force = 0;
	
	//Linear interpolation for each axis
	switch(axis)
	{
		case 'x':
		force = 2.0/(612.0-408.0)*(raw-408)-1;
		break;
		case 'y':
		force = (2.0/(607.0-400.0))*(raw-400)-1;
		break;
		case 'z':
		force = (2.0/(613.0-410.0))*(raw-410)-1;
		break;
	}
	return force;
}

int moving_average(int lastdata)
{
	const int window_size = 9;							//YOU CAN CHANGE THAT!!
	static int window[9];						//Size of averaging window
	static unsigned char index = 0;						//Helps to fill up array when it is empty
	float sum = 0;										//Sum of all values in array
	static int previous_average;
	static int new_average;
	
	if(index != window_size)							//Procedure at beginning when array is not fully filled yet
	{
		window[index]=lastdata;							//Filling up the array
		for(unsigned char i = 0; i < window_size; i++)
		{
			sum += window[i];							//Sum up all elements
		}
		index++;
		return new_average = round(sum/(index));		//Average them and return
	}
	else
	{
		for(unsigned char i = 0; i < window_size; i++)	//Shifting array elements to left for one place
		{
			if(i != (window_size - 1))
			window[i] = window[i+1];
			else
			window[i] = lastdata;						//Putting on the last place the newest data
		}
		previous_average = new_average;
		
		//Returning new average that is equal to previous average minus oldest element plus newest element.
		//Those two elements are divided by size of window
		//Cast float is needed for float division
		return new_average = previous_average + round((-window[0] + window[window_size-1])/(float)window_size);
	}
}

void update_values(int *maximum_value, int *minimum_value, int *threshold_value, const int samples[])
{
	//All values are passed by reference, no return is needed
	
	*maximum_value = samples[0];
	*minimum_value = samples[0];
	
	for(int i = 0; i < 50-1; i++)				//Finds the maximum value
	{
		if(*maximum_value < samples[i+1])
		*maximum_value = samples[i+1];
	}
	
	for(int i = 0; i < 50-1; i++)				//Finds the minimum value
	{
		if(*minimum_value > samples[i+1])
		*minimum_value = samples[i+1];
	}
	
	*threshold_value = round((*maximum_value + *minimum_value)/2.0);
}

void usb_fix()
{
	// Clear usb interrupt flags
	USBINT = 0;
	UDINT  = 0;
	for (uint8_t _i = 0; _i < 6; _i++)
	{ // For each USB endpoint
		UENUM = _i; // select the _i-th endpoint
		UEINTX = UEIENX = 0; // Clear interrupt flags for that endpoint
	}
}

void get_adcvalues(int *xVal, int *yVal, int *zVal)
{
	*xVal = adcRead(xOut);
	*yVal = adcRead(yOut);
	*zVal = adcRead(zOut);
}

int get_gvector(int xVal, int yVal, int zVal)
{
	return round(sqrt((long)xVal * (long)xVal + (long)yVal * (long)yVal + (long)zVal * (long)zVal));	
}

void update_values_rutine(int *maximum_value, int *minimum_value, int *threshold_value, const int samples[], int gVal)
{
	static int k = 0;			//Counter for updating max, min, threshold
	if(k == 50)
	{
		update_values(maximum_value, minimum_value, threshold_value, sample_window);
		k = 0;
	}
	sample_window[k] = gVal;
	k++;
}

int figure_out_step(int gVal)
{
	static int step_exclusion = 0; 
	
	step_exclusion++;
	if(gVal < threshold_value && previous_positive)
	{
		if(step_exclusion > 11)
		{
			step_count++;
			previous_positive = false;
			step_exclusion = 0;
		}
		step_exclusion = 0;
	}
	if(gVal > threshold_value)
	{
		previous_positive = true;
	}
	return step_count;
}