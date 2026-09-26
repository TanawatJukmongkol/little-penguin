# Sourced by run.sh inside the guest, as root; helpers come from lib.sh.
# Needs the USB keyboard dongle: boot.sh redirects it through SPICE when this
# prints READY_FOR_USB_PLUG, and unplugs it on READY_FOR_USB_UNPLUG.
check "udev rule and helper script installed" "cp $S/ex04/10-keyboard.rules /etc/udev/rules.d/10-keyboard.rules \
	&& cp $S/ex04/keyboard_driver.sh /usr/local/bin/keyboard_driver.sh \
	&& chmod +x /usr/local/bin/keyboard_driver.sh \
	&& sed -i 's|\$KMOD_DIR|$S/ex04|g; s|\$KNAME|keyboard-driver|g' /usr/local/bin/keyboard_driver.sh \
	&& udevadm control --reload-rules"
check "module isn't loaded before plugging in" '[ ! -d /sys/module/keyboard_driver ]'
echo READY_FOR_USB_PLUG > /dev/console
i=0
while [ ! -d /sys/module/keyboard_driver ] && [ $i -lt 60 ]; do
	sleep 1
	i=$((i + 1))
done
sleep 2
check "module is loaded after plugging in a keyboard" '[ -d /sys/module/keyboard_driver ]'
check "keyboard interface is bound to keyboard_driver" 'ls /sys/bus/usb/drivers/keyboard_driver | grep -q :'
echo READY_FOR_USB_UNPLUG > /dev/console
i=0
while [ -d /sys/module/keyboard_driver ] && [ $i -lt 60 ]; do
	sleep 1
	i=$((i + 1))
done
check "module is unloaded after unplugging" '[ ! -d /sys/module/keyboard_driver ]'
check "no kernel BUG or Oops" "! dmesg | grep -qE 'BUG:|Oops|Call Trace'"
