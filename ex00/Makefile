ROOT			= ..
KERN_BUILD		= linux
BAK_CFG			= .config
FULL_SRC_PATH	= $(ROOT)/$(KERN_BUILD)
LOG				= $(abspath kernel.log)
KERNEL_IMG		= $(FULL_SRC_PATH)/arch/x86/boot/bzImage

BUILD_JOBS	?= $(shell expr $(shell nproc) \* 3 / 2)

all: linux unpatch build log

linux:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) linux BAK_CFG=$(abspath $(BAK_CFG))

savecfg:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) savecfg BAK_CFG=$(abspath $(BAK_CFG))

# ex02 patches this same tree; take its EXTRAVERSION back out.
unpatch:
	$(MAKE) -C $(ROOT)/ex02 patch-r

mrproper: unpatch
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) mrproper BAK_CFG=$(abspath $(BAK_CFG))

config:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) config BAK_CFG=$(abspath $(BAK_CFG))

build: linux
	KERN_BUILD=$(KERN_BUILD) \
	BUILD_JOBS=$(BUILD_JOBS) \
	$(MAKE) -C $(ROOT) build

# Only build the kernel when there's no image yet; `make build` rebuilds.
$(KERNEL_IMG):
	$(MAKE) build

driver: build
	KERN_BUILD=$(KERN_BUILD) \
	BUILD_JOBS=$(BUILD_JOBS) \
	$(MAKE) -C $(ROOT) driver

log: $(KERNEL_IMG)
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) log LOG=$(LOG)

clean:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) clean

fclean:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) fclean

re: clean mrproper all

vm: $(KERNEL_IMG)
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) vm

vm-gui: $(KERNEL_IMG)
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) vm-gui

debug:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) debug

.PHONY: all linux unpatch config savecfg mrproper build driver log clean fclean re vm vm-gui debug
