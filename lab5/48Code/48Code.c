#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/twi.h>
#include<util/delay.h>
#include<stdlib.h>
#include<stdio.h>
#include"twi_master.h"
#include"48Code.h"
#include"uart_functions_m48.h"

uint8_t celsius[2];
uint8_t readTwiBuffer[2]; //twi read buffer
char strBuffer[10]; //uart send buffer

int main(){
   DDRD |= 0x10; //for testing
   init_twi(); //initialize twi
   uart_init(); //initialize uart

   celsius[0] = 0x00; //load pointer register
   celsius[1] = 0x00;
   twi_start_wr(SENSOR_ADDR, celsius, 2); //sets up registers on temp sensor

   sei(); //used for twi
   while(1){
      //checks uart receive complete flag
      if(UCSR0A & (1<<RXC0)){
	 PORTD |= 0x10; //turn on led
	 celsius[0] = UDR0; //clear receive complete flag
	 twi_start_rd(SENSOR_ADDR, readTwiBuffer, 2); //read twi
	 readTwiBuffer[0] = (readTwiBuffer[0]<<1) | ((readTwiBuffer[1]&0x80) ? 1 : 0); //shifts upper byte left and loads msb of lower byte

	 //creates string to send. Always prints 2 digits after decimal
	 sprintf(strBuffer, "%d.%02d", readTwiBuffer[0], ((readTwiBuffer[1] & 0x40) ? 50 : 0) + ((readTwiBuffer[1] & 0x20) ? 25 : 0)) ;
	 uart_puts(strBuffer); //sent to uart

	 PORTD &= ~0x10; //turn off led
      }
   }
}
