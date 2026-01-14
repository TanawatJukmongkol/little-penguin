
BAK_CFG		= ex00/config
CC			= cc

BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
MAKE_FLAGS	= \
	LLVM=1 \
	ARCH=x86_64 \
	-j$(BUILD_JOBS) -l$(shell nproc)

VM_DISK     = /ft_linux/lfs.qcow2

all: linux build

# Latest as of project start. ("5 weeks ago")

linux:
	git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
	cp ex00/config linux/.config

.clang-format:
	ln -s linux/.clang-format .

linux/.config: linux .clang-format
	make -C linux defconfig

mrproper:
	make -C linux mrproper
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
		-bios /usr/share/ovmf/OVMF.fd \
		-hda $(VM_DISK) \
		-k de \
		-m 4G \
		-smp sockets=1,cores=4,threads=4 \
		-enable-kvm \
		-machine type=pc,accel=kvm \

KERN_FLAGS = \
		root=/dev/sda4 \
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

vm-usb:
	qemu-system-x86_64 \
		$(KERNEL_DEBUG) $(BASE_QEMU) $(DEBUG_QEMU) \
		-usb \
		-device piix3-usb-uhci,id=uhci \
		-device usb-host,hostbus=1,hostport=2,bus=uhci.0 \
		-device usb-host,hostbus=1,hostport=1,bus=uhci.0

vm-gui:
	qemu-system-x86_64 \
		$(KERNEL_NORM) $(BASE_QEMU) \
		-vga virtio

debug:
	gdb linux/vmlinux -tui

.PHONY: mrproper config build install vm vm-usb vm-usb-disabled
