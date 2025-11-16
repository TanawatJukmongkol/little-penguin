
# Settings
NAME		= driver
CC			= cc
CFLAGS		= -Wall -Werror -Wextra -g -DDEBUG
BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
MAKE_FLAGS	= -j$(BUILD_JOBS) -l$(shell nproc)

# Kernel settings
KERN_VERSION	= $(shell uname -r)
KERN_BUILD		= ../linux

# Source files
SRCS		= $(addprefix src/, \
	main.c \
)

HEADERS		= $(addprefix include/, \
)

# Kernel build variables
$(NAME)-y	+= $(SRCS:.c=.o)
ccflags-y	+= $(CFLAGS)
obj-m		+= $(NAME).o

all: ../linux $(NAME).ko

../linux:
	make -C ..

$(NAME).ko: $(SRCS) $(HEADERS)
	make -C $(KERN_BUILD) \
	M=$(shell pwd) CC=$(CC) $(MAKE_FLAGS) \
	modules EXTRA_CFLAGS="$(CFLAGS)"

clean:
	make -C $(KERN_BUILD) \
	M=$(shell pwd) \
	$(MAKE_FLAGS) clean

format: all
	../linux/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(SRCS)

install-rules:
	sudo cp 99-fourtytwo.rules /etc/udev/rules.d/99-fourtytwo.rules
	sudo udevadm control --reload-rules && sudo udevadm trigger

uninstall-rules:
	sudo rm -f /etc/udev/rules.d/99-fourtytwo.rules
	sudo udevadm control --reload-rules && sudo udevadm trigger

reinstall-rules: uninstall-rules install-rules

re: clean all install-rules uninstall-rules

.PHONY: all clean format re
