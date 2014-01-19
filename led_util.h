#define LED_R 2
#define LED_G 4
#define LED_B 3
#define LED_PORT PORTC

// LED chooser logic vars
uint8_t led_anim = 1;
uint8_t led_speed = 40;
uint8_t led_counter = 0;
uint8_t led_anim_index = 0;

const uint8_t LED_ANIM_0_LENGTH = 7;
const uint8_t LED_ANIM_0[] = { 0b00000100, 0b00001000, 
							   0b00001100, 0b00010000, 0b00010100, 
							   0b00011000, 0b00011100 };

const uint8_t LED_ANIM_1_LENGTH = 7;
const uint8_t LED_ANIM_1[] = { 0b00000100, 0b00001100, 0b00001000,
							   0b00010000, 0b00011000, 0b00001000, 
							   0b00001100 };
