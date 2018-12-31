/*
 * i8kmon-ng.c -- Temp & fan monitor & control using i8k kernel module on Dell laptops 
 * Copyright (C) 2018-2019 https://github.com/ru-ace
 * Copyright (C) 2013-2017 Vitor Augusto <vitorafsr@gmail.com>
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
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i8k.h"
#include "i8kmon-ng.h"

static int i8k_fd;

struct t_cfg
{
    int verbose;
    unsigned long period;
    unsigned long jump_timeout;
    int jump_temp_delta;
    int t_low;
    int t_mid;
    int t_high;
} cfg;

void set_default_cfg()
{
    cfg.verbose = 0;
    cfg.period = 1000;
    cfg.jump_timeout = 2000;
    cfg.jump_temp_delta = 5;
    cfg.t_low = 45;
    cfg.t_mid = 60;
    cfg.t_high = 80;
}

int i8k_set_fan_state(int fan, int state)
{
    int args[2] = {fan, state}, rc;
    if ((rc = ioctl(i8k_fd, I8K_SET_FAN, &args)) < 0)
        return rc;
    return args[0];
}

int i8k_get_fan_state(int fan)
{
    int args[1] = {fan}, rc;
    if ((rc = ioctl(i8k_fd, I8K_GET_FAN, &args)) < 0)
        return rc;
    return args[0];
}

int i8k_get_cpu_temp()
{
    int args[1], rc;
    if ((rc = ioctl(i8k_fd, I8K_GET_TEMP, &args)) < 0)
        return rc;
    return args[0];
}

void usage()
{
}

void monitor()
{
    int fan = 0;
    int temp_prev = i8k_get_cpu_temp();
    int skip_once = 0;

    while (1)
    {
        int temp = i8k_get_cpu_temp();
        int fan_left = i8k_get_fan_state(I8K_FAN_LEFT);
        int fan_right = i8k_get_fan_state(I8K_FAN_RIGHT);
        if (temp - temp_prev > cfg.jump_temp_delta && skip_once == 0)
        {
            if (cfg.verbose)
            {
                printf("  %d   ", temp);
                fflush(stdout);
            }
            skip_once = 1;

            usleep(cfg.jump_timeout * 1000);
            continue;
        }
        else
        {
            if (cfg.verbose)
                printf("%d/%d/%d ", temp, fan_left, fan_right);
        }
        if (temp <= cfg.t_low)
            fan = 0;
        else if (temp > cfg.t_high)
            fan = 2;
        else if (temp >= cfg.t_mid)
            fan = fan == 2 ? 2 : 1;

        if (fan != fan_left || fan != fan_right)
        {
            i8k_set_fan_state(I8K_FAN_LEFT, fan);
            i8k_set_fan_state(I8K_FAN_RIGHT, fan);
            if (cfg.verbose)
                printf(" --%d-- ", fan);
        }
        temp_prev = temp;
        skip_once = 0;
        if (cfg.verbose)
            fflush(stdout);
        usleep(cfg.period * 1000);
    }
}
void load_cfg()
{
    if (access(CFG_FILE, F_OK) == -1)
        return;
    FILE *fh;
    if ((fh = fopen(CFG_FILE, "r")) == NULL)
        return;
    char *str = malloc(256);
    while (!feof(fh))
        if (fgets(str, 254, fh))
        {
            char *pos = strstr(str, "\n");
            pos[0] = '\0';
            //printf("%s: [%s]\n", CFG_FILE, str);
            if (str[0] == '#' || str[0] == ';' || str[0] == '\0' || isspace(str[0]))
                continue;
            pos = str;
            while (!isspace(pos[0]) && pos[0] != '\0')
                pos++;
            if (pos[0] == '\0')
            {
                cfg_error(str);
                return;
            }

            pos[0] = '\0';
            pos++;
            while (!isdigit(pos[0]) && pos[0] != '\0')
                pos++;

            if (pos[0] == '\0')
            {
                cfg_error(str);
                return;
            }
            int value = atoi(pos);
            set_cfg(str, value);
        }

    free(str);
    fclose(fh);
}

void cfg_error(char *str)
{
    printf("Config error in line [%s]\n", str);
}

void set_cfg(char *key, int value)
{
    if (cfg.verbose)
        printf("%s: %s = %d\n", CFG_FILE, key, value);

    if (strcmp(key, "verbose") == 0)
        cfg.verbose = value;
    else if (strcmp(key, "period") == 0)
        cfg.period = value;
    else if (strcmp(key, "jump_timeout") == 0)
        cfg.jump_timeout = value;
    else if (strcmp(key, "jump_temp_delta") == 0)
        cfg.jump_temp_delta = value;
    else if (strcmp(key, "t_low") == 0)
        cfg.t_low = value;
    else if (strcmp(key, "t_mid") == 0)
        cfg.t_mid = value;
    else if (strcmp(key, "t_high") == 0)
        cfg.t_high = value;
    else
        printf("Unknown param %s\n", key);
}

int main(int argc, char **argv)
{
    set_default_cfg();
    if (argc > 1)
    {
        if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
        {
            usage();
            exit(0);
        }
        else if ((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "--verbose") == 0))
        {
            cfg.verbose = 1;
        }
    }

    i8k_fd = open(I8K_PROC, O_RDONLY);
    if (i8k_fd < 0)
    {
        perror("can't open " I8K_PROC);
        exit(-1);
    }

    load_cfg();

    monitor();
}
