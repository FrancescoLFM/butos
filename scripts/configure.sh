#!/bin/bash

size=$(stat --printf %s $1)
config="$2"

# Allign to 512
if [ $(expr $size % 512) != 0 ]
then
    size=$(expr $size / 512 + 1)
    size=$(expr $size \* 512 - 1)
    dd if=/dev/zero of=$1 bs=1 count=1 seek=$size 2>/dev/null 1>/dev/null
fi

size=$(stat --printf %s $1)
size=$(expr $size / 512)

printf "%x" $size > $config/sectors.dat

exit
