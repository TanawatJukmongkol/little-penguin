
BAK_CFG=/boot/config-6.14.0
CC=cc
MAKE_FLAGS="-j8"

all: config build install

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

.PHONY: mrproper config build install
