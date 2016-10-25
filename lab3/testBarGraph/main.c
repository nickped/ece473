#include<avr/io.h>
#include<avr/interrupt.h>
#define F_CPU 16000000UL
#include<util/delay.h>

void spi_init(void){
  DDRB  |=   0x07;	           //Turn on SS, MOSI, SCLK
  DDRD  |=   0x04;
  SPCR  |=   (1<<SPE) | (1<<MSTR);  //set up SPI mode
  SPSR  |=   (1<<SPI2X);           // double speed operation
}//spi_init

void write2Bar(uint8_t val){
   SPDR = val;
   while(bit_is_clear(SPSR, SPIF));
   PORTD |= 0x04;
   PORTD &= ~0x04;
}
/*  count_7ms++;                //increment count every 7.8125 ms 
  if ((count_7ms % 64)==0){ //?? interrupts equals one half second 
    SPDR = display_count;		//send to display 
    while(bit_is_clear(SPSR, SPIF));	//wait till data sent out (while spin loop)
    PORTB |=  0x01;			//strobe HC595 output data reg - rising edge
    PORTB &=  ~0x01;			//falling edge
    display_count = display_count << 1; //shift display bit for next time 
  }
  if (display_count == 0x00){display_count= 0x01;} //back to 1st positon
}*/

int main(){
   uint8_t barVal = 0;
   spi_init();

   while(1){
      write2Bar(barVal);
      _delay_ms(200);
      barVal++;
   }
   return 0;
}
