PORT_DIR := $(call port_dir,$(REP_DIR)/ports/iot_gateway_linux)

MIRROR_FROM_PORT_DIR := $(addprefix linux/,$(shell cat $(REP_DIR)/src/drivers/$(DRIVER)/imx8mm/source.list)) \
                        $(addprefix linux/,$(shell cat $(REP_DIR)/src/drivers/$(DRIVER)/imx8mm/deps.list))

MIRROR_FROM_DDE_DIR  := src/include/lx_emul \
                        src/include/lx_kit \
                        src/include/lx_user \
                        src/include/spec/arm_64/lx_kit \
                        src/lib/lx_emul \
                        src/lib/lx_kit

MIRROR_FROM_REP_DIR  := src/drivers/$(DRIVER)/imx8mm \
                        src/drivers/linux_imx8mm_drv_target.inc \
                        src/include/imx8mm \
                        src/lib/imx8mm

IMX_SRC_DIR          := $(GENODE_DIR)/repos/imx/src/drivers/sd_card/imx8mq

IMX_SRC_FILES        := $(filter-out target.mk,$(filter-out main.cc,$(notdir $(wildcard $(IMX_SRC_DIR)/*.*))))

MIRROR_FROM_IMX_DIR  := $(addprefix src/drivers/sd_card/imx8mq/,$(IMX_SRC_FILES)) \
                        src/include/imx8mq \
                        src/lib/imx8mq

content: LICENSE $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_PORT_DIR) $(MIRROR_FROM_DDE_DIR) $(MIRROR_FROM_IMX_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp $(PORT_DIR)/$@ $@

$(MIRROR_FROM_DDE_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/dde_linux/$@ $@

$(MIRROR_FROM_IMX_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/imx/$@ $@

LICENSE:
	( echo "GNU General Public License version 2, see:"; \
	  echo "https://www.kernel.org/pub/linux/kernel/COPYING" ) > $@
