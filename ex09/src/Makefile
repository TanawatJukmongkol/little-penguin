# Kbuild fragment for the mymounts driver. This directory is symlinked
# in-tree at $(KERN_BUILD)/drivers/misc/mymounts and registered with the
# surrounding Kconfig/Makefile by the top-level ../Makefile's `hook`
# target, so the module builds through normal KBUILD, gated on
# CONFIG_MYMOUNTS, instead of the M= external-module path.

obj-$(CONFIG_MYMOUNTS)	+= mymounts.o
mymounts-y			+= $(patsubst src/%.c,%.o,$(SRCS))

# struct mount/mnt_namespace are only fully defined in fs/mount.h, which
# is private to fs/ and not on the normal module include path.
ccflags-y			+= -I$(srctree)/fs
