#!/bin/bash

size=$(stat --printf %s $1)

len=$(bc <<< "($size / 512 + 1)")

if [ $(expr $size % 512) != 0 ]
then
    tmp=$(expr $len \* 512)
    dd if=/dev/zero of=$1 bs=1 count=1 seek=$((tmp - 1)) 2>/dev/null 1>/dev/null
else
    len=$(expr $len - 1)
fi

printf "0x%x" $len
exit
