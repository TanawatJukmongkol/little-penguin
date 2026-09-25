ROOT			= ..
KERN_BUILD		= linux-next
BAK_CFG			= config
FULL_SRC_PATH	= $(ROOT)/$(KERN_BUILD)
LOG				= $(abspath kernel.log)
KERNEL_IMG		= $(FULL_SRC_PATH)/arch/x86/boot/bzImage

BUILD_JOBS	?= $(shell expr $(shell nproc) \* 3 / 2)

PATCHES		= 0001-fs-buffer-fix-NULL-deref-on-folio-less-bh-in-__bh_submit.patch

all: linux patch build log

# linux-next isn't the root's v6.14 tag, so clone it here first; the root
# `linux` rule then only seeds .config.
$(FULL_SRC_PATH):
	git clone --depth 1 \
		https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git \
		$(FULL_SRC_PATH)
	cd $(FULL_SRC_PATH) && git fetch --tags

# Idempotent: skip patches that are already applied (patch-r: not applied),
# instead of patch(1) stopping to ask about a "reversed" patch.
patch: linux
	for p in $(PATCHES); do \
		if ! patch -R -p1 -s -f --dry-run -d $(FULL_SRC_PATH) < $$p >/dev/null; then \
			patch -p1 -d $(FULL_SRC_PATH) < $$p || exit 1; \
		fi; \
	done

patch-r: linux
	for p in $(PATCHES); do \
		if patch -R -p1 -s -f --dry-run -d $(FULL_SRC_PATH) < $$p >/dev/null; then \
			patch -R -p1 -d $(FULL_SRC_PATH) < $$p || exit 1; \
		fi; \
	done

linux: $(FULL_SRC_PATH)
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) linux BAK_CFG=$(abspath $(BAK_CFG))

savecfg:
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) savecfg BAK_CFG=$(abspath $(BAK_CFG))

mrproper:
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

.PHONY: all linux config savecfg mrproper patch patch-r build driver log clean re vm vm-gui debug
