#
# Makefile -- makefile for compiling the i8k Linux Utilities
#
# Copyright (C) 2001-2003  Massimo Dal Zotto <dz@debian.org>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#

# Override with: make module KERNEL_SOURCE=/usr/src/kernel-headers-2.4.17
KERNEL_SOURCE = /usr/src/linux
PREFIX	      = /usr
POLL_TIMEOUT  = 100
#MODVERSIONS  = -DMODVERSIONS

VERSION	      = $(shell head -1 debian/changelog | sed 's/.*(//;s/).*//')
DISTFILE      = i8kutils-$(VERSION).tar.bz2
SIGNFILE      = $(DISTFILE).sign
SRCDIR        = $(shell pwd | sed 's|.*/||g')
CC            = gcc
CFLAGS        = -O2 -Wall
KERNEL_FLAGS  = -D__KERNEL__ -DMODULE $(MODVERSIONS)
BINDIR	      = $(PREFIX)/bin
MANDIR	      = $(PREFIX)/share/man
INCLUDE       = -I. -I$(KERNEL_SOURCE)/include

ifdef MODVERSIONS
  INCLUDE    +=	-include $(KERNEL_SOURCE)/include/linux/modversions.h
endif

all:		i8kbuttons i8kctl i8kmon

i8kbuttons:	i8kbuttons.c
		$(CC) -g $(CFLAGS) -DPOLL_TIMEOUT=$(POLL_TIMEOUT) -I. -o $@ $<

i8kctl:		i8kctl.c
		$(CC) -g $(CFLAGS) -I. -o $@ $<
		ln -fs $@ i8kfan

install:	i8kbuttons i8kctl i8kmon
		cp -fp i8kbuttons i8kctl i8kmon $(DESTDIR)/$(BINDIR)/
		ln -fs i8kctl $(DESTDIR)/$(BINDIR)/i8kfan

install-man:
		for f in *.1; do \
		    gzip -f -9 < $$f > $(DESTDIR)/$(MANDIR)/man1/$$f.gz; \
		done

install_man:	install-man

clean:
		rm -f *.o *~ "#*#"

distclean:	clean
		rm -f i8kbuttons i8kctl i8kfan smm

dist:		distclean
		-fakeroot ./debian/rules clean
		-make i8k.o
		cd .. \
		&& tar -cjvf $(DISTFILE) $(SRCDIR)/* \
		&& gpg --armor --detach-sign --yes -o $(SIGNFILE) $(DISTFILE)

deb:
		dpkg-buildpackage -rfakeroot -us -uc

# I8k module, not needed once it is included in the kernel sources
module:		i8k.o

modversion:
		$(MAKE) i8k.o MODVERSIONS=-DMODVERSIONS

i8k.o:		i8k.c
		$(CC) $(CFLAGS) $(KERNEL_FLAGS) $(INCLUDE) -o $@ -c $<
		strip --strip-unneeded \
		    -K force -K restricted -K power_status \
		    -K handle_buttons -K repeat_delay -K repeat_rate \
		    $@

module-install:	i8k.o
		mkdir -p /lib/modules/$$(uname -r)/misc/
		cp -fp i8k.o /lib/modules/$$(uname -r)/misc/
		chown 0.0 /lib/modules/$$(uname -r)/misc/i8k.o

insmod:		i8k.o
		lsmod | grep -qw i8k || insmod ./i8k.o || insmod -f ./i8k.o

rmmod:
		lsmod | grep -qw i8k && rmmod i8k

# SMM test program, DON'T USE UNLESS YOU KNOW WHAT YOU ARE DOING!!!
smm:		smm.c
		$(CC) -g $(CFLAGS) -I. -o $@ $<

# end of file
