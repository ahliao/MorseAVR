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
#include "led_util.h"

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
#define BOOL_HERO_POINT 4

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
void cstr2morse(char * cstr, char * result, int in_length);
void morse2cstr_hero(uint8_t morse, char * rtstr);
void morse2cstr(uint8_t,char *);

// The game modes
void mode_prac1(void);
uint8_t prac1_get_question(void);

void mode_hero(void);
void mode_hero_easy(void);
void mode_hero_hard(void);
uint8_t mode_hero_dir(void);

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
	//start_menu();
	
	//init_morse();
	// For debugging:
	unit_time = 35;
	
	uint8_t selected_mode = 0;
	while (1) {
		selected_mode = main_menu();
	
		// Go the whatever the selected mode is
		// Could use a switch later
		if (selected_mode == 0)
			mode_prac1();
		else if (selected_mode == 2)
			mode_hero();
		
		LCDClear();
	}
	LCDWriteStringXY(0,0,"MHACKS");

	return 0;	// Should never run
}

uint8_t main_menu() {
	LCDClear();	// Clear LCD
	
	int8_t menu_x = 0;
	uint8_t menu_size = 3;
	LCDWriteStringXY(0,0,"Game Mode");
	update_main(menu_x);
	while (1) {	// for now just using infinite loops with breaks
		if ((BUTTON_PIN & (1 << BUTTON_POS))) {
			_delay_ms(1000);
			return menu_x;
		}
		// Handle what happens when the right or left btns are pressed
		else if ((BTN_MOVE_L_PIN & (1 << BTN_MOVE_L_POS))) {
			--menu_x;
			if (menu_x < 0) menu_x = menu_size - 1;
			update_main(menu_x);
			_delay_ms(200);
		} else if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
			++menu_x;
			if (menu_x >= menu_size) menu_x = 0;
			update_main(menu_x);
			_delay_ms(200);
		}
		
		
		// Handle the CTC timer
		if (TIFR1 & (1 << OCF1A)) {
			// Increment the counter if the button is down
			++counter;
			TIFR1 |= (1 << OCF1A);
			
			
			
			// handle LED stuff
			if (led_counter++ > led_speed) {
				led_counter = 0;
				if (led_anim == 0) {
					LED_PORT &= ~((1 << LED_R) | (1 << LED_B) | (1 << LED_G));
					LED_PORT |= LED_ANIM_0[led_anim_index];
					++led_anim_index;
					if (led_anim_index >= LED_ANIM_0_LENGTH)
						led_anim_index = 0;
				} else if (led_anim == 1) {
					LED_PORT &= ~((1 << LED_R) | (1 << LED_B) | (1 << LED_G));
					LED_PORT |= LED_ANIM_1[led_anim_index];
					++led_anim_index;
					if (led_anim_index >= LED_ANIM_1_LENGTH)
						led_anim_index = 0;
				}
			}
		}
	}
	return 99;
}

void update_main(uint8_t i) {
	if (i == 0) { LCDWriteStringXY(0,1,"<-Practice 1->"); }
	else if (i == 1) { LCDWriteStringXY(0,1,"<-Practice 2->"); }
	else if (i == 2) { LCDWriteStringXY(0,1,"<-Morse Hero->"); }
}

// morse hero mode
// It's like guitar hero
void mode_hero(void) {
	// Setup the hero
	uint8_t difficulty = mode_hero_dir();
	
	LCDClear();
	
	// The easy mode of hero
	mode_hero_easy();

}

