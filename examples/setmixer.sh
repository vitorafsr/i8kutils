#!/bin/sh
#
# From: Jan Setzer <jan.setzer@fms-computer.com>
# Date: Thu, 31 Jan 2002 11:04:30 +0100
# Subject: i8k-keys
# 
# Hello,
# 
# I've got the i8k-utils from you and the work fine. I wrote a small 
# shell-script for use the mute-button in a toggle way with the alsa-driver.
# I will attach this script to this mail.
# 
# Jan Setzer
#
# Usage:
#
#   i8kbuttons -u "setmixer.sh up" -d "setmixer.sh down" -m "setmixer.sh mute"

MASTER=`amixer get Master`;

if [ "$1" = "mute" ]; then 

  MASTER=`echo $MASTER | grep '\[on\]'`;

  if [ "$MASTER" = "" ]; then
    amixer set Master unmute >/dev/null 2>&1
  else
    amixer set Master mute >/dev/null 2>&1
  fi

elif [ "$1" = "up" ]; then

  MASTER=`echo $MASTER |awk '{ if (/%/) { gsub(/%\].*$/,""); gsub(/^.*\[/,""); print $0+5 }}'`

  amixer set Master ${MASTER}% >/dev/null 2>&1

elif [ "$1" = "down" ]; then

  MASTER=`echo $MASTER |awk '{ if (/%/) { gsub(/%\].*$/,""); gsub(/^.*\[/,""); print $0-5 }}'`

  amixer set Master ${MASTER}% >/dev/null 2>&1

fi
