TARGET    = imx8mm_reset_controller_drv

REQUIRES  = arm_v8

LIBS += base

INC_DIR += $(PRG_DIR)
INC_DIR += $(REP_DIR)/src/drivers/reset_controller

SRC_CC += main.cc
SRC_CC    += reset_controller_driver.cc

vpath %.cc $(REP_DIR)/src/drivers/reset_controller
vpath %.cc $(REP_DIR)
