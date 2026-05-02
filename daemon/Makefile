CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := 

TARGET := pzxd
SRCS := $(wildcard core/*.c init/*.c modules/*.c third_party/*/*.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))

CFLAGS += -I./include -I./third_party
LDFLAGS += -lpthread

export OBJS

.PHONY: all lib clean start

all: lib $(TARGET)

lib:
	@echo "do nothing"

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

start:
	./$(TARGET)
