NAME	= 0001-Update-EXTRAVERSION-in-Makefile.patch

all:	patch build
revert:	patch-r build

../linux:
	make -C .. linux

patch: ../linux
	patch ../linux/Makefile $(NAME)

patch-r: ../linux
	patch -R ../linux/Makefile $(NAME)

build:
	make -C .. build

format: ../linux
	../linux/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(NAME)

.PHONY: patch patch-r build format
