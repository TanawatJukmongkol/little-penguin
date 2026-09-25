
# Settings
NAME		= fortytwo
CC			= cc
CFLAGS		= -Wall -Werror -Wextra -Wno-unused-parameter -Wno-sign-compare -g -DDEBUG
BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
MAKE_FLAGS	= -j$(BUILD_JOBS) -l$(shell nproc)

# Kernel settings
KERN_VERSION	= $(shell uname -r)
KERN_BUILD		?= /lib/modules/$(KERN_VERSION)/build

# Source files
SRCS		= $(addprefix src/, \
	main.c \
	chardev_utils.c \
	chardev_fops.c \
)

HEADERS		= $(addprefix include/, \
	main.h \
)

# Kernel build variables
$(NAME)-y	+= $(SRCS:.c=.o)
ccflags-y	+= $(CFLAGS)
obj-m		+= $(NAME).o

all: $(NAME).ko

$(NAME).ko: $(SRCS) $(HEADERS)
	bear -- make -C $(KERN_BUILD) \
	M=$(shell pwd) CC=$(CC) $(MAKE_FLAGS) \
	modules EXTRA_CFLAGS="$(CFLAGS)"

clean:
	make -C $(KERN_BUILD) \
	M=$(shell pwd) \
	$(MAKE_FLAGS) clean

format: all
	$(KERN_BUILD)/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(SRCS)

install-rules:
	sudo cp 99-fortytwo.rules /etc/udev/rules.d/99-fortytwo.rules
	sudo udevadm control --reload-rules && sudo udevadm trigger

uninstall-rules:
	sudo rm -f /etc/udev/rules.d/99-fortytwo.rules
	sudo udevadm control --reload-rules && sudo udevadm trigger

reinstall-rules: uninstall-rules install-rules

re: clean all

.PHONY: all $(NAME).ko clean format re install-rules uninstall-rules reinstall-rules
