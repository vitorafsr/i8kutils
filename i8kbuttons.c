/*
 * i8kbuttons.c -- Daemon to monitor the status of Fn-buttons on Dell Inspiron
 *                 laptops.
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
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "i8k.h"

#define PROG_VERSION "v1.3 07/11/2001"

/* For compatibilty with kernel driver 1.1 */
#define I8K_VOL_MUTE_1_1 3

#ifndef POLL_TIMEOUT
#define POLL_TIMEOUT 100
#endif

#ifdef DEBUG
#define DPRINTF(fmt, args...) printf(fmt, args)
#else
#define DPRINTF(fmt, args...)
#endif

/* Default commands, must be overriden with command-line args */
static char *cmd_up = 
    "echo specify your cmd_up command with -u and don\\'t send bug reports";
static char *cmd_down =
    "echo specify your cmd_down command with -d and don\\'t send bug reports";
static char *cmd_mute =
    "echo specify your cmd_mute command with -m and don\\'t send bug reports";

static int verbose = 0;

static void
exec_cmd(char *cmd)
{
    int pid;

    if (verbose) {
	printf("%s\n", cmd);
    }

    if ((pid=fork()) < 0) {
	perror("fork failed");
	return;
    }

    if (pid == 0) {
	execl("/bin/sh", "sh", "-c", cmd, NULL);
	exit(0);
    }
}

static int
fn_status(int fd)
{
    int args[1];
    int rc;

    if ((rc=ioctl(fd, I8K_FN_STATUS, &args)) < 0) {
	perror("ioctl failed");
	return rc;
    }

    return args[0];
}

static void
usage(char *progname)
{
    printf("Usage:  %s [<option>...]\n", progname);
    printf("\n");
    printf("Options:\n");
    printf("\n");
    printf("	-v|--verbose\n");
    printf("	-u|--up <command>\n");
    printf("	-d|--down <command>\n");
    printf("	-m|--mute <command>\n");
    printf("	-r|--repeat <milliseconds>\n");
    printf("	-t|--timeout <milliseconds>\n");
    printf("\n");
}

int
main(int argc, char **argv)
{
    int fd, status;
    int timeout = POLL_TIMEOUT;
    int repeat = 0;
    int last = 0;
    int delay = 0;
    int timer = 0;
    char *progname;
    struct timespec req;

    if ((progname=strrchr(argv[0],'/'))) {
	progname++;
    } else {
	progname = argv[0];
    }
    argv++; argc--;

    for (; argc>0; argv++,argc--) {
	if ((strcmp(argv[0],"-h")==0) || (strcmp(argv[0],"--help")==0)) {
	    usage(progname);
	    exit(0);
	}
	if ((strcmp(argv[0],"-v")==0) || (strcmp(argv[0],"--verbose")==0)) {
	    verbose = 1;
	    continue;
	}
	if ((strcmp(argv[0],"-u")==0) || (strcmp(argv[0],"--up")==0)) {
	    if (argc < 2) break;
	    cmd_up = strdup(argv[1]);
	    argv++,argc--;
	    continue;
	}
	if ((strcmp(argv[0],"-d")==0) || (strcmp(argv[0],"--down")==0)) {
	    if (argc < 2) break;
	    cmd_down = strdup(argv[1]);
	    argv++,argc--;
	    continue;
	}
	if ((strcmp(argv[0],"-m")==0) || (strcmp(argv[0],"--mute")==0)) {
	    if (argc < 2) break;
	    cmd_mute = strdup(argv[1]);
	    argv++,argc--;
	    continue;
	}
	if ((strcmp(argv[0],"-t")==0) || (strcmp(argv[0],"--timeout")==0)) {
	    if (argc < 2) break;
	    timeout = atoi(argv[1]);
	    if ((timeout < 50) || (timeout > 999)) {
		fprintf(stderr, "invalid timeout value: %s\n", argv[1]);
		exit(1);
	    }
	    argv++,argc--;
	    continue;
	}
	if ((strcmp(argv[0],"-r")==0) || (strcmp(argv[0],"--repeat")==0)) {
	    if (argc < 2) break;
	    repeat = atoi(argv[1]);
	    if ((repeat != 0) && ((repeat < 100) || (repeat > 1000))) {
		fprintf(stderr, "invalid repeat value: %s\n", argv[1]);
		exit(1);
	    }
	    argv++,argc--;
	    continue;
	}
	break;
    }
    if (argc > 0) {
	usage(progname);
	exit(1);
    }

    if (verbose) {
	printf("%s %s - Copyright (C) Massimo Dal Zotto <dz@debian.org>\n",
	       progname, PROG_VERSION);
    }

    if ((fd=open(I8K_PROC, O_RDONLY)) < 0) {
	fprintf(stderr, "unable to open i8k proc file: %s\n", I8K_PROC);
	exit(1);
    }

    req.tv_sec  = 0;
    req.tv_nsec = timeout * 1000000;
    DPRINTF("req.tv_sec=%d req.tv_nsec=%d\n", req.tv_sec, req.tv_nsec);

    signal(SIGCHLD, SIG_IGN);

    while (1) {
	nanosleep(&req, NULL);

	if ((status=fn_status(fd)) < 0) {
	    exit(1);
	}
	DPRINTF("%d\n", status);

	if (status == last) {
	    if (!repeat || (status == 0)) {
		continue;
	    }
	    if (delay > 0) {
		DPRINTF("delay=%d\n", delay);
		delay -= timeout;
		continue;
	    }
	    if (timer > 0) {
		DPRINTF("timer=%d\n", timer);
		timer -= timeout;
		continue;
	    }
	    timer = repeat;
	} else {
	    last = status;
	    delay = 250;
	    timer = 0;
	}

	switch (status) {
	case I8K_VOL_UP:
	    exec_cmd(cmd_up);
	    break;
	case I8K_VOL_DOWN:
	    exec_cmd(cmd_down);
	    break;
	case I8K_VOL_MUTE:
	case I8K_VOL_MUTE_1_1:
	    exec_cmd(cmd_mute);
	    break;
	case 0:
	    break;
	default:
	    fprintf(stderr, "invalid button status: %d\n", status);
	    break;
	}
    }
}

/* end of file */
