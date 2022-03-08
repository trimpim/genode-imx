DRIVER := usdhc
PORT_DIR := $(call port_dir,$(REP_DIR)/ports/iot_gateway_linux)

MIRROR_FROM_PORT_DIR := $(addprefix linux/,$(shell cat $(REP_DIR)/src/drivers/$(DRIVER)/imx8mm/source.list)) \
                        $(addprefix linux/,$(shell cat $(REP_DIR)/src/drivers/$(DRIVER)/imx8mm/deps.list))

MIRROR_FROM_OS_DIR := src/lib/genode_c_api/block.cc

MIRROR_FROM_DDE_DIR  := src/include/lx_emul \
                        src/include/lx_kit \
                        src/include/lx_user \
                        src/include/spec/arm_64/lx_emul \
                        src/include/spec/arm_64/lx_kit \
                        src/lib/lx_emul \
                        src/lib/lx_kit

MIRROR_FROM_REP_DIR  := src/drivers/$(DRIVER)/imx8mm \
                        src/include/imx8mm \
                        src/lib/imx8mm

content: LICENSE $(MIRROR_FROM_OS_DIR) $(MIRROR_FROM_REP_DIR) $(MIRROR_FROM_PORT_DIR) $(MIRROR_FROM_DDE_DIR)

$(MIRROR_FROM_OS_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/os/$@ $@

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp $(PORT_DIR)/$@ $@

$(MIRROR_FROM_DDE_DIR):
	mkdir -p $(dir $@)
	cp -r $(GENODE_DIR)/repos/dde_linux/$@ $@

LICENSE:
	( echo "GNU General Public License version 2, see:"; \
	  echo "https://www.kernel.org/pub/linux/kernel/COPYING" ) > $@
