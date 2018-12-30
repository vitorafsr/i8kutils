#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    cfg.period = 100;
    cfg.jump_timeout = 2000;
    cfg.jump_temp_delta = 5;
    cfg.t_low = 45;
    cfg.t_mid = 60;
    cfg.t_high = 80;
}

int i8k_set_fan_status(int fan, int status)
{
    int args[2] = {fan, status}, rc;
    if ((rc = ioctl(i8k_fd, I8K_SET_FAN, &args)) < 0)
        return rc;
    return args[0];
}

int i8k_get_fan_status(int fan)
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
        int fan_left = i8k_get_fan_status(I8K_FAN_LEFT);
        int fan_right = i8k_get_fan_status(I8K_FAN_RIGHT);
        if (temp - temp_prev > cfg.jump_temp_delta && skip_once == 0)
        {
            if (cfg.verbose)
                printf("  %d   ", temp);
            skip_once = 1;
            fflush(stdout);
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
            i8k_set_fan_status(I8K_FAN_LEFT, fan);
            i8k_set_fan_status(I8K_FAN_RIGHT, fan);
            if (cfg.verbose)
                printf(" --%d-- ", fan);
        }
        temp_prev = temp;
        skip_once = 0;
        fflush(stdout);
        usleep(cfg.period * 1000);
    }
}

int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0))
        {
            usage();
            exit(0);
        }
    }

    i8k_fd = open(I8K_PROC, O_RDONLY);
    if (i8k_fd < 0)
    {
        perror("can't open " I8K_PROC);
        exit(-1);
    }
    set_default_cfg();

    monitor();
}
