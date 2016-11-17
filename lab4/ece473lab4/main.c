/*
 * main.c
 *
 *  Created on: Oct 26, 2016
 *      Author: Nick Pederson
 *         MCU:	ATMEGA128
 *     Purpose: This code is designed to be used as an alarm clock.
 */

#include<avr/io.h>
#include<avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include<util/delay.h>
#include<stdio.h>
#include<string.h>
#define nop2() asm volatile("nop\n\tnop")
#include"hd44780.h"
#include"sound.h"


//#define QUICK_TIME
#define LCD_BUFFER 16
#define MIN_UP 0x02
#define MIN_DOWN 0x01
#define HR_UP 0x08
#define HR_DOWN 0x04
#define CLOCK_TIME 0x00
#define CLOCK_ALARM 0x01
#define START_VOL 0x0100

#define alarmOn 0x01
#define alarmBuzz 0x02
#define alarmChange 0x80
#define alarmSounding 0x40



volatile char lcdString[LCD_BUFFER] = "",      	//string to write to the lcd
	      alarmString[] = "Alarm:";		//Constant string for the lcd

static volatile uint8_t mode = 0x00,		//The current mode
			lastButton = 0x00,	//last pushbutton state
			debounceButtonCnt = 0,	//current number of debounces
			lcdIterator = 0x00,    	//iterator for the lcd string
			lcdStringSize = 0,	//the size of the string for the lcd

			alarmMode = 0x00,	//The alarm mode
			
			//Clock time
			timeSeconds = 0x00,
			timeMinutes = 0x00,
			timeHours   = 0x00,

			//Alarm time
			alarmSeconds = 0x00,
			alarmMinutes = 0x00,
			alarmHours   = 0x00;

//Different user modes
enum modeTypes{
   normal = 0x00,
   alarm = 0x10,
   setClock = 0x20,
   alarmArm = 0x40
};

void write2Bar(uint8_t val);
void iterateTime(int8_t hr, int8_t min, int8_t sec);
void checkAlarm();
void adcInit();
uint8_t adcOffset(uint8_t adcVal);

//Keeps track of time
ISR(TIMER0_COMP_vect){
   iterateTime(0, 0, 1); //iterates time
   checkAlarm(); //checks to see if time = alarm
}

//write to bar
//write to lcd
//reads adc and sets ocr2
ISR(TIMER2_COMP_vect){
   ADCSRA |= (1<<ADSC); //poke ADSC and start conversion
   SPDR = mode; //send mode to bar graph

   while(bit_is_clear(SPSR, SPIF)); //waits till spi is done
   PORTD |= 0x04; //updates bar graph
   PORTD &= ~0x04;   

   //Writes the lcd string to the lcd
   //1 char per interrupt
   //only writes string when the alarmChange is set
   //after done writting string clears alarmChange
   if(alarmMode & alarmChange){
      if(lcdIterator == lcdStringSize){ 
	 lcdIterator = 0;
	 alarmMode &= ~alarmChange;
      }
      else if(lcdIterator == 0){
	 clear_display();
	 char2lcd(lcdString[lcdIterator]);
	 lcdIterator++;
      }
      else{ 
	 char2lcd(lcdString[lcdIterator]); 
	 lcdIterator++; 
      }
   }

   while(!bit_is_clear(ADCSRA, ADSC)); //spin while running flag is set

   OCR2 = adcOffset(ADCH); //adjust the brightness
}

//plays tone
ISR(TIMER1_COMPA_vect){
   PORTC ^= 0x01;
}

/* Input:
 * Output:
 * Purpose: Initialize timer/counters
 */
