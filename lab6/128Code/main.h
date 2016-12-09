#ifndef _MAIN
#define _MAIN

#define nop2() asm volatile("nop\n\tnop")

void timerInit();
uint8_t adcOffset(uint8_t adcVal);
void checkAlarm();
void alarmSet();
void setLcdString();
void iterateTime(int8_t hr, int8_t min, int8_t sec);
void iterateAlarm(int8_t hr, int8_t min, int8_t sec);
void adcInit();
int readButton();
void changeTime(uint8_t buttons, uint8_t clock);
void spiInit();
uint8_t dec2seg(uint8_t dec);
void write7Seg(uint8_t hr, uint8_t min, uint8_t sec);
void readEncoders();
uint8_t readSPI();

#endif
