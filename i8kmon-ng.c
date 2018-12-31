/*
 * i8kmon-ng.c -- Fan monitor and control using i8k kernel module on Dell laptops.
 * Copyright (C) 2019 https://github.com/ru-ace
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

#include "i8kmon-ng.h"

int i8k_fd;

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

void i8k_set_fan_state(int fan, int state)
{
    int args[2] = {fan, state};
    if (ioctl(i8k_fd, I8K_SET_FAN, &args) != 0)
    {
        perror("i8k_set_fan_state ioctl error");
        exit(-1);
    }
}
int i8k_get_fan_state(int fan)
{
    int args[1] = {fan};
    if (ioctl(i8k_fd, I8K_GET_FAN, &args) == 0)
        return args[0];
    perror("i8k_get_fan_state ioctl error");
    exit(-1);
}

int i8k_get_cpu_temp()
{
    int args[1];
    if (ioctl(i8k_fd, I8K_GET_TEMP, &args) == 0)
        return args[0];
    perror("i8k_get_cpu_temp ioctl error");
    exit(-1);
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
        printf("%s = %d\n", key, value);

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

void usage()
{
    puts("i8kmon-ng v0.9 by https://github.com/ru-ace");
    puts("Fan monitor and control using i8k kernel module on Dell laptops.\n");

    puts("Usage: i8kmon-ng [OPTIONS]");
    puts("  -h  Show this help");
    puts("  -v  Verbose mode");
    printf("Params(see %s to explains):\n", CFG_FILE);
    puts("  --period value");
    puts("  --jump_timeout value");
    puts("  --jump_temp_delta value");
    puts("  --t_low value");
    puts("  --t_mid value");
    puts("  --t_high value");
    puts("");
}
void parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            usage();
            exit(0);
        }
        else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0))
        {
            cfg.verbose = 1;
        }
        else if (argv[i][0] == '-' && argv[i][1] == '-')
        {
            char *key = argv[i];
            key += 2;
            i++;
            if (i == argc || !isdigit(argv[i][0]))
            {
                printf("Param %s need value\n", key);
                break;
            }
            else
            {
                int value = atoi(argv[i]);
                set_cfg(key, value);
            }
        }
        else
        {
            printf("Unknown param %s\n", argv[i]);
        }
    }
}

void open_i8k()
{
    i8k_fd = open(I8K_PROC, O_RDONLY);
    if (i8k_fd < 0)
    {
        perror("can't open " I8K_PROC);
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    set_default_cfg();
    load_cfg();
    parse_args(argc, argv);
    open_i8k();
    monitor();
}
