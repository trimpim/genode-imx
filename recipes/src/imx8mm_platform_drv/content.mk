SRC_DIR = src/drivers/platform
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

MIRROR_FROM_OS_DIR  := src/drivers/platform
MIRROR_FROM_IMX_DIR := src/drivers/platform/imx8mq
MIRROR_FROM_IMX8MM_DIR := src/drivers/platform/imx8mm

content: $(MIRROR_FROM_OS_DIR) $(MIRROR_FROM_IMX_DIR) $(MIRROR_FROM_IMX8MM_DIR)

$(MIRROR_FROM_OS_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $@

$(MIRROR_FROM_IMX_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/imx/$@ $@

$(MIRROR_FROM_IMX8MM_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/imx8mm/$@ $@
