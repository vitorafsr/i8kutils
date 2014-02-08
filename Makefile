# Makefile for i8k Linux Utilities
#
# Copyright (C) 2013       Vitor Augusto <vitorafsr@gmail.com>
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

all: i8kctl

i8kctl: i8kctl.c

i8k:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f i8kctl i8k.ko

obj-m += i8k.o
