CC=avr-g++

DEVICE=328p

EFUSE=0xfd
HFUSE=0xd9
LFUSE=0xe2

# http://www.engbedded.com/fusecalc/
#
# EFUSE=0xfd => brown out detection at 2.7V
# EFUSE=0xff => brown out detection disabled
#
# LFUSE=0xe2 internal clock 8MHz with no divider => F_CPU = 8MHz
# LFUSE=0x62 internal clock 8MHz with divider 8 => F_CPU = 1MHz
# LFUSE=0x6f external quartz 16MHz with divider 8 => F_CPU = 2MHz

# Device Signature
# ATmega8a 0x1E 0x93 0x07

CFLAGS=-g -std=gnu++11 -Os -Wall -mcall-prologues -mmcu=atmega$(DEVICE) -DF_CPU=8000000
## Use short (8-bit) data types
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

LDFLAGS = -Wl,-Map,$(TARGET).map
## Optional, but often ends up with smaller code
LDFLAGS += -Wl,--gc-sections

OBJ2HEX=avr-objcopy

SIZE=avr-size

UISP=avrdude

TARGET=atmegaClockCalendar

C_FILES = $(wildcard *.c*)
C_FILES += $(wildcard fonts/*.c)

OBJS = $(C_FILES:.c=.o)

all : build

#program : flash eeprom
program : flash

fuse :
	$(UISP) -p m$(DEVICE) -c USBasp -v -U hfuse:w:${HFUSE}:m -U lfuse:w:${LFUSE}:m 

flash : $(TARGET).hex
	$(UISP) -p m$(DEVICE) -c USBasp -v -U flash:w:$(TARGET).hex:i

eeprom : $(TARGET).eep
	$(UISP) -p m$(DEVICE) -c USBasp -v -U eeprom:w:$(TARGET).eep:i

build : $(TARGET).hex $(TARGET).eep
	ls -l $(TARGET).*

%.hex : %.elf
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

%.eep : %.elf
	$(OBJ2HEX) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@


%.elf : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) -t $@

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	@rm -f *.hex *.eep *.elf *.o

help :
	@echo "make [help | clean | build | eeprom | flash | program | fuse]"