void mode_hero_easy(void) {	
	int8_t out_x = 15;
	int8_t frame_counter = 0;
	int8_t unit_speed = 30; // maybe use unit_time
	int8_t frame_compare = unit_time;
	int points = 0;
	
	char substr[16];
	uint8_t in_x = 0;
	
	char round1[] = "Clark is a cool guy One day he went to eat chicken and ate chicken so Cool";
	uint8_t round1_length = sizeof(round1)/sizeof(round1[0]);
	char result[384];
	result[0] = 0;
	cstr2morse(round1, result, round1_length);
	int16_t str_offset = 0;
	
	uint8_t speaker_counter = 0;
	
	while (1) {
		
		if (frame_counter > frame_compare) {
			bools &= ~(1 << BOOL_HERO_POINT);
			LED_PORT ^= (1 << LED_B);
			LED_PORT &= ~(1 << LED_G);
			LED_PORT &= ~(1 << LED_R);
			// add in scrolling
			// write only a substring
			
			strncpy(substr, result+str_offset, 15);
			substr[15] = 0;
			LCDWriteStringXY(out_x,0,substr);
			LCDGotoXY(0,1);
				
			if (out_x > 0) --out_x;
			else {
				if (substr[0] == '-') 
					frame_compare = unit_time * 3.5;
				else frame_compare = unit_time * 1;
				++str_offset;
				if (result[str_offset] == '\0') {
					str_offset = 0;
					out_x = 15;
					break;	// Go back to the menu
				}
			}
			frame_counter = 0;
		}
		
		// Handle the input
		if ((BUTTON_PIN & (1 << BUTTON_POS)) &&
			!(bools & (1 << BOOL_BTN_DOWN)) &&
			!(bools & (1 << BOOL_HERO_POINT))) {
				bools |= (1 << BOOL_START_COUNT);
				bools |= (1 << BOOL_BTN_DOWN);
				bools |= (1 << BOOL_HERO_POINT);
				TCNT1 = 0;	// reset timer1
			
				// If the dot time is known and input has already started
				if (unit_time != 0) {
					if (counter <= unit_time*1) {
						// It's a new part of the current letter
						// This isn't really too important atm
					} else {
					
					} 
				}

				counter = 0; // reset the counter for the down state
			} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
				(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted

				if (counter >= unit_time * 2) {
					// We know that it is a dash
					LCDWriteStringXY(0,1,"-");
					input |= (1 << input_index);
					if (str_offset > 0 && 
						(result[str_offset-1] == '-')) {
						LED_PORT |= (1 << LED_G);
						LED_PORT &= ~(1 << LED_B);
						LED_PORT &= ~(1 << LED_R);
						++points;
					} else {
						--points;
						LED_PORT |= (1 << LED_R);
						LED_PORT &= ~(1 << LED_B);
						LED_PORT &= ~(1 << LED_G);
					}
				} else {
					LCDWriteStringXY(0,1,".");
					input &= ~(1 << input_index);
					if (str_offset > 0 && result[str_offset-1] == '.') {
						LED_PORT |= (1 << LED_G);
						LED_PORT &= ~(1 << LED_B);
						LED_PORT &= ~(1 << LED_R);
						++points;
					} else {
						--points;
						LED_PORT |= (1 << LED_R);
						LED_PORT &= ~(1 << LED_B);
						LED_PORT &= ~(1 << LED_G);
					}
					// set it to 0, as in don't do anything
				}
				if (points < 0) points = 0;
				LCDWriteIntXY(12,1,points,2);
				counter = 0;	// Reset the counter
				bools &= ~(1 << BOOL_BTN_DOWN);	// Set the button to DOWN		
				
			} else {
				//LED_PORT &= ~(1 << LED_G);
				//LED_PORT &= ~(1 << LED_R);
			}
			if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
				// Go back to main menu
				break;
			}
		// Speaker fun
		if (bools & (1 << BOOL_BTN_DOWN)) {
			if (speaker_counter > 0) {
				speaker_counter = 0;
				if (OCR0B != 0) OCR0B = 0;
				else OCR0B = 140;
			}
		}
		// Handle the CTC timer
		if (TIFR1 & (1 << OCF1A)) {
			// Increment the counter if the button is down
			++frame_counter;
			++speaker_counter;
			++counter;
			
			TIFR1 |= (1 << OCF1A);
		}
	}
	_delay_ms(2000);
}

void mode_hero_hard(void) {
	
}

uint8_t mode_hero_dir(void) {
	uint8_t dif = 0;	// EASY
	
	LCDClear();
	
	LCDWriteString("MORSE HERO!");
	LCDWriteStringXY(0,1,"Play it like");
	
	while(!(BUTTON_PIN & (1 << BUTTON_POS)));
	_delay_ms(400);	
	LCDClear();
	LCDWriteStringXY(0,0,"Guitar Hero!");
	LCDWriteStringXY(1,1,"Easy");
	LCDWriteStringXY(10,1,"Hard");
	LCDWriteStringXY(6, 1, "<--");
	LCDGotoXY(15,1);
	while(!(BUTTON_PIN & (1 << BUTTON_POS))) {
		// Handle what happens when the right or left btns are pressed
		if ((BTN_MOVE_L_PIN & (1 << BTN_MOVE_L_POS))) {
			dif = 0;
			LCDWriteStringXY(6, 1, "<--");
			LCDGotoXY(15,1);
		} else if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
			dif = 1;
			LCDWriteStringXY(6, 1, "-->");
			LCDGotoXY(15,1);
		}
	}
	_delay_ms(1000);	
	return dif;
}

