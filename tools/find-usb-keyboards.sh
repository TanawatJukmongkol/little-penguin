#!/usr/bin/env bash
# Finds USB boot-protocol keyboards (HID class 03, subclass 01, protocol 01 --
# the same match used by ex04/10-keyboard.rules) currently attached to the
# host and prints their "vendorid:productid" pairs, one per line.
#
# Used by `make vm-usb` so USB passthrough doesn't depend on host-specific
# bus/port numbers, which differ on every machine.
set -euo pipefail
shopt -s nullglob

for intf in /sys/bus/usb/devices/*:*/bInterfaceClass; do
	[ "$(cat "$intf")" = "03" ] || continue
	dir=$(dirname "$intf")
	[ "$(cat "$dir/bInterfaceSubClass" 2>/dev/null)" = "01" ] || continue
	[ "$(cat "$dir/bInterfaceProtocol" 2>/dev/null)" = "01" ] || continue
	dev="/sys/bus/usb/devices/$(basename "$dir" | cut -d: -f1)"
	[ -f "$dev/idVendor" ] && [ -f "$dev/idProduct" ] || continue
	echo "$(cat "$dev/idVendor"):$(cat "$dev/idProduct")"
done | sort -u
