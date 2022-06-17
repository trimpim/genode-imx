DRIVER := nic/fec

include $(REP_DIR)/recipes/src/linux_mnt_reform2_drv_content.inc

###
### Applying malicious patch
###

FEC_MAIN_PATH        = linux/drivers/net/ethernet/freescale/fec_main.c
MIRROR_PATCH_REP_DIR = src/test/fec_malicious/fec_malicious_pagefault.patch

content: $(MIRROR_PATCH_REP_DIR)

$(MIRROR_PATCH_REP_DIR):
	patch $(FEC_MAIN_PATH) < $(REP_DIR)/$(MIRROR_PATCH_REP_DIR)
