#include"sound.h"

void soundOn(){
   TCCR1B |= (1 << CS10);
}

void soundOff(){
   TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));
}

void buzzer(){
   static uint8_t note = 0x00;
   switch(note){
      case 0:
	 OCR1A = 15000;
	 break;
      case 1:
	 OCR1A = 1500;
      default:
	 note = 0x00;
   };
}
