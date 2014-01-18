// MHacks
// Alex Liao
// AVR Morse Code Educational Toy
// main.c --- Entry point

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

// PORT Assignments
#define BUTTON_PORT DDRB	// Port B1
#define BUTTON_PIN PINB
#define BUTTON_POS 1		

// Bool Positions
#define BOOL_BTN_DOWN 1

// Function prototypes
void init(void);
void start_menu(void);

int main(void) {
	// Initialize the chip
	init();
	
	// The start menu of the game
	start_menu();
	
	uint16_t counter = 0;
	uint8_t bools = 0;
	
	while(1) {
		// Figure out what is a dot vs dash
		if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			!(bools & (1 << BOOL_BTN_DOWN))) {
			LCDWriteStringXY(1,1,"Pressed Down");
			bools |= (1 << BOOL_BTN_DOWN);
			_delay_ms(1000); 
		} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
			(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted
			//LCDClear();
			LCDWriteStringXY(1,1,"Lifted up");
			bools &= (0 << BOOL_BTN_DOWN);
			_delay_ms(1000);
		} else if ((BUTTON_PIN & (1 << BUTTON_POS))) {
			LCDWriteStringXY(1,1,"Still Pressed");
		}
		_delay_loop_2(3);
	}

	return 0;	// Should never run
}

void init(void) {
	//Initialize LCD module
	LCDInit(LS_BLINK|LS_ULINE);
	
	// Set PORTB to input
	BUTTON_PORT &= ~(1 << BUTTON_POS);
}

void start_menu(void) {
	//Clear the screen
	LCDClear();
	
	//Simple string printing
	LCDWriteString("MorseAVR");
	LCDWriteStringXY(0,1,"Press to start");
	while(!(BUTTON_PIN & (1 << BUTTON_POS)));
	
	_delay_ms(100);
}
