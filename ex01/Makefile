
# Settings
NAME		= main
CC		= cc
CFLAGS		= -Wall -Werror -Wextra
BUILD_JOBS	= $(shell expr $(shell nproc) \* 3 / 2)
MAKE_FLAGS	= -j$(BUILD_JOBS) -l$(shell nproc)

# Kernel settings
KERN_VERSION	= $(shell uname -r)
KERN_BUILD	= ../linux

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

all: ../linux $(NAME).ko

../linux:
	make -C ..

$(NAME).ko: $(SRCS) $(HEADERS)
	make -C $(KERN_BUILD) \
	M=$(shell pwd) CC=$(CC) CFLAGS=$(CFLAGS) \
	$(MAKE_FLAGS) modules

clean:
	make -C $(KERN_BUILD) \
	M=$(shell pwd) CC=$(CC) CFLAGS=$(CFLAGS) \
	$(MAKE_FLAGS) clean

format: all
	../linux/scripts/checkpatch.pl -strict -no-tree -fix-inplace -file $(SRCS)

re: clean all

.PHONY: all clean format re
