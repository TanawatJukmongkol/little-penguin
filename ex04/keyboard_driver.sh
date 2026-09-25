#!/usr/bin/env bash

grep -q '^keyboard_driver ' /proc/modules || /sbin/insmod $KMOD_DIR/$KNAME.ko
