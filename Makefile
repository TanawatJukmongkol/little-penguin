# Libvirt configurations
VM_NAME = kernel-debug-vm
VM_DISK = ft_linux/lfs.qcow2
ROOT_PART = /dev/sda4
OVMF_PATH ?= /usr/share/ovmf/OVMF.fd
# Overridden by the flake's shellHook when running inside `nix develop`;
# falls back to the conventional system path for bare-host use.
QEMU_BIN ?= /usr/bin/qemu-system-x86_64
# host-passthrough only makes the vCPU *model* match the host; the
# <topology> block is a separate, literal claim libvirt hands to the guest
# via CPUID, so it's probed here rather than hardcoded, keeping it correct
# on whichever machine `make vm` actually runs on.
CPU_SOCKETS ?= $(shell lscpu | awk -F: '/^Socket\(s\):/{gsub(/ /,"",$$2); print $$2}')
CPU_CORES ?= $(shell lscpu | awk -F: '/^Core\(s\) per socket:/{gsub(/ /,"",$$2); print $$2}')
CPU_THREADS ?= $(shell lscpu | awk -F: '/^Thread\(s\) per core:/{gsub(/ /,"",$$2); print $$2}')
VCPU_COUNT ?= $(shell nproc)
# Highest valid vcpu id, for the NUMA cell's cpus="0-N" range in virtmgr.xml.
VCPU_LAST ?= $(shell echo $$(($(VCPU_COUNT) - 1)))

# Kernel configurations
KERN_BUILD ?= linux
CMDLINE ?= root=$(ROOT_PART) loglevel=4 console=ttyS0 nokaslr
# Fixed, not ?=: the nix dev shell exports CC=clang, and a different compiler
# name changes every object's saved command, so kbuild rebuilds the whole tree.
CC = cc
BAK_CFG = ex00/.config

# Project build (1st line = in-tree kernels, 2nd line = modules)
PROJECTS = \
	ex09 \
	ex01 ex03 ex04 ex05 ex07 ex08

BUILD_JOBS ?= $(shell expr $(shell nproc) \* 3 / 2)

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

# Boot log capture (`make log`). When SERIAL_LOG is set, vm-xml-boot writes
# the serial console to that file instead of a pty and doesn't attach to it.
LOG ?= $(PWD)/kernel.log
# Seconds to wait for the guest to power itself off before pulling the plug.
BOOT_TIMEOUT ?= 180
# Command run once the guest has booted; its output lands in the log.
LOG_RUN ?= /bin/uname -a
# systemd.run (systemd-run-generator) boots, runs $(LOG_RUN) with its output
# on the console, then powers off cleanly either way. ignore_loglevel puts
# every kernel message on the serial console, and panic=-1 turns a panic into
# a reboot, which the domain (on_reboot=destroy) treats as a shutdown instead
# of hanging forever. TERM=dumb is handed to init, so systemd prints its
# status without colors.
LOG_CMDLINE ?= root=$(ROOT_PART) console=ttyS0 nokaslr ignore_loglevel \
               panic=-1 TERM=dumb systemd.run="$(LOG_RUN)" \
               systemd.run_success_action=poweroff \
               systemd.run_failure_action=poweroff
SERIAL_FILTER = $(if $(SERIAL_LOG), \
	sed -e 's|<serial type="pty">|<serial type="file"><source path="$(SERIAL_LOG)" append="off"/>|' \
	    -e '/<console type="pty">/d', \
	cat)

all: linux build

# Latest as of project start. ("5 weeks ago")

linux:
	@if [ ! -d "$(KERN_BUILD)" ]; then \
		echo "Cloning Linux v6.14 into $(KERN_BUILD)..."; \
		git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git "$(KERN_BUILD)"; \
	fi
	@if [ ! -f "$(KERN_BUILD)/.config" ]; then \
		cp $(BAK_CFG) "$(KERN_BUILD)/.config"; \
	fi

.clang-format:
	ln -s $(KERN_BUILD)/.clang-format .

$(KERN_BUILD)/.config: | linux .clang-format
	make -C $(KERN_BUILD) $(MAKE_FLAGS) defconfig

