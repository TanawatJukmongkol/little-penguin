
# Settings
NAME		= mymounts
CC			= cc
CFLAGS		= -Wall -Werror -Wextra -Wno-unused-parameter -Wno-sign-compare -g -DDEBUG
BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
# LLVM=1/LD=ld/ARCH=x86_64 mirror ../Makefile: this tree only has clang
# available, and ld.lld corrupts the x86 real-mode trampoline, so host
# tools (HOSTCC) and the module build both need to be pointed at clang
# explicitly, not just $(CC).
MAKE_FLAGS	= LLVM=1 LD=ld ARCH=x86_64 -j$(BUILD_JOBS) -l$(shell nproc)

# Kernel settings
KERN_VERSION	= $(shell uname -r)
KERN_BUILD		= ../linux

# In-tree location: rather than the M= out-of-tree module build used by
# the other exercises, this driver's src/ is symlinked straight into the
# kernel source tree and wired into its Kconfig/Makefile, so it builds
# under normal KBUILD and is toggled via CONFIG_MYMOUNTS (menuconfig, or
# the `config` target below) instead of being forced on with obj-m.
MOD_SUBDIR		= drivers/misc
CONFIG_SYM		= CONFIG_MYMOUNTS
TREE_DIR		= $(KERN_BUILD)/$(MOD_SUBDIR)/$(NAME)
TREE_KCONFIG	= $(KERN_BUILD)/$(MOD_SUBDIR)/Kconfig
TREE_MAKEFILE	= $(KERN_BUILD)/$(MOD_SUBDIR)/Makefile

# Source files
SRCS		= $(addprefix src/, \
	main.c \
	mounts.c \
)

HEADERS		= $(addprefix include/, \
)

# Built-in code has no standalone .ko artifact - it's only produced by
# building the kernel image itself, so `all` just runs the normal
# top-level kbuild target instead of targeting a module path.
all: ../linux $(SRCS) $(HEADERS) config
	bear -- make -C $(KERN_BUILD) \
	CC=$(CC) SRCS="$(SRCS)" $(MAKE_FLAGS)

../linux:
	make -C ..

# Symlink the driver's source directory into the kernel tree so KBUILD
# discovers it as drivers/misc/mymounts.
$(TREE_DIR):
	ln -s $(abspath src) $(TREE_DIR)

# Register the driver with the surrounding Kconfig/Makefile. Idempotent,
# since a fresh `make linux` clone regenerates drivers/misc/{Kconfig,Makefile}
# without our hooks every time.
hook: $(TREE_DIR)
	grep -q '$(MOD_SUBDIR)/$(NAME)/Kconfig' $(TREE_KCONFIG) || \
		sed -i 's#^endmenu#source "$(MOD_SUBDIR)/$(NAME)/Kconfig"\nendmenu#' $(TREE_KCONFIG)
	grep -q '$(CONFIG_SYM)' $(TREE_MAKEFILE) || \
		printf 'obj-$$($(CONFIG_SYM))\t\t+= $(NAME)/\n' >> $(TREE_MAKEFILE)

# Remove the in-tree symlink and its Kconfig/Makefile hooks.
unhook:
	rm -f $(TREE_DIR)
	sed -i '\#source "$(MOD_SUBDIR)/$(NAME)/Kconfig"#d' $(TREE_KCONFIG)
	sed -i '\#$(CONFIG_SYM)#d' $(TREE_MAKEFILE)

# Enable CONFIG_MYMOUNTS=y non-interactively: built directly into the
# kernel image (not a loadable .ko), so the driver can link against VFS
# helpers (seq_path_root(), security_sb_show_options(), ...) that are
# real (non-static) symbols but aren't EXPORT_SYMBOL'd for module use.
# `make -C $(KERN_BUILD) menuconfig` works too, under Device Drivers >
# Misc devices.
config: hook
	$(KERN_BUILD)/scripts/config --file $(KERN_BUILD)/.config --enable $(CONFIG_SYM)
	make -C $(KERN_BUILD) CC=$(CC) $(MAKE_FLAGS) olddefconfig

clean: unhook
	make -C $(KERN_BUILD) $(MAKE_FLAGS) clean

format: all
	../linux/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(SRCS)

re: clean all

.PHONY: all clean format re install-rules uninstall-rules hook config unhook
