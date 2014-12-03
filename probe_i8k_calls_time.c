#include "i8k.h"
#include "i8kctl.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int i8k_fd;
double t;

struct timespec tmst;

inline double timestamp()
{
	clock_gettime(CLOCK_REALTIME, &tmst);
	t = tmst.tv_nsec;
	t /= 1000000000;
	t += tmst.tv_sec;
	return t;
}

int main()
{
	i8k_fd = open(I8K_PROC, O_RDONLY);
	if (i8k_fd < 0) {
		perror("can't open " I8K_PROC);
		_exit(-1);
	}

	double ts[10];

	init();

	ts[0] = timestamp();
	i8k_get_bios_version();
	ts[1] = timestamp();
	i8k_get_machine_id();
	ts[2] = timestamp();
	i8k_get_cpu_temp();
	ts[3] = timestamp();
	i8k_get_fan_status(I8K_FAN_LEFT);
	ts[4] = timestamp();
	i8k_get_fan_status(I8K_FAN_RIGHT);
	ts[5] = timestamp();
	i8k_get_fan_speed(I8K_FAN_LEFT);
	ts[6] = timestamp();
	i8k_get_fan_speed(I8K_FAN_RIGHT);
	ts[7] = timestamp();
	i8k_get_power_status();
	ts[8] = timestamp();
	i8k_get_fn_status();
	ts[9] = timestamp();

	finish();

	printf("functions time\n");
	printf("i8k_get_bios_version() = %lf\n", ts[1]-ts[0]);
	printf("i8k_get_machine_id() = %lf\n", ts[2]-ts[1]);
	printf("i8k_get_cpu_temp() = %lf\n", ts[3]-ts[2]);
	printf("i8k_get_fan_status() = %lf\n", ts[4]-ts[3]);
	printf("i8k_get_fan_status() = %lf\n", ts[5]-ts[4]);
	printf("i8k_get_fan_speed() = %lf\n", ts[6]-ts[5]);
	printf("i8k_get_fan_speed() = %lf\n", ts[7]-ts[6]);
	printf("i8k_get_power_status() = %lf\n", ts[8]-ts[7]);
	printf("i8k_get_fn_status() = %lf\n", ts[9]-ts[8]);

	return 0;
}