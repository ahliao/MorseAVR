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

#define BTN_MOVE_L_PORT	DDRC	// PORT C 0
#define BTN_MOVE_L_PIN	PINC	
#define BTN_MOVE_L_POS	0	

#define BTN_MOVE_R_PORT	DDRC	// PORT C 1
#define BTN_MOVE_R_PIN	PINC	
#define BTN_MOVE_R_POS	1	

// Bool Positions
#define BOOL_BTN_DOWN 	0
#define BOOL_INIT_DONE 	1
#define BOOL_INIT_DOT 	2

// Function prototypes
void print_mo_char(void);
void incr_output_x(void);

void init_morse(void);
void init(void);
void start_menu(void);
void timer1_init(void);

// Globals, because this is easier and I'm tired
uint16_t counter = 0;
uint16_t unit_time = 0;
//	uint16_t dash_time = 0;
//	uint16_t init_input_time = 0;
uint8_t bools = 0;
uint8_t input = 0;
uint8_t input_index = 0;

uint8_t output_x = 0;
uint8_t output_y = 1;

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
	

	
	while(1) {
		// Figure out what is a dot vs dash
		if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			!(bools & (1 << BOOL_BTN_DOWN))) {
			//LCDWriteStringXY(1,1,"Pressed Down");
			bools |= (1 << BOOL_BTN_DOWN);
			TCNT1 = 0;	// reset timer1
			
			// If the dot time is known and input has already started
			if (unit_time != 0) {
				if (counter <= unit_time*1) {
					// It's a new part of the current letter
					// This isn't really too important atm
				} else {
					print_mo_char();
					incr_output_x();
				} 
			}
			counter = 0; // reset the counter for the down state
		} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
			(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted

			if (unit_time == 0) {
				// The very first character is 'H' (can change)
				// So we know that the first hit is a dot
				unit_time = counter;
				if (unit_time < 25) unit_time = 25;
				LCDWriteStringXY(input_index,0,".");
				input_index++;
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
			LCDWriteIntXY(10,1,input_index,2);
			
			counter = 0;	// Reset the counter
			bools &= ~(1 << BOOL_BTN_DOWN);	// Set the button to DOWN		
		} else if (counter > 7 * unit_time && input_index > 0) {
			//output_x -= 2;	// fix the double increase
			print_mo_char();
			LCDWriteStringXY(0,0,"          ");
			input_index = 0;
			input = 0;
			counter = 0;
		}
		
		// Handle what happens when the right or left btns are pressed
		if ((BTN_MOVE_L_PIN & (1 << BTN_MOVE_L_POS))) {
			if (output_x > 0) {
				--output_x;
				LCDWriteIntXY(14, 0, output_x,2);
				input = 0;
				input_index = 0;
				counter = 0;
				_delay_ms(300);
			}
		} else if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
			if (output_x < 15) {
				++output_x;
				LCDWriteIntXY(14, 0, output_x,2);
				input = 0;
				input_index = 0;
				counter = 0;
				_delay_ms(300); // delay is for button and user 
			}
		}
		
		// Handle the CTC timer
		if (TIFR1 & (1 << OCF1A)) {
			// Increment the counter if the button is down
			++counter;
			TIFR1 |= (1 << OCF1A);
		}
	}
}

void print_mo_char(void) {
	// It's a new letter
	// Assign the size of the letter to the bitstring
	input |= (input_index << 5);
	// Check if it's an actual letter
	// Find out which letter it is
	for (int i = 0; i < MO_LENGTH; ++i) {
		if (input == MO_CHAR[i]) {
			LCDWriteStringXY(output_x,output_y,
			MO_CHAR_SYM[i]);
			break;
		}
	}
}

void incr_output_x(void) {
	if (counter <= 7 * unit_time) {
		++output_x;
		LCDWriteIntXY(15, 0, output_x,1);
		input_index = 0; // reset the index
		input = 0;
	}
	if (counter > 7 * unit_time) {
		// It's a new word
		output_x += 2;
		LCDWriteIntXY(15, 0, output_x,1);
		input_index = 0; // reset the index
		input = 0;
	}	
}

void init(void) {
	//Initialize LCD module
	LCDInit(LS_BLINK|LS_ULINE);
	
	// Set PORTB to input
	BUTTON_PORT &= ~(1 << BUTTON_POS);
	
	BTN_MOVE_R_PORT &= ~(1 << BTN_MOVE_R_POS); // Theright btn
	BTN_MOVE_L_PORT &= ~(1 << BTN_MOVE_L_POS); // The left btn
	
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
