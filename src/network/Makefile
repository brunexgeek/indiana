#!/bin/make -f

CC=msp430-gcc

OBJECTS=network.o \
	nrf24/nrf24.o \
	nrf24/nrf24drv.o \
	demo.o \
	ncmp.o \
	../hosts/router/table.o

REMOVE_OBJECTS:=$(REMOVE_OBJECTS) $(OBJECTS)

default: $(OBJECTS)
	$(CC) $(CFLAGS) -o demo $(OBJECTS) -I../

include ../Makefile.in
