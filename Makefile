# Makefile for i8k Linux Utilities
#
# Copyright (C) 2013-2014  Vitor Augusto <vitorafsr@gmail.com>
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

all: i8kctl probe_i8k_calls_time

i8kctl: i8kctl.c i8kctl.o
	gcc -Wall i8kctl.c -o i8kctl

probe_i8k_calls_time: probe_i8k_calls_time.c
	gcc -Wall -c -g -DLIB i8kctl.c
	gcc -Wall -c -g -DLIB probe_i8k_calls_time.c
	gcc -o probe_i8k_calls_time i8kctl.o probe_i8k_calls_time.o

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f i8kctl i8k.ko probe_i8k_calls_time *.o

obj-m += i8k.o
