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
    int foolproof_checks;
    int daemon;
} cfg;

void set_default_cfg()
{
    cfg.verbose = false;
    cfg.period = 1000;
    cfg.jump_timeout = 2000;
    cfg.jump_temp_delta = 5;
    cfg.t_low = 45;
    cfg.t_mid = 60;
    cfg.t_high = 80;
    cfg.foolproof_checks = true;
    cfg.daemon = false;
}

void i8k_set_fan_state(int fan, int state)
{
    int args[2] = {fan, state};
    if (ioctl(i8k_fd, I8K_SET_FAN, &args) != 0)
    {
        perror("i8k_set_fan_state ioctl error");
        exit(EXIT_FAILURE);
    }
}
int i8k_get_fan_state(int fan)
{
    int args[1] = {fan};
    if (ioctl(i8k_fd, I8K_GET_FAN, &args) == 0)
        return args[0];
    perror("i8k_get_fan_state ioctl error");
    exit(EXIT_FAILURE);
}

int i8k_get_cpu_temp()
{
    int args[1];
    if (ioctl(i8k_fd, I8K_GET_TEMP, &args) == 0)
        return args[0];
    perror("i8k_get_cpu_temp ioctl error");
    exit(EXIT_FAILURE);
}

void monitor()
{
    int fan = I8K_FAN_OFF;
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
            fan = I8K_FAN_OFF;
        else if (temp > cfg.t_high)
            fan = I8K_FAN_HIGH;
        else if (temp >= cfg.t_mid)
            fan = fan == I8K_FAN_HIGH ? fan : I8K_FAN_LOW;

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

void foolproof_checks()
{
    int check_failed = false;
    check_failed += (cfg.t_low < 30) ? foolproof_error("t_low >= 30") : false;
    check_failed += (cfg.t_high > 90) ? foolproof_error("t_high <= 90") : false;
    check_failed += (cfg.t_low < cfg.t_mid && cfg.t_mid < cfg.t_high) ? false : foolproof_error("thresholds t_low < t_mid < t_high");
    check_failed += (cfg.period < 100 || cfg.period > 5000) ? foolproof_error("period in [100,5000]") : false;
    check_failed += (cfg.jump_timeout < 100 || cfg.jump_timeout > 5000) ? foolproof_error("jump_timeout in [100,5000]") : false;
    check_failed += (cfg.jump_temp_delta < 2) ? foolproof_error("jump_temp_delta > 2") : false;
    if (check_failed)
    {
        printf("foolproof_checks() was failed.\nIf you are sure what you do, checks can be disable using \"--foolproof_checks 0\" in command line or add \"foolproof_checks 0\" in %s\n", CFG_FILE);
        exit_failure();
    }
}

int foolproof_error(char *str)
{
    printf("foolproof_checks(): awaiting %s\n", str);
    return true;
}

void exit_failure()
{
    i8k_set_fan_state(I8K_FAN_LEFT, I8K_FAN_HIGH);
    i8k_set_fan_state(I8K_FAN_RIGHT, I8K_FAN_HIGH);
    exit(EXIT_FAILURE);
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
                cfg_error(str);

            pos[0] = '\0';
            pos++;
            while (!isdigit(pos[0]) && pos[0] != '\0')
                pos++;

            if (pos[0] == '\0')
                cfg_error(str);

            int value = atoi(pos);
            set_cfg(str, value);
        }

    free(str);
    fclose(fh);
}

void cfg_error(char *str)
{
    printf("Config error in line [%s]\n", str);
    exit_failure();
}

void set_cfg(char *key, int value)
{
    if (cfg.verbose)
        printf("%s = %d\n", key, value);
    if (strcmp(key, "daemon") == 0)
        cfg.daemon = value;
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
    else if (strcmp(key, "foolproof_checks") == 0)
        cfg.foolproof_checks = value;
    else
    {
        printf("Unknown param %s\n", key);
        exit_failure();
    }
}

void usage()
{
    puts("i8kmon-ng v0.9 by https://github.com/ru-ace");
    puts("Fan monitor and control using i8k kernel module on Dell laptops.\n");

    puts("Usage: i8kmon-ng [OPTIONS]");
    puts("  -h  Show this help");
    puts("  -v  Verbose mode");
    puts("  -d  Daemon mode (detach from console)");
    printf("Params(see %s for explains):\n", CFG_FILE);
    printf("  --period MILLISECONDS         (default: %ld ms)\n", cfg.period);
    printf("  --jump_timeout MILLISECONDS   (default: %ld ms)\n", cfg.jump_timeout);
    printf("  --jump_temp_delta CELSIUS     (default: %d°)\n", cfg.jump_temp_delta);
    printf("  --t_low CELSIUS               (default: %d°)\n", cfg.t_low);
    printf("  --t_mid CELSIUS               (default: %d°)\n", cfg.t_mid);
    printf("  --t_high CELSIUS              (default: %d°)\n", cfg.t_high);
    puts("");
}
void parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
        {
            usage();
            exit(EXIT_SUCCESS);
        }
        else if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0))
        {
            cfg.verbose = true;
        }
        else if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--daemon") == 0))
        {
            cfg.daemon = true;
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
            exit_failure();
        }
    }
}

void daemonize()
{
    cfg.verbose = false;

    int pid = fork();
    if (pid == -1) //failure
    {
        perror("can't fork()");
        exit_failure();
    }
    else if (pid == 0) //child
        cfg.verbose = false;
    else //parent
        exit(EXIT_SUCCESS);
}

void open_i8k()
{
    i8k_fd = open(I8K_PROC, O_RDONLY);
    if (i8k_fd < 0)
    {
        perror("can't open " I8K_PROC);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    open_i8k();
    set_default_cfg();
    load_cfg();
    parse_args(argc, argv);

    if (cfg.foolproof_checks)
        foolproof_checks();
    if (cfg.daemon)
        daemonize();
    monitor();
}
