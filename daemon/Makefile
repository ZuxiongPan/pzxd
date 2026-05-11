CC = gcc

CFLAGS = -O2 -Wall -std=gnu11 \
         -I./include -I../lib/libuv/include

LDFLAGS = -L../lib/libuv -luv -lpthread -ldl \
          -Wl,-rpath=\$$ORIGIN/../lib/libuv

SRC = \
    init/main.c \
    init/module_regcalls.c \
    core/module.c \
    core/module_registry.c \
    modules/system/monitor.c

OBJ = $(SRC:.c=.o)

TARGET = armd

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
