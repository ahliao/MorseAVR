// MHacks
// Alex Liao
// AVR Morse Code Educational Toy
// main.c --- Entry point

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"
#include "morse_util.h"

// PORT Assignments
#define BUTTON_PORT DDRB	// Port B1
#define BUTTON_PIN PINB
#define BUTTON_POS 1		

// Bool Positions
#define BOOL_BTN_DOWN 	0
#define BOOL_INIT_DONE 	1
#define BOOL_INIT_DOT 	2

// Function prototypes
void init_morse(void);
void init(void);
void start_menu(void);
void timer1_init(void);

int main(void) {
	// Initialize the chip
	init();
	
	// The start menu of the game
	start_menu();
	
	init_morse();

	return 0;	// Should never run
}

void init_morse(void) {
	LCDClear();
	
	uint16_t counter = 0;
	uint16_t unit_time = 0;
	uint16_t dash_time = 0;
	uint16_t init_input_time = 0;
	uint8_t bools = 0;
	uint8_t input = 0;
	uint8_t input_index = 0;
	
	while(1) {
		// Figure out what is a dot vs dash
		if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			!(bools & (1 << BOOL_BTN_DOWN))) {
			//LCDWriteStringXY(1,1,"Pressed Down");
			bools |= (1 << BOOL_BTN_DOWN);
			TCNT1 = 0;	// reset timer1
			
			// If the dot time is known and input has already started
			if (unit_time != 0) {
				if (counter <= unit_time) {
					// It's a new part of the current letter
					
				} else if (counter <= 3 * unit_time) {
					// It's a new letter
					
				} else {
					// It's a new word
					
				}
			}
		} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
			(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted

			if (unit_time == 0) {
				// The very first character is 'H' (can change)
				// So we know that the first hit is a dot
				unit_time = counter;
				LCDWriteStringXY(input_index,0,".");
				input_index++;
				//input |= (0 << input_index++);
			}
			
			else if (counter >= unit_time * 3) {
				// We know that it is a dash
				LCDWriteStringXY(input_index,0,"-");
				input |= (1 << input_index++);
				
			} else {
				LCDWriteStringXY(input_index,0,".");
				input_index++;
				
				// set it to 0, as in don't do anything
			}
			
			counter = 0;
			bools &= ~(1 << BOOL_BTN_DOWN);
			
			//LCDClear();
			//LCDWriteStringXY(1,0,"Lifted up");
			
			// If the init_time_input is 0 then this is the very
			// first input
			/*if (init_input_time == 0) {
				init_input_time = counter;
			}
			
			// If the counter is close to the last input time
			// then it is the same, if it 3x+ then it is a dash
			else if (counter >= init_input_time * 3) {
				//LCDWriteStringXY(1,1,"Dash");
				// Dash is around 3 init and init is dot
				unit_time = counter;
				dash_time = counter * 3;
				bools |= (1 << BOOL_INIT_DOT);
				bools |= (1 << BOOL_INIT_DONE);
			} else if (counter <= init_input_time / 3) {
				//LCDWriteStringXY(1,1,"Dot");
				unit_time = counter / 3;
				dash_time = counter;
				bools |= (1 << BOOL_INIT_DONE);
			} else if (bools & (1 << BOOL_INIT_DONE)) {
				if (counter <= dash_time) {
					// We know for sure it is a dot
					//LCDWriteStringXY(1,1,"Dot");
				} else {
					// We know for sure it is a dash
					//LCDWriteStringXY(1,1,"Dash");
				}
			} else {
				// We're not sure what a dot or dash is yet
				//LCDWriteStringXY(1,1,"HALP");
				init_input_time = 0;
			} 
			
			counter = 0;
			bools &= ~(1 << BOOL_BTN_DOWN);*/
		} else if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			(TIFR1 & (1 << OCF1A))) {
			// Increment the counter if the button is down
			++counter;
			LCDWriteIntXY(1,1,counter,4);
			TIFR1 |= (1 << OCF1A);
		} else {
			// The button is up for a while
			
		}
	}
}

void init(void) {
	//Initialize LCD module
	LCDInit(LS_BLINK|LS_ULINE);
	
	// Set PORTB to input
	BUTTON_PORT &= ~(1 << BUTTON_POS);
	
	DDRC = 0xFF;	// Debugging stuff
	
	timer1_init();	// Initialize the timer
}

void timer1_init(void) {
	// set up timer with prescaler = 64 and CTC mode
    TCCR1B |= (1 << WGM12)|(1 << CS11);
 
    // initialize counter
    TCNT1 = 0;
 
    // initialize compare value
    OCR1A = 4999;
}

void start_menu(void) {
	//Clear the screen
	LCDClear();
	
	//Simple string printing
	LCDWriteString("MorseAVR");
	LCDWriteStringXY(0,1,"Press to start");
	while(!(BUTTON_PIN & (1 << BUTTON_POS)));
	
	_delay_ms(1000);
}