savecfg:
	cp $(KERN_BUILD)/.config $(BAK_CFG)

mrproper: linux
	make CC=$(CC) $(MAKE_FLAGS) -C $(KERN_BUILD) mrproper
	cp $(BAK_CFG) $(KERN_BUILD)/.config

config: $(KERN_BUILD)/.config
	make -C $(KERN_BUILD) $(MAKE_FLAGS) menuconfig
	cp $(KERN_BUILD)/.config $(BAK_CFG)

build: linux
	KERN_BUILD=$(KERN_BUILD) make CC=$(CC) $(MAKE_FLAGS) -C $(KERN_BUILD)

driver:
	for folder in $(PROJECTS); do \
		KERN_BUILD=$(abspath $(KERN_BUILD)) $(MAKE) --no-print-directory -C $$folder; \
	done

clean:
	for folder in $(PROJECTS); do \
		KERN_BUILD=$(abspath $(KERN_BUILD)) $(MAKE) --no-print-directory -C $$folder clean; \
	done

format:
	for folder in $(PROJECTS); do \
		KERN_BUILD=$(abspath $(KERN_BUILD)) $(MAKE) --no-print-directory -C $$folder format; \
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
	 CMDLINE='$(CMDLINE)' \
	 VM_DISK_ABS="$(VM_DISK_ABS)" \
	 PWD="$(PWD)" \
	 OVMF_PATH="$(OVMF_PATH)" \
	 QEMU_BIN="$(QEMU_BIN)" \
	 CPU_SOCKETS="$(CPU_SOCKETS)" \
	 CPU_CORES="$(CPU_CORES)" \
	 CPU_THREADS="$(CPU_THREADS)" \
	 VCPU_COUNT="$(VCPU_COUNT)" \
	 VCPU_LAST="$(VCPU_LAST)" \
	 VIDEO_DEVICE='$(VIDEO_DEVICE)' \
	 envsubst '$$VM_NAME $$KERNEL_IMG $$CMDLINE $$VM_DISK_ABS $$PWD $$OVMF_PATH $$QEMU_BIN $$CPU_SOCKETS $$CPU_CORES $$CPU_THREADS $$VCPU_COUNT $$VCPU_LAST $$VIDEO_DEVICE' < virtmgr.xml \
	 | $(SERIAL_FILTER) > /tmp/$(VM_NAME).xml
	@virsh --connect qemu:///session define /tmp/$(VM_NAME).xml
	@rm -f /tmp/$(VM_NAME).xml
	@echo "Starting virtual domain..."
	@virsh --connect qemu:///session start $(VM_NAME)
	@if [ "$(IS_GUI)" != "1" ] && [ -z "$(SERIAL_LOG)" ]; then \
		virsh --connect qemu:///session console $(VM_NAME); \
	fi

# Attach to the guest's gdb stub (-gdb tcp::1122 in virtmgr.xml).
debug:
	gdb $(KERN_BUILD)/vmlinux -tui -ex 'target remote :1122'

# Boot $(KERN_BUILD) headless, save the serial console to $(LOG), and wait
# for the guest to shut itself down.
log:
	@$(MAKE) --no-print-directory vm CMDLINE='$(LOG_CMDLINE)' SERIAL_LOG="$(LOG)"
	@echo "Booting, serial console -> $(LOG) (timeout $(BOOT_TIMEOUT)s)..."
	@for i in $$(seq $(BOOT_TIMEOUT)); do \
		[ "$$(virsh --connect qemu:///session domstate $(VM_NAME) 2>/dev/null)" = "shut off" ] && break; \
		sleep 1; \
	done; \
	if [ "$$(virsh --connect qemu:///session domstate $(VM_NAME) 2>/dev/null)" != "shut off" ]; then \
		echo "Guest did not power off within $(BOOT_TIMEOUT)s, destroying it."; \
	fi
	@$(MAKE) --no-print-directory vm-clean
	@echo "Boot log saved to $(LOG)"

.PHONY: all linux savecfg mrproper config \
        build driver format clean re debug \
	    vm vm-gui vm-clean vm-xml-boot log
