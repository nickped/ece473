PRG            =main

OBJ            = $(PRG).o

MCU_TARGET     = atmega128
OPTIMIZE       = -O2    # options are 1, 2, 3, s
CC             = avr-gcc
F_CPU          = 16000000UL
PROGRAMMER     = usbasp

override CFLAGS        = -std=gnu99 -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: $(PRG).elf lst text eeprom

p: clean program

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) radio.c sound.c hd44780.c uart_functions.c twi_master.c si4734.c $(LDFLAGS) -o $@ $^ $(LIBS) -DF_CPU=$(F_CPU)

clean: 
	rm -rf *.o $(PRG).elf *.bin *.hex *.srec *.bak  
	rm -rf $(PRG)_eeprom.bin $(PRG)_eeprom.hex $(PRG)_eeprom.srec
	rm -rf *.lst *.map 

#setup for for USB programmer
#may need to be changed depending on your programmer
program: $(PRG).hex
	sudo avrdude -c $(PROGRAMMER) -p m128 -e -U flash:w:$(PRG).hex  -v

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@
