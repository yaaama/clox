#
# nicer
#
# @file
# @version 0.1

NAME := nicer
# Commands:
RM := rm -f
# Flags for "make" itself
MAKEFLAGS += --no-print-directory
DIR_DUP     = mkdir -p $(@D)


##################
# # Sources, etc #
##################

# Source files
SRCS = $(wildcard src/*.c)
# Object files
OBJS = $(addprefix build/,$(notdir $(SRCS:.c=.o)))

# Compiler options
CFLAGS := -Wall -g3 -O0 -std=c11 -Wextra -Wunused -pedantic
CPPFLAGS := -MMD -MP -I include
# Compiler
CC = gcc


#############
# # Targets #
#############


# default target
all: build/nicer

build/nicer: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	$(info CREATED $(NAME))

build/%.o: src/%.c
	$(DIR_DUP)
	$(CC) $(CFLAGS) -c -o $@ $<
	$(info CREATED $@)



# Cleans build directory
clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

run:
	$(MAKE) fclean
	bear -- make all
	./build/nicer

.PHONY: clean fclean run build/nicer build/%.o
.SILENT:


# end
