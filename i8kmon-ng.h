
#ifndef _I8KMON_NG_H
#define _I8KMON_NG_H

#define CFG_FILE "/etc/i8kmon-ng.conf"

void set_default_cfg();
int i8k_set_fan_status(int, int);
int i8k_get_fan_status(int);
int i8k_get_cpu_temp();
void usage();
void monitor();
void cfg_error(char *);
void set_cfg(char *, int);
#endif
