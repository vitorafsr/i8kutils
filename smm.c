/*
 * smm.c -- Utility to test SMM BIOS calls on Inspiron 8000 laptops
 *
 * Copyright (C) 2001  Massimo Dal Zotto <dz@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * WARNING!!! READ CAREFULLY BEFORE USING THIS PROGRAM!!!
 *
 * THIS PROGRAM IS VERY DANGEROUS. IT CAN CRASH YOUR COMPUTER, DESTROY DATA
 * ON THE HARDISK, CORRUPT THE BIOS, PHYSICALLY DAMAGE YOUR HARDWARE AND
 * MAKE YOUR COMPUTER TOTALLY UNUSABLE.
 *
 * DON'T USE THIS PROGRAM UNLESS YOU REALLY KNOW WHAT YOU ARE DOING. I WILL
 * NOT BE RESPONSIBLE FOR ANY DIRECT OR INDIRECT DAMAGE CAUSED BY USING THIS
 * PROGRAM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/io.h>

typedef struct {
        unsigned int eax;
        unsigned int ebx __attribute__ ((packed));
        unsigned int ecx __attribute__ ((packed));
        unsigned int edx __attribute__ ((packed));
        unsigned int esi __attribute__ ((packed));
        unsigned int edi __attribute__ ((packed));
} SMMRegisters;

static int
i8k_smm(SMMRegisters *regs)
{
    int rc;
    int eax = regs->eax;

    printf("eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\n",
	   regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

    asm("pushl %%eax\n\t" \
	"movl 0(%%eax),%%edx\n\t" \
	"push %%edx\n\t" \
	"movl 4(%%eax),%%ebx\n\t" \
	"movl 8(%%eax),%%ecx\n\t" \
	"movl 12(%%eax),%%edx\n\t" \
	"movl 16(%%eax),%%esi\n\t" \
	"movl 20(%%eax),%%edi\n\t" \
	"popl %%eax\n\t" \
	"out %%al,$0xb2\n\t" \
	"out %%al,$0x84\n\t" \
	"xchgl %%eax,(%%esp)\n\t"
	"movl %%ebx,4(%%eax)\n\t" \
	"movl %%ecx,8(%%eax)\n\t" \
	"movl %%edx,12(%%eax)\n\t" \
	"movl %%esi,16(%%eax)\n\t" \
	"movl %%edi,20(%%eax)\n\t" \
	"popl %%edx\n\t" \
	"movl %%edx,0(%%eax)\n\t" \
	"lahf\n\t" \
	"shrl $8,%%eax\n\t" \
	"andl $1,%%eax\n" \
	: "=a" (rc)
	: "a" (regs)
	: "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory");

    printf("eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x %d\n\n",
	   regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi,
	   rc);

    if ((rc != 0) || ((regs->eax & 0xffff) == 0xffff) || (regs->eax == eax)) {
	return -EINVAL;
    }

    return 0;
}

int
main(int argc, char **argv)
{
    SMMRegisters regs = { 0, 0, 0, 0, 0, 0 };

    if (argc < 2) {
	fprintf(stderr, "Usage:  %s eax ebx ecx edx esi edi\n", argv[0]);
	exit(1);
    }

    if (argc > 1) regs.eax = strtol(argv[1],NULL,16);
    if (argc > 2) regs.ebx = strtol(argv[2],NULL,16);
    if (argc > 3) regs.ecx = strtol(argv[3],NULL,16);
    if (argc > 4) regs.edx = strtol(argv[4],NULL,16);
    if (argc > 5) regs.esi = strtol(argv[5],NULL,16);
    if (argc > 6) regs.edi = strtol(argv[6],NULL,16);

    ioperm(0x84, 1, 1);
    ioperm(0xb2, 1, 1);

    return (i8k_smm(&regs) != 0);
}

/* end of file */
