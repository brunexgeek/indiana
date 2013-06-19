#!/bin/make -f


CFLAGS:=-Wall -ggdb
OBJECTS=network.o \
	nrf24.o \
	nrf24drv.o \
	demo.o \
	ncmp.o

default: $(OBJECTS)
	$(CC) -o demo $(OBJECTS) -I. -fno-exceptions

clean:
	$(RM) $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
