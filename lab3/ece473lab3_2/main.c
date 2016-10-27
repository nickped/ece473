/*
 * main.c
 *
 *  Created on: Oct 26, 2016
 *      Author: Nick Pederson
 *         MCU:	ATMEGA128
 *     Purpose: This code reads the mode from the push buttons and displays them using the bar graph. When the encoders are turned it adds or subtracts the number displayed on the 7 segment; the minimum and maximum value of this number is 0 and 1023. If more then one mode is selected it will not change the value on the 7 segment.
 */
#include<avr/io.h>
#include<avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include<util/delay.h>

static volatile uint8_t mode = 0x01,		//The current mode
			lastButton = 0x00,	//last pushbutton state
			debounceButtonCnt = 0,	//current number of debounces
			encoder1State = 0x00,	//encoder1 debounce state
			encoder2State = 0x00;	//encoder2 debounce state

static int16_t 		val7Seg = 0x0000;	//value displayed on 7 seg

/* Reads push button on timer 1 overflow*/
ISR(TIMER0_OVF_vect){
   uint8_t buttons;

   DDRA = 0x00;  //Sets portA for input
   PORTB = 0x70; //Enable the pushbuttons
   PORTA = 0xFF; //Sets the pull up resistor for pushbuttons

   buttons = PINA ^ 0xFF; //Reads push buttons

   //Resets debounce if pushbuttons doesn't equel lastButton
   if (buttons != lastButton){
      lastButton = buttons;
      debounceButtonCnt = 0;
   }
   // Iterates debounce count without rolling over
   else if(debounceButtonCnt <= 6)
      debounceButtonCnt++;

   //Changes mode if debounce is met
   if(debounceButtonCnt == 6) mode ^= buttons; 
   DDRA = 0xFF; //resets portA to output
}

/* Input:
 * Output:
 * Purpose: Initialize timer/counters
 */
void timerInit(){
   //**************************
   //initalizing timer0
   //**************************
   TIMSK |= (1<<TOIE0); //enable interrupts
   TCCR0 |= (1<<CS02) | (1<<CS00); //normal mode, clk/128
}

/* Input:
 * Output:
 * Purpose: Initialize SPI
 */
void spiInit(){
  DDRB |= 0x07; //Turn on SS, MOSI, SCLK
  DDRD |= 0x04; //Sets enable pin for bargraph
  DDRE |= 0x40; //Sets enable pin for encoders
  SPCR |= (1<<SPE) | (1<<MSTR); //set up SPI mode
  SPSR |= (1<<SPI2X); // double speed operation
}//spi_init

/* Input:
 * Output:
 * Purpose: Writes the mode to the barGraph
 */
void write2Bar(){
   SPDR = mode; //Initiates spi transfer
   while(bit_is_clear(SPSR, SPIF)); //waits till done (8 cycles)

   //Updates bargraph with new values
   PORTD |= 0x04; 
   PORTD &= ~0x04;
}

/* Input:
 * Output: current encoder state
 * Purpose: Reads the encoders
 */
uint8_t readSPI(){
   //Save encoder states into shift reg
   PORTB = 0x00;
   PORTB = 0x01;

   PORTE = 0x00; //Enable encoder serial read
   SPDR = 0x00; //Initiates SPI transfer
   while(bit_is_clear(SPSR, SPIF)); //waits till done (8 cycles)
   PORTE = 0x40; //Disable encoder serial read

   return SPDR; //returns the data read from spi
}

/* Input: decimal value to be converted. Must be a single digit
 * Output: Returns the 7seg representation of the input
 * Purpose: converts a decimal digit to 7seg display
 */
uint8_t dec2seg(uint8_t dec){
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
 * Purpose: Displays val7Seg to 7seg
 */
void write7Seg(){
	DDRA = 0xFF; //Sets port A to output
	uint8_t dig1,
	        dig2,
		dig3,
		dig4;

	dig1 = dec2seg(val7Seg % 10    );	// Calculates the 7seg number for the first digit
	dig2 = dec2seg(val7Seg % 100   / 10);	// Calculates the 7seg number for the second digit
	dig3 = dec2seg(val7Seg % 1000  / 100);	// Calculates the 7seg number for the third digit
	dig4 = dec2seg(val7Seg % 10000 / 1000);	// Calculates the 7seg number for the fourth digit

	PORTB = 0x00; 			// Enables the first digit
	PORTA = dig1 ^ 0xFF; 		// writes the first digit
	_delay_us(10);			// Deghosting delay

	PORTB = 0x10;			// Enables the second digit
	PORTA = dig2 ^ 0xFF;		// writes the second digit
	_delay_us(10);			// Deghosting delay

	PORTB = 0x30;			// Enables the third digit
	PORTA = dig3 ^ 0xFF;		// writes the third digit
	_delay_us(10);			// Deghosting delay

	PORTB = 0x40;			// Enables the fourth digit
	PORTA = dig4 ^ 0xFF;		// writes the fourth digit
	_delay_us(10);			// Deghosting delay

	PORTA = 0xFF;
}

/* Input:
 * Output: 1 if mode is valid and 0 otherwise
 * Purpose: Checks to see if mode is valid
 */
uint8_t validMode(){
   switch(mode){
      case 0x00: return 1;
      case 0x01: return 1;
      case 0x02: return 1;
      case 0x04: return 1;
      case 0x08: return 1;
      case 0x10: return 1;
      case 0x20: return 1;
      case 0x40: return 1;
      case 0x80: return 1;
      default:   return 0;
   }
}

/* Input:
 * Output:
 * Purpose: Reads, debounces, and adds encoder ticks to val7Seg
 */
void readEncoders(){
   uint8_t currentEncoderState = readSPI(); //Gets current state of encoders

   //Shifts in encoder1 val to encoder1State
   if(~(encoder1State == 0xFF) || ~(currentEncoderState & 0x08))
      encoder1State = (encoder1State<<1) | ((currentEncoderState & 0x08) ? 1 : 0);

   //Shifts in encoder2 val to encoder2State
   if(~(encoder2State == 0xFF) || ~(currentEncoderState & 0x02))
      encoder2State = (encoder2State<<1) | ((currentEncoderState & 0x02) ? 1 : 0);

   //Checks to see if mode is valid
   if(validMode() == 0) return; 

   //If encoder1 is debounced (debounces 6 times)
   //and has a falling edge then either add or subtract mode
   //from val7Seg depending on direction of encoder
   if((encoder1State & 0x3F) == 0x3E){
      if(currentEncoderState & 0x04) val7Seg += mode;
      else val7Seg -= mode;
   }
   //If encoder2 is debounced (debounces 6 times)
   //and has a falling edge then either add or subtract mode
   //from val7Seg depending on direction of encoder
   if((encoder2State & 0x3F) == 0x3E){
      if(currentEncoderState & 0x01) val7Seg += mode;
      else val7Seg -= mode;
   }
}

int main(){
   DDRA = 0xFF; //Sets portA to output
   DDRB = 0xFF; //Sets portB to output

   spiInit();	//Initialize SPI
   timerInit(); //Initialize timers

   sei();	//Enable interrupts

   while(1){
      write2Bar();	//Write mode to barGraph
      write7Seg();	//Write val7Seg to 7 segment
      readEncoders();	//Reads encoders
      if(val7Seg < 0) val7Seg += 1024;	     //Rolls number into valid range
      if(val7Seg > 1023) val7Seg -= val7Seg; //Rolls number into valid range
   }
}
