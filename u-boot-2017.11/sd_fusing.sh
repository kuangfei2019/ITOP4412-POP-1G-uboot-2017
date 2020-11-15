#!/bin/bash

if [ -f u-boot-iTOP-4412.bin ] ; then
	echo ""
else
	echo "u-boot-iTOP-4412.bin is not found!!!"
	exit 0
fi

if [ -z $1 ]
then
    echo "usage: ./sd_fusing.sh <SD Reader's device file>"
    exit 0
fi

echo "writting ..."

sudo dd iflag=dsync oflag=dsync if=u-boot-iTOP-4412.bin of=$1 seek=1

echo "writting success"
