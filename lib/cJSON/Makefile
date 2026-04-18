TARGET := libcjson.so
SRCS := cJSON.c
OBJS := $(patsubst %.c, %.o, $(SRCS))

CFLAGS += -fPIC -I.
LDFLAGS += -shared

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
