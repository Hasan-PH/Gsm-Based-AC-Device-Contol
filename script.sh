avr-gcc -g -Os -mmcu=atmega32 -c led.c
avr-gcc -g -mmcu=atmega32 -o led.elf led.o
avr-objcopy -j .text -j .data -O ihex led.elf led.hex
avr-size --format=avr --mcu=atmega32 led.elf
avrdude -c usbasp -p m32 -P /dev/ttyACM0 -B10 -U flash:w:led.hex -F
