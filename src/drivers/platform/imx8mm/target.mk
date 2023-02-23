TARGET   = imx8mm_platform_drv
REQUIRES = arm_v8
SRC_CC   = ccm_imx8mm.cc

include $(call select_from_repositories,src/drivers/platform/target.inc)
PLATFORM_IMX_DIR := $(call select_from_repositories,src/drivers/platform/imx8mq)

INC_DIR          += $(PLATFORM_IMX_DIR)

vpath %.cc $(PLATFORM_IMX_DIR)
