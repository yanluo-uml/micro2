#!/bin/bash

insmod pp_adc.ko
# Change these Major/Minor numbers to match up with
#  those found in /proc/devices
#                           |  |
#                           v  v
mknod -m 666 /dev/pp0adc c 250 0
