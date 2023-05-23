MIRROR_FROM_REP_DIR  := src/drivers/spi/imx8q \
                        src/drivers/spi/component.h \
                        src/drivers/spi/main.cc \
                        src/drivers/spi/spi_driver.h \
                        src/drivers/spi/target.inc

content: LICENSE $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

LICENSE:
	cp $(GENODE_DIR)/LICENSE $@
