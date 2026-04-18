TARGET := pzxd
SRCS := $(wildcard *.c modules/*.c core/*.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))

CFLAGS += -I$(COMM_LIB)/cJSON
LDFLAGS += -L$(COMM_LIB)/cJSON -lcjson -lpthread -Wl,-rpath,'$$ORIGIN/../lib/cJSON'

export OBJS

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
