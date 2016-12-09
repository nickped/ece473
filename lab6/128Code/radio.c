#include<avr/io.h>
#include<avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include<util/delay.h>
#include"radio.h"
#include"si4734.h"
#include"twi_master.h"
#include"main.h"

#define START_FM_FREQ 9990UL

extern uint16_t current_fm_freq,
                current_volume;
extern volatile uint8_t STC_interrupt;
volatile uint8_t radioMode = 0x00;
extern uint8_t si4734_tune_status_buf[8]; //buffer for holding tune_status data  

ISR(INT7_vect){STC_interrupt = 0x01;}

void radioIntInit(){
   EICRB |= (1<<ISC71) & (1<<ISC70);
   EIMSK |= (1<<INT7);
}

void radioInit(){
   DDRE |= 0x04;
   PORTE |= 0x04;

   PORTE &= ~(1<<PE7);
   DDRE |= 0x80;
   PORTE |= (1<<PE2);
   _delay_us(200);
   PORTE &= ~(1<<PE2);
   _delay_us(30);

   DDRE &= ~(0x80);

   radioIntInit();
   
   fm_pwr_up();
   //current_fm_freq = 9990;
   current_fm_freq = START_FM_FREQ;
   //fm_tune_freq();
}

void fmTune(int8_t ticks){
   if(ticks > 0){
      while(ticks && (current_fm_freq < 10790)){
	 ticks--;
	 current_fm_freq += 20;
      }
   }
   else{
      while(ticks && (current_fm_freq > 8800)){
	 ticks++;
	 current_fm_freq -= 20;
      }
   }
   fm_tune_freq();
}

uint8_t signalStr(){
   while(twi_busy());
   fm_rsq_status();
   //TODO: not sure how large SNR can get for radio
   return (si4734_tune_status_buf[5] * 100) / 35;
}

void radioOn(){
   current_volume = 60;
   set_volume();
}

void radioOff(){
   current_volume = 0;
   set_volume();
}

void radioCheckUp(){
   //checking volume
   if(radioMode & 0x30){
      if((radioMode & 0x10) && (current_volume)) current_volume -= 1;
      else current_volume += 1;
      set_volume(); //sets the new volume
      radioMode &= ~0x30; //clears flags
   }

   //tunning
   if(radioMode & 0x03){
      if(radioMode & 0x01) current_fm_freq -= 20;
      else current_fm_freq += 20;

      if(current_fm_freq > 10800) current_fm_freq = 10790;
      if(current_fm_freq < 8800) current_fm_freq = 8810;
      radioMode &= ~0x03;

      fm_tune_freq();
      uint8_t dig1,
	      dig2,
	      dig3,
	      dig4;
      uint8_t temp[2];

      for(uint16_t i = 0x00; i < 0xFFF0; i++){
	DDRA = 0xFF; //Sets port A to output
	PORTA = 0xFF;

	temp[0] = (current_fm_freq/10) % 100;
	temp[1] = (current_fm_freq/1000);

	dig1 = dec2seg(temp[0] % 10);
	dig2 = dec2seg(temp[0] / 10);
	dig3 = dec2seg(temp[1] % 10);
	dig4 = dec2seg(temp[1] / 10);

	PORTB = 0x00 | (PINB & 0x80);	// Enables the first digit
	PORTA = dig1 ^ 0xFF; 		// writes the first digit
	_delay_us(5);			// Deghosting delay
	PORTA = 0xFF;
	nop2();

	PORTB = 0x10 | (PINB & 0x80);	// Enables the second digit
	PORTA = (dig2 | 0x80) ^ 0xFF;		// writes the second digit
	_delay_us(5);			// Deghosting delay
	PORTA = 0xFF;
	nop2();

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


	 if(radioMode & 0x03){
	    if(radioMode & 0x01) current_fm_freq -= 20;
	    else current_fm_freq += 20;

	    if(current_fm_freq > 10800) current_fm_freq = 10790;
	    if(current_fm_freq < 8800) current_fm_freq = 8810;
	    radioMode &= ~0x03;

	    fm_tune_freq();

	    i = 0;
	 }
      }
   }
}
