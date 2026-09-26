#!/bin/sh
# Runs inside the guest as root (started by boot.sh through systemd.run).
# Each argument is exNN, to run tools/tests/exNN/guest.sh into
# tools/tests/exNN/guest.log, or proof:exNN, to run exNN/proof.sh into the
# exercise's exNN/proof.log.
S=/mnt/qemu_share
T=$S/tools/tests

mountpoint -q $S || mount -t 9p -o trans=virtio,version=9p2000.L qemu_share $S
. $T/lib.sh

for arg; do
	case $arg in
	proof:*) ex=${arg#proof:}; script=$T/$ex/proof.sh; out=$S/$ex/proof.log ;;
	*) ex=$arg; script=$T/$ex/guest.sh; out=$T/$ex/guest.log ;;
	esac
	if [ -f $script ]; then
		(. $script) > $out 2>&1
	else
		echo "not ok - no $script" > $out
	fi
done
sync
echo TESTS_DONE > /dev/console
