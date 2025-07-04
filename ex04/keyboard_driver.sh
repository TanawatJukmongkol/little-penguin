#!/usr/bin/env bash

DEVPATH="$1"
USB_SYSFS=/sys/bus/usb/drivers/usbhid/$(basename "/sys$DEVPATH" | cut -f1 -d":"):*

# Loop over all interfaces like 2-1:1.0, 2-1:1.1, etc.

for intf_path in $USB_SYSFS; do
    intf_name=$(basename "$intf_path")
    if [ -e "/sys/bus/usb/drivers/usbhid/$intf_name" ]; then
        # Unbinds current driver in use
        echo -n "$intf_name" > /sys/bus/usb/drivers/usbhid/unbind
        # Rebinds driver to our custom driver
        echo -n "$intf_name" > /sys/bus/usb/drivers/keyboard_driver/bind
    fi
done

/sbin/insmod $KMOD_DIR/$KNAME.ko
