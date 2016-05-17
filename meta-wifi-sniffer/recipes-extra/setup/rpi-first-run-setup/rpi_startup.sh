#!/bin/sh

OP_CHECKFILE='/etc/rpi/first-boot'
OP_FIRSTRUN='/usr/rpi/scripts/first-run-setup.sh'

if [ -f $OP_CHECKFILE ]; then
    echo -e "\nOP_STARTUP: $OP_CHECKFILE exists, not first boot."
elif [ -f $OP_FIRSTRUN ]; then
    $OP_FIRSTRUN
fi

