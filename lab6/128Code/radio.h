#ifndef _RADIO
#define _RADIO

void radioInit();
void fmTune(int8_t ticks);
uint8_t signalStr(); 
void radioCheckUp();
void radioOn();
void radioOff();

#endif
