/*
 * main.c
 *
 *  Created on: Sep 21, 2016
 *      Author: Nick Pederson,
 *         MCU: ATMEGA128
 *     Purpose: Counts the number of times a button is pushed
 *              and displays it on the 8 led display in BCD
 */

#include<avr/io.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include<util/delay.h>

//*******************************************************************************
//                            debounce_switch
// Adapted from Ganssel's "Guide to Debouncing"
// Checks the state of pushbutton S0 It shifts in ones till the button is pushed.
// Function returns a 1 only once per debounced button push so a debounce and toggle
// function can be implemented at the same time.  Expects active low pushbutton on
// Port D bit zero.  Debounce time is determined by external loop delay times 12.
//*******************************************************************************
int8_t debounce_switch() {
  static uint16_t state = 0; //holds present state
  state = (state << 1) | (! bit_is_clear(PIND, 0)) | 0xE000;
  if (state == 0xF000) return 1;
  return 0;
}

/* Input: decimal value to be converted
 * Output: BCD value of input
 * It will take any and truncate it to two decimal digits and
 * and converts the number into an 8-bit BCD value
 */
int8_t num2BCD(int8_t dec){
	return /*upper half*/((dec / 10) % 10 << 4) + /*lower half*/(dec % 10);
}

int main(){
	DDRB = 0xFF; //Sets port B to output
	int cnt = 0; //Stores the current count

	while(1){
		_delay_ms(2); //Helps to debounce
		if(debounce_switch()){
			cnt++; //iterates the current count
			if(cnt > 99) cnt = 0; //Resets the count to 0 if max is reached
			PORTB = num2BCD(cnt); //Writes the BCD value of cnt to port B
		}
	}
	return 0;
}

