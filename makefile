SDCCOPTS ?= --iram-size 128

SRC=main.c  delay.c common.c radio.c oled.c spi.c
#SRC = $(wildcard *.c)
OBJ=$(patsubst %.c,build/%.rel, $(SRC))

build/%.rel: %.c
	mkdir -p $(dir $@)
	sdcc $(SDCCOPTS) -o build/ -c $<

all: main

main: $(OBJ)
	sdcc -o build/ $(SDCCOPTS) $^
	cp build/$@.ihx build/$@.hex
	#hex2bin build/$@.hex

clean:
	rm -f build/*
