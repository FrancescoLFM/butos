#!/bin/bash

len=$(bc <<< "($(stat --printf %s $1) / 512 + 1)")
echo "SECTORS: $len"
len=$(($len * 512))
echo "SIZE: $len"
dd if=/dev/zero of=$1 bs=1 count=1 seek=$((len - 1))
exit
