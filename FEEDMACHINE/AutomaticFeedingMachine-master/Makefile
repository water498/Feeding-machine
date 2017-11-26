#################################################################
#          Makefile for the automatic feeding machine           #
#################################################################

CC=clang
CFLAGS=-Wall -Wextra -Wconversion -Wpedantic -Werror -g
LDFLAGS=-lpthread
RM=rm -f
SOURCES=scheduler.c
BIN=scheduler output_file.txt
TARGETS=$(SOURCES:.c=)

.PHONY: all clean

all: $(TARGETS)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN)
