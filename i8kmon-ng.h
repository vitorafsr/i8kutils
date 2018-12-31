
#ifndef _I8KMON_NG_H
#define _I8KMON_NG_H

#define CFG_FILE "/etc/i8kmon-ng.conf"

#define I8K_PROC "/proc/i8k"
#define I8K_FAN_LEFT 1
#define I8K_FAN_RIGHT 0
#define I8K_GET_TEMP _IOR('i', 0x84, size_t)
#define I8K_GET_FAN _IOWR('i', 0x86, size_t)
#define I8K_SET_FAN _IOWR('i', 0x87, size_t)

void set_default_cfg();
void i8k_set_fan_status(int, int);
int i8k_get_fan_status(int);
int i8k_get_cpu_temp();
void usage();
void monitor();
void load_cfg();
void set_cfg(char *, int);
void cfg_error(char *);
void parse_args(int, char **);
void open_i8k();
#endif
