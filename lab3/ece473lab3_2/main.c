#include<avr/io.h>
#include<avr/interrupt.h>
//#ifndef F_CPU
#define F_CPU 16000000UL
//#endif
#include<util/delay.h>

#define DEBOUNCE_CNT 6
static volatile uint8_t mode = 0x01,
			lastButton = 0x00,
			debounceButtonCnt = 0,
			val7Seg = 0x00;

ISR(TIMER0_OVF_vect){
   //TODO: write ISR to check buttons
   //	   Triggered with timer0

   uint8_t buttons;

   DDRA = 0x00;
   PORTB = 0x70;
   PORTA = 0xFF;

   buttons = PINA ^ 0xFF;
   //mode = buttons;

   if (buttons != lastButton){
      lastButton = buttons;
      debounceButtonCnt = 0;
   }
   else if(debounceButtonCnt <= 6)
      debounceButtonCnt++;

   if(debounceButtonCnt == 6) mode ^= buttons;
   DDRA = 0xFF;
}

void timerInit(){
   //**************************
   //initalizing timer0
   //**************************
   TIMSK |= (1<<TOIE0); //enable interrupts
   TCCR0 |= (1<<CS02) | (1<<CS00); //normal mode, clk/128
}

void spiInit(){
  DDRB |= 0x07; //Turn on SS, MOSI, SCLK
  DDRD |= 0x04; //Sets enable pin for bargraph
  DDRE |= 0x40; //Sets enable pin for encoders
  SPCR |= (1<<SPE) | (1<<MSTR); //set up SPI mode
  SPSR |= (1<<SPI2X); // double speed operation
}//spi_init

void write2Bar(){
   SPDR = mode; //Initiates spi transfer
   while(bit_is_clear(SPSR, SPIF)); //waits till done (8 cycles)

   //Updates bargraph with new values
   PORTD |= 0x04; 
   PORTD &= ~0x04;
}

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

int main(){
   DDRA = 0xFF;
   DDRB = 0xFF;

   spiInit();
   write2Bar();
   timerInit();

   sei();

   while(1){
      write2Bar();
      write7Seg();
   }
}
