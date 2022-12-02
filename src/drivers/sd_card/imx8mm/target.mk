TARGET  := imx8mm_sd_card_drv
REQUIRES = arm_v8a

LIBS     = base

SRC_CC  += main.cc
SRC_CC  += time.cc
SRC_CC  += lx_emul/shared_dma_buffer.cc
SRC_C   += block.c
SRC_C   += dummies.c
SRC_C   += generated_dummies.c
SRC_C   += lx_emul.c

INC_DIR  = $(PRG_DIR)
INC_DIR  += $(REP_DIR)/src/drivers/sd_card/imx8mq

vpath block.c   $(REP_DIR)/src/drivers/sd_card/imx8mq
vpath dummies.c $(REP_DIR)/src/drivers/sd_card/imx8mq
vpath main.cc   $(REP_DIR)/src/drivers/sd_card/imx8mq
vpath time.cc   $(REP_DIR)/src/drivers/sd_card/imx8mq

#
# Genode C-API backends
#

SRC_CC  += genode_c_api/block.cc

vpath genode_c_api/block.cc $(subst /genode_c_api,,$(call select_from_repositories,src/lib/genode_c_api))

# Driver-specific device-tree binary data
BOARDS                       := imx8mm_iot_gate
DTS_PATH(imx8mm_iot_gate)    := arch/arm64/boot/dts/compulab/sb-iotgimx8.dts
DTS_EXTRACT(imx8mm_iot_gate) := --select usdhc3 usdhc2

include $(REP_DIR)/src/drivers/linux_mnt_reform2_drv_target_arm_v8.inc
