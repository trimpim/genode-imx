TARGET   := imx_usdhc_drv
REQUIRES = arm_v8a
LIBS     = base
INC_DIR  = $(PRG_DIR)

LX_CONTRIB_DIR  := $(call select_from_ports,iot_gateway_linux)/linux

SRC_CC  += main.cc
SRC_CC  += time.cc
SRC_CC  += lx_emul/shared_dma_buffer.cc
SRC_C   += block.c
SRC_C   += dummies.c
SRC_C   += lx_emul.c
SRC_C   += $(notdir $(wildcard $(PRG_DIR)/generated_dummies.c))

# Lx_emul specific to driver
SRC_C += lx_emul/shadow/kernel/softirq.c

#
# Genode C-API backends
#

SRC_CC  += genode_c_api/block.cc

vpath genode_c_api/block.cc $(subst /genode_c_api,,$(call select_from_repositories,src/lib/genode_c_api))

#
# Generate driver-specific device-tree binary data
#
# The rules below use the tool/dts/extract tool to generate a device tree
# containing the driver parameters for a given board.
#
# The resulting dtb file is named <driver>-<board>.dtb
#

BOARDS := imx8mm_iot_gate-usdhc2 \
          imx8mm_iot_gate-usdhc3

DTS_PATH(imx8mm_iot_gate-usdhc2)    := arch/arm64/boot/dts/compulab/sb-iotgimx8.dts
DTS_EXTRACT(imx8mm_iot_gate-usdhc2) := --select usdhc2

DTS_PATH(imx8mm_iot_gate-usdhc3)    := arch/arm64/boot/dts/compulab/sb-iotgimx8.dts
DTS_EXTRACT(imx8mm_iot_gate-usdhc3) := --select usdhc3

DTS_TOOL           := $(BASE_DIR)/../../tool/dts/extract
CUSTOM_TARGET_DEPS += $(addprefix $(INSTALL_DIR)/$(TARGET)-,$(addsuffix .dtb,$(BOARDS)))

include $(REP_DIR)/src/drivers/linux_imx8mm_drv_target.inc
