// MHacks
// Alex Liao
// AVR Morse Code Educational Toy
// main.c --- Entry point

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

// PORT Assignments
#define BUTTON_PORT DDRB	// Port B1
#define BUTTON_POS 1		

// Function prototypes
void init(void);

int main(void) {
	// Initialize the chip
	init();
	
	//Clear the screen
	LCDClear();
	
	//Simple string printing
	LCDWriteString("MorseAVR");
	LCDWriteStringXY(0,1,"Press the button");
	//while((PINB & (1 << PB1)));
	
	while(1) {
		if (PINB & (1 << PB1)) {
			LCDWriteStringXY(1,1,"Pressed");
		} else {
			LCDClear();
		}
	}

	return 0;	// Should never run
}

void init(void) {
	//Initialize LCD module
	LCDInit(LS_BLINK|LS_ULINE);
	
	// Set PORTB to input
	BUTTON_PORT &= ~(1 << BUTTON_POS);
}
