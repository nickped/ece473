#include<avr/io.h>
#include<avr/interrupt.h>
#define F_CPU 16000000UL
#include<util/delay.h>

static volatile uint8_t val = 0xFF;

void spi_init(void){
  DDRB  |=   0x07;	           //Turn on SS, MOSI, SCLK
  DDRE  |=   0x40;
  SPCR  |=   (1<<SPE) | (1<<MSTR);  //set up SPI mode
  SPSR  |=   (1<<SPI2X);           // double speed operation

  DDRD |= 0x04;
}//spi_init

void readSPI(){
   PORTB = 0x00;
   _delay_ms(2);
   PORTB = 0x01;

   PORTE = 0x00;
   SPDR = 0x00;
   while(bit_is_clear(SPSR, SPIF));
   PORTE = 0x40;

   val = SPDR;
}



int main(){
   DDRA = 0xFF;
   DDRB = 0xFF;
   PORTA = 0xFF;
   spi_init();

   while(1){
      readSPI();

      PORTB = 0x00;
      PORTA = (val & 0x08) ? 0x3F ^ 0xFF : 0x06 ^ 0xFF;
      _delay_ms(2);
      PORTB = 0x10;
      PORTA = (val & 0x04) ? 0x3F ^ 0xFF : 0x06 ^ 0xFF;
      _delay_ms(2);
      PORTB = 0x30;
      PORTA = (val & 0x02) ? 0x3F ^ 0xFF : 0x06 ^ 0xFF;
      _delay_ms(2);
      PORTB = 0x40;
      PORTA = (val & 0x01) ? 0x3F ^ 0xFF : 0x06 ^ 0xFF;
      _delay_ms(2);
   }
}
