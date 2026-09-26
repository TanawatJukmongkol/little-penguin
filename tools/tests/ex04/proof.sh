# Sourced by run.sh inside the guest, as root; writes ex04/proof.log.
echo "== ex04: module auto-loaded when a USB keyboard is plugged in =="
run uname -r
run cp $S/ex04/10-keyboard.rules /etc/udev/rules.d/10-keyboard.rules
run cp $S/ex04/keyboard_driver.sh /usr/local/bin/keyboard_driver.sh
run chmod +x /usr/local/bin/keyboard_driver.sh
run "sed -i 's|\$KMOD_DIR|$S/ex04|g; s|\$KNAME|keyboard-driver|g' /usr/local/bin/keyboard_driver.sh"
run cat /etc/udev/rules.d/10-keyboard.rules /usr/local/bin/keyboard_driver.sh
run udevadm control --reload-rules
run "lsmod | grep keyboard_driver"
echo "-- plugging in the USB keyboard (host passes the dongle through now)"
echo READY_FOR_USB_PLUG > /dev/console
i=0
while [ ! -d /sys/module/keyboard_driver ] && [ $i -lt 120 ]; do
	sleep 1
	i=$((i + 1))
done
run sleep 2
run "lsusb 2>/dev/null | grep -i 0c45:fefe || cat /sys/bus/usb/devices/*/product"
run "lsmod | grep keyboard_driver"
run "dmesg | grep -iE 'usb [0-9-]+: new|Bridge75|keyboard_driver|input:' | tail -n 12"
echo "-- the keyboard interface is now taken over by keyboard_driver"
run "ls /sys/bus/usb/drivers/keyboard_driver/ | grep :"
run "ls /sys/bus/usb/drivers/usbhid/ | grep :"
echo "-- unplugging the USB keyboard (host stops the redirection now)"
echo READY_FOR_USB_UNPLUG > /dev/console
i=0
while [ -d /sys/module/keyboard_driver ] && [ $i -lt 60 ]; do
	sleep 1
	i=$((i + 1))
done
run sleep 2
run "lsmod | grep keyboard_driver"
run "dmesg | grep -iE 'USB disconnect|keyboard_driver' | tail -n 8"
