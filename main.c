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
void timer1_init(void);

int main(void) {
	// Initialize the chip
	init();
	
	// The start menu of the game
	start_menu();
	
	LCDClear();
	
	uint8_t counter = 0;
	uint8_t bools = 0;
	
	while(1) {
		// Figure out what is a dot vs dash
		if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			!(bools & (1 << BOOL_BTN_DOWN))) {
			LCDWriteStringXY(1,1,"Pressed Down");
			bools |= (1 << BOOL_BTN_DOWN);
			//_delay_ms(1000); 
		} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
			(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted
			//LCDClear();
			LCDWriteStringXY(1,0,"Lifted up");
			
			LCDWriteIntXY(1,1,counter,4);
			counter = 0;
			bools &= (0 << BOOL_BTN_DOWN);
			//_delay_ms(1000);
		} else if ((BUTTON_PIN & (1 << BUTTON_POS))) {
			++counter;
			LCDWriteIntXY(1,1,counter,4);
			_delay_ms(10);
		} /*else {
			LCDWriteStringXY(1,0,"START!");
			_delay_ms(100);
		}*/
		//_delay_loop_2(3);
		
        
	}

	return 0;	// Should never run
}

void init(void) {
	//Initialize LCD module
	LCDInit(LS_BLINK|LS_ULINE);
	
	// Set PORTB to input
	BUTTON_PORT &= ~(1 << BUTTON_POS);
	
	DDRC = 0xFF;
	
	timer1_init();
}

void timer1_init(void) {
	// set up timer with prescaler = 64 and CTC mode
    TCCR1B |= (1 << WGM12)|(1 << CS12)|(1 << CS10);
 
    // initialize counter
    TCNT1 = 0;
 
    // initialize compare value
    OCR1A = 11000;//62499;//1249;
}

void start_menu(void) {
	//Clear the screen
	LCDClear();
	
	//Simple string printing
	LCDWriteString("MorseAVR");
	LCDWriteStringXY(0,1,"Press to start");
	while(!(BUTTON_PIN & (1 << BUTTON_POS))) {
		if (TIFR1 & (1 << OCF1A)) // NOTE: '>=' used instead of '=='
        {
            PORTC ^= (1 << 0); // toggles the led
            TIFR1 |= (1 << OCF1A);
            TCNT1 = 0;
        }
	}
	
	_delay_ms(100);
}
