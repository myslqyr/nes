CC = gcc
CFLAGS = -Iinclude -Wall -g
SRCS = src/main.c \
       src/cpu.c \
       src/opcodes.c \
       src/disassembly.c \
       src/memory.c \
       src/irq.c	\
	   src/bus.c
OBJS = $(SRCS:.c=.o)
TARGET = nes

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.log

.PHONY: all clean
run:
	./nes
	