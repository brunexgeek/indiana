#!/bin/make -f

SHELL=/bin/sh
RM=/bin/rm -f
prefix=/usr/local
exec_prefix=${prefix}
INSTALL=/usr/bin/install -c

SUBDIRS=network

default:
	for d in $(SUBDIRS); do \
	  if test -f $$d/Makefile; then \
	    $(MAKE) --no-keep-going --directory=$$d || break; \
	  fi; \
	done

all:
	for d in $(SUBDIRS); do \
	  if test -f $$d/Makefile; then \
	    $(MAKE) --no-keep-going --directory=$$d all || break; \
	  fi; \
	done

install:
	for d in $(SUBDIRS); do \
	  if test -f $$d/Makefile; then \
	    $(MAKE) --no-keep-going --directory=$$d install || break; \
	  fi; \
	done

clean:
	for d in $(SUBDIRS); do \
	  if test -f $$d/Makefile; then \
	    $(MAKE) --no-keep-going --directory=$$d clean || break; \
	  fi; \
	done


distclean:
	for d in $(SUBDIRS); do \
	  if test -f $$d/Makefile; then \
	    $(MAKE) --no-keep-going --directory=$$d distclean || break; \
	  fi; \
	done

uninstall:
	for d in $(SUBDIRS); do \
	  if test -f $$d/Makefile; then \
	    $(MAKE) --no-keep-going --directory=$$d uninstall || break; \
	  fi; \
	done
