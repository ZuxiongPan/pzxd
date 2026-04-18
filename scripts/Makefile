include configs.mk
include directories.mk

.PHONY: all clean start libs services

all: libs services

libs:
	for dir in $(LIB_DIR-y); do \
		$(MAKE) -C $$dir; \
	done

services:
	for dir in $(SERVICE_DIR-y); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SERVICE_DIR-y); do \
		$(MAKE) -C $$dir clean; \
	done
	for dir in $(LIB_DIR-y); do \
		$(MAKE) -C $$dir clean; \
	done

start:
	$(TOP_DIR)/daemon/pzxd &
