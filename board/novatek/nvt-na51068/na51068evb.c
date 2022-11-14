/**
    NVT evb board file
    To handle na51068 basic init.
    @file       na51068evb.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/mach-types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/na51068evb.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;
extern int nvt_mmc_init(int id);
#ifdef CONFIG_NOVATEK_MAC_ENET
extern int na51068_eth_initialize(bd_t *bis);
#endif
const char *boardinfo[] = {
	"Board:"_CHIP_NAME_"EVB\n"
};

void reset_cpu(unsigned long ignored)
{
	nvt_ivot_reset_cpu();
}

/**
 * @brief board_early_init_f
 *
 * @return 0
 */
int board_early_init_f(void)
{
	//nvt_ipc_init();
	return 0;
}

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	int ret = 0;
#if 0//!defined(CONFIG_NVT_FPGA_EMULATION)
//	unsigned int sctlr;
	unsigned int  cval_high;
	unsigned int  cval_low;
#endif

	printf("Relocation to 0x%08lx, Offset is 0x%08lx sp at %08lx\n", gd->relocaddr, gd->reloc_off, gd->start_addr_sp);

//	asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
//	printf("Before %s SCTLR: 0x%08x\n", __func__, sctlr);

	icache_disable();
	dcache_disable();

	invalidate_icache_all();
	invalidate_dcache_all();

#if 0//!defined(CONFIG_NVT_FPGA_EMULATION)
	asm volatile ("mrrc p15, 1, %0, %1, c15" : "=r" (cval_low), "=r" (cval_high));
	cval_low &= ~0x40;
	asm volatile ("mcrr p15, 1, %0, %1, c15" : : "r" (cval_low), "r" (cval_high));
	cval_low |= 0x40;
	asm volatile ("mcrr p15, 1, %0, %1, c15" : : "r" (cval_low), "r" (cval_high));
	asm volatile ("mrrc p15, 1, %0, %1, c15" : "=r" (cval_low), "=r" (cval_high));
#endif

	icache_enable();
	dcache_enable();

//	asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
//	printf("After %s SCTLR: 0x%08x\n", __func__, sctlr);

	nvt_shminfo_init();
	timer_init();
	ret = nvt_fdt_init(false);
	if (ret < 0) {
		printf("fdt init fail\n");
	}

	#if 0//def CONFIG_NVT_FPGA_EMULATION
	nvt_writel(0xFFFFFFFF, (ulong)0xF0E40048);
	printf("0xF0E40048 = 0x%08x\r\n", nvt_readl(0xF0E40048));
	printf("0xF0E4004C = 0x%08x\r\n", nvt_readl(0xF0E4004C));
	#endif

	nvt_print_system_info();

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_NVT_MMC
	int i, CONFIG_NVT_MMC_CHANNEL = 0;
#endif

#ifdef CONFIG_NVT_MMC_CHANNEL_SDIO1
	CONFIG_NVT_MMC_CHANNEL |= 1 << 0;
#endif

#ifdef CONFIG_NVT_MMC_CHANNEL_SDIO2
	CONFIG_NVT_MMC_CHANNEL |= 1 << 1;
#endif

#ifdef CONFIG_NVT_MMC_CHANNEL_SDIO3
	CONFIG_NVT_MMC_CHANNEL |= 1 << 2;
#endif
#ifdef CONFIG_NVT_MMC
	for (i = 0; i < CONFIG_NVT_MMC_MAX_NUM; i++) {
		if((CONFIG_NVT_MMC_CHANNEL >> i) & 0x1) {
			ret = nvt_mmc_init(i);
			if(ret)
				break;
		}
	}
#endif
	return ret;
}

#if 0
int board_nand_init(bd_t *bis)
{
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_NOVATEK_MAC_ENET
	rc = na51068_eth_initialize(bis);
#endif

	return rc;
}

