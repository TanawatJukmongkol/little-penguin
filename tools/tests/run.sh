#!/bin/sh
# Runs inside the guest as root (started by boot.sh through systemd.run).
# Each argument is exNN, to run tools/tests/exNN/guest.sh into
# tools/tests/exNN/guest.log, or proof:exNN, to run exNN/proof.sh into the
# exercise's exNN/proof.log.
S=/mnt/qemu_share
T=$S/tools/tests

mountpoint -q $S || mount -t 9p -o trans=virtio,version=9p2000.L qemu_share $S
. $T/lib.sh

# On a KASAN kernel, each exercise's guest tests also check that they didn't
# trigger a new KASAN report.
KASAN=0
dmesg | grep -q 'KernelAddressSanitizer initialized' && KASAN=1

for arg; do
	case $arg in
	proof:*) ex=${arg#proof:}; script=$T/$ex/proof.sh; out=$S/$ex/proof.log ;;
	*) ex=$arg; script=$T/$ex/guest.sh; out=$T/$ex/guest.log ;;
	esac
	if [ -f $script ]; then
		before=$(dmesg | grep -c 'BUG: KASAN')
		(. $script) > $out 2>&1
		case $arg in proof:*|kasan) continue ;; esac
		[ $KASAN = 1 ] && check "no new KASAN reports" "[ \$(dmesg | grep -c 'BUG: KASAN') -eq $before ] \
			|| { dmesg | grep -A 40 'BUG: KASAN' | tail -n 80; exit 1; }" >> $out
	else
		ko "no $script" > $out
	fi
done
sync
echo TESTS_DONE > /dev/console