void timerInit(){
   adcInit();

  //**************************
  //initalizing timer0
  //**************************
  ASSR  |= (1<<AS0); //use external 32kHz crystal
  TIMSK |= (1<<OCIE0); //enable interrupts
  TCNT0  = 0; //sets the initial time at 0
  TCCR0 |= (1<<WGM01) |(1<<CS02) | (1<<CS00); //ctc mode, 128 prescaler,
  OCR0 = 250;

  //**************************
  //initalizing timer1
  //**************************
  TCCR1B |= (1 << WGM12); //ctc mode
  TIMSK |= (1 << OCIE1A); //enable interrupt

  //**************************
  //initalizing timer2
  //**************************
  TCNT2 = 0;
  TIMSK |= (1<<OCIE2); //enable interrupt
  TCCR2 |= (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (0 << COM20) | (1 << CS22); //fast pwm, set on comare, 256 prescaler
  OCR2 = 0x20; // gets reset after first timer2 interrupt 

  //**************************
  //initalizing timer3
  //**************************
  //fast pwm mode, toggle oc1a, no prescaler
  TCCR3A = (1 << WGM31) | (1 << WGM30) | (1 << COM3A1);
  TCCR3B = (1 << WGM32) | (1 << CS30);
  OCR3A = START_VOL;
}

/* Input:   The high value read in from the adc
 * Output:  the offset for the ocr
 * Purpose: Used to get the ocr value for the ambient light
 */
uint8_t adcOffset(uint8_t adcVal){
   if(adcVal > 0xF0) return 0xF0;
   if(adcVal > 0xE0) return 0xE0;
   if(adcVal > 0xD0) return 0xD0;
   if(adcVal > 0xC0) return 0xC0;
   return adcVal;
}
   
/* Input:   
 * Output:  
 * Purpose: Checks to see if alarm matches time. If so turns on timer 1
 */
void checkAlarm(){
   if(!(alarmMode & alarmOn)) return; //checks to see that alarm is turned on
   if((timeSeconds != alarmSeconds) | 
      (timeMinutes != alarmMinutes) |
      (timeHours   != alarmHours))
      return;
   TCCR1B |= (1 << CS10); // no prescaler
}

/* Input:   
 * Output:  
 * Purpose: Toggles alarm and sets the alarmChange flag
 */
void alarmSet(){
   //if alarm is off turn it on
   if(!(alarmMode & alarmOn)){
      alarmMode |= alarmOn;
//      if(alarmMode & alarmBuzz)
	 sprintf((char*)lcdString, "%s Buzz", alarmString);
   }
   //if alarm is on turn it off
   else{
      alarmMode &= ~alarmOn;
      sprintf((char*)lcdString, "%s Off", alarmString);
   }
   lcdStringSize = strlen((char*)lcdString); //gets size of new string
   alarmMode |= alarmChange; // sets alarmChange flag
}

/* Input:   hours added
 * 	    minutes added
 * 	    seconds added
 * Output:  
 * Purpose: adds to time
 */
void iterateTime(int8_t hr, int8_t min, int8_t sec){
   timeSeconds += sec;
   timeMinutes += min;
   timeHours   += hr;

   //roll over handling
   if(timeSeconds >= 60){timeMinutes++; timeSeconds = 0;}
   if(timeMinutes >= 60){timeHours++;   timeMinutes = 0;}
   if(timeHours   >= 24){		timeHours   = 0;}

   //roll under hangling
   if(timeSeconds < 0){timeMinutes--; timeSeconds = 59;}
   if(timeMinutes < 0){timeHours--;   timeMinutes = 59;}
   if(timeHours   < 0){		      timeHours   = 23;}
}

/* Input:   hours added
 * 	    minutes added
 * 	    seconds added
 * Output:  
 * Purpose: adds to alarm
 */
void iterateAlarm(int8_t hr, int8_t min, int8_t sec){
   alarmSeconds += sec;
   alarmMinutes += min;
   alarmHours   += hr;

   //roll over handling
   if(alarmSeconds >= 60){alarmMinutes++; alarmSeconds = 0;}
   if(alarmMinutes >= 60){alarmHours++;   alarmMinutes = 0;}
   if(alarmHours   >= 24){		  alarmHours   = 0;}

   //roll under handling
   if(alarmSeconds < 0){alarmMinutes--;   alarmSeconds = 59;}
   if(alarmMinutes < 0){alarmHours--;     alarmMinutes = 59;}
   if(alarmHours   < 0){		  alarmHours   = 23;}
}

/* Input:   
 * Output:  
 * Purpose: initializes adc
 */
void adcInit(){
   DDRF  &= ~(_BV(DDF7)); //make port F bit 7 is ADC input  
   PORTF &= ~(_BV(PF7));  //port F bit 7 pullups must be off

   ADMUX = (1<<REFS0) | (1 << ADLAR) | (1<<MUX0) | (1<<MUX1) | (1<<MUX2); //single-ended, input PORTF bit 7, left adjusted, 10 bits

   ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); //ADC enabled, don't start yet, single shot mode 
}

