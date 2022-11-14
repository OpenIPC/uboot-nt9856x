/*
 * Copyright (C) 2016 Novatek Microelectronics Corp. All rights reserved.
 * Author: iVoT-IM <iVoT_MailGrp@novatek.com.tw>
 *
 * Configuration settings for the Novatek NA51068 SOC.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_NA51068_H
#define __CONFIG_NA51068_H

#include <linux/sizes.h>

/*#define CONFIG_DEBUG	1	*/

#ifdef CONFIG_DEBUG
#define DEBUG						1
#endif

/*#define CONFIG_NVT_FW_UPDATE_LED*/
/*#define CONFIG_NVT_PWM*/

/*
 * High Level Configuration Options
 */

#define CONFIG_SYS_NAND_BASE                            0xFA900000

#define CONFIG_SYS_HZ					1000

//#define CONFIG_USE_ARCH_MEMCPY
//#define CONFIG_USE_ARCH_MEMSET

/*RTC Default Date*/
#define RTC_YEAR 2000
#define RTC_MONTH 1
#define RTC_DAY 1

/*-----------------------------------------------------------------------
 * IP address configuration
 */
#ifdef CONFIG_NOVATEK_MAC_ENET
#define CONFIG_ETHNET
#endif

#define FIXED_ETH_PARAMETER

#ifdef FIXED_ETH_PARAMETER
#ifdef CONFIG_ETHNET
#define CONFIG_ETHADDR				{0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x01}
#define CONFIG_IPADDR				192.168.1.99	/* Target IP address */
#define CONFIG_NETMASK				255.255.255.0
#define CONFIG_SERVERIP				192.168.1.11	/* Server IP address */
#define CONFIG_GATEWAYIP			192.168.1.254
#define CONFIG_HOSTNAME				"soclnx"
#endif
#endif

#define ETH_PHY_HW_RESET
#define NVT_PHY_RST_PIN D_GPIO(1)
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
 * NVT LED CONFIG
 *
 * LED GPIO selection
 * C_GPIO(x)
 * P_GPIO(x)
 * S_GPIO(x)
 * L_GPIO(x)
 * D_GPIO(x)
 * Duration Unit: ms
 */
#ifdef CONFIG_NVT_FW_UPDATE_LED
#ifdef CONFIG_NVT_PWM
#define PWM_SIGNAL_NORMAL 0
#define PWM_SIGNAL_INVERT 1
#define NVT_PWMLED (PWMID_0 | PWMID_1)
#define PWM_SIGNAL_TYPE PWM_SIGNAL_INVERT
#define	PWM_LED_ERASE 50
#define	PWM_LED_PROGRAM 5
#else
#define NVT_LED_PIN P_GPIO(12)
#define NVT_LED_ERASE_DURATION 30
#define NVT_LED_PROGRAM_DURATION 10
#endif
#endif


/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */

#define CONFIG_NR_DRAM_BANKS				1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1					0x00000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE				CONFIG_MEM_SIZE	/* DDR size 512MB */

/*
 * To include nvt memory layout
 */
#include "novatek/na51068_ca9.h"

#define CONFIG_SYS_SDRAM_BASE				CONFIG_LINUX_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE				CONFIG_LINUX_SDRAM_SIZE + CONFIG_UBOOT_SDRAM_SIZE

#define NVT_LINUX_BOOT_PARAM_ADDR			(CONFIG_LINUX_SDRAM_BASE + 0x100)

#define CONFIG_SYS_INIT_SP_ADDR				(CONFIG_UBOOT_SDRAM_BASE + CONFIG_UBOOT_SDRAM_SIZE - 0x1000)

/*
 * Our DDR memory always starts at 0x00000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 * However, set this to a 32MB offset to allow for easier Linux kernel
 * booting as the default is often used as the kernel load address.
 */
#define CONFIG_SYS_LOAD_ADDR				CONFIG_LINUX_SDRAM_START

#define CONFIG_STANDALONE_LOAD_ADDR			0x1A000000

#define CONFIG_SYS_UBOOT_START				CONFIG_SYS_TEXT_BASE

