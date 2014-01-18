// MHacks
// Alex Liao
// AVR Morse Code Educational Toy
// main.c --- Entry point

#include <avr/io.h>
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>

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
#define BOOL_START_COUNT 3

// Function prototypes
char print_mo_char(uint8_t x_r, uint8_t y_r, char str_in[],
	char str_comp[]);
char * comp_mo_char(uint8_t in, char str_in[]);
void incr_output_x(void);

void init_morse(void);
void init(void);
void start_menu(void);
uint8_t main_menu(void);
void update_main(uint8_t i);
void timer1_init(void);

// get the c-string representation of a morse binary code
void morse2cstr(uint8_t,char *);

// The game modes
void mode_prac1(void);
uint8_t prac1_get_question(void);

// Globals, because this is easier and I'm tired
uint16_t counter = 0;
uint16_t unit_time = 0;
uint8_t bools = 0;
uint8_t input = 0;
uint8_t input_index = 0;
const uint8_t INPUT_LENGTH = 64;
uint8_t str_input_index = 0;

const char * STR_HAT = "HAT";

uint8_t output_x = 0;
uint8_t output_y = 1;

int main(void) {
	
	srand(10);
	// Initialize the chip
	init();
	
	// The start menu of the game
	start_menu();
	
	//init_morse();
	// For debugging:
	unit_time = 13;
	
	uint8_t selected_mode = 0;
	selected_mode = main_menu();
	
	// Go the whatever the selected mode is
	// Could use a switch later
	if (selected_mode == 0)
		mode_prac1();
		
	LCDClear();
	LCDWriteStringXY(0,0,"HI DENNY");

	return 0;	// Should never run
}

uint8_t main_menu() {
	LCDClear();	// Clear LCD
	
	uint8_t menu_x = 0;
	uint8_t menu_size = 3;
	LCDWriteStringXY(0,0,"Game Mode");
	update_main(menu_x);
	while (1) {	// for now just using infinite loops with breaks
		if ((BUTTON_PIN & (1 << BUTTON_POS))) {
			return menu_x;
		}
		// Handle what happens when the right or left btns are pressed
		else if ((BTN_MOVE_L_PIN & (1 << BTN_MOVE_L_POS))) {
			--menu_x;
			if (menu_x < 0) menu_x = menu_size - 1;
			update_main(menu_x);
			_delay_ms(300);
		} else if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
			++menu_x;
			if (menu_x >= menu_size) menu_x = 0;
			update_main(menu_x);
			_delay_ms(300);
		}
	}
	return 99;
}

void update_main(uint8_t i) {
	if (i == 0) { LCDWriteStringXY(0,1,"<-Practice 1->"); }
	else if (i == 1) { LCDWriteStringXY(0,1,"<-Practice 2->"); }
	else if (i == 2) { LCDWriteStringXY(0,1,"<-Morse Hero->"); }
}

// practice1 mode
// will be typing the alphabet randomly
void mode_prac1(void) {
	// First Clear the LCD
	LCDClear();
	
	// Small delay to avoid button bounces
	_delay_ms(200);
	
	bools = 0;
	char str_input[INPUT_LENGTH];
	for (int i = 0; i < INPUT_LENGTH; ++i) str_input[i] = 0;
	const uint8_t INPUT_DEFAULT = 0;
	input = 0;
	input_index = INPUT_DEFAULT;
	
	uint8_t points = 0;	// goal is to get 20 points
	const uint8_t goal = 10;
	
	// Get the question + display
	uint8_t rand_val = prac1_get_question();
	counter = 0;
	
	while (points < goal) {
		if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			!(bools & (1 << BOOL_BTN_DOWN))) {
			bools |= (1 << BOOL_START_COUNT);
			bools |= (1 << BOOL_BTN_DOWN);
			TCNT1 = 0;	// reset timer1
			
			// If the dot time is known and input has already started
			if (unit_time != 0) {
				if (counter <= unit_time*1) {
					// It's a new part of the current letter
					// This isn't really too important atm
				} /*else {
					if (print_mo_char(6,1,str_input,MO_CHAR_SYM[rand_val])) {
						// Got is right
						rand_val = prac1_get_question();
						//++points;
						LCDClear();
					}
					incr_output_x();
				} */
			}
			counter = 0; // reset the counter for the down state
		} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
			(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted

			if (counter >= unit_time * 3) {
				// We know that it is a dash
				LCDWriteStringXY(input_index,1,"-");
				input |= (1 << input_index);
				//input++;
				++input_index;
				//LCDWriteIntXY(13,1,input,3);
			} else {
				LCDWriteStringXY(input_index,1,".");
				//input_index++;
				input &= ~(1 << input_index);
				++input_index;	
				//LCDWriteIntXY(13,1,input,3);
				// set it to 0, as in don't do anything
			}
			LCDWriteIntXY(10,1,input_index,2);
			
			counter = 0;	// Reset the counter
			bools &= ~(1 << BOOL_BTN_DOWN);	// Set the button to DOWN		
		} else if (counter > 7 * unit_time && input_index > INPUT_DEFAULT) {
			int match = 0;
			//input &= 0b00011111;
				//LCDWriteIntXY(12,0,input,3);
				input |= (input_index << 5);
				//++input;
				//input = 0b01100001;
				//LCDWriteIntXY(12,1,input,3);
				//input = 0b01100001;
				if (input == MO_CHAR[rand_val]) {
					match = 1;
				}
			if (match){// && print_mo_char(6,1,str_input,MO_CHAR_SYM[rand_val])) {
				// Got it right
				// Get a new question and add a point
				LCDClear();
				
				//LCDWriteStringXY(0,0,"              ");
				rand_val = prac1_get_question();
				++points;
				LCDWriteIntXY(14,0,points,2);
			}
			LCDWriteStringXY(0,1,"              ");
			input_index = INPUT_DEFAULT;
			char * c = comp_mo_char(input, str_input);
			LCDWriteStringXY(6,1,c);
			input = 0b00000000;
			input_index = 0;
			counter = 0;
		}
		
		// Handle what happens when the right or left btns are pressed
		/*if ((BTN_MOVE_L_PIN & (1 << BTN_MOVE_L_POS))) {
			// Reset the current entry
			input_index = INPUT_DEFAULT;
			input = 0;
			str_input_index = 0;
			str_input[0] = 0;
		} else if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
			// Go back to main menu
		}*/
		
		// Handle the CTC timer
		if (TIFR1 & (1 << OCF1A)) {
			// Increment the counter if the button is down
			if (bools & (1 << BOOL_START_COUNT)) ++counter;
			TIFR1 |= (1 << OCF1A);
		}
	}
}

