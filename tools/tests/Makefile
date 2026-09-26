
# Settings
ROOT			= ../..
KERN_BUILD		?= linux
FULL_SRC_PATH	= $(if $(filter /%,$(KERN_BUILD)),$(KERN_BUILD),$(abspath $(ROOT)/$(KERN_BUILD)))

# Exercises to test; narrow it down with e.g. `make test EX="ex05 ex07"`.
# Override with kasan to do a sanity kasan self check
EX			?= ex00 ex01 ex02 ex03 ex04 ex05 ex06 ex07 ex08 ex09

# Each exNN/ folder holds up to three scripts:
#   host.sh   checks run on the host (logs, config, patch, checkpatch, build)
#   guest.sh  checks run in the VM (load the module and exercise it)
#   proof.sh  writes the exercise's exNN/proof.log transcript, in the VM
HOST_TESTS	= $(strip $(foreach e,$(EX),$(if $(wildcard $(e)/host.sh),$(e))))
GUEST_TESTS	= $(strip $(foreach e,$(EX),$(if $(wildcard $(e)/guest.sh),$(e))))
PROOFS		= $(strip $(foreach e,$(EX),$(if $(wildcard $(e)/proof.sh),$(e))))
LOGS		= $(HOST_TESTS:%=%/host.log) $(GUEST_TESTS:%=%/guest.log)

# Result lines from lib.sh's ok/ko, with or without their colour codes
PASS_RE		= ^(\e\[[0-9;]*m)?OK\b
FAIL_RE		= ^(\e\[[0-9;]*m)?KO\b

# KASAN is switched on in $(KERN_BUILD)'s own .config, so it works for linux
# and linux-next alike. `make mrproper` in ex00/ex06 restores the saved config.
KASAN_CFG	= KASAN KASAN_GENERIC KASAN_INLINE KASAN_VMALLOC
KCONFIG_FLAGS	= CC=cc LLVM=1 LD=ld ARCH=x86_64 HOSTCC=clang HOSTCXX=clang++

# The guest boots this image; it's built through the root Makefile if missing
KIMG		= $(FULL_SRC_PATH)/arch/x86/boot/bzImage

# Modules built against the tested kernel tree before the checks
MODULES		= $(filter ex01 ex03 ex04 ex05 ex07 ex08,$(EX))

all: test

$(KIMG):
	$(MAKE) --no-print-directory -C $(ROOT) build KERN_BUILD=$(FULL_SRC_PATH)

modules:
	for folder in $(MODULES); do \
		$(MAKE) --no-print-directory -C $(ROOT)/$$folder KERN_BUILD=$(FULL_SRC_PATH); \
	done
	$(if $(filter kasan,$(EX)),$(MAKE) --no-print-directory -C kasan KERN_BUILD=$(FULL_SRC_PATH))

host:
	for ex in $(HOST_TESTS); do \
		R=$(abspath $(ROOT)) K=$(FULL_SRC_PATH) EX=$$ex \
		sh -c '. ./lib.sh; . ./$$EX/host.sh' > $$ex/host.log 2>&1; \
	done

guest:
	$(if $(GUEST_TESTS),KERN_BUILD=$(FULL_SRC_PATH) ./boot.sh $(GUEST_TESTS))

# ex04's guest and proof scripts need the USB keyboard dongle on the host.
test: $(if $(GUEST_TESTS),$(KIMG)) modules
	$(if $(filter-out $(wildcard $(EX)),$(EX)),$(error no such test: $(filter-out $(wildcard $(EX)),$(EX))))
	rm -f $(LOGS)
	-$(MAKE) --no-print-directory host guest
	@fail=0; missing=0; \
	for log in $(LOGS); do \
		ex=$${log%%/*}; \
		if [ ! -s $$log ]; then printf '%s: \033[31mKO\033[0m - %s is missing\n' $$ex $$log; fail=1; missing=$$((missing + 1)); continue; fi; \
		sed "s|^|$$ex: |" $$log; \
		grep -qP '$(FAIL_RE)' $$log && fail=1; \
		grep -qP '$(PASS_RE)' $$log || fail=1; \
	done; \
	echo -e "\e[33m" \
	     "$$(cat $(LOGS) 2>/dev/null | grep -cP '$(PASS_RE)') passed," \
	     "$$(($$(cat $(LOGS) 2>/dev/null | grep -cP '$(FAIL_RE)') + missing)) failed" \
		 "\e[0m"; \
	exit $$fail

kasan:
	$(FULL_SRC_PATH)/scripts/config --file $(FULL_SRC_PATH)/.config $(addprefix --enable ,$(KASAN_CFG))
	$(MAKE) --no-print-directory -C $(FULL_SRC_PATH) $(KCONFIG_FLAGS) olddefconfig
	$(MAKE) --no-print-directory -C $(ROOT) build KERN_BUILD=$(FULL_SRC_PATH)

# Every guest test also fails on a new KASAN report (see run.sh).
test-kasan: kasan
	$(MAKE) --no-print-directory test

proof: $(if $(PROOFS),$(KIMG)) modules
	$(if $(PROOFS),KERN_BUILD=$(FULL_SRC_PATH) ./boot.sh $(PROOFS:%=proof:%))

clean:
	rm -f ex*/*.log *.log $(ROOT)/tools/vm/proof-domain.xml

fclean: clean
	for folder in $(MODULES); do \
		$(MAKE) --no-print-directory -C $(ROOT)/$$folder KERN_BUILD=$(FULL_SRC_PATH) fclean; \
	done
	$(if $(filter kasan,$(EX)),$(MAKE) --no-print-directory -C kasan KERN_BUILD=$(FULL_SRC_PATH) fclean)

re: fclean all

.PHONY: all modules host guest test kasan test-kasan proof clean fclean re
