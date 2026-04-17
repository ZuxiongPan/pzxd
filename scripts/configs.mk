PWD := $(shell pwd)
TOP_DIR := $(PWD)/..
COMM_INCLUDE := $(TOP_DIR)/include
COMM_LIB := $(TOP_DIR)/lib

CC := gcc
CFLAGS := -Wall -Wextra -O2 -I$(COMM_INCLUDE)
LDFLAGS := 

export CC CFLAGS LDFLAGS SERVICE_DIR
export COMM_INCLUDE COMM_LIB