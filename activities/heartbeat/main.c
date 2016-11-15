//heartint.c
//setup TCNT1 in pwm mode, TCNT3 in normal mode 
//set OC1A (PB5) as pwm output 
//pwm frequency:  (16,000,000)/(1 * (61440 + 1)) = 260hz
//
//Timer TCNT3 is set to interrupt the processor at a rate of 30 times a second.
//When the interrupt occurs, the ISR for TCNTR3 changes the duty cycle of timer 
//TCNT1 to affect the brightness of the LED connected to pin PORTB bit 5.
//
//to download: 
//wget http://www.ece.orst.edu/~traylor/ece473/inclass_exercises/timers_and_counters/heartint_skeleton.c
//to print: a2ps -P <printer> -1 --font-size=9 heartint_skeleton.c

#include <avr/io.h>
#include <avr/interrupt.h>

#define TRUE  1
#define FALSE 0

//Traverse the array up then down with control statements or double the size of
//the array and make the control easier.  Values from 0x0100 thru 0xEF00 work 
//well for setting the brightness level.

uint16_t brightness[10] = {	0x0100,
				0x2000,
				0x7000,
				0xA000,
				0xC000,
				0xD000,
				0xD000,
				0xDF00,
				0xE000,
				0xEF00} ;
static uint8_t direction = 0, cnt = 0;

ISR(TIMER3_OVF_vect) {
   if(direction == 0){
      OCR1A = brightness[cnt];
      cnt++;
      if(cnt >= 9) direction ^= 0x01;
   }
   else{
      OCR1A = brightness[cnt];
      cnt--;
      if(cnt <= 0) direction ^= 0x01;
   }

}

int main() {

  DDRB    = 0x20;                            //set port B bit five to output

//setup timer counter 1 as the pwm source

  TCCR1A |= (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);                            //fast pwm, set on match, clear@bottom, 
                                        //(inverting mode) ICR1 holds TOP

  TCCR1B |= (1 << WGM13) | (1<<WGM12) | (1<<CS10);                            //use ICR1 as source for TOP, use clk/1

  TCCR1C  = 0x00;                            //no forced compare 

  ICR1    = 0xF000;                            //clear at 0xF000                               

  
//setup timer counter 3 as the interrupt source, 30 interrupts/sec
// (16,000,000)/(8 * 2^16) = 30 cycles/sec

  TCCR3A = 0x00;                             //normal mode

  TCCR3B = (1 << CS31);                             //use clk/8  (15hz)  

  TCCR3C = 0x00;                             //no forced compare 

  ETIMSK = (1<<TOIE3);                             //enable timer 3 interrupt on TOV

  sei();                                //set GIE to enable interrupts
  while(1); //do forever
}  // main
