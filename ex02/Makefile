ROOT			= ..
KERN_BUILD		= linux
# Same tree as Assignment 00, so it shares that config.
BAK_CFG			= $(ROOT)/ex00/.config
FULL_SRC_PATH	= $(ROOT)/$(KERN_BUILD)
LOG				= $(abspath kernel.log)
KERNEL_IMG		= $(FULL_SRC_PATH)/arch/x86/boot/bzImage

BUILD_JOBS	?= $(shell expr $(shell nproc) \* 3 / 2)

PATCHES		= 0001-Makefile-set-EXTRAVERSION-to-thor_kernel.patch

# CONFIG_LOCALVERSION_AUTO appends "-g<sha>" past a release tag, and "-dirty"
# on uncommitted changes. setlocalversion looks for an annotated tag named
# after the version (v6.14-thor_kernel for 6.14.0-thor_kernel): with HEAD
# exactly on it and a clean tree, the kernel is plain 6.14.0-thor_kernel.
TAG			= v6.14-thor_kernel

all: linux unhook patch build log

GIT_TREE	= git -C $(FULL_SRC_PATH)
PATCH_ID	= git patch-id --stable | cut -d' ' -f1

# ex09's hooks edit tracked files in this tree, which would add "-dirty".
unhook: linux
	$(MAKE) -C $(ROOT)/ex09 unhook KERN_BUILD=$(abspath $(FULL_SRC_PATH))

# Commit the patches (git am) instead of leaving them as working-tree changes,
# so the version string doesn't get "-dirty". Idempotent: already-committed
# patches are skipped, and an uncommitted copy is taken out before committing.
# Then (re)tag HEAD, see TAG.
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
	$(GIT_TREE) tag -f -a -m "EXTRAVERSION = -thor_kernel (ex02)" $(TAG) HEAD

# Drop the patch commit again (only when it's the top commit; --keep leaves
# other local changes alone), or reverse an uncommitted copy. The tag goes too.
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
	-$(GIT_TREE) tag -d $(TAG) 2>/dev/null

linux:
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

format: linux
	$(FULL_SRC_PATH)/scripts/checkpatch.pl --strict --no-tree $(PATCHES)

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
else
driver clean fclean:
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

.PHONY: all linux unhook config savecfg mrproper patch patch-r build driver log clean fclean format re vm vm-gui debug
