DRIVER := sd_card/imx8mm

MIRROR_FROM_OS_DIR := src/lib/genode_c_api/block.cc

MIRROR_FROM_REP_DIR  := src/drivers/sd_card/imx8mq/lx_emul.h \
                        src/drivers/sd_card/imx8mq/main.cc \
                        src/drivers/sd_card/imx8mq/time.cc \
                        src/drivers/sd_card/imx8mq/block.c \
                        src/drivers/sd_card/imx8mq/dummies.c

content: $(MIRROR_FROM_OS_DIR) $(MIRROR_FROM_REP_DIR) 

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_OS_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $@

include $(REP_DIR)/recipes/src/linux_mnt_reform2_drv_content.inc
