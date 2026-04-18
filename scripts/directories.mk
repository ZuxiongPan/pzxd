include pzxd_config

LIB_DIR-y :=
SERVICE_DIR-y :=

LIB_DIR-y += $(COMM_LIB)/cJSON

SERVICE_DIR-y += $(TOP_DIR)/daemon
SERVICE_DIR-y += $(TOP_DIR)/sendcmd
