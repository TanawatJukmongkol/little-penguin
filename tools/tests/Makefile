
# Settings
ROOT			= ../..
KERN_BUILD		?= linux
FULL_SRC_PATH	= $(if $(filter /%,$(KERN_BUILD)),$(KERN_BUILD),$(abspath $(ROOT)/$(KERN_BUILD)))

# Exercises to test; narrow it down with e.g. `make test EX="ex05 ex07"`.
EX			?= ex00 ex01 ex02 ex03 ex04 ex05 ex06 ex07 ex08 ex09

# Each exNN/ folder holds up to three scripts:
#   host.sh   checks run on the host (logs, config, patch, checkpatch, build)
#   guest.sh  checks run in the VM (load the module and exercise it)
#   proof.sh  writes the exercise's exNN/proof.log transcript, in the VM
HOST_TESTS	= $(strip $(foreach e,$(EX),$(if $(wildcard $(e)/host.sh),$(e))))
GUEST_TESTS	= $(strip $(foreach e,$(EX),$(if $(wildcard $(e)/guest.sh),$(e))))
PROOFS		= $(strip $(foreach e,$(EX),$(if $(wildcard $(e)/proof.sh),$(e))))
LOGS		= $(HOST_TESTS:%=%/host.log) $(GUEST_TESTS:%=%/guest.log)

# Modules built against the tested kernel tree before the checks
MODULES		= $(filter ex01 ex03 ex04 ex05 ex07 ex08,$(EX))

all: test

modules:
	for folder in $(MODULES); do \
		$(MAKE) --no-print-directory -C $(ROOT)/$$folder KERN_BUILD=$(FULL_SRC_PATH); \
	done

host:
	for ex in $(HOST_TESTS); do \
		R=$(abspath $(ROOT)) K=$(FULL_SRC_PATH) EX=$$ex \
		sh -c '. ./lib.sh; . ./$$EX/host.sh' > $$ex/host.log 2>&1; \
	done

guest:
	$(if $(GUEST_TESTS),KERN_BUILD=$(FULL_SRC_PATH) ./boot.sh $(GUEST_TESTS))

# ex04's guest and proof scripts need the USB keyboard dongle on the host.
test: modules
	rm -f $(LOGS)
	-$(MAKE) --no-print-directory host guest
	@fail=0; \
	for log in $(LOGS); do \
		ex=$${log%%/*}; \
		if [ ! -s $$log ]; then echo "$$ex: not ok - $$log is missing"; fail=1; continue; fi; \
		sed "s|^|$$ex: |" $$log; \
		grep -q '^not ok' $$log && fail=1; \
		grep -q '^ok' $$log || fail=1; \
	done; \
	echo "$$(cat $(LOGS) 2>/dev/null | grep -c '^ok') passed," \
	     "$$(cat $(LOGS) 2>/dev/null | grep -c '^not ok') failed"; \
	exit $$fail

proof: modules
	$(if $(PROOFS),KERN_BUILD=$(FULL_SRC_PATH) ./boot.sh $(PROOFS:%=proof:%))

clean:
	rm -f ex*/*.log *.log $(ROOT)/tools/vm/proof-domain.xml

fclean: clean
	for folder in $(MODULES); do \
		$(MAKE) --no-print-directory -C $(ROOT)/$$folder KERN_BUILD=$(FULL_SRC_PATH) fclean; \
	done

re: fclean all

.PHONY: all modules host guest test proof clean fclean re
