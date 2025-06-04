
BAK_CFG		= /boot/config-6.14.0
CC			= cc

BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
MAKE_FLAGS	= ARCH=x86_64 -j$(BUILD_JOBS) -l$(shell nproc)

all: linux config build

# Latest as of project start. ("5 weeks ago")

linux:
	git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

.clang-format:
	ln -s linux/.clang-format .

linux/.config: linux .clang-format
	make -C linux defconfig

mrproper:
	make -C linux mrproper
	cp $(BAK_CFG) linux/.config

config: linux/.config
	make -C linux ${MAKE_FLAGS} menuconfig

build:
	make CC=${CC} ${MAKE_FLAGS} -C linux

install:
	sudo bash tools/install-linux.sh

# WARNING! Experimental.
# TODO: Fully debuggable environment without the use of previous LFS image.

vm:
	qemu-system-x86_64 \
		-kernel ./linux/arch/x86_64/boot/bzImage \
		-append "console=ttyS0 root=/dev/sda4 nokaslr" \
		-fsdev local,id=fsdev0,path=$(shell pwd),security_model=none \
		-net nic \
		-net user,id=vmnic,hostfwd=tcp::2222-:22 \
		-device virtio-9p-pci,id=fs0,fsdev=fsdev0,mount_tag=qemu_share \
		-bios /nix/store/0wbr8qhmbddqd419hfapj3pkzn71xrq1-OVMF-202402-fd/FV/OVMF.fd \
		-hda ./img/lfs.qcow2 \
		-k de \
		-usb \
		-m 4G \
		-smp sockets=1,cores=4,threads=4 \
		-enable-kvm \
		-machine type=pc,accel=kvm \
		-gdb tcp::1122 \
		-nographic

.PHONY: mrproper config build install vm
