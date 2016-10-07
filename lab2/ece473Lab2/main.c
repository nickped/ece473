/*
 * main.c
 *
 *  Created on: Sep 29, 2016
 *      Author: Nick
 */
#include<avr/io.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include<util/delay.h>

#define LED_ON 1 // loops this many time when LED is on
#define DEBOUNCE_TIME 6 // Number of times it read until it acknowledge

static uint16_t LED_num = 0;
static uint8_t currentButton = 0,
			   debounceCnt = 0;

uint8_t readButtons(){
	uint8_t result;
	DDRA = 0x00; //1.5 clk to switch ddr
	_delay_us(2); // delays 32 clk cycles

	PORTB = 0xF0; // Enables buttons and disable 7 seg
	PORTA = 0xFF; // Enables all push buttons with pull up resister
	result = PINA ^ 0xFF;
	_delay_us(2); // Delay for reading PIN

	//if buttons change reset currentButtons and debounceCnt
	if(result != currentButton){
		currentButton = result;
		debounceCnt = 0;
		return 0x00;
	}
	if(result == 0x00) return 0x00; // returns no pushed buttons
	if(result == currentButton && debounceCnt <= DEBOUNCE_TIME) debounceCnt++; //if the result equals the previous pushed button it iterates the debounceCnt without risk of rolling over
	return (debounceCnt == DEBOUNCE_TIME) ? result : 0x00; // returns result if debounce is true otherwise returns no pushed Buttons
}

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

void write7seg(){
	DDRA = 0xFF; //Needs 1.5 clk to switch
	uint8_t dig1,
		    dig2,
			dig3,
			dig4;

	dig1 = dec2bin(LED_num % 10    );
	dig2 = dec2bin(LED_num % 100   / 10);
	dig3 = dec2bin(LED_num % 1000  / 100);
	dig4 = dec2bin(LED_num % 10000 / 1000);

	for(uint8_t i = 0; i < LED_ON; i++){
		PORTA = 0xFF;

		PORTB = 0x00; // Enables the first digit
		PORTA = dig1 ^ 0xFF; // writes the first digit
		_delay_ms(2);
		//PORTA = 0xFF;

		PORTB = 0x10;
		PORTA = dig2 ^ 0xFF;
		_delay_ms(2);
		//PORTA = 0xFF;

		PORTB = 0x30;
		PORTA = dig3 ^ 0xFF;
		_delay_ms(2);
		//PORTA = 0xFF;

		PORTB = 0x40;
		PORTA = dig4 ^ 0xFF;
		_delay_ms(2);
		//PORTA = 0xFF;
	}
}

int main(){
	DDRB = 0xF0;
	LED_num = 0;
	while(1){
		write7seg();
		//LED_num++;
		LED_num += readButtons();
		//if(LED_num > 9999) LED_num = 0;
		//_delay_ms(2);
	}
}
