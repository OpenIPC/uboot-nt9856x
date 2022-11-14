# Copyright (C) 2015-2020 Novatek corp.
#
# SPDX-License-Identifier:	GPL-2.0+
#
KBUILD_CFLAGS := -fno-builtin -Werror -Iboard/$(BOARDDIR)/../common/include -fshort-wchar -Iinclude

obj-$(CONFIG_NVT_IVOT_SOC) += nvt_ivot_common.o
obj-$(CONFIG_NVT_IVOT_OPTEE_SUPPORT) += nvt_ivot_optee_utils.o nvt_optee/nvt_optee_smc.o


obj-$(CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT) += nvt_ivot_fw_update_utils.o nvt_ivot_fw_update.o nvt_ivot_pack.o
obj-$(CONFIG_NVT_IVOT_SOC_UTILS) += nvt_ivot_soc_utils.o
obj-$(CONFIG_NVT_TK_EMMC_UTILS) += nvt_tk_gpt.o nvt_emmc_fwupdate.o nvt_emmc_partition.o nvt_emmc_xbootdat.o nvt_emmc_utils.o nvt_common_utils.o
obj-$(CONFIG_NVT_TK_NAND_UTILS) += nt72_npt.o   nvt_nand_fwupdate.o nvt_nand_partition.o nvt_nand_xbootdat.o nvt_nand_utils.o nvt_common_utils.o
obj-$(CONFIG_NVT_MMC_INFO) += nvt_emmc_info.o
obj-$(CONFIG_NVT_TK_SPI_UTILS) += nvt_spi_fwupdate.o nvt_spi_partition.o
obj-$(CONFIG_NVTTK_EBURNER_V2) += nvt_eburner.o
obj-$(CONFIG_NVT_FM_TOOL) += nvt_tkfm.o
obj-$(CONFIG_NVT_BOOTM_LOAD_SECOS) += nvt_tzone.o nvt_sys.o
obj-$(CONFIG_NVT_GWRITE) += nvt_gwrite.o
obj-$(CONFIG_NVT_SECURE_BOOT) += nvt_kx_hash.o nvt_vtbl_check.o
obj-$(CONFIG_NVT_IVOT_TZPC_SUPPORT) += tzpc/nvt_ivot_tzpc_utils.o tzpc/nvt_ivot_protected.o
obj-$(CONFIG_NVT_IVOT_CRYPTO_CMD) += nvt_ivot_crypto_cmd.o
obj-$(CONFIG_NVT_IVOT_HASH_CMD) += nvt_ivot_hash_cmd.o
obj-$(CONFIG_NVT_IVOT_RSA_CMD) += nvt_ivot_rsa_cmd.o
obj-$(CONFIG_SET_DFU_ALT_INFO) += nvt_dfu.o
obj-$(CONFIG_VIDEO_LOGO) += nvt_logo/jpg.o_shipped nvt_logo/cmd_bootlogo.o
obj-$(CONFIG_NVT_PCIE_CASCADE) += nvt_pcie_bootep.o
