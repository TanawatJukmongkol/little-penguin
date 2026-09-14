ROOT			= ..
LINUX_SRC		= linux-next
BAK_CFG			= config
FULL_SRC_PATH	= $(ROOT)/$(LINUX_SRC)

CC			?= cc
BUILD_JOBS	?= $(shell expr $(shell nproc) \* 3 / 2)

PROJECTS	= ex01 ex03 ex04 ex05 ex07 ex08 ex09

MAKE_FLAGS	= \
	LLVM=1 \
	LD=ld \
	ARCH=x86_64 \
	-j$(BUILD_JOBS) -l$(shell nproc)

all: linux mrproper build

linux: $(FULL_SRC_PATH)

$(FULL_SRC_PATH):
	git clone --depth 1 \
		https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git \
		$(FULL_SRC_PATH)
	cd $(FULL_SRC_PATH) && git fetch --tags

savecfg:
	cp $(FULL_SRC_PATH)/.config $(BAK_CFG)

mrproper:
	make -C $(FULL_SRC_PATH) mrproper
	cp $(BAK_CFG) $(FULL_SRC_PATH)/.config

config: $(FULL_SRC_PATH)/.config
	make -C $(FULL_SRC_PATH) $(FULL_SRC_PATH) menuconfig
	cp $(FULL_SRC_PATH)/.config $(BAK_CFG)

build: linux
	LINUX_SRC=$(LINUX_SRC) \
	CC=$(CC) \
	BUILD_JOBS=$(BUILD_JOBS) \
	$(MAKE) -C $(ROOT) build

driver: linux
	LINUX_SRC=$(LINUX_SRC) \
	CC=$(CC) \
	BUILD_JOBS=$(BUILD_JOBS) \
	$(MAKE) -C $(ROOT) driver

clean:
	$(MAKE) -C $(ROOT) clean

re: clean mrproper all

vm: linux
	LINUX_SRC=$(LINUX_SRC) \
	$(MAKE) -C $(ROOT) vm

vm-usb: linux
	LINUX_SRC=$(LINUX_SRC) \
	$(MAKE) -C $(ROOT) vm-usb

vm-gui: linux
	LINUX_SRC=$(LINUX_SRC) \
	$(MAKE) -C $(ROOT) vm-gui

debug: linux
	LINUX_SRC=$(LINUX_SRC) \
	$(MAKE) -C $(ROOT) debug

.PHONY: all linux config savecfg mrproper build driver clean re vm vm-usb vm-gui debug
