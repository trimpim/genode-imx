TARGET   = imx8mm_platform_drv
REQUIRES = arm_v8
SRC_CC   = ccm.cc
SRC_CC  += device.cc
SRC_CC  += device_component.cc
SRC_CC  += device_model_policy.cc
SRC_CC  += main.cc
SRC_CC  += session_component.cc
SRC_CC  += root.cc
LIBS     = base

PLATFORM_OS_DIR := $(dir $(call select_from_repositories,src/drivers/platform/device.cc))
PLATFORM_IMX_DIR := $(call select_from_repositories,src/drivers/platform/imx8mq)

INC_DIR           = $(PRG_DIR) $(PLATFORM_OS_DIR) $(PLATFORM_IMX_DIR)

vpath %.cc $(PRG_DIR)
vpath %.cc $(PLATFORM_IMX_DIR)
vpath %.cc $(PLATFORM_OS_DIR)
