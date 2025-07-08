CC = gcc
CFLAGS = -Iinclude -Wall -O2
SRCS = src/main.c src/cpu.c
OBJS = $(SRCS:.c=.o)
TARGET = nes

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	