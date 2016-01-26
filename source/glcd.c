

/* As graphic LCD used the DEM128064A model (128x64 pixels).

   Created by Serasidis Vassilis and finished at 03.Aug.2007
   Home: http://www.serasidis.gr
   e-mail: info@serasidis.gr and avrsite@yahoo.gr

   This code is ditributed under GPL licence.
*/

#include "glcd.h"

void glcdInit (void)
{		 
		data_port_pins = 0;		//DATA Port Low
		data_port_ddr = 0xff;	//Make DATA port output

		ctrl_port = 0;			//CONTROL Port Low
		ctrl_port_ddr = 0xff;	//Make CONTROL port outputs

		ctrl_port |= (1<<lcdrst);
		ctrl_port |= (1<<lcdcs1);		 //Enable the CS1 of the display 
        ctrl_port |= (1<<lcdcs2);		 //Enable the CS2 of the display
        ctrl_port &= ~(1<<lcdrs);        //Clear RS  \_
        ctrl_port &= ~(1<<lcdrw);        //Clear RW  /  Command mode

		delayTime(10000);
		data_port = 0b11000000;		 //Display start line = 0 (0-63)
        eStrobe();
	 

		delayTime(10000);
        data_port = 0b01000000;		 //Set address = 0 (0-63)
        eStrobe();

		delayTime(10000);
        data_port = 0b10111000;		 //Set page = 0 (0-7)
        eStrobe();

		delayTime(10000);
        data_port = 0b00111111;		 //Display ON 
        eStrobe();

        ctrl_port &= ~(1<<lcdcs1);	 //Disable the CS1 of display 
        ctrl_port &= ~(1<<lcdcs2);	 //Disable the CS2 of display

		createRaster(); // Δημιούργησε το raster...
		showTheWave();	// και εμφάνισέ το στην LCD.
      
}

//====================================================================
//  create "welcome" screen
//====================================================================
void createWelcomeScreen (void)
{	
	unsigned int size;
	lcdAddress = 0;
	flashAddress =0;
	const char *data = welcomeScreen;

	for (size = 0; size<1024; size++)
		lcdBuffer[lcdAddress++] = pgm_read_byte(data++);

}
//====================================================================
// Αντιγραφή των 1024 bytes απο την flash στη RAM.
// Copy the 1024 bytes from flash to RAM.
//====================================================================
void createRaster (void)
{	
	unsigned int size;
	lcdAddress = 0;
	flashAddress =0;
	const char *data = LcdRaster;

	for (size = 0; size<1024; size++)
		lcdBuffer[lcdAddress++] = pgm_read_byte(data++);

}
//====================================================================
//
//====================================================================
void changeLine (unsigned char lineData)
{
		ctrl_port |= (1<<lcdcs1);	//Enable the CS1 of the display 
        ctrl_port |= (1<<lcdcs2);	//Enable the CS2 of the display
        ctrl_port &= ~(1<<lcdrs);   //Clear RS. Command mode
		ctrl_port &= ~(1<<lcdrw);   //Clear RW. Command mode

		lineData += 0b10111000; 
        data_port = lineData;		//Set page = 0 (0-7)
        eStrobe();

        data_port = 0b01000000;		//Set address = 0 (0-63)
        eStrobe();
		
		ctrl_port |= (1<<lcdrs);	//Set RS. Data mode

}

//====================================================================
//
//====================================================================

void fillDataLcdBuffer (unsigned char address, unsigned char data)
{
	
	dataLcdBuffer[address] = data;
}

//====================================================================
//
//====================================================================
void showTheWave (void)
{
	for(lcdAddress = 0; lcdAddress < 1024 ;lcdAddress++)
	{

		if	(line == 8)
			line = 0;

		if (column == 128)
		{
			column = 0;
			line++;
			if (line == 8)
				line = 0;
			changeLine(line);
		} 

		if (column <= 63)
			enable_cs1();

		if (column == 64)
			enable_cs2();

		delayTime(10);
		ctrl_port |= (1<<lcdrs);	// "DATA SEND" mode
		ctrl_port &= ~(1<<lcdrw);	
		data_port = lcdBuffer[lcdAddress];
		eStrobe();

		column++;					// increase column (maximum 128).
	}

}

//====================================================================
// Γραψε τα δεδομένα για μια κυματομορφή στον buffer dataLcdBuffer.
// Write the data for 1 waveform to the buffer dataLcdBuffer.
//====================================================================
void createWave (void)
{
	unsigned char data;
	unsigned char byte;
	unsigned char i = 0;

	for (i=0; i<128; i++)
	{
		byte = 0b10000000;
		lcdAddress = 996; //996 είναι το τελευταίο pixel μέσα στο ράστερ του παλμογράφου.
						  //996 is the final pixel of the oscilloscope's raster.
		if(i<100) // Να μην εμφανίζονται τα pixels εκτός του ράστερ της δέσμης (πχ "AVR SCOPE...).
				  // Do not show pixels out of the beam's raster (exlude area with text "AVR SCOPE...").
		{
			data = dataLcdBuffer[i];
			for (; data>7; data-=8)
				lcdAddress -= 128;
		
			lcdAddress -= i;
		
			for(; data>0; data--)
				byte >>= 1;	
			lcdBuffer[lcdAddress] |= byte;
		}
	}
}

//====================================================================
//
//====================================================================
void enable_cs1 (void)
{
	ctrl_port |= (1<<lcdcs1);		 //Enable the CS1 of the display 
	ctrl_port &= ~(1<<lcdcs2);		 //Disable the CS2 of the display
}

//====================================================================
//
//====================================================================
void enable_cs2 (void)
{
	ctrl_port |= (1<<lcdcs2);		 //Enable the CS2 of the display 
	ctrl_port &= ~(1<<lcdcs1);		 //Disable the CS1 of the display
}

//====================================================================
//		Wait for graphics LCD to be unbusy
//====================================================================
void glcdWait (void)
{
	unsigned char dataIn;
		
	data_port_ddr = 0;			//Make portB all inputs
	ctrl_port |= (1<<lcdrw);	//Set r/w pin to read
	ctrl_port &= ~(1<<lcdrs);	//Set register select to command

	do
	{	
		eStrobe();
		dataIn = data_port_pins;	//Read busy flag
	}while bit_is_set(dataIn, 7);	// loop until bit7 of 'dataIn' will be cleared.

	data_port_ddr = 0xff;		//Make portB all outputs
	ctrl_port &= ~(1<<lcdrw);	//Set r/w pin to write
	ctrl_port |= (1<<lcdrs);	//Set register select to data


}
//====================================================================
//
//====================================================================
void eStrobe (void)
{
        ctrl_port |= (1<<lcde);		 //lcd 'E' pin high
		delayTime(10);
        ctrl_port &= ~(1<<lcde);	 //lcd 'E' pin low
		delayTime(10);
}

//====================================================================
//
//====================================================================
void delayTime (unsigned long counter)
{
	while (counter--);
}
