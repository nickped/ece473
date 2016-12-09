#ifndef _SOUND
#define _SOUND

#include<avr/io.h>
#include<avr/interrupt.h>

#define VOL1 0x0100
#define VOL2 0x0200
#define VOL3 0x0300

void soundOn();
void soundOff();
void buzzer();


#endif
