SHARED_LIB := yes

LIBS       += vfs_i2c

INC_DIR    += $(REP_DIR)/src/lib/vfs/i2c

SRC_CC     += imx/driver_imx.cc

vpath % $(REP_DIR)/src/lib/vfs/i2c
