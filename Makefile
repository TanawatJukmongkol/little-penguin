
BAK_CFG		= ex00/config
CC			= cc

BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)

# ld.lld (LLVM=1's default linker) corrupts the x86 real-mode trampoline's
# elf32-i386 sub-link: the fields in struct real_mode_header come out
# containing garbage, and the kernel page-faults in setup_real_mode() within
# 0.2s of boot, every time, on every clang version tested. Forcing bfd `ld`
# for the link step (keeping clang for compilation) avoids it entirely.

MAKE_FLAGS	= \
	LLVM=1 \
	LD=ld \
	ARCH=x86_64 \
	-j$(BUILD_JOBS) -l$(shell nproc)

VM_DISK		= ft_linux/lfs.qcow2
ROOT_PART	= /dev/sda4

# Overridden by the flake's shellHook when running inside `nix develop`;
# falls back to the Debian ovmf package path for bare-host use.
OVMF_PATH	?= /usr/share/ovmf/OVMF.fd

all: linux build

# Latest as of project start. ("5 weeks ago")

linux:
	git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
	cp ex00/config linux/.config

.clang-format:
	ln -s linux/.clang-format .

linux/.config: linux .clang-format
	make -C linux ${MAKE_FLAGS} defconfig

mrproper:
	make CC=${CC} ${MAKE_FLAGS} -C linux mrproper
	cp $(BAK_CFG) linux/.config

config: linux/.config
	make -C linux ${MAKE_FLAGS} menuconfig
	cp linux/.config ${BAK_CFG}

build:
	make CC=${CC} ${MAKE_FLAGS} -C linux

re: mrproper all

# WARNING! Experimental.
# TODO: Fully debuggable environment without the use of previous LFS image.

BASE_QEMU = \
		-fsdev local,id=fsdev0,path=$(shell pwd),security_model=none \
		-net nic \
		-net user,id=vmnic,hostfwd=tcp::2222-:22 \
		-device virtio-9p-pci,id=fs0,fsdev=fsdev0,mount_tag=qemu_share \
		-bios $(OVMF_PATH) \
		-hda $(VM_DISK) \
		-boot menu=on \
		-k de \
		-m 4G \
		-cpu host,+topoext \
		-smp sockets=1,cores=8,threads=2 \
		-enable-kvm \
		-machine type=pc,accel=kvm \

KERN_FLAGS = \
		root=$(ROOT_PART) \
		loglevel=4

KERN_FLAGS_DEBUG = $(KERN_FLAGS) \
		console=ttyS0 \
		nokaslr

KERNEL_NORM = \
		-kernel ./linux/arch/x86_64/boot/bzImage \
		-append "$(KERN_FLAGS)"

DEBUG_QEMU = \
		-gdb tcp::1122 \
		-nographic

KERNEL_DEBUG = \
		-kernel ./linux/arch/x86_64/boot/bzImage \
		-append "$(KERN_FLAGS_DEBUG)"

vm:
	qemu-system-x86_64 \
		$(KERNEL_DEBUG) $(BASE_QEMU) $(DEBUG_QEMU)

# hostbus/hostport identify a physical USB port, which differs on every
# machine. Passthrough is matched by vendorid:productid instead, discovered
# via find-usb-keyboards.sh (or overridden with USB_DEV="vid:pid ..."), so
# this works regardless of the host.
vm-usb:
	@devs="$(USB_DEV)"; \
	[ -n "$$devs" ] || devs=$$(find-usb-keyboards.sh); \
	if [ -z "$$devs" ]; then \
		echo "vm-usb: no USB boot-protocol keyboard detected."; \
		echo "        Plug one in, or override with: make vm-usb USB_DEV=\"vid:pid ...\""; \
		exit 1; \
	fi; \
	args=""; \
	for vp in $$devs; do \
		args="$$args -device usb-host,vendorid=0x$${vp%%:*},productid=0x$${vp##*:},bus=uhci.0"; \
	done; \
	echo "vm-usb: passing through $$devs"; \
	qemu-system-x86_64 \
		$(KERNEL_DEBUG) $(BASE_QEMU) $(DEBUG_QEMU) \
		-usb -device piix3-usb-uhci,id=uhci $$args

vm-gui:
	qemu-system-x86_64 \
		$(KERNEL_NORM) $(BASE_QEMU) \
		-vga virtio

debug:
	gdb linux/vmlinux -tui

.PHONY: mrproper config build install vm vm-usb vm-usb-disabled