/* Input:   
 * Output:  push button change
 * Purpose: gets and debounces push buttons
 */
int readButton(){
   uint8_t buttons;

   DDRA = 0x00;  //Sets portA for input
   PORTB = 0x70 | (PINB & 0x80); //Enable the pushbuttons
   PORTA = 0xFF; //Sets the pull up resistor for pushbuttons

   buttons = PINA ^ 0xFF; //Reads push buttons

   //Resets debounce if pushbuttons doesn't equel lastButton
   if (buttons != lastButton){
      lastButton = buttons;
      debounceButtonCnt = 0;
   }
   // Iterates debounce count without rolling over
   else if(debounceButtonCnt <= 12)
      debounceButtonCnt++;

   DDRA = 0xFF; //resets portA to output
   //Changes mode if debounce is met
   return (debounceButtonCnt == 12) ? buttons : 0;
}

/* Input:   the buttons state in lower nibble (upperDiv up, upperDiv down, lowerDiv up, lowerDiv down)
 * 	    clock to change
 * Output:  
 * Purpose: Inteprets buttons for time/alarm change
 */
void changeTime(uint8_t buttons, uint8_t clock){
#ifdef QUICK_TIME
   	// change time clock
	if(clock == CLOCK_TIME){
		if(buttons & MIN_DOWN) iterateTime(0, 0, -1);
		if(buttons & MIN_UP)   iterateTime(0, 0, 1);
		if(buttons & HR_DOWN)  iterateTime(0, -1, 0);
		if(buttons & HR_UP)    iterateTime(0, 1, 0);
	}
	// change alarm clock
	else if(clock == CLOCK_ALARM){
		if(buttons & MIN_DOWN) iterateAlarm(0, 0, -1);
		if(buttons & MIN_UP)   iterateAlarm(0, 0, 1);
		if(buttons & HR_DOWN)  iterateAlarm(0, -1, 0);
		if(buttons & HR_UP)    iterateAlarm(0, 1, 0);
	}
#else
	//change time clock
	if(clock == CLOCK_TIME){
		if(buttons & MIN_DOWN) iterateTime(0, -1, 0);
		if(buttons & MIN_UP)   iterateTime(0, 1, 0);
		if(buttons & HR_DOWN)  iterateTime(-1, 0, 0);
		if(buttons & HR_UP)    iterateTime(1, 0, 0);
	}
	//change alarm clock
	else if(clock == CLOCK_ALARM){
		if(buttons & MIN_DOWN) iterateAlarm(0, -1, 0);
		if(buttons & MIN_UP)   iterateAlarm(0, 1, 0);
		if(buttons & HR_DOWN)  iterateAlarm(-1, 0, 0);
		if(buttons & HR_UP)    iterateAlarm(1, 0, 0);
	}
#endif
}

/* Input:
 * Output:
 * Purpose: Initialize SPI
 */