#define CONFIG_CMD_MEMORY				1

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS				64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE				1024
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE				(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE				CONFIG_SYS_CBSIZE
										/* even with bootdelay=0 */
/*#define CONFIG_AUTOBOOT_KEYED				1*/
#define CONFIG_AUTOBOOT_STOP_STR			"~"

/* MMC */
#define CONFIG_SUPPORT_EMMC_BOOT					/* Support emmc boot partition */

/* MTD */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_LZO							/* required by CONFIG_CMD_UBIFS */
#define CONFIG_LZMA							/* required by uitron decompress */
#define CONFIG_MTD_DEVICE
#define CONFIG_SYS_BOOTM_LEN				(25 << 20)

#define CONFIG_CMDLINE_TAG				1		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS			1
#define CONFIG_INITRD_TAG				1
#define CONFIG_REVISION_TAG				1

#define CONFIG_USE_BOOTARGS
#define CONFIG_BOOTARGS_COMMON				"earlyprintk console=ttyS0,115200 rootwait nprofile_irq_duration=on "

/* NVT boot related setting */
#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	#define CONFIG_NVT_LINUX_AUTODETECT					/* Support for detect FW98321A.bin/FW98321T.bin automatically. (Only working on mtd device boot method) */
	#define CONFIG_NVT_BIN_CHKSUM_SUPPORT					/* This option will check rootfs/uboot checksum info. during update image flow */
	#if defined(_NVT_ROOTFS_TYPE_RAMDISK_)
		/* the ramdisk dram base/size will be defined in itron modelext info. */
		#define CONFIG_NVT_LINUX_RAMDISK_BOOT 				/* Loading ramdisk image rootfs.bin from SD card */
		#define CONFIG_BOOTARGS 		CONFIG_BOOTARGS_COMMON "root=/dev/ram0 rootfstype=ramfs rdinit=/linuxrc "
		#define CONFIG_CMD_UBI						/* UBI-formated MTD partition support */
		#define CONFIG_CMD_UBIFS					/* Read-only UBI volume operations */
	#elif defined(_NVT_ROOTFS_TYPE_NAND_UBI_)				/* UBIFS rootfs boot */
		#define CONFIG_NVT_LINUX_SPINAND_BOOT
		#define CONFIG_NVT_UBIFS_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "root=ubi0:rootfs rootfstype=ubifs ubi.fm_autoconvert=1 init=/linuxrc "
		#define CONFIG_CMD_UBI						/* UBI-formated MTD partition support */
		#define CONFIG_CMD_UBIFS					/* Read-only UBI volume operations */
	#elif defined(_NVT_ROOTFS_TYPE_NAND_SQUASH_)				/* SquashFs rootfs boot */
		#define CONFIG_NVT_LINUX_SPINAND_BOOT
		#define CONFIG_NVT_SQUASH_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "ubi.block=0,0 root=/dev/ubiblock0_0 rootfstype=squashfs init=/linuxrc "
		#define CONFIG_CMD_UBI						/* UBI-formated MTD partition support */
		#define CONFIG_CMD_UBIFS					/* Read-only UBI volume operations */
	#elif defined(_NVT_ROOTFS_TYPE_NAND_JFFS2_)				/* JFFS2 rootfs boot */
		#define CONFIG_NVT_LINUX_SPINAND_BOOT				/* Boot from spinand or spinor (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_JFFS2_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=jffs2 rw "
	#elif defined(_NVT_ROOTFS_TYPE_NOR_SQUASH_)				/* Squashfs rootfs boot */
		#define CONFIG_NVT_LINUX_SPINOR_BOOT				/* Boot from spinand or spinor (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_SQUASH_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=squashfs ro "
	#elif defined(_NVT_ROOTFS_TYPE_NOR_JFFS2_)				/* JFFS2 rootfs boot */
		#define CONFIG_NVT_LINUX_SPINOR_BOOT				/* Boot from spinand or spinor (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_JFFS2_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=jffs2 rw "
	#elif defined(_NVT_ROOTFS_TYPE_EMMC_)
		#define CONFIG_NVT_LINUX_EMMC_BOOT				/* Boot from emmc (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_EXT4_SUPPORT
		#define CONFIG_FASTBOOT_FLASH
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=ext4 rw "
	#else
		#define CONFIG_NVT_LINUX_SD_BOOT				/* To handle RAW SD boot (e.g. itron.bin, uImage.bin, uboot.bin...) itron.bin u-boot.bin dsp.bin dsp2.bin must be not compressed.*/
		#define CONFIG_BOOTARGS 		CONFIG_BOOTARGS_COMMON "root=/dev/mmcblk0p2 noinitrd rootfstype=ext3 init=/linuxrc "
	#endif /* _NVT_ROOTFS_TYPE_ */
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

/*#define CONFIG_IMAGE_FORMAT_LEGACY*/
#define CONFIG_BOOTCOMMAND				"nvt_boot"

#endif /* __CONFIG_NA51068_H */
