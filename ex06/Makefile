ROOT			= ..
KERN_BUILD		= linux-next
BAK_CFG			= .config
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

GIT_TREE	= git -C $(FULL_SRC_PATH)
PATCH_ID	= git patch-id --stable | cut -d' ' -f1

# Commit the patches (git am) instead of leaving them as working-tree changes,
# so the version string doesn't get "-dirty". Idempotent: already-committed
# patches are skipped, and an uncommitted copy is taken out before committing.
patch: linux
	for p in $(PATCHES); do \
		id=$$(cat $$p | $(PATCH_ID)); \
		files=$$($(GIT_TREE) apply --numstat $(CURDIR)/$$p | cut -f3); \
		$(GIT_TREE) log --no-merges -n 5 -p -- $$files | $(PATCH_ID) | grep -qx "$$id" && continue; \
		if $(GIT_TREE) apply -R --check $(CURDIR)/$$p 2>/dev/null; then \
			$(GIT_TREE) apply -R $(CURDIR)/$$p || exit 1; \
		fi; \
		$(GIT_TREE) am $(CURDIR)/$$p || exit 1; \
	done

# Drop the patch commit again (only when it's the top commit; --keep leaves
# other local changes alone), or reverse an uncommitted copy.
patch-r: linux
	for p in $(PATCHES); do \
		id=$$(cat $$p | $(PATCH_ID)); \
		files=$$($(GIT_TREE) apply --numstat $(CURDIR)/$$p | cut -f3); \
		if [ "$$($(GIT_TREE) show HEAD -- $$files | $(PATCH_ID))" = "$$id" ]; then \
			$(GIT_TREE) reset --keep HEAD~1 || exit 1; \
		elif $(GIT_TREE) apply -R --check $(CURDIR)/$$p 2>/dev/null; then \
			$(GIT_TREE) apply -R $(CURDIR)/$$p || exit 1; \
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

log: $(KERNEL_IMG)
	KERN_BUILD=$(KERN_BUILD) \
	$(MAKE) -C $(ROOT) log LOG=$(LOG)

# From the root's loops (NESTED set) these do nothing: the root already
# covers them. Run directly, they hand over to the root.
ifndef NESTED
driver: build
	KERN_BUILD=$(KERN_BUILD) NESTED=1 \
	BUILD_JOBS=$(BUILD_JOBS) \
	$(MAKE) -C $(ROOT) driver

clean:
	KERN_BUILD=$(KERN_BUILD) NESTED=1 \
	$(MAKE) -C $(ROOT) clean

fclean:
	KERN_BUILD=$(KERN_BUILD) NESTED=1 \
	$(MAKE) -C $(ROOT) fclean

format:
	KERN_BUILD=$(KERN_BUILD) NESTED=1 \
	$(MAKE) -C $(ROOT) format
else
driver clean fclean format:
	@:
endif

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

.PHONY: all linux config savecfg mrproper patch patch-r build driver log clean fclean format re vm vm-gui debug
