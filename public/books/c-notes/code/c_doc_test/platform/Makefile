CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
TARGET := linked_make
OBJS := link_main.o link_value.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
