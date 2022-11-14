#
# (C) Copyright 2017
# Novatek Microelectronic corp.
#
# SPDX-License-Identifier:	GPL-2.0+
#
ifdef CONFIG_NVT_IVOT_SOC
obj-$(CONFIG_NVT_SPI_NOR) += spinor.o
ifdef CONFIG_CMD_NAND
obj-$(CONFIG_NVT_SPI_NAND) += spinand.o
obj-$(CONFIG_NVT_SPI_NONE) += spinand.o
endif
endif

ifdef CONFIG_TARGET_NA51068
obj-y += spi020_nand.o
endif

