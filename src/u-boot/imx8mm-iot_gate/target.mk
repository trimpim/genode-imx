#
# Tool to build u-boot for the CompuLab IoT gateway.
#
# Thie description from which this is derived can be found at:
#   https://github.com/compulab-yokneam/meta-bsp-imx8mm/blob/iot-gate-imx8-r3.2.1/Documentation/imx_boot_image_build.md
#
# Currently we build the version without OP-TEE.
#

TARGET              = imx8mm_iot_gate_uboot
REQUIRES            = arm_v8a

CUSTOM_TARGET_DEPS  = flash.bin

UBOOT_CONTRIB_DIR   = $(call select_from_ports,$(TARGET))/uboot-imx
OPTEE_CONTRIB_DIR   = $(call select_from_ports,$(TARGET))/imx-optee-os
ATF_CONTRIB_DIR     = $(call select_from_ports,$(TARGET))/imx-atf
FWI_CONTRIB_DIR     = $(call select_from_ports,$(TARGET))/firmware

PATCH_DIR           = $(REP_DIR)/patches/
OUT_DIR             = $(shell pwd)/$(PRG_REL_DIR)

# do not confuse third-party sub-makes
unexport BOARD
unexport MAKEFLAGS
unexport .SHELLFLAGS


export ARCH=arm64


define _port_missing_msg


Port not prepared or outdated:
  $(1)

You can prepare respectively update it as follows:
  <genode-dir>/tool/ports/prepare_port $(1)


endef


define _toolchain_missing_msg


You need the gcc-arm 10.3 toolchain.

It is expected to be installed in:
  $(1)

You can download it from:
  https://developer.arm.com/downloads/-/gnu-a/10-3-2021-07
    - search for -x86_64-aarch64-none-linux-gnu
    - download gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz
    - download gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz.asc
    - verivy the download by comparing
       md5sum gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz
       to
       cat gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz.asc

Install
 - sudo tar -C /opt -xf ~/Downloads/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu.tar.xz


endef


#
# check if the port is prepared, exit if isn't
# copied form <genode-dir>/tool/depot/mk/content_env.mk
#
_assert           = $(if $1,$1,$(error Error: $2))
_checked_port_dir = $(call _assert,$(call _port_dir,$1),$(call _port_missing_msg,$(notdir $1)))
port_dir          = $(call _checked_port_dir,$(TARGET))


#
# This uses the arm-gcc toolchain
#
TOOL_CHAIN = gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu

TOOL_CHAIN_DIR = /opt/$(TOOL_CHAIN)
CROSS_PREFIX  = $(TOOL_CHAIN_DIR)/bin/aarch64-none-linux-gnu-
ifeq ($(shell command -v $(CROSS_PREFIX)gcc), )
$(error $(call _toolchain_missing_msg,$(TOOL_CHAIN_DIR)))
endif
CROSS_COMPILE=$(CROSS_PREFIX)

#
# Firmware files for memory training
#
FW_FILES = $(shell find $(FWI_CONTRIB_DIR)/* | awk '/train|hdmi_imx8|dp_imx8/' ORS=" ")


prepare:
	$(VERBOSE)rm -Rf $(OUT_DIR)
	$(VERBOSE)mkdir -p $(OUT_DIR)


copy_fw_files: prepare
	$(VERBOSE)cp -v $(FW_FILES) $(OUT_DIR)


make_atf: prepare
	$(VERBOSE)make -j8 -C $(ATF_CONTRIB_DIR) \
	               BUILD_BASE=build \
	               PLAT=imx8mm \
	               IMX_BOOT_UART_BASE=0x30880000 \
	               BL32_BASE=0x56000000 \
	               BL32_SIZE=0x2000000 \
	               CROSS_COMPILE=$(CROSS_PREFIX) bl31
	$(VERBOSE)ln -s $(ATF_CONTRIB_DIR)/build/imx8mm/release/bl31.bin $(OUT_DIR)


make_uboot: copy_fw_files make_atf
	@# first create the default config
	$(VERBOSE)make -j8 -C $(UBOOT_CONTRIB_DIR) \
	               O=$(OUT_DIR) MACHINE=iot-gate-imx8 \
	               ARCH=arm \
	               CROSS_COMPILE=$(CROSS_PREFIX) -f defconfig.mk
	@# copy the configuration for the build and the default environment to the build directory
	@# TODO: for release there need to be non development version s of these two files
	@#       and a way to select the correct version of them.
	$(VERBOSE)cp $(REP_DIR)/src/u-boot/imx8mm-iot_gate/config-develop        $(OUT_DIR)/.config
	$(VERBOSE)cp $(REP_DIR)/src/u-boot/imx8mm-iot_gate/iot_gate-develop.env  $(OUT_DIR)/iot_gate.env
	@# build the actual image to flash
	$(VERBOSE)make -j8 -C $(UBOOT_CONTRIB_DIR) \
	               O=$(OUT_DIR) \
	               ATF_LOAD_ADDR=0x00920000 \
	               TEE_LOAD_ADDR=0x56000000 \
	               CROSS_COMPILE=$(CROSS_PREFIX) flash.bin


$(CUSTOM_TARGET_DEPS): make_uboot
