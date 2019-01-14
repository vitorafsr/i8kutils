# NAME

i8kmon-ng - temperature monitor for Dell Laptops

# SYNOPSIS

i8kmon-ng \[*OPTION*\]...

# DESCRIPTION

The i8kmon-ng is a daemon that monitors the cpu temperature, and control
the fan speed on some Dell laptops.  
It does that by means of regularly reading the system temperature, and
choosing appropriate speed of the fans.

All options can be customized at **/etc/i8kmon-ng.conf**, or via
command-line options

On Debian and derivated OSes, i8kmon-ng starts in the background by
default as a service.

# ACCESS MODE

i8kmon-ng can access for temp and fan control with two modes (**--mode**
\<*mode*\>):

  - *mode* 0: use dell-smm-hwmon(i8k) kernel module. This mode is more
    safety and can be used by unprivileged user.

  - *mode* 1: use direct SMM BIOS calls. USE ON YOUR OWN RISK. To use
    this mode, i8kmon-ng needs root privileges. 

# DISABLE BIOS FAN CONTROL

USE ON YOUR OWN RISK.  
i8kmon-ng can try disable BIOS fan control with two
methods(**--bios\_disable\_method** \<*method*\>):

  - *method* 1: using code from *https://github.com/clopez/dellfan*

  - *method* 2: identical with
    *https://github.com/TomFreudenberg/dell-bios-fan-control*

To use this feature, i8kmon-ng needs root privileges.

Before set this option in config file, stop i8kmon-ng (sudo service
i8kmon-ng stop) and try every method in verbose mode (sudo i8kmon-ng -v
--bios\_disable\_method 2 --fan\_ctrl\_logic\_mode 0). Wait until cpu
temp will be greater **t\_mid** and fans was on. If during cpu temp
lowering BIOS doesn't try change fan state: seems this method works.

On exit (SIGTERM, SIGINT) i8kmon-ng set fans speed to max and try to
restore BIOS fan control with corresponding *method*

Use *method* 0 for disable this feature.

# FAN CONTROL LOGIC

i8kmon-ng has two modes of fans control
logic(**--fan\_ctrl\_logic\_mode** \<*mode*\>) regarding 3 temperature
thresholds **t\_low**, **t\_mid**, **t\_high** and corresponding fan
states **t\_low\_fan**, **t\_mid\_fan**, **t\_high\_fan**:

  - *mode* 0: Default logic (with default t\_\*\_fan)
    
      - When temperature rising: if temp in \[t\_low,t\_mid\] and
        fan\_state = OFF: fan\_state = OFF if temp \> t\_mid: fan\_state
        = LOW if temp \> t\_high: fan\_state = HIGH
    
      - When temperature lowering: if temp \< t\_low: fan\_state = OFF
        if temp in \[t\_low,t\_mid\] and fan\_state = LOW: fan\_state =
        LOW if temp in \[t\_low, t\_high\] and fan\_state = HIGH:
        fan\_state = HIGH
    
    <!-- end list -->
    
      - Real code: if (temp \<= cfg.t\_low) fan\_state =
        cfg.t\_low\_fan; else if (temp \> cfg.t\_high) fan\_state =
        cfg.t\_high\_fan; else if (temp \>= cfg.t\_mid) fan\_state =
        fan\_state \> cfg.t\_mid\_fan ? fan\_state : cfg.t\_mid\_fan;

<!-- end list -->

  - *mode* 1: Simple logic which allow BIOS to control fans. Stop/start
    fans оnly at boundary temps if temp \< t\_low: fan\_state =
    t\_low\_fan if temp \> t\_high: fan\_state = t\_high\_fan

# ABNORMAL TEMP JUMP DETECTION

i8kmon-ng has buildin mechanism for detect abnormal cpu temp jumps:
short(1-2 seconds) cpu temp jumps on +20/+30 celsius which are cause of
starting fans. You can control it via **--jump\_temp\_delta** and
**--jump\_timeout** options.

To disable this feature set **jump\_temp\_delta** to 100.

# OPTIONS

