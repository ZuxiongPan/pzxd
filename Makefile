.PHONY: all clean lib daemon

all: lib daemon

lib:
	$(MAKE) -C daemon/lib/cJSON

daemon: lib
	$(MAKE) -C daemon

clean:
	$(MAKE) -C daemon/lib/cJSON clean
	$(MAKE) -C daemon clean
