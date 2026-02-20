all: $(NEXT) build

$(NEXT):
	git clone --depth 1 --branch v6.14 git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
	cp ex00/config linux/.config
