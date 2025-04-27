
BAK_CFG		= /boot/config-6.14.0
CC		= cc

BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
MAKE_FLAGS	= -j$(BUILD_JOBS) -l$(shell nproc)

all: config build

# Latest as of project start. ("5 weeks ago")

linux:
	git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
	ln -s linux/.clang-format .

linux/.config: linux
	make -C linux defconfig

mrproper:
	make -C linux mrproper
	cp $(BAK_CFG) linux/.config

config: linux/.config
	make -C linux menuconfig

build:
	make CC=${CC} ${MAKE_FLAGS} -C linux

install:
	sudo bash tools/install-linux.sh

# WARNING! Experimental.
# TODO: Fully debuggable environment without the use of previous LFS image.
vm:
	qemu-system-x86_64 \
		-kernel ./linux/arch/x86_64/boot/bzImage \
		-append "console=ttyS0" \
		-m 512 \
		--enable-kvm \
		-cpu host

.PHONY: mrproper config build install vm
