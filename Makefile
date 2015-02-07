# Makefile for i8k Linux Utilities
#
# Copyright (C) 2013-2015  Vitor Augusto <vitorafsr@gmail.com>
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

ccflags-y = -Wall

pkgver := "1.43" #$(shell dpkg-parsechangelog --show-field Version)"
CXXFLAGS = -DPROG_VERSION=${pkgver}
arch := "$(shell uname -m)"

all: i8kctl smm

i8kctl: i8kctl.c
	gcc -Wall -c -g -DPROG_VERSION='${pkgver}' i8kctl.c
	gcc i8kctl.o -o i8kctl

probe_i8k_calls_time: probe_i8k_calls_time.c i8kctl.c
	gcc -Wall -c -g -DLIB -DPROG_VERSION='${pkgver}' i8kctl.c
	gcc -Wall -c -g -DLIB probe_i8k_calls_time.c
	gcc -o probe_i8k_calls_time i8kctl.o probe_i8k_calls_time.o -lrt

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

smm: smm.c
	gcc -Wall -Werror -D${arch} smm.c -o smm

clean:
	rm -f i8kctl i8k.ko probe_i8k_calls_time smm *.o

obj-m += i8k.o