i8kmon-ng accepts the following command-line options

  - **-h**, **--help**  
    Show help and current values of all options. Before showing help
    programm load /etc/i8kmon-ng.conf. So it can be used for config file
    validation.

  - **-v**, **--verbose**  
    Report hardware status and program actions on stdout.

  - **-m**, **--monitor\_only**  
    Report hardware status on stdout in get\_only mode. Enable verbose
    output.

  - **-d**, **--daemon**  
    Reports child PID in stdout and detach from console. Please don't
    use this option, until you know what you do: current version of the
    startup scripts works only with default value. Default is 0
    (foreground mode).

  - **--mode** \<*mode*\>  
    Set mode for accessing temp and fan controls. Default is 0.  
    *mode* 0: use dell-smm-hwmon(i8k) kernel module  
    *mode* 1: use direct SMM BIOS calls. USE ON YOUR OWN RISK. 

  - **--fan\_ctrl\_logic\_mode** \<*mode*\>  
    Set fan control logic. Default is 0.  
    *mode* 0: default logic (see above)  
    *mode* 1: allow BIOS to control fans. Stop/start fans оnly at
    boundary temps(see above)

  - **--bios\_disable\_method** \<*method*\>  
    USE ON YOUR OWN RISK. Set disable BIOS fans control method. Not
    always works. Default is 0.  
    *method* 0: don't disablе BIOS fans control  
    *method* 1: use DISABLE\_BIOS\_METHOD1  
    *method* 2: use DISABLE\_BIOS\_METHOD2

  - **--period** \<*milliseconds*\>  
    Specifies the interval at which the daemon checks the hardware
    status. Default is 1000 milliseconds.

  - **--fan\_check\_period** \<*milliseconds*\>  
    Specifies the interval at which the daemon checks the fans speed and
    set it. Useful if bios\_disable\_method don't works. Default is 1000
    milliseconds.

  - **--monitor\_fan\_id** \<*FAN\_ID*\>  
    Fan ID for monitoring: 1 = left, 0 = right. State of this fan will
    shows in verbose mode. Default is 1. 

  - **--jump\_timeout** \<*milliseconds*\>  
    Specifies the interval at which the daemon ignore cpu temperature,
    after an abnormal temperature jump detected. Default is 2000
    milliseconds.

  - **--jump\_temp\_delta** \<*celsius*\>  
    Temperature difference between checks, at which the new value is
    considered abnormal. Default is 5° celsius. 

  - **--t\_low** \<*celsius*\>  
    Temperature threshold "low" in celsius. Default is 45° celsius.

  - **--t\_mid** \<*celsius*\>  
    Temperature threshold "middle" in celsius. Default is 60° celsius.

  - **--t\_high** \<*celsius*\>  
    Temperature threshold "high" in celsius. Default is 80° celsius. 

  - **--t\_low\_fan** \<*fan\_state\_id*\>  
    Fan state corresponding to temperature threshold "low". Default is 0
    (OFF).

  - **--t\_low\_fan** \<*fan\_state\_id*\>  
    Fan state corresponding to temperature threshold "middle". Default
    is 1 (LOW).

  - **--t\_low\_fan** \<*fan\_state\_id*\>  
    Fan state corresponding to temperature threshold "high". Default is
    2 (HIGH).

# CONFIGURATION

i8kmon-ng has builtin default values of all options. User can see
current values using **--help** option.

All options with double dash described before can be changed in
/etc/i8kmon-ng.conf using same name.

# FILES

*/etc/i8kmon-ng.conf*

# AUTHOR

ace (https://github.com/ru-ace)

# CREDITS

Code for access to temp and fan control using dell-smm-hwmon(i8k) kernel
module from *https://github.com/vitorafsr/i8kutils*  
Code for enable/disable BIOS fan control and direct SMM BIOS calls from
*https://github.com/clopez/dellfan*

# COPYRIGHT

i8kmon-ng and all the i8kutils programs, scripts and other files are
distributed under the GNU General Public License (GPL).  
On Debian GNU/Linux systems, the complete text of the GNU General Public
License can be found in \`/usr/share/common-licenses/GPL'.

# SEE ALSO

**i8kctl**(1), **i8kmon**(1)
