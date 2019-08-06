#!/bin/sh


i=899 # 0:start,1:stop(低电平开启)

if [ ! -d "/sys/class/gpio/gpio$i" ];then
    echo $i > /sys/class/gpio/export
    echo out > /sys/class/gpio/gpio$i/direction
fi

echo $1 > /sys/class/gpio/gpio$i/value

