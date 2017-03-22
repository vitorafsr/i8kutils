# Makefile for i8kutils
#
# Copyright (C) 2013-2017  Vitor Augusto <vitorafsr@gmail.com>
# Copyright (C) 2001-2005  Massimo Dal Zotto <dz@debian.org>
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

CFLAGS:=-g -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wall
LDFLAGS:=-Wl,-Bsymbolic-functions -Wl,-z,relro

export DEB_BUILD_MAINT_OPTIONS = hardening=+all

all: i8kctl

i8kctl: i8kctl.c i8k.h i8kctl.h

i8kctl_DLIB.o: i8kctl.c i8k.h i8kctl.h
	$(CC) $(CFLAGS) -Wall -c -g -DLIB i8kctl.c -o i8kctl_DLIB.o

probe_i8k_calls_time: i8kctl_DLIB.o probe_i8k_calls_time.c
	$(CC) $(CFLAGS) -Wall -c -g -DLIB probe_i8k_calls_time.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -o probe_i8k_calls_time i8kctl_DLIB.o probe_i8k_calls_time.o

clean:
	rm -f i8kctl probe_i8k_calls_time *.o
