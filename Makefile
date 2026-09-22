# Libvirt configurations
VM_NAME = kernel-debug-vm
VM_DISK = ft_linux/lfs.qcow2
ROOT_PART = /dev/sda4
OVMF_PATH ?= /usr/share/ovmf/OVMF.fd
KERN_BUILD ?= linux

# Kernel configurations
KERN_BUILD ?= linux
CMDLINE ?= root=$(ROOT_PART) loglevel=4 console=ttyS0 nokaslr
CC = cc
BAK_CFG = ex00/config

# Project build (1st line = in-tree kernels, 2nd line = modules)
PROJECTS = \
	ex09 \
	ex01 ex03 ex04 ex05 ex07 ex08

BUILD_JOBS = $(shell expr $(shell nproc) \* 3 / 2)

# ld.lld (LLVM=1's default linker) corrupts the x86 real-mode trampoline's
# elf32-i386 sub-link: the fields in struct real_mode_header come out
# containing garbage, and the kernel page-faults in setup_real_mode() within
# 0.2s of boot, every time, on every clang version tested. Forcing bfd `ld`
# for the link step (keeping clang for compilation) avoids it entirely.

MAKE_FLAGS = \
             LLVM=1 \
             LD=ld \
             ARCH=x86_64 \
             HOSTCC=clang \
             HOSTCXX=clang++ \
             -j$(BUILD_JOBS) -l$(shell nproc)

# House cleaning, and misc variables
# Handle paths properly inside the shell environment scope
PWD = $(shell pwd)
VM_DISK_ABS = $(PWD)/$(VM_DISK)
KERNEL_IMG ?= $(PWD)/$(KERN_BUILD)/arch/x86_64/boot/bzImage
VIDEO_DEVICE = <!-- Headless Mode emulated video card dropped -->

all: linux mrproper build

# Latest as of project start. ("5 weeks ago")

linux:
	@if [ ! -d "$(KERN_BUILD)" ]; then \
		echo "Cloning Linux v6.14 into $(KERN_BUILD)..."; \
		git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git "$(KERN_BUILD)"; \
	fi
	@if [ ! -f "$(KERN_BUILD)/.config" ]; then \
		cp ex00/config "$(KERN_BUILD)/.config"; \
	fi

.clang-format:
	ln -s $(KERN_BUILD)/.clang-format .

$(KERN_BUILD)/.config: linux .clang-format
	make -C $(KERN_BUILD) $(MAKE_FLAGS) defconfig

savecfg:
	cp $(KERN_BUILD)/.config $(BAK_CFG)

mrproper:
	make CC=$(CC) $(MAKE_FLAGS) -C $(KERN_BUILD) mrproper
	cp $(BAK_CFG) $(KERN_BUILD)/.config

config: $(KERN_BUILD)/.config
	make -C $(KERN_BUILD) $(MAKE_FLAGS) menuconfig
	cp $(KERN_BUILD)/.config $(BAK_CFG)

build:
	KERN_BUILD=${KERN_BUILD} make CC=$(CC) $(MAKE_FLAGS) -C $(KERN_BUILD)

driver:
	for folder in $(PROJECTS); do \
		KERN_BUILD=$(abspath $(KERN_BUILD)) $(MAKE) --no-print-directory -C $$folder; \
	done

clean:
	for folder in $(PROJECTS); do \
		$(MAKE) --no-print-directory -C $$folder clean; \
	done

re: clean all

vm: vm-clean vm-xml-boot

vm-gui:
	@$(MAKE) vm \
		KERNEL_IMG="$(KERNEL_IMG)" \
		CMDLINE="root=$(ROOT_PART) loglevel=4" \
		VIDEO_DEVICE='<video><model type="virtio"/></video>' \
		IS_GUI=1

vm-clean:
	-@virsh --connect qemu:///session destroy $(VM_NAME) >/dev/null 2>&1 || true
	-@virsh --connect qemu:///session undefine $(VM_NAME) >/dev/null 2>&1 || true

vm-xml-boot:
	@echo "Interpolating variables via envsubst and defining domain profile..."
	@VM_NAME="$(VM_NAME)" \
	 KERNEL_IMG="$(KERNEL_IMG)" \
	 CMDLINE="$(CMDLINE)" \
	 VM_DISK_ABS="$(VM_DISK_ABS)" \
	 PWD="$(PWD)" \
	 OVMF_PATH="$(OVMF_PATH)" \
	 VIDEO_DEVICE='$(VIDEO_DEVICE)' \
	 envsubst '$$VM_NAME $$KERNEL_IMG $$CMDLINE $$VM_DISK_ABS $$PWD $$OVMF_PATH $$VIDEO_DEVICE' < virtmgr.xml > /tmp/$(VM_NAME).xml
	@virsh --connect qemu:///session define /tmp/$(VM_NAME).xml
	@rm -f /tmp/$(VM_NAME).xml
	@echo "Starting virtual domain..."
	@virsh --connect qemu:///session start $(VM_NAME)
	@if [ "$(IS_GUI)" != "1" ]; then \
		virsh --connect qemu:///session console $(VM_NAME); \
	fi

.PHONY: savecfg mrproper config \
        build driver clean re install \
	    vm vm-gui vm-clean vm-xml-boot
