CC      := gcc
CFLAGS  := -Wall -Wextra -g -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET  := fsh
SRCS    := shell.c utils.c
OBJS    := $(SRCS:.c=.o)
DEPS    := utils.h

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)