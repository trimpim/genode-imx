DDE_LINUX_DIR = $(GENODE_DIR)/repos/dde_linux

###
### Duplicate and adapted from dde_linux/recipes/src/fec_nic_drv
###

LIB_MK := lib/mk/fec_nic_include.mk \
          $(foreach SPEC,arm arm_64,lib/mk/spec/$(SPEC)/lx_kit_setjmp.mk)

PORT_DIR := $(call port_dir,$(DDE_LINUX_DIR)/ports/dde_linux)


MIRROR_FROM_DDE_LINUX_DIR := $(LIB_MK) \
                             src/drivers/nic/linux_network_session_base.cc \
                             src/drivers/nic/linux_network_session_base.h \
                             lib/import/import-fec_nic_include.mk \
                             src/include/legacy src/lib/legacy/lx_kit \
                             src/lib/lx_kit/spec \
                             $(foreach SPEC,arm arm_64 arm_v7,src/include/spec/$(SPEC)) \
                             $(shell cd $(DDE_LINUX_DIR); find src/drivers/nic/fec -type f)

MIRROR_FROM_PORT_DIR := $(shell cd $(PORT_DIR); find src/drivers/nic/fec -type f | grep -v ".git")
MIRROR_FROM_PORT_DIR := $(filter-out $(MIRROR_FROM_DDE_LINUX_DIR),$(MIRROR_FROM_PORT_DIR))

content: $(MIRROR_FROM_DDE_LINUX_DIR) $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_DDE_LINUX_DIR):
	mkdir -p $(dir $@)
	cp -r $(DDE_LINUX_DIR)/$@ $@

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp $(PORT_DIR)/$@ $@

content: LICENSE
LICENSE:
	( echo "GNU General Public License version 2, see:"; \
	  echo "https://www.kernel.org/pub/linux/kernel/COPYING" ) > $@

###
### Applying malicious patch
###

FEC_MAIN_PATH        = src/drivers/nic/fec/drivers/net/ethernet/freescale/fec_main.c
MIRROR_PATCH_REP_DIR = src/test/fec_malicious/fec_malicious_pagefault.patch

content: $(MIRROR_PATCH_REP_DIR)

$(MIRROR_PATCH_REP_DIR):
	patch $(FEC_MAIN_PATH) < $(REP_DIR)/$(MIRROR_PATCH_REP_DIR)
