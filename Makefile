TARGET_NAME       = main
DEVICE  = atmega8
F_CPU   = 8000000	# в Герцах

FUSE_L  = ff
FUSE_H  = 09

AVREAL=avreal +ATmega8 -p1 -as -fBLEV=1,BSIZ=0,BOOTRST=0,EESV=1,RSTDSBL=1,WDTON=1,CKOPT=0,BODEN=0,CKSEL=F -o0 -e -w -v -c main.hex
 
CFLAGS  = 
OBJECTS = main.o uart.o

COMPILE = avr-gcc  -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)
NM = avr-nm

# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo ""
	@echo "make all ....... to build *.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make clean ..... to delete objects and hex file"
	@echo "make erase ..... erase AVR chip"
	@echo "make sym.. ..... create symbols map *.sym"

all: $(TARGET_NAME).hex

program: flash 

# правило для прошивки firmware:
flash: $(TARGET_NAME).hex
	$(AVREAL) 

# правило для удаления файлов зависимостей (которые могут быть построены утилитой Make):
clean:
	rm -f *.zip *.bin *.hex *.lst *.obj *.cof *.list *.map *.hex *.elf *.o *.sym

# Обычное (generic) правило для компилирования файлов на языке C:
.c.o:
	$(COMPILE) -c $< -o $@

# Обычное (generic) правило для компиляции файлов на языке ассемблера:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" не должен быть необходим, поскольку тип файлов
#  с расширением .S (здесь заглавная S) задан по умолчанию. Однако, символы
#  в верхнем регистре не всегда сохраняются в именах на Windows. Чтобы обеспечить
#  совместимость с WinAVR задайте тип файла вручную.

# Generic правило для компилирования файлов языка C в ассемблер, успользуется только для отладки.
.c.s:
	$(COMPILE) -S $< -o $@

main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)


$(TARGET_NAME).hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf $(TARGET_NAME).hex
	avr-objcopy -I ihex main.hex -O binary main.bin
#	avr-objcopy --debugging -O coff-ext-avr $< main.cof
	avr-size main.elf

# цели отладки:
disasm:	main.elf
	avr-objdump -h -S main.elf

# Create a symbol table from ELF output file.
sym: main.elf
	$(NM) -n $< > main.sym

# Probably preprocessed files output
cpp:
	$(COMPILE) -E main.c

erase:
	$(AVREAL) -e

