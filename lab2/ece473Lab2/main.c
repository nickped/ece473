/*
 * main.c
 *
 *  Created on: Sep 29, 2016
 *      Author: Nick Pederson
 *         MCU:	ATMEGA128
 *     Purpose: This code reads the pushbutton board and displays the running sum on the 7 segment board
 *     		The buttons are read as B0 = 1, B1 = 2, B2 = 4, ..., B7 = 128
 */
#include<avr/io.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include<util/delay.h>

#define LED_ON 1 		// loops this many time through 7 seg
#define DEBOUNCE_TIME 6 	// Number of button reads until it acknowledge button press

static uint16_t LED_num = 0; 	// Current sum
static uint8_t lastButton = 0, 	// The buttons press from the last check
	       debounceCnt = 0; // Current count of button checks without changing lastButton

/* Input:
 * output: The debounced value of the push buttons
 * Purpose: Reads and debounces the push buttons
 */
uint8_t readButtons(){
	uint8_t result;
	DDRA = 0x00; // Set port A for input

	PORTB = 0xF0; // Enables buttons and disable 7 seg
	PORTA = 0xFF; // Enables all push buttons with pull up resister
	result = PINA ^ 0xFF;
	_delay_us(2); // Delay for reading PIN

	//***************************
	// Debouncing
	// **************************
	//if button change, reset lastButton and debounceCnt
	if(result != lastButton){
		lastButton = result;
		debounceCnt = 0;
		return 0x00;
	}
	if(result == lastButton && debounceCnt <= DEBOUNCE_TIME) debounceCnt++; //if the result equals the previous pushed button it iterates the debounceCnt without risk of rolling over
	return (debounceCnt == DEBOUNCE_TIME) ? result : 0x00; // returns result if debounce is true otherwise returns no pushed Buttons
}

/* Input: decimal value to be converted. Must be a single digit
 * Output: Returns the 7seg representation of the input
 * Purpose: converts a decimal digit to 7seg display
 */
uint8_t dec2bin(uint8_t dec){
	switch(dec){
		case 0: return 0x3F;
		case 1: return 0x06;
		case 2: return 0x5B;
		case 3: return 0x4F;
		case 4: return 0x66;
		case 5: return 0x6D;
		case 6: return 0x7D;
		case 7: return 0x07;
		case 8: return 0x7F;
		case 9: return 0x67;
	}
	return 0;
}

/* Input:
 * Output:
 * Purpose: Displays LED_num to 7seg
 */
void write7seg(){
	DDRA = 0xFF; //Sets port A to output
	uint8_t dig1,
	        dig2,
		dig3,
		dig4;

	dig1 = dec2bin(LED_num % 10    );	// Calculates the 7seg number for the first digit
	dig2 = dec2bin(LED_num % 100   / 10);	// Calculates the 7seg number for the second digit
	dig3 = dec2bin(LED_num % 1000  / 100);	// Calculates the 7seg number for the third digit
	dig4 = dec2bin(LED_num % 10000 / 1000);	// Calculates the 7seg number for the fourth digit

	for(uint8_t i = 0; i < LED_ON; i++){
		PORTB = 0x00; 			// Enables the first digit
		PORTA = dig1 ^ 0xFF; 		// writes the first digit
		_delay_ms(2);			// Deghosting delay

		PORTB = 0x10;			// Enables the second digit
		PORTA = dig2 ^ 0xFF;		// writes the second digit
		_delay_ms(2);			// Deghosting delay

		PORTB = 0x30;			// Enables the third digit
		PORTA = dig3 ^ 0xFF;		// writes the third digit
		_delay_ms(2);			// Deghosting delay

		PORTB = 0x40;			// Enables the fourth digit
		PORTA = dig4 ^ 0xFF;		// writes the fourth digit
		_delay_ms(2);			// Deghosting delay
	}
}

int main(){
	DDRB = 0xF0; // Sets upper 4 bits of port B to output for decoder
	LED_num = 0; // Sets current number displayed as 0
	while(1){
		write7seg(); // Writes current number to 7seg
		LED_num += readButtons(); //Read and add pushbuttons to current number
	}
}