static int nvt_handle_fw_abin(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};

	/* To handle firmware update */
	ret = nvt_fw_update(false);
	if (ret < 0) {
		switch (ret) {
		case ERR_NVT_UPDATE_FAILED:
			printf("%sUpdate fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
			break;
		case ERR_NVT_UPDATE_OPENFAILED:
			printf("Open SD fail:%s No SD device? %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			break;
		case ERR_NVT_UPDATE_READ_FAILED:
			printf("%sRead SD fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
			break;
		case ERR_NVT_UPDATE_NO_NEED:
			printf("%sNo need to update (%s) %s\r\n", ANSI_COLOR_YELLOW, get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW), ANSI_COLOR_RESET);
			break;
		default:
			break;
		}
		ret = nvt_fdt_init(true);
		if (ret < 0) {
			printf("modelext init fail\n");
		}
	} else {
		printf("%sUpdate successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	}
	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);

	return 0;
}

static int nvt_handle_fw_tbin(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};

	ret = nvt_fw_load_tbin();
	if (ret < 0) {
		switch (ret) {
		case ERR_NVT_UPDATE_OPENFAILED:
			printf("Open SD fail:%s No SD device? (%s) %s\r\n", ANSI_COLOR_YELLOW, get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), ANSI_COLOR_RESET);
			break;
		case ERR_NVT_UPDATE_NO_NEED:
		case ERR_NVT_UPDATE_READ_FAILED:
			printf("%sRead SD fail (%s) %s\r\n", ANSI_COLOR_RED, get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), ANSI_COLOR_RESET);
			return -1;
			break;
		default:
			break;
		}
	} else
		printf("%sLoad successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);

	ret = nvt_fdt_init(false);
	if (ret < 0) {
		printf("modelext init fail\n");
		return ret;
	}

	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);

	return 0;
}

