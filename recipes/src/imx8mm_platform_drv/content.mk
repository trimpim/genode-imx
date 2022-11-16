SRC_DIR = src/drivers/platform/imx8mm
include $(GENODE_DIR)/repos/os/recipes/src/platform_drv/content.inc


MIRROR_FROM_REP_DIR := src/drivers/platform/imx8mq/main.cc \
                       src/drivers/platform/imx8mq/gpc.h \
                       src/drivers/platform/imx8mq/ccm.h \
                       src/drivers/platform/imx8mq/src.h

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/imx/$@ $@

