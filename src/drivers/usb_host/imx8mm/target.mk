TARGET  := imx8mm_usb_host
REQUIRES = arm_v8a
LIBS     = base
INC_DIR  = $(PRG_DIR)
SRC_CC  += main.cc
SRC_CC  += time.cc
SRC_CC  += lx_emul/shared_dma_buffer.cc
SRC_C   += dummies.c
SRC_C   += lx_emul.c
SRC_C   += lx_emul/usb.c
SRC_C   += $(notdir $(wildcard $(PRG_DIR)/generated_dummies.c))

CC_OPT_drivers/usb/chipidea/trace  += -I$(LX_CONTRIB_DIR)/drivers/usb/chipidea -DCONFIG_USB_CHIPIDEA_HOST
CC_OPT_drivers/usb/chipidea/host   += -DCONFIG_USB_CHIPIDEA_HOST
CC_OPT_drivers/usb/host/xhci-trace += -I$(LX_CONTRIB_DIR)/drivers/usb/host
CC_OPT_generated_dummies           += -I$(LX_CONTRIB_DIR)/drivers/usb/chipidea
CC_OPT_dummies                     += -I$(LX_CONTRIB_DIR)/drivers/usb/chipidea

#CC_OPT += -UCONFIG_PERF_EVENTS

#
# Genode C-API backends
#

SRC_CC  += genode_c_api/usb.cc

vpath genode_c_api/usb.cc $(subst /genode_c_api,,$(call select_from_repositories,src/lib/genode_c_api))

# Driver-specific device-tree binary data
BOARDS                       := imx8mm_iot_gate
DTS_PATH(imx8mm_iot_gate)    := arch/arm64/boot/dts/compulab/sb-iotgimx8.dts
DTS_EXTRACT(imx8mm_iot_gate) := --select usbotg1 --select usbotg2

include $(REP_DIR)/src/drivers/linux_mnt_reform2_drv_target_arm_v8.inc