void spiInit(){
  DDRB |= 0x07; //Turn on SS, MOSI, SCLK
  DDRD |= 0x04; //Sets enable pin for bargraph
  SPCR |= (1<<SPE) | (1<<MSTR); //set up SPI mode
  SPSR |= (1<<SPI2X); // double speed operation
}//spi_init

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
void write7Seg(uint8_t hr, uint8_t min, uint8_t sec){
	//TODO: civilian/military time
	DDRA = 0xFF; //Sets port A to output
	uint8_t dig1,
	        dig2,
		dig3,
		dig4;

#ifdef QUICK_TIME
	dig1 = dec2seg(sec % 10);	// Calculates the 7seg number for the first digit
	dig2 = dec2seg(sec % 100   / 10);	// Calculates the 7seg number for the second digit
	dig3 = dec2seg(min % 10);	// Calculates the 7seg number for the third digit
	dig4 = dec2seg(min % 100 / 10);	// Calculates the 7seg number for the fourth digit
#else
	dig1 = dec2seg(min % 10);	// Calculates the 7seg number for the first digit
	dig2 = dec2seg(min % 100   / 10);	// Calculates the 7seg number for the second digit
	dig3 = dec2seg(hr % 10);	// Calculates the 7seg number for the third digit
	dig4 = dec2seg(hr % 100 / 10);	// Calculates the 7seg number for the fourth digit
#endif /* QUICK_TIME */



	PORTB = 0x00 | (PINB & 0x80);	// Enables the first digit
	PORTA = dig1 ^ 0xFF; 		// writes the first digit
	_delay_us(5);			// Deghosting delay
	PORTA = 0xFF;
	nop2();

	PORTB = 0x10 | (PINB & 0x80);	// Enables the second digit
	PORTA = dig2 ^ 0xFF;		// writes the second digit
	_delay_us(5);			// Deghosting delay
	PORTA = 0xFF;
	nop2();

	if(timeSeconds & 0x01){
	   PORTB = 0x20 | (PINB & 0x80);
	   PORTA = 0x03 ^ 0xFF;
	   _delay_us(5);
	   PORTA = 0xFF;
	   nop2();
	   nop2();
	}

	PORTB = 0x30 | (PINB & 0x80);	// Enables the third digit
	PORTA = dig3 ^ 0xFF;		// writes the third digit
	_delay_us(5);			// Deghosting delay
	PORTA = 0xFF;
	nop2();

	if(dig4 != dec2seg(0)){
	   PORTB = 0x40 | (PINB & 0x80);// Enables the fourth digit
	   PORTA = dig4 ^ 0xFF;		// writes the fourth digit
	   _delay_us(5);		// Deghosting delay
	   PORTA = 0xFF;
	   nop2();
	}

	PORTA = 0xFF;
}

int main(int argc, char **argv){
   //sets ports to output (dont need all of e or c)
   DDRA = 0xFF;
   DDRB = 0xFF; 
   DDRE = 0xFF;
   DDRC = 0xFF;

   spiInit();	//Initialize SPI
   lcd_init();  //Initialize lcd
   timerInit(); //Initialize timers

   alarmSet();  //enable alarm (used to rise lcd write flag)
   alarmSet();  //disable alarm
   sei();	//Enable interrupts

   while(1){
      mode &= 0xF0; //clears the clock set push button states
      (mode & alarm) ? write7Seg(alarmHours, alarmMinutes, alarmSeconds) : write7Seg(timeHours, timeMinutes, timeSeconds);	//Write val7Seg to 7 segment
      mode ^= readButton(); //get changes to mode
      switch(mode & 0xF0){
      	  case alarm: //change alarm mode
		     changeTime(mode, CLOCK_ALARM);
		     break;
	     case alarmArm: //alarm arm/disarm mode
		     alarmSet();
		     mode &= ~alarmArm;
		     break;
	     case setClock: //set time mode
      		  changeTime(mode, CLOCK_TIME);
      		  break;
	  case normal: // normal mode (nothing on)
		  if((alarmMode & alarmOn) && (alarmMode & alarmSounding))
		     //if(alarmMode & alarmBuzz)
			buzzer();

		  break;
      };
   }
}
