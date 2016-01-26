
/* AVR oscilloscope with 128x64 graphic LCD.

   Created by Serasidis Vassilis and finished at 03.Aug.2007
   Home: http://www.serasidis.gr
   e-mail: info@serasidis.gr and avrsite@yahoo.gr

   This code is ditributed under GPL licence.
*/


#include <avr/io.h>		// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>
#include "glcd.c"

void USART_Init(unsigned int baudrate);
void timePause(unsigned long count);
void ADC_init (void);
void initTimer (void);

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define msUp				1
#define msDwn				4      
#define YposUp				0
#define YposDwn				3
#define freeze				2

#define AC 					0
#define DC 					1
#define SQUARE				2
#define TRUE				0
#define FALSE				1

unsigned int counter = 0;
unsigned int dataCounter = 0;
unsigned int timeDiv = 52; 
unsigned int analogInput = 0;
unsigned char trigger = 2;
unsigned char cnt = 0;
unsigned char empty = 0;
unsigned int out = 0;
unsigned char findZero = 0;
unsigned char pressedButton = 0;
unsigned char upLimit = 0;
unsigned char dnLimit = 255;
unsigned char limitBkup = 0;
unsigned char  voltageType = AC;
unsigned char  complete = TRUE;

signed char Ypos   = 0; 
signed char Ypos2   = 0;
signed char position  = 0;

int main (void)
{
	DDRC = 0b00000000; 
	PORTC = 0b11111111;

	DDRA = 0b00000000; 

	unsigned char temp1;
	unsigned int temp2;
	unsigned char i;

	glcdInit();
	ADC_init();
	createWelcomeScreen();
	showTheWave();
	timePause(10000000);

	for(;;)
	{
	//---------


			if (pressedButton == 0)
			{
	        	if (~PINC & (1<<msUp) && (timeDiv <= 1000))
				{
					if(timeDiv == 0)
						timeDiv = 52;
					else
	            		timeDiv += 145;
					pressedButton = 1;
				}
				
				if (~PINC & (1<<msDwn) && (timeDiv >= 52))
				{
					if(timeDiv <= 145)
						timeDiv = 0;
					else
						timeDiv -= 145;
					pressedButton = 1;
				}
			
				if (~PINC & (1<<YposUp) && (Ypos2 <= 60))
	            	Ypos2++;
				
				if (~PINC & (1<<YposDwn) && (Ypos2 >= -60))
					Ypos2--;

				if (~PINC & (1<<freeze))
					while (~PINC & (1<<freeze)); // It freezes the display to watch the wave.
				
			}
			else
			{
				temp1 = PINC;
				if (temp1 == 255)
					pressedButton = 0;
			}
			
//--------v
// This routine makes the trigger. 

			dataCounter = 0;
			findZero = 0;
			upLimit = 0;
			dnLimit = 255;


		for (i=99; i>0; i--)
		{
			ADCSRA |= (1 << ADSC); // Enable ADC
			loop_until_bit_is_set(ADCSRA, ADIF);		// wait until conversion complete.
			temp1 = ADCL;
			temp2 = ADCH;
			timePause(timeDiv); //(7);

			if (upLimit < temp2) // find the higher voltage level of the input waveform.
				upLimit = temp2;
			if (dnLimit > temp2) // find the lower voltage level of the input waveform.
				dnLimit = temp2;

			if (temp2 > 0)
			{
				temp2 += 5;
				temp2 /= 5;
				temp2 += 2;
			}else temp2 = 2; 
			
			position = temp2 + Ypos2 +5; 
			if (position <= 63 && position >= 0)  // Adjust Up-Down the wave.
				fillDataLcdBuffer(i,position);
			else
				fillDataLcdBuffer(i,0);

		}
		if(upLimit != dnLimit)
			trigger = (((upLimit - dnLimit)/2)+ dnLimit); // find the middle of the wave, to be used it as a trigger.
//--------^


		createRaster();
		createWave();
		showTheWave();

				
				dataCounter = 0;
				do 
				{
					limitBkup = temp2;

					ADCSRA |= (1 << ADSC); // Enable ADC
					loop_until_bit_is_set(ADCSRA, ADIF);
					temp1 = ADCL;
					temp2 = ADCH;
					//timerPause(timeDiv); 

					if(limitBkup == temp2) 
					{
						dataCounter++;
						if(dataCounter >= 500)
							voltageType = DC;
						else
							voltageType = AC;
					}

					if((voltageType == AC) && ((temp2 == trigger) && (limitBkup < temp2)))
						complete = TRUE;
					else
					if((voltageType == DC) && (limitBkup == temp2) && (upLimit != dnLimit))
					{
						dataCounter = 0;
						do
						{
							ADCSRA |= (1 << ADSC); // Enable ADC
							loop_until_bit_is_set(ADCSRA, ADIF);
							temp1 = ADCL;
							temp2 = ADCH;
							complete = TRUE;
							dataCounter++;
						}while ((temp2 > trigger) && (dataCounter <1000));
					
						dataCounter = 0;
						do
						{
							ADCSRA |= (1 << ADSC); // Enable ADC
							loop_until_bit_is_set(ADCSRA, ADIF);
							temp1 = ADCL;
							temp2 = ADCH;
							complete = TRUE;
							dataCounter++;
						}while ((temp2 < trigger) && (dataCounter <1000));
					}
					else
					if((voltageType == DC) && (limitBkup == temp2))
						complete = TRUE;
					else
						complete = FALSE;
										

				} while(complete == FALSE);

	}	
}

//===============================================================================
//
//===============================================================================

void ADC_init (void)
{
	ADMUX = 0b01100000; // PA0 -> ADC0, ADLAR=1 (8bit)
	ADCSRA = 0b11000100; // ADC prescaler at 16
}


//===============================================================================
//
//===============================================================================
void timePause(unsigned long count) 
{
    while(count--);
}

//===============================================================================
//
//===============================================================================


