

#ifndef ACCEL_H_
#define ACCEL_H_

#ifdef __cplusplus
extern "C" {
	#endif
	
#include <avr/io.h>
#include <math.h> 
#include <stdbool.h>

#define xOut 3	 //Change according to adc channels
#define yOut 4	 //These are channels 11, 12, 13,
#define zOut 5	 //Inside adcRead function Mux5 bit activates them
	
	
/*-----------------------------------------*/
/* Prototypes for accelometer data readings*/	

extern int sample_window[50];
extern int maximum_value;
extern int minimum_value;
extern int threshold_value;
extern int step_count;
extern bool previous_positive;
//////////////////////////////////////////////////////////
int adcRead(uint8_t channel);
float rawtoForce(int raw);
float rawtoForceCalibrated(int raw, char axis);	
int moving_average(int lastdata);
void update_values(int *maximum_value, int *minimum_value, int *threshold_value, const int samples[]);
void usb_fix();						
void get_adcvalues(int *xVal, int *yVal, int *zVal);
int get_gvector(int xVal, int yVal, int zVal);
void update_values_rutine(int *maximum_value, int *minimum_value, int *threshold_value, const int samples[], int gVal);
int figure_out_step(int gVal);
#ifdef __cplusplus
}
#endif
#endif /* ACCEL_H_ */