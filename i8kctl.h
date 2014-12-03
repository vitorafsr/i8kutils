int i8k_get_bios_version();
int i8k_get_machine_id();
int i8k_get_cpu_temp();
int i8k_get_fan_status(int);
int i8k_get_fan_status(int);
int i8k_get_fan_speed(int);
int i8k_get_fan_speed(int);
int i8k_get_power_status();
int i8k_get_fn_status();
int i8k_set_fan(int, int);

#ifdef LIB
void init();
void finish();
#endif