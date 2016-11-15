// thermo3_skel.c
// R. Traylor
// 11.15.2011

//Implements a digital themometer with an LM73.  
//For use with a mega128 board.
//Uses interrupts for TWI.
//PD0 is SCL. 
//PD1 is SDA. 
//10K pullups are present on the board.

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd_functions.h"
#include "lm73_functions_skel.h"
#include "twi_master.h"

char    lcd_string_array[16];  //holds string for the LCD
uint8_t i;                     //general purpose index


//declare the external variables for the lm73 data buffers
extern uint8_t lm73_wr_buf[2];
extern uint8_t lm73_rd_buf[2];

//**********************************************************************
//                            spi_init                              
//Initalizes the SPI port on the mega128. Does not do any further   
//external device specific initalizations.                          
//*********************************************************************
void spi_init(void){
  DDRB |=  0x07;  //Turn on SS, MOSI, SCLK
  //mstr mode, sck=clk/2, cycle 1/2 phase, low polarity, MSB 1st, no interrupts 
  SPCR=(1<<SPE) | (1<<MSTR); //enable SPI, clk low initially, rising edge sample
  SPSR=(1<<SPI2X); //SPI at 2x speed (8 MHz)  
 }//spi_init


/***********************************************************************/
/*                                main                                 */
/***********************************************************************/
int main ()
{     
uint16_t lm73_temp;  //place to assemble the lm73_temp

//port initialization
DDRF |= 0x08;  //port F bit 3 is enable for LCD

spi_init();       //initalize SPI port
lcd_init();       //initalize LCD
init_twi();       //initalize TWI

//set LM73 to read temperature mode
lm73_wr_buf[0] = 0; //put code to set pointer in lm73_wr_buf[0]
twi_start_wr(LM73_ADDRESS, lm73_wr_buf, 1); //start write process
_delay_ms(2);                  //wait for it to finish

sei();            //enable interrupts before entering loop

while(1){         //main while loop

  for(i=0;i<100;i++){_delay_ms(1);} //tenth second wait
  clear_display();                  //clean up the display

  twi_start_rd(LM73_ADDRESS, lm73_rd_buf, 2);  //read the lm73 for the temperature data
  _delay_ms(2);                  //wait for it to finish

  lm73_temp = lm73_rd_buf[0];  //save the high temperature byte
  lm73_temp = lm73_temp << 1;  //push it into upper 8 bits 
  lm73_temp |= lm73_rd_buf[1] >> 7;  //save the low temperature byte
  //lm73_temp |= (lm73_wr_buf[1]) ? 0x01 : 0x00;

  sprintf(lcd_string_array, "%i", lm73_temp);  //convert int to ascii, put in string array
  string2lcd(lcd_string_array); //send string array to lcd

////////////////////
  } //while
} //main
