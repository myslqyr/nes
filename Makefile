CC = gcc
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)
CFLAGS = -Iinclude -Wall -O3 -g -pg $(SDL2_CFLAGS)
SRCS = src/main.c \
	src/cpu.c \
	src/disassembly.c \
	src/irq.c \
	src/bus.c \
	src/ppu.c \
	src/sdl.c \
	src/cartridge.c \
	src/mapper.c \
	src/mapper0.c
OBJS = $(SRCS:.c=.o)
TARGET = nes

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(SDL2_LIBS) -pg

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.log

.PHONY: all clean
run:
	./nes
	