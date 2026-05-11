CC = gcc

TARGET_STATIC = libuv.a
TARGET_SHARED = libuv.so

CFLAGS = -O2 -fPIC -Wall -D_GNU_SOURCE -std=gnu11 \
         -I./include -I./src

LDFLAGS = -lpthread -ldl

COMMON_SRCS = \
src/uv-common.c \
src/uv-data-getter-setters.c \
src/version.c \
src/idna.c \
src/inet.c \
src/strtok.c \
src/strscpy.c \
src/thread-common.c \
src/threadpool.c \
src/timer.c \
src/random.c \
src/fs-poll.c

UNIX_SRCS = \
src/unix/async.c \
src/unix/core.c \
src/unix/dl.c \
src/unix/fs.c \
src/unix/getaddrinfo.c \
src/unix/getnameinfo.c \
src/unix/linux.c \
src/unix/loop.c \
src/unix/loop-watcher.c \
src/unix/pipe.c \
src/unix/poll.c \
src/unix/process.c \
src/unix/procfs-exepath.c \
src/unix/random-getrandom.c \
src/unix/random-devurandom.c \
src/unix/random-sysctl-linux.c \
src/unix/signal.c \
src/unix/stream.c \
src/unix/tcp.c \
src/unix/thread.c \
src/unix/tty.c \
src/unix/udp.c \
src/unix/proctitle.c

SRCS = $(COMMON_SRCS) $(UNIX_SRCS)

OBJ = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET_STATIC) $(TARGET_SHARED)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_STATIC): $(OBJ)
	ar rcs $@ $^

$(TARGET_SHARED): $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET_STATIC) $(TARGET_SHARED)