static int nvt_handle_fw_update_fw(void)
{
	int ret = 0;
	unsigned long val = 0;

	printf("%sStarting to update firmware runtime %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	/* To handle firmware update */
	ret = nvt_fw_update(true);
	if (ret < 0) {
		printf("%sUpdate fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		val = (nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & ~COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK) | COMM_UBOOT_BOOT_FUNC_BOOT_NG;
		nvt_writel(val, (ulong)nvt_shminfo_comm_uboot_boot_func);
		flush_dcache_all();
		return -1;
	} else {
		printf("%sUpdate firmware successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);

		// Update finished.
		val = (nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & ~COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK) | COMM_UBOOT_BOOT_FUNC_BOOT_DONE;
		nvt_writel(val, (ulong)nvt_shminfo_comm_uboot_boot_func);
		flush_dcache_all();
		while(1) {
			// Waiting for itron trigger reboot.
			printf(".");
			mdelay(1000);
		}
	}

	return 0;
}

static int nvt_handle_update_fw_by_usb_eth(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};

	printf("%sStarting to update firmware from USB/ETH%s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	/* To handle firmware update */
	ret = nvt_fw_update(true);
	if (ret < 0) {
		printf("%sUpdate fail %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		return -1;
	} else {
		printf("%sUpdate firmware successfully %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	}

	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 * Continue to boot
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);
	return 0;
}

static int nvt_handle_format_rw_rootfs(void)
{
	int ret = 0;
	ret = nvt_process_rootfs_format();

	return ret;
}

static int nvt_handle_recovery_sys(void)
{
	int ret = 0;
	char cmdline[1024] = {0};
	char buf[1024] = {0};
	ret = nvt_process_sys_recovery();

	/*
	 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
	 */
	sprintf(buf,"%s ",env_get("bootargs"));
	strcat(cmdline, buf);
	ret = nvt_part_config((char *)cmdline, NULL);
	if (ret < 0)
		return ret;

	env_set("bootargs",cmdline);

	return ret;
}

#ifdef CONFIG_NVT_LINUX_AUTODETECT
static int nvt_handle_fw_auto(void)
{
	int ret = 0;
	unsigned long boot_reason = nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & COMM_UBOOT_BOOT_FUNC_BOOT_REASON_MASK;

	if (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM) {
		// Update by itron: firmware will be loaded by itron.
		ret = nvt_handle_fw_update_fw();
		if (ret < 0)
			return ret;
	} else if ((boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPD_FRM_USB) || \
				(boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPD_FRM_ETH)){
		// Update by USB/ETH: firmware will be loaded by loader.
		ret = nvt_handle_update_fw_by_usb_eth();
		if (ret < 0)
			return ret;
	} else if (nvt_detect_fw_tbin()) {
		ret = nvt_handle_fw_tbin();
		if (ret < 0)
			return ret;

		#if defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) || defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
		#else
		char cmdline[512] = {0};
		char buf[255] = {0};
		/*
		 * To handle bootargs expanding for the kernel /proc/cmdline and uboot mtdids env setting
		 */
		sprintf(buf,"%s ",env_get("bootargs"));
		strcat(cmdline, buf);
		ret = nvt_part_config((char *)cmdline, NULL);
		if (ret < 0)
			return ret;

		env_set("bootargs",cmdline);
		#endif /* CONFIG_NVT_LINUX_RAMDISK_SUPPORT || CONFIG_NVT_LINUX_SPINAND_BOOT */
	} else if (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_FORMAT_ROOTFS){
		// Format R/W rootfs, we should check if this partition is existed firstly.
		ret = nvt_handle_format_rw_rootfs();
		if (ret < 0)
			return ret;
	} else if (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_RECOVERY_SYS){
		// Recovery system if loader send event to do system recovery. (EMMC boot only)
		ret = nvt_handle_recovery_sys();
		if (ret < 0)
			return ret;
	} else {
		ret = nvt_handle_fw_abin();
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif /* CONFIG_NVT_LINUX_AUTODETECT */

#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
static int nvt_emmc_init(void)
{
	int ret = 0;
	char command[128] = {0};

	/* Switch to emmc bus and user partition access config */
	sprintf(command, "mmc dev %d", CONFIG_NVT_IVOT_EMMC);
	ret = run_command(command, 0);
	if (ret < 0)
		return ret;
	sprintf(command, "mmc partconf %d 1 1 0", CONFIG_NVT_IVOT_EMMC);
	ret = run_command(command, 0);

	return ret;
}
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */
/**
 * @brief misc_init_r - To do nvt update and board init.
 *
 * @return 0
 */
int misc_init_r(void)
{
	int ret = 0;

	printf("%sFirmware name: %s %s %s %s \n", ANSI_COLOR_YELLOW,
						  get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW),
						  get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW),
						  get_nvt_bin_name(NVT_BIN_NAME_TYPE_MODELEXT),
						  ANSI_COLOR_RESET);
#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	ret = nvt_emmc_init();
	if (ret < 0)
		return ret;
#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */
#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	#ifdef CONFIG_NVT_LINUX_AUTODETECT
	ret = nvt_handle_fw_auto();
	if (ret < 0)
		return ret;
	#else /* !CONFIG_NVT_LINUX_AUTODETECT */
	#ifdef CONFIG_NVT_LINUX_SPINAND_BOOT
	ret = nvt_handle_fw_abin();
	if (ret < 0)
		return ret;
	#else /* !CONFIG_NVT_LINUX_SPINAND_BOOT */
	ret = nvt_handle_fw_tbin();
	if (ret < 0)
		return ret;
	#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */
	#endif /* CONFIG_NVT_LINUX_AUTODETECT */
#else /* !CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */
	/* FIXME: To do customized boot */
	ret = nvt_fdt_init(true);
	if (ret < 0) {
		printf("modelext init fail\n");
		return ret;
	}
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

	/* Why to do this again?
	 * This is because modelext maybe updated, we should init again here.
	 */
	ret = nvt_board_init();
	if (ret < 0) {
		printf("board init fail\n");
		return ret;
	}

	return 0;
}

/*
 * dram init.
 */
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/*
 * get_board_rev() - get board revision
 */
u32 get_board_rev(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}
