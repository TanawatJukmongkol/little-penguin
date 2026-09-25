
# Settings
NAME		= debugfs
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
	debugfs.c \
	$(addprefix debug/, \
		debug_foo.c \
		debug_id.c \
		debug_jiffies.c \
	) \
)

HEADERS		= $(addprefix include/, \
	debugfs.h \
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

fclean: clean
	rm -rf compile_commands.json .cache

format: all
	$(KERN_BUILD)/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(SRCS)

re: fclean all

.PHONY: all $(NAME).ko clean fclean format re
