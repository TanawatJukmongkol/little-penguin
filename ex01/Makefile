
# Settings
NAME		= hello
CC		= cc
MAKE_FLAGS	= "-j8"

# Kernel settings
KERN_VERSION	= $(shell uname -r)
KERN_BUILD	= /lib/modules/$(KERN_VERSION)/build

# Source files
SRCS		= $(addprefix src/, \
	main.c \
)

HEADERS		= $(addprefix include/, \
	main.h \
)

# Kernel build variables
$(NAME)-y	= $(SRCS:.c=.o)
obj-m		= $(NAME).o

all: $(NAME).ko

$(NAME).ko: $(SRCS) $(HEADERS)
	make -C $(KERN_BUILD) \
	M=$(shell pwd) CC=$(CC) \
	$(MAKE_FLAGS) modules

clean:
	make -C $(KERN_BUILD) \
	M=$(shell pwd) CC=$(CC) \
	$(MAKE_FLAGS) clean

format:
	../linux/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(SRCS)

re: clean all

.PHONY: all clean format re
