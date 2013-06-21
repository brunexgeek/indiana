#!/bin/make -f


OBJECTS=network.o \
	nrf24/nrf24.o \
	nrf24/nrf24drv.o \
	demo.o \
	ncmp.o

REMOVE_OBJECTS:=$(REMOVE_OBJECTS) $(OBJECTS)

default: $(OBJECTS)
	$(CC) -o demo $(OBJECTS) -I.

include ../Makefile.in 
