#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/twi.h>
#include<util/delay.h>
#include<stdlib.h>
#include<stdio.h>
#include"twi_master.h"
#include"48Code.h"
#include"uart_functions_m48.h"

#define SEND_FLAG 0x10

uint8_t celsius[2];
uint8_t readTwiBuffer[2];
char strBuffer[10];
uint8_t flag = 0x00;

int main(){
   DDRD |= 0x10; //for testing
   init_twi();
   uart_init();

   celsius[0] = 0x00;
   celsius[1] = 0x00;
   twi_start_wr(SENSOR_ADDR, celsius, 2); //sets up registers on temp sensor

   sei();
   while(1){
      if(UCSR0A & (1<<RXC0)){
	 PORTD |= 0x10;
	 celsius[0] = UDR0;
	 flag &= ~SEND_FLAG;
	 twi_start_rd(SENSOR_ADDR, readTwiBuffer, 2);
	 readTwiBuffer[0] = (readTwiBuffer[0]<<1) | ((readTwiBuffer[1]&0x80) ? 1 : 0);

	 //sprintf(strBuffer, "%d.%02d", readTwiBuffer[0], ((readTwiBuffer[1] & 0x40) ? 50 : 0) + ((readTwiBuffer[1] & 0x20) ? 25 : 0)) ;
	 //uart_puts(strBuffer);
	 uart_putc(readTwiBuffer[0]);
	 uart_putc(((readTwiBuffer[1]&0x40) ? 5 : 0) + ((readTwiBuffer[1]&0x20) ? 2 : 0));

	 PORTD &= ~0x10;
      }
   }
}