// practice1 mode
// will be typing the alphabet randomly
void mode_prac1(void) {
	// First Clear the LCD
	LCDClear();
	
	// Small delay to avoid button bounces
	_delay_ms(200);
	
	uint8_t speaker_counter = 0;
	
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

			if (led_anim == 0) {
				LED_PORT &= ~((1 << LED_R) | (1 << LED_B) | (1 << LED_G));
				LED_PORT |= LED_ANIM_0[led_anim_index];
				++led_anim_index;
				if (led_anim_index >= LED_ANIM_0_LENGTH)
					led_anim_index = 0;
			} else if (led_anim == 1) {
				LED_PORT &= ~((1 << LED_R) | (1 << LED_B) | (1 << LED_G));
				LED_PORT |= LED_ANIM_1[led_anim_index];
				++led_anim_index;
				if (led_anim_index >= LED_ANIM_1_LENGTH)
					led_anim_index = 0;
			}

			counter = 0; // reset the counter for the down state
		} else if (!(BUTTON_PIN & (1 << BUTTON_POS)) && // Btn not pushed
			(bools & (1 << BOOL_BTN_DOWN))) {	// And it was lifted

			//if (led_n > 4) led_n = 2;

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
			
			if (led_anim == 0) {
				LED_PORT &= ~((1 << LED_R) | (1 << LED_B) | (1 << LED_G));
				LED_PORT |= LED_ANIM_0[led_anim_index];
				++led_anim_index;
				if (led_anim_index >= LED_ANIM_0_LENGTH)
					led_anim_index = 0;
			} else if (led_anim == 1) {
				LED_PORT &= ~((1 << LED_R) | (1 << LED_B) | (1 << LED_G));
				LED_PORT |= LED_ANIM_1[led_anim_index];
				++led_anim_index;
				if (led_anim_index >= LED_ANIM_1_LENGTH)
					led_anim_index = 0;
			}
			
			counter = 0;	// Reset the counter
			bools &= ~(1 << BOOL_BTN_DOWN);	// Set the button to DOWN		
		} else if (counter > 7 * unit_time && input_index > INPUT_DEFAULT) {
			int match = 0;
			
				input |= (input_index << 5);
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
		if ((BTN_MOVE_L_PIN & (1 << BTN_MOVE_L_POS))) {
			// Reset the current entry
			input_index = INPUT_DEFAULT;
			input = 0;
		} else if ((BTN_MOVE_R_PIN & (1 << BTN_MOVE_R_POS))) {
			// Go back to main menu
			break;
		}
		// Speaker fun
		if (bools & (1 << BOOL_BTN_DOWN)) {
			if (speaker_counter > 0) {
				speaker_counter = 0;
				if (OCR0B != 0) OCR0B = 0;
				else OCR0B = 140;
			}
		}
		// Handle the CTC timer
		if (TIFR1 & (1 << OCF1A)) {
			// Increment the counter if the button is down
			if (bools & (1 << BOOL_START_COUNT)) { 
				++counter;
				++speaker_counter;
			}
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
	//uint8_t morse_length = (MO_CHAR[ra] << 5);
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
	
	// Set PORTC 2,3,4 to output
	DDRC |= (1 << 2) | (1 << 3) | (1 << 4);
	
	timer1_init();	// Initialize the timer
	timer0_init();
}

void timer1_init(void) {
	// set up timer with prescaler = 64 and CTC mode
    TCCR1B |= (1 << WGM12)|(1 << CS11);
 
    // initialize counter
    TCNT1 = 0;
 
    // initialize compare value
    OCR1A = 4999;
}

void timer0_init(void) {
	// initialize timer0 in PWM mode
    TCCR0A |= (1<<WGM00)|(1<<COM0B1)|(1<<WGM01);
    TCCR0B |= (1 << CS00);
 
    // make sure to make OC0 pin (pin PB3 for atmega32) as output pin
    DDRD |= (1<<5);
    
    OCR0B = 120;
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

void cstr2morse(char * cstr, char * result, int in_length) {
	char temp[5];
	char space2[] = "  ";
	char space4[] = "    ";
	for (int i = 0; i < in_length; ++i) {
		if (cstr[i] == ' ') {
			// between words is 7 spaces
			strcat(result, space4);
		} else {
			morse2cstr_hero(MO_CHAR[cstr[i]-65], temp);
			strcat(result, temp);
			strcat(result, space2);
		}
	}
	//result[] = 0;
	strcat(result, "\0");
}

void morse2cstr_hero(uint8_t morse, char * rtstr) {
	uint8_t str_index = 0;
	uint8_t morse_length = (morse >> 5);
	//char cstr[morse_length];
	//LCDWriteIntXY(0,0,morse_length,2);
	for (uint8_t i = 0; i < morse_length; ++i) {
		if (morse & (1 << i)) {
			rtstr[str_index++] = (char) '-';
			//rtstr[str_index++] = (char) '-';
			//rtstr[str_index++] = (char) '-';
		}
		else rtstr[str_index++] = (char) '.';
		//rtstr[str_index++] = (char) 'a';
	}
	rtstr[str_index] = 0;
	//rtstr = cstr;
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
