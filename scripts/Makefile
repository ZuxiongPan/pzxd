include configs.mk
include directories.mk

.PHONY: all clean

all:
	for dir in $(SERVICE_DIR-y); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SERVICE_DIR-y); do \
		$(MAKE) -C $$dir clean; \
	done