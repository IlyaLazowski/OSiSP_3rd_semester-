CC = gcc
CFLAGS_DEBUG = -W -Wall -Wextra -std=c11
CFLAGS_RELEASE = -W -Wall -Wextra -std=c11
.SUFFIXES:
.SUFFIXES: .c .o

DEBUG = ./build/debug
RELEASE = ./build/release
OUT_DIR = $(DEBUG)

vpath %.c src
vpath %.h src
vpath %.o $(OUT_DIR)

ifeq ($(MODE), release)
    CFLAGS = $(CFLAGS_RELEASE)
    OUT_DIR = $(RELEASE)
    vpath %.o $(RELEASE)
else
    CFLAGS = $(CFLAGS_DEBUG)
endif

objects_parent = $(OUT_DIR)/parent.o
objects_child = $(OUT_DIR)/child.o
parent = $(OUT_DIR)/parent
child = $(OUT_DIR)/child


all: $(parent) $(child)

$(parent): $(objects_parent)
	$(CC) $(CFLAGS) $^ -o $@

$(child): $(objects_child)
	$(CC) $(CFLAGS) $^ -o $@

$(OUT_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	@rm -rf $(DEBUG)/* $(RELEASE)/* parent child
