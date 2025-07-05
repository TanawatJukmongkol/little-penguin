
# Settings
NAME		= keyboard-driver
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
	usb_events.c \
)

HEADERS		= $(addprefix include/, \
	main.h \
)

# Kernel build variables
$(NAME)-y	+= $(SRCS:.c=.o)
ccflags-y	+= $(CFLAGS)
obj-m		+= $(NAME).o

all: ../linux $(NAME).ko Makefile

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
	../linux/scripts/checkpatch.pl -strict -no-tree -file $(SRCS) $(HEADERS)

re: clean all

install-rules:
	sudo cp 10-keyboard.rules /etc/udev/rules.d/10-keyboard.rules
	sudo cp keyboard_driver.sh /usr/local/bin/keyboard_driver.sh
	sudo chmod +x /usr/local/bin/keyboard_driver.sh
	sudo sed -i 's|$$KMOD_DIR|'${PWD}'|g' /usr/local/bin/keyboard_driver.sh
	sudo sed -i 's|$$KNAME|'${NAME}'|g' /usr/local/bin/keyboard_driver.sh
	sudo udevadm control --reload-rules && sudo udevadm trigger

uninstall-rules:
	sudo rm -f /etc/udev/rules.d/10-keyboard.rules
	sudo rm -f /usr/local/bin/keyboard_driver.sh

reinstall-rules: uninstall-rules install-rules

.PHONY: all clean format re install-rules uninstall-rules reinstall-rules
