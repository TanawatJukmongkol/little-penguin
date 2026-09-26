#!/usr/bin/env bash
# Boot little-penguin, run run.sh [exNN|proof:exNN...] in the guest as root,
# redirect the USB dongle when the guest asks for it, wait for power-off.
# Usage: KERN_BUILD=<kernel tree> tools/tests/boot.sh ex01 proof:ex04 ...
# VM_NAME, VM_DISK (relative to the repo root, or absolute) and ROOT_PART
# default to the root Makefile's values, which it passes down.
# The serial console goes to tools/tests/serial.log.
set -u
cd "$(dirname "$0")/../.."
EXS="$*"
LOG="$PWD/tools/tests/serial.log"; KTREE="${KERN_BUILD:-$PWD/linux}"; KIMG="$KTREE/arch/x86/boot/bzImage"; TIMEOUT=300
VM_NAME="${VM_NAME:-little-penguin}"; VM_DISK="${VM_DISK:-ft_linux/lfs.qcow2}"; ROOT_PART="${ROOT_PART:-/dev/sda4}"
case $VM_DISK in /*) ;; *) VM_DISK="$PWD/$VM_DISK" ;; esac
# On a KASAN kernel, print every report, not only the first one of the boot.
KASAN_ARGS=""
grep -q "^CONFIG_KASAN=y" "$KTREE/.config" 2>/dev/null && KASAN_ARGS=" kasan_multi_shot"
CMDLINE="root=$ROOT_PART console=ttyS0 nokaslr loglevel=4 panic=-1 TERM=dumb$KASAN_ARGS systemd.run=\"/bin/sh -c 'mountpoint -q /mnt/qemu_share || mount -t 9p -o trans=virtio,version=9p2000.L qemu_share /mnt/qemu_share; exec /bin/sh /mnt/qemu_share/tools/tests/run.sh $EXS'\" systemd.run_success_action=poweroff systemd.run_failure_action=poweroff"
V="virsh --connect qemu:///session"
SCR="$PWD/tools/tests"
DOMAIN="$PWD/tools/vm/proof-domain.xml"
[ -f "$KIMG" ] || { echo "no kernel image at $KIMG"; exit 1; }
$V destroy "$VM_NAME" >/dev/null 2>&1; $V undefine "$VM_NAME" >/dev/null 2>&1
export VM_NAME KERNEL_IMG="$KIMG" CMDLINE VM_DISK_ABS="$VM_DISK" PWD \
	OVMF_PATH="${OVMF_PATH:-/usr/share/ovmf/OVMF.fd}" QEMU_BIN="${QEMU_BIN:-/usr/bin/qemu-system-x86_64}" \
	CPU_SOCKETS=$(lscpu | awk -F: '/^Socket\(s\):/{gsub(/ /,"",$2); print $2}') \
	CPU_CORES=$(lscpu | awk -F: '/^Core\(s\) per socket:/{gsub(/ /,"",$2); print $2}') \
	CPU_THREADS=$(lscpu | awk -F: '/^Thread\(s\) per core:/{gsub(/ /,"",$2); print $2}') \
	VCPU_COUNT=$(nproc) VCPU_LAST=$(($(nproc) - 1)) VIDEO_DEVICE=''
envsubst '$VM_NAME $KERNEL_IMG $CMDLINE $VM_DISK_ABS $PWD $OVMF_PATH $QEMU_BIN $CPU_SOCKETS $CPU_CORES $CPU_THREADS $VCPU_COUNT $VCPU_LAST $VIDEO_DEVICE' < tools/vm/little-penguin.xml \
	| sed -e "s|<serial type=\"pty\">|<serial type=\"file\"><source path=\"$LOG\" append=\"off\"/>|" -e '/<console type="pty">/d' > "$DOMAIN"
$V define "$DOMAIN" >/dev/null && $V start "$VM_NAME" || exit 1
# SPICE USB redirection (what virt-manager's "Redirect USB device" does):
# spicy plugs the dongle in on connect, and unplugs it when it exits.
attached=0; spicy_pid=""
for i in $(seq $TIMEOUT); do
	st=$($V domstate "$VM_NAME" 2>/dev/null)
	[ "$st" = "shut off" ] && break
	if [ $attached = 0 ] && grep -aq READY_FOR_USB_PLUG "$LOG" 2>/dev/null; then
		echo "redirecting USB at ${i}s"
		spicy --uri="$($V domdisplay "$VM_NAME")" \
			--spice-usbredir-redirect-on-connect='-1,0x0c45,0xfefe,-1,1' >"$SCR/spicy.log" 2>&1 &
		spicy_pid=$!; attached=1
	fi
	if [ $attached = 1 ] && grep -aq READY_FOR_USB_UNPLUG "$LOG" 2>/dev/null; then
		echo "unplugging USB at ${i}s"; kill $spicy_pid; attached=2
	fi
	sleep 1
done
[ -n "$spicy_pid" ] && kill $spicy_pid 2>/dev/null
st=$($V domstate "$VM_NAME" 2>/dev/null)
echo "final state: $st after ${i}s"
$V destroy "$VM_NAME" >/dev/null 2>&1; $V undefine "$VM_NAME" >/dev/null 2>&1
[ "$st" = "shut off" ] || { echo "guest didn't power off within ${TIMEOUT}s"; exit 1; }