uint8_t prac1_get_question(void) {
	// Choose a random letter and display it
	// No seed atm, need to use ADC to get it
	
	uint8_t ra = 
		(uint8_t)((double)rand() / ((double)RAND_MAX + 1) * (MO_LENGTH+1));
	LCDWriteStringXY(6,0,MO_CHAR_SYM[ra]);
	
	// Display the dot-dash form
	uint8_t morse_length = (MO_CHAR[ra] << 5);
	char str_morse[10];
	morse2cstr(MO_CHAR[ra], str_morse);
	LCDWriteStringXY(0,0,str_morse);
	
	return ra;
}

void init_morse(void) {
	LCDClear();
	
	LCDWriteStringXY(0,0,"Input .... .- -");
	char str_input[INPUT_LENGTH];
	for (int i = 0; i < INPUT_LENGTH; ++i) str_input[i] = 0;
	bools = 0;
	
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
					if (print_mo_char(0,1,str_input,STR_HAT)) {
						// Finished init, go to menu
						LCDWriteStringXY(0,1,"Unit Init Done");
						break;
					}
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
			if (print_mo_char(0,1,str_input,STR_HAT)) {
				// Finished init, go to menu
				LCDWriteStringXY(0,1,"Unit Init Done");
				break;
			}
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
	_delay_ms(1000);
}

char print_mo_char(uint8_t x_r, uint8_t y_r, char str_in[],
	char str_comp[]) {
	// It's a new letter
	// Assign the size of the letter to the bitstring
	//input = 0;
	input &= 0b00011111;
	input |= (input_index << 5);
	// Check if it's an actual letter
	// Find out which letter it is
	for (int i = 0; i < MO_LENGTH; ++i) {
		if (input == MO_CHAR[i]) {
			//LCDWriteStringXY(output_x,output_y,
            //     MO_CHAR_SYM[i]);
			//strcat(str_in, MO_CHAR_SYM[i]);
			/*LCDWriteStringXY(x_r, y_r,str_in);
			if (!strcmp(str_in, str_comp)) {
				return 1;
			}
			break;*/
			return 1;
		}
	}
	return 0;
}

char * comp_mo_char(uint8_t in, char str_in[]) {
	// It's a new letter
	// Assign the size of the letter to the bitstring
	
	// Check if it's an actual letter
	// Find out which letter it is
	for (int i = 0; i < MO_LENGTH; ++i) {
		if (in == MO_CHAR[i]) {
			return MO_CHAR_SYM[i];
		}
	}
	return 0;
}

void incr_output_x(void) {
	if (counter > unit_time * 1 && counter <= 7 * unit_time) {
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

void morse2cstr(uint8_t morse, char * rtstr) {
	
	uint8_t morse_length = (morse >> 5);
	//char cstr[morse_length];
	//LCDWriteIntXY(0,0,morse_length,2);
	for (uint8_t i = 0; i < morse_length; ++i) {
		if (morse & (1 << i))
			rtstr[i] = (char) '-';
		else rtstr[i] = (char) '.';
	}
	rtstr[morse_length] = 0;
	//rtstr = cstr;
}
