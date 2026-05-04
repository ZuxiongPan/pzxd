CC := gcc
CFLAGS := -Wall -Wextra -O2 -DDAEMON_DEBUG
LDFLAGS := 

TARGET := pzxd
SRCS := $(wildcard core/*.c init/*.c modules/*.c third_party/*/*.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))

CFLAGS += -I./include -I./third_party
LDFLAGS += -lpthread

export OBJS

.PHONY: all lib clean start

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

start:
	./$(TARGET)
