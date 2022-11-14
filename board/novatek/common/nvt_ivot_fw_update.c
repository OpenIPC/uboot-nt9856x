/**
    NVT firmware update
    To do all-in-one firmware check and update
    @file       nvt_ivot_fw_update.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2018.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <fs.h>
#include <u-boot/md5.h>
#include <malloc.h>
#include <nand.h>
#include <mmc.h>
#include <spi_flash.h>

#include <linux/libfdt.h>
#include <fdt.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/nvt_storage.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_sha_smc.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#include <asm/nvt-common/nvt_ivot_rsa_smc.h>
#include <asm/arch/IOAddress.h>
#include "nvt_ivot_pack.h"

#if defined(CONFIG_NVT_FW_UPDATE_LED)
#include <asm/arch/pwm.h>
#endif

#define UPDATE_ALL_IN_ONE_RETRY_TIMES	3
#define CFG_MODEL_EXT_PARTITION 1

#define UINT32_SWAP(data)           (((((UINT32)(data)) & 0x000000FF) << 24) | \
                                     ((((UINT32)(data)) & 0x0000FF00) << 8) | \
                                     ((((UINT32)(data)) & 0x00FF0000) >> 8) | \
                                     ((((UINT32)(data)) & 0xFF000000) >> 24))   ///< Swap [31:24] with [7:0] and [23:16] with [15:8].

static int nvt_update_partitions(unsigned int addr, unsigned int size, u64 part_off, u64 PartitionSize)
{
	char command[128];
	#if defined(CONFIG_NVT_SPI_NAND)
	u32 align_size = ALIGN_CEIL(size, nand_get_block_size());
	#elif defined(CONFIG_NVT_SPI_NOR)
	u32 align_size = ALIGN_CEIL(size, nvt_get_flash_erasesize());
	#elif defined(CONFIG_NVT_IVOT_EMMC)
	u32 align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN);
	/* Using block unit */
	align_size /= MMC_MAX_BLOCK_LEN;
	u64 align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN);
	/* Using block unit */
	align_off /= MMC_MAX_BLOCK_LEN;
	#endif

	if (size > (unsigned int)PartitionSize) {
		printf("%s partition size is too small (0x%08x > 0x%08llx) %s\r\n", ANSI_COLOR_RED, size, PartitionSize, ANSI_COLOR_RESET);
		return -1;
	}

	memset(command, 0, sizeof(command));
	#if defined(CONFIG_NVT_SPI_NAND)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "nand erase 0x%llx 0x%llx", part_off, PartitionSize);
	run_command(command, 0);
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_reload(NVT_PWMLED, PWM_LED_PROGRAM, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "nand write 0x%x 0x%llx 0x%x", addr, part_off, align_size);
	run_command(command, 0);
	#elif defined(CONFIG_NVT_SPI_NOR)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "sf erase 0x%llx +0x%llx", part_off, PartitionSize);
	run_command(command, 0);
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_reload(NVT_PWMLED, PWM_LED_PROGRAM, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "sf write 0x%x 0x%llx 0x%x", addr, part_off, align_size);
	run_command(command, 0);
	#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	//sprintf(command, "mmc erase 0x%x 0x%x", part_off, align_size);
	//run_command(command, 0);
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_reload(NVT_PWMLED, PWM_LED_PROGRAM, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "mmc write 0x%x 0x%llx 0x%x", addr, align_off, align_size);
	run_command(command, 0);
	#endif /* CONFIG_NVT_LINUX_EMMC_BOOT */
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif

	return 0;
}

static int nvt_read_partitions(unsigned int addr, unsigned int size, u64 part_off, unsigned int pat_id, unsigned short embtype)
{
	char command[128];

	memset(command, 0, sizeof(command));
	#if defined(CONFIG_NVT_SPI_NAND)
	#if 0
		#if 1
		unsigned short *buf = malloc(ALIGN_CEIL(size, nand_get_block_size()));
		sprintf(command, "nand read 0x%x 0x%llx 0x%x", buf, part_off, ALIGN_CEIL(size, nand_get_block_size()));
		printk("%s\n",command);
		run_command(command, 0);
		memcpy((void *)addr, buf,size);
		free(buf);
		#else
		sprintf(command, "nand read 0x%x 0x%llx 0x%x", addr, part_off, ALIGN_CEIL(size, nand_get_block_size()));
		printk("%s\n",command);
		run_command(command, 0);

		#endif
	#else
		/* We should avoid read size is not block size alignment problem occurred */
		unsigned short *buf = malloc(nand_get_block_size());
		unsigned int first_part_size = ALIGN_FLOOR(size, nand_get_block_size());
		unsigned int second_part_size = size - first_part_size;
		unsigned long tmp_addr = addr + first_part_size;

		struct mtd_info *mtd;
		u64 offset=0;
		unsigned int bad_count=0;
		//mtd = nand_info[nand_curr_device];
		mtd = get_nand_dev_by_index(nand_curr_device);
		//nand_block_isbad(mtd, offset);
		for(offset=part_off; offset<= part_off+ALIGN_FLOOR(size, nand_get_block_size()); offset = offset+ nand_get_block_size())
		{
		    if (nand_block_isbad(mtd, offset))
		    {
			    bad_count++;
		    }
		}
		sprintf(command, "nand read 0x%x 0x%llx 0x%x", addr, part_off, first_part_size);
		run_command(command, 0);
		if(second_part_size > 0)
		{
		    sprintf(command, "nand read 0x%lx 0x%llx 0x%x", (unsigned long)buf, part_off+first_part_size+bad_count*nand_get_block_size(), nand_get_block_size());
		    run_command(command, 0);
		    memcpy((void *)tmp_addr, buf, second_part_size);
		}
		free(buf);
	#endif
	#elif defined(CONFIG_NVT_SPI_NOR)
		sprintf(command, "sf read 0x%x 0x%llx 0x%x", addr, part_off, size);
		run_command(command, 0);
	#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
		u64 align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN);
		/* Using block unit */
		align_off /= MMC_MAX_BLOCK_LEN;
		u32 align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN);
		/* Using block unit */
		align_size /= MMC_MAX_BLOCK_LEN;
		sprintf(command, "mmc read 0x%x 0x%llx 0x%x", addr, align_off, align_size);
		printf("%s\n", command);
		run_command(command, 0);
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

	return 0;
}

static int nvt_update_fs_partition(unsigned int addr, unsigned int size, unsigned int part_off, unsigned int part_size, EMB_PARTITION *pEmb)
{
	char command[128];

#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip nvt head info
	addr += 64;
	size -= 64;
#endif

	if (size > part_size) {
		printf("%s partition size is too small (0x%08x > 0x%08x) %s\r\n", ANSI_COLOR_RED, size, part_size, ANSI_COLOR_RESET);
		return -1;
	}

	memset(command, 0, sizeof(command));
#if defined(CONFIG_NVT_SPI_NAND)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "nand erase 0x%x 0x%x", part_off, part_size);
	run_command(command, 0);
	sprintf(command, "nand info");
	run_command(command, 0);
#elif defined(CONFIG_NVT_IVOT_EMMC)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
#elif defined(CONFIG_NVT_SPI_NOR)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "sf erase 0x%x +0x%x", part_off, part_size);
	run_command(command, 0);
#else
	printf("nvt_update_fs_partition flash not support\n");
#endif

#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_reload(NVT_PWMLED, PWM_LED_PROGRAM, PWM_SIGNAL_TYPE);
#endif

#if defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT)
	#if defined(CONFIG_NVT_SPI_NAND)
		sprintf(command, "nand write.trimffs 0x%x 0x%x 0x%x", addr, part_off, size);
		printf("%s\n",command);
		run_command(command, 0);
	#elif defined(CONFIG_NVT_SPI_NOR)
		size = ALIGN_CEIL(size, 4096);
		sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
		run_command(command, 0);
	#elif defined(CONFIG_NVT_IVOT_EMMC)
		u32 align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN);
		/* Using block unit */
		align_size /= MMC_MAX_BLOCK_LEN;
		u64 align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN);
		/* Using block unit */
		align_off /= MMC_MAX_BLOCK_LEN;
		if (pEmb->OrderIdx == 0) {
			/* Ramdisk update for rootfs partition idx 0 */
			sprintf(command, "mmc write 0x%x 0x%llx 0x%x", addr, align_off, align_size);
			run_command(command, 0);
		} else {
			/* Other partition is ext4 */
			nvt_sparse_image_update(addr, part_off, size, part_size);
		}
	#else
		printf("nvt_update_fs_partition flash not support!\n");
	#endif
#elif defined(CONFIG_NVT_JFFS2_SUPPORT)
	printf("Update: %s %s\n", __func__, "JFFS2");
	#if defined(CONFIG_NVT_SPI_NAND)
	sprintf(command, "nand write.trimffs 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#else
	printf("flash type error, not support JFFS2\n");
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

#elif defined(CONFIG_NVT_SQUASH_SUPPORT)
	printf("Update: %s %s\n", __func__, "SquashFS");
	#if defined(CONFIG_NVT_SPI_NAND)
	sprintf(command, "nand write.trimffs 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#elif defined(CONFIG_NVT_IVOT_EMMC)
		u32 align_size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN);
		/* Using block unit */
		align_size /= MMC_MAX_BLOCK_LEN;
		u64 align_off = ALIGN_CEIL(part_off, MMC_MAX_BLOCK_LEN);
		/* Using block unit */
		align_off /= MMC_MAX_BLOCK_LEN;
		if (pEmb->OrderIdx == 0) {
			/* Ramdisk update for rootfs partition idx 0 */
			sprintf(command, "mmc write 0x%x 0x%llx 0x%x", addr, align_off, align_size);
			run_command(command, 0);
		} else {
			/* Other partition is ext4 */
			nvt_sparse_image_update(addr, part_off, size, part_size);
		}
	#else
	printf("flash type error, not support SQUASH\n");
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif
#elif defined(CONFIG_NVT_EXT4_SUPPORT)
	printf("Update: %s %s size=0x%08x\n", __func__, "EXT4 or FAT", size);
	nvt_sparse_image_update(addr, part_off, size, part_size);
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif
#else
	printf("Update: %s %s\n", __func__, "UBIFS");

	sprintf(command, "nand write.trimffs 0x%x 0x%x 0x%x", addr, part_off, size);
	printf("%s\n", command);
	run_command(command, 0);
#endif /* !CONFIG_NVT_JFFS2_SUPPORT && !CONFIG_NVT_SQUASH_SUPPORT */

	return 0;
}


static int nvt_update_user_partition(unsigned int addr, unsigned int size, unsigned int part_off, unsigned int part_size, EMB_PARTITION *pEmb)
{
	char command[128];

	printf("user partition %u\n", GET_USER_PART_NUM(pEmb->EmbType));

#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip nvt head info
	addr += 64;
	size -= 64;
#endif

	if (size > part_size) {
		printf("%s partition size is too small (0x%08x > 0x%08x) %s\r\n", ANSI_COLOR_RED, size, part_size, ANSI_COLOR_RESET);
		return -1;
	}

	memset(command, 0, sizeof(command));
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "nand erase 0x%x 0x%x", part_off, part_size);
	run_command(command, 0);
	sprintf(command, "nand info");
	run_command(command, 0);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	//sprintf(command, "mmc erase 0x%x 0x%x", part_off, part_size);
	//run_command(command, 0);
#else
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "sf erase 0x%x +0x%x", part_off, part_size);
	run_command(command, 0);
#endif

#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_reload(NVT_PWMLED, PWM_LED_PROGRAM, PWM_SIGNAL_TYPE);
#endif

#if defined(CONFIG_NVT_JFFS2_SUPPORT)
	printf("Update: %s %s\n", __func__, "JFFS2");
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	sprintf(command, "nand write.trimffs 0x%x rootfs%u 0x%x", addr, pEmb->OrderIdx, size);
	run_command(command, 0);
	#else
	sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

#elif defined(CONFIG_NVT_SQUASH_SUPPORT)
	printf("Update: %s %s\n", __func__, "SquashFS");
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	sprintf(command, "nand write 0x%x rootfs%u 0x%x", addr, pEmb->OrderIdx, size);
	run_command(command, 0);
	#else
	sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif
#elif defined(CONFIG_NVT_EXT4_SUPPORT)
	printf("Update: %s %s size=0x%08x\n", __func__, "EXT4 or FAT", size);
	nvt_sparse_image_update(addr, part_off, size, part_size);
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif
#else
	printf("Update: %s %s\n", __func__, "UBIFS");

	sprintf(command, "nand write.trimffs 0x%x user%u_%d 0x%x", addr, GET_USER_PART_NUM(pEmb->EmbType), pEmb->OrderIdx, size);
	printf("%s\n", command);
	run_command(command, 0);
#endif /* !CONFIG_NVT_JFFS2_SUPPORT && !CONFIG_NVT_SQUASH_SUPPORT */

	return 0;
}

static int nvt_update_rootfsl_partition(unsigned int addr, unsigned int size, unsigned int part_off, unsigned int part_size, EMB_PARTITION *pEmb)
{
	char command[128];

#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip nvt head info
	addr += 64;
	size -= 64;
#endif

	if (size > part_size) {
		printf("%s partition size is too small (0x%08x > 0x%08llx) %s\r\n", ANSI_COLOR_RED, size, part_size, ANSI_COLOR_RESET);
		return -1;
	}

	memset(command, 0, sizeof(command));
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "nand erase 0x%x 0x%x", part_off, part_size);
	run_command(command, 0);
	sprintf(command, "nand info");
	run_command(command, 0);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	//sprintf(command, "mmc erase 0x%x 0x%x", part_off, part_size);
	//run_command(command, 0);
#else
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_setup_start(NVT_PWMLED, PWM_LED_ERASE, PWM_SIGNAL_TYPE);
	#endif
	sprintf(command, "sf erase 0x%x +0x%x", part_off, part_size);
	run_command(command, 0);
#endif

#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_reload(NVT_PWMLED, PWM_LED_PROGRAM, PWM_SIGNAL_TYPE);
#endif

#if defined(CONFIG_NVT_JFFS2_SUPPORT)
	printf("Update: %s %s\n", __func__, "JFFS2");
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	sprintf(command, "nand write.trimffs 0x%x rootfs%u 0x%x", addr, pEmb->OrderIdx, size);
	run_command(command, 0);
	#else
	sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

#elif defined(CONFIG_NVT_SQUASH_SUPPORT)
	printf("Update: %s %s\n", __func__, "SquashFS");
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	sprintf(command, "nand write 0x%x rootfs%u 0x%x", addr, pEmb->OrderIdx, size);
	run_command(command, 0);
	#else
	sprintf(command, "sf write 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
	#endif /* !CONFIG_NVT_LINUX_SPINAND_BOOT */

	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif
#elif defined(CONFIG_NVT_EXT4_SUPPORT)
	printf("Update: %s %s size=0x%08x\n", __func__, "EXT4 or FAT", size);
	nvt_sparse_image_update(addr, part_off, size, part_size);
	#if defined(CONFIG_NVT_FW_UPDATE_LED) && defined(CONFIG_NVT_PWM)
	pwm_close(NVT_PWMLED, 0);
	#endif
#else
	printf("Update: %s %s\n", __func__, "UBIFS");

	sprintf(command, "nand write.trimffs 0x%x rootfs%u 0x%x", addr, pEmb->OrderIdx, size);
	printf("%s\n", command);
	run_command(command, 0);
#endif /* !CONFIG_NVT_JFFS2_SUPPORT && !CONFIG_NVT_SQUASH_SUPPORT */

	return 0;
}

static int nvt_read_fs_partition(unsigned int addr, unsigned int size, unsigned int part_off, unsigned int part_size, unsigned int pat_id, EMB_PARTITION *pEmb)
{
	char command[128];
	int ret = 0;

	memset(command, 0, sizeof(command));
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip nvt head info
	addr += 64;
	size -= 64;
#endif

#if defined(CONFIG_NVT_SPI_NAND)
	#if 0
		#if 1
		unsigned short *buf = malloc(ALIGN_CEIL(size, nand_get_block_size()));
		sprintf(command, "nand read 0x%x 0x%x 0x%x", buf, part_off, ALIGN_CEIL(size, nand_get_block_size()));
		printk("%s\n",command);
		run_command(command, 0);
		memcpy((void *)addr, buf,size);
		free(buf);
		#else
		sprintf(command, "nand read 0x%x 0x%x 0x%x", addr, part_off, ALIGN_CEIL(size, nand_get_block_size()));
		printk("%s\n",command);
		run_command(command, 0);

		#endif
	#else
		/* We should avoid read size is not block size alignment problem occurred */
		struct mtd_info *mtd;
		unsigned int offset=0;
		unsigned int bad_count=0;
		//mtd = nand_info[nand_curr_device];
		mtd = get_nand_dev_by_index(nand_curr_device);
		//nand_block_isbad(mtd, offset);
		for(offset=part_off; offset<= part_off+ALIGN_FLOOR(size, nand_get_block_size()); offset = offset+ nand_get_block_size())
		{
			if (nand_block_isbad(mtd, offset))
			{
				printk("%x\n", offset);
				bad_count++;
			}
		}
		unsigned short *buf = malloc(nand_get_block_size());
		unsigned int first_part_size = ALIGN_FLOOR(size, nand_get_block_size());
		unsigned int second_part_size = size - first_part_size;
		unsigned long tmp_addr = addr + first_part_size;

		sprintf(command, "nand read 0x%x 0x%x 0x%x", addr, part_off, ALIGN_FLOOR(size, nand_get_block_size()));
		run_command(command, 0);
		if(second_part_size > 0)
		{
			sprintf(command, "nand read 0x%lx 0x%x 0x%x", (unsigned long)buf, part_off+first_part_size+bad_count*nand_get_block_size(), nand_get_block_size());
			run_command(command, 0);
			memcpy((void *)tmp_addr, buf, second_part_size);
		}
		free(buf);
	#endif
#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(command, "sf read 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	printf("Read: %s %s size=0x%08x\n", __func__, "EXT4 or FAT", size);
	nvt_sparse_image_readback(addr, part_off, size, part_size);
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */


#if defined(CONFIG_NVT_UBIFS_SUPPORT) && (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_SPI_NAND))
	/*test ubifs*/
	printf("Read: %s %s\n", __func__, "UBIFS");
	if(pEmb->EmbType == EMBTYPE_ROOTFS){

		#if (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_SPI_NAND))
			if (pEmb->OrderIdx == 0){
				/*rootfs0 is ramdisk no need to check ubifs*/
				return ret;
			}
		#endif
		sprintf(command, "ubi part rootfs%u", pEmb->OrderIdx);
		run_command(command, 0);
		sprintf(command, "ubifsmount ubi:rootfs");
		run_command(command, 0);
	}
	else if(pEmb->EmbType == EMBTYPE_APP){
		sprintf(command, "ubi part app");
		run_command(command, 0);
		sprintf(command, "ubifsmount ubi:app");
		run_command(command, 0);
	}
	else{
		printk("partition type%x , not support UBIFS\n",pEmb->EmbType);
		return -1;
	}

	sprintf(command, "ubifsls");
	run_command(command, 0);
#elif defined(CONFIG_NVT_SQUASH_SUPPORT)
	printf("Read: %s %s\n", __func__, "SQUASHFS");
#elif defined(CONFIG_NVT_EXT4_SUPPORT)
	printf("Read: %s %s\n", __func__, "EXT4");
#else
	printf("Read: %s\n", __func__);
#endif

	return ret;
}

static int nvt_read_rootfsl_partition(unsigned int addr, unsigned int size, unsigned int part_off, unsigned int part_size, unsigned int pat_id, EMB_PARTITION *pEmb)
{
	char command[128];
	int ret = 0;

	memset(command, 0, sizeof(command));

#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip nvt head info
	addr += 64;
	size -= 64;
#endif

#if defined(CONFIG_NVT_SPI_NAND)
	/* We should avoid read size is not block size alignment problem occurred */
	unsigned short *buf = malloc(nand_get_block_size());
	unsigned int first_part_size = ALIGN_FLOOR(size, nand_get_block_size());
	unsigned int second_part_size = size - first_part_size;
	unsigned long tmp_addr = addr + first_part_size;

	sprintf(command, "nand read 0x%x 0x%x 0x%x", addr, part_off, ALIGN_FLOOR(size, nand_get_block_size()));
	run_command(command, 0);
	sprintf(command, "nand read 0x%lx 0x%x 0x%x", (unsigned long)buf, part_off+first_part_size, nand_get_block_size());
	run_command(command, 0);
	memcpy((void *)tmp_addr, buf, second_part_size);
	free(buf);
#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(command, "sf read 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	printf("Read: %s %s size=0x%08x\n", __func__, "EXT4 or FAT", size);
	nvt_sparse_image_readback(addr, part_off, size, part_size);
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */

#if defined(CONFIG_NVT_UBIFS_SUPPORT)
	printf("Read: %s %s\n", __func__, "UBIFS");
	sprintf(command, "ubi part rootfs%u", pEmb->OrderIdx);
	run_command(command, 0);

	sprintf(command, "ubifsmount ubi:rootfs");
	run_command(command, 0);

	sprintf(command, "ubifsls");
	run_command(command, 0);
#elif defined(CONFIG_NVT_SQUASH_SUPPORT)
	printf("Read: %s %s\n", __func__, "SQUASHFS");
#elif defined(CONFIG_NVT_EXT4_SUPPORT)
	printf("Read: %s %s\n", __func__, "EXT4");
#else
	printf("Read: %s %s\n", __func__, "JFFS2");
#endif

	return ret;
}

static int nvt_read_user_partition(unsigned int addr, unsigned int size, unsigned int part_off, unsigned int part_size, unsigned int pat_id, EMB_PARTITION *pEmb)
{
	char command[128];
	int ret = 0;

	printf("user partition %u\n", GET_USER_PART_NUM(pEmb->EmbType));
	memset(command, 0, sizeof(command));

#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip nvt head info
	addr += 64;
	size -= 64;
#endif

#if defined(CONFIG_NVT_SPI_NAND)
	/* We should avoid read size is not block size alignment problem occurred */
	unsigned short *buf = malloc(nand_get_block_size());
	unsigned int first_part_size = ALIGN_FLOOR(size, nand_get_block_size());
	unsigned int second_part_size = size - first_part_size;
	unsigned long tmp_addr = addr + first_part_size;

	sprintf(command, "nand read 0x%x 0x%x 0x%x", addr, part_off, ALIGN_FLOOR(size, nand_get_block_size()));
	run_command(command, 0);
	sprintf(command, "nand read 0x%lx 0x%x 0x%x", (unsigned long)buf, part_off+first_part_size, nand_get_block_size());
	run_command(command, 0);
	memcpy((void *)tmp_addr, buf, second_part_size);
	free(buf);
#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(command, "sf read 0x%x 0x%x 0x%x", addr, part_off, size);
	run_command(command, 0);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	printf("Read: %s %s size=0x%08x\n", __func__, "EXT4 or FAT", size);
	nvt_sparse_image_readback(addr, part_off, size, part_size);
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */

#if defined(CONFIG_NVT_UBIFS_SUPPORT)
	printf("Read: %s %s\n", __func__, "UBIFS");
	sprintf(command, "ubi part rootfs%u", pEmb->OrderIdx);
	run_command(command, 0);

	sprintf(command, "ubifsmount ubi:rootfs");
	run_command(command, 0);

	sprintf(command, "ubifsls");
	run_command(command, 0);
#elif defined(CONFIG_NVT_SQUASH_SUPPORT)
	printf("Read: %s %s\n", __func__, "SQUASHFS");
#elif defined(CONFIG_NVT_EXT4_SUPPORT)
	printf("Read: %s %s\n", __func__, "EXT4");
#else
	printf("Read: %s %s\n", __func__, "JFFS2");
#endif

	return ret;
}


static UINT32 MemCheck_CalcCheckSum16Bit(UINT32 uiAddr,UINT32 uiLen)
{
	UINT32 i,uiSum = 0;
	UINT16 *puiValue = (UINT16 *)uiAddr;

	for (i=0; i<(uiLen >> 1); i++)
	{
	uiSum += (*(puiValue + i) + i);
	}

	uiSum &= 0x0000FFFF;

	return uiSum;
}

static int nvt_chk_last_ebr(ulong ebr_addr, u64 ebr_part_offset, EMB_PARTITION *pEmb)
{
	unsigned char *buf;
	u32 disk_capacity, new_size, old_size, part_addr;
	int i, last_part_idx, partition_number, last_ebr_idx, first_mbr_idx;

	//Find the index of the last pstore/rootfs partition
	if(pEmb==NULL)
	{
		printf("failed to get current partition resource.\r\n");
		return CMD_RET_FAILURE;
	}

	last_part_idx = -1;
	for(i = (EMB_PARTITION_INFO_COUNT - 1) ; i >= 0 ; i--)
	{
		if(pEmb[i].EmbType == EMBTYPE_UNKNOWN)
			continue;
		else if(pEmb[i].EmbType == EMBTYPE_ROOTFS || pEmb[i].EmbType == EMBTYPE_ROOTFSL){
			if(pEmb[i].PartitionSize != 0){
				printf("size of last partition is not 0, no need to fix\r\n");
				return CMD_RET_SUCCESS;
			}
			else{
				last_part_idx = i;
			}
			break;
		}
		else{
			printf("last partition is of type %d, not rootfs or pstore, no need to fix\r\n", pEmb[i].EmbType);
			return CMD_RET_SUCCESS;
		}
	}

	if(last_part_idx == -1){
		printf("no rootfs,pstore partition, no need to fix\r\n");
		return CMD_RET_SUCCESS;
	}

	//Find the index of the last ebr
	last_ebr_idx = -1;
	for(i = (EMB_PARTITION_INFO_COUNT - 1) ; i >= 0 ; i--)
		if(pEmb[i].EmbType == EMBTYPE_MBR){
			last_ebr_idx = i;
			break;
		}

	if(last_ebr_idx == -1){
		printf("fail to get last ebr's index\r\n");
		return CMD_RET_FAILURE;
	}

	//Find the index of mbr
	first_mbr_idx = -1;
	for(i = 0 ; i < EMB_PARTITION_INFO_COUNT ; i++)
		if(pEmb[i].EmbType == EMBTYPE_MBR){
			first_mbr_idx = i;
			break;
		}

	if(first_mbr_idx == -1){
		printf("fail to get 1st mbr's index\r\n");
		return CMD_RET_FAILURE;
	}

	//If ebr from argument is neither 1st mbr nor last ebr, ignore it
	if(ebr_part_offset != pEmb[first_mbr_idx].PartitionOffset && ebr_part_offset != pEmb[last_ebr_idx].PartitionOffset)
		return CMD_RET_SUCCESS;

	//Get pstore/rootfs partition number
	partition_number = 0;
	for(i = 0 ; i < EMB_PARTITION_INFO_COUNT ; i++)
		if(pEmb[i].EmbType == EMBTYPE_ROOTFS || pEmb[i].EmbType == EMBTYPE_ROOTFSL)
			partition_number++;

	//If the last partition ls logical, make sure there is a EBR partition ahead
	if(partition_number >= 4){
		if((last_part_idx - 1) != last_ebr_idx){
			printf("last ebr index(%d) is not right ahead of the last mbr ebr index(%d)\r\n", last_ebr_idx, last_part_idx);
			return CMD_RET_FAILURE;
		}
	}
	//rootfs,pstore number < 4, there should be no ebr, so last_ebr_idx should eb equal to first_mbr_idx
	else if(first_mbr_idx != last_ebr_idx){
		printf("only %d rootfs,pstore partions, but mbr index(%d) != last ebr index(%d)\r\n", partition_number, first_mbr_idx, last_ebr_idx);
		return CMD_RET_FAILURE;
	}

	//Check mbr/ebr signature
	buf = (unsigned char*)ebr_addr;
	if(buf[510] != 0x55 || buf[511] != 0xAA){
		printf("invalid mbr ebr signature 0x%x 0x%x, they should be 0x55 0xAA\r\n", (unsigned int)buf[510], (unsigned int)buf[511]);
		return CMD_RET_FAILURE;
	}
#ifdef CONFIG_NVT_MMC
	//Get emmc's max capacity
	extern u32 get_emmc_capacity(void);
	disk_capacity = get_emmc_capacity();
	printf("emmc capacity is %d sectors\r\n", disk_capacity);
	if(disk_capacity == 0){
		printf("fail to get emmc's capacity\r\n");
		return CMD_RET_FAILURE;
	}
#endif
	//Fix MBR's size field
	if(ebr_part_offset == pEmb[first_mbr_idx].PartitionOffset){
		//Fix primary partition's size
		if(partition_number < 4){
			new_size = disk_capacity - (u32)(pEmb[last_part_idx].PartitionOffset/MMC_MAX_BLOCK_LEN);
			buf = (unsigned char*)(ebr_addr + 446 + ((partition_number - 1) * 16) + 12);
			old_size = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
			printf("last primary partition old size = %d ; new size = %d\r\n", old_size, new_size);
			buf[0] = (new_size & 0x0FF);
			buf[1] = ((new_size & 0x0FF00) >> 8);
			buf[2] = ((new_size & 0x0FF0000) >> 16);
			buf[3] = ((new_size & 0xFF000000) >> 24);
		}
		//Fix extend partition's size
		else{
			buf = (unsigned char*)(ebr_addr + 494 + 8);
			part_addr = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
			new_size = disk_capacity - part_addr;
			buf = (unsigned char*)(ebr_addr + 494 + 12);
			old_size = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
			printf("extended partition old size = %d ; new size = %d\r\n", old_size, new_size);
			buf[0] = (new_size & 0x0FF);
			buf[1] = ((new_size & 0x0FF00) >> 8);
			buf[2] = ((new_size & 0x0FF0000) >> 16);
			buf[3] = ((new_size & 0xFF000000) >> 24);
		}
	}
	//Fix EBR's size field
	else{
		new_size = disk_capacity - (u32)(pEmb[last_part_idx].PartitionOffset/MMC_MAX_BLOCK_LEN);
		buf = (unsigned char*)(ebr_addr + 446 + 12);
		old_size = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
		printf("last ebr old size = %d ; new size = %d\r\n", old_size, new_size);
		buf[0] = (new_size & 0x0FF);
		buf[1] = ((new_size & 0x0FF00) >> 8);
		buf[2] = ((new_size & 0x0FF0000) >> 16);
		buf[3] = ((new_size & 0xFF000000) >> 24);
	}
	return CMD_RET_SUCCESS;
}

static int nvt_chk_app(ulong addr, unsigned int size, unsigned int pat_id){

	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("app pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	return 0;

}

static int nvt_chk_loader(ulong addr, unsigned int size, unsigned int pat_id)
{
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("loader pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	return 0;
}

static int nvt_chk_modelext(ulong addr, unsigned int size, unsigned int pat_id)
{
	int  ret;

	ret = nvt_check_isfdt(addr);
	if (ret < 0)
		printf("fdt pat%d, res check sum fail.\r\n", pat_id);

	return ret;
}

static int nvt_chk_uitron(ulong addr, unsigned int size, unsigned int pat_id)
{
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("uitron pat%d, res check sum fail.\r\n", pat_id);
		return -1;
	}
	return 0;
}

static int nvt_chk_ecos(ulong addr, unsigned int size, unsigned int pat_id)
{
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("ecos pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	return 0;
}

static int nvt_chk_rtos(ulong addr, unsigned int size, unsigned int pat_id)
{
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("ecos pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	return 0;
}

static int nvt_chk_atf(ulong addr, unsigned int size, unsigned int pat_id)
{
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0) {
		printf("atf pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	return 0;
}

static int nvt_chk_teeos(ulong addr, unsigned int size, unsigned int pat_id){


	#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("teeos pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	#else
		printf("please enable CONFIG_NVT_IVOT_OPTEE_SUPPORT\n");
		return -1;
	#endif
	return 0;
}

static int nvt_chk_ai(ulong addr, unsigned int size, unsigned int pat_id)
{
	if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
	{
		printf("ai pat%d, res check sum fail.\r\n",pat_id);
		return -1;
	}
	return 0;
}


static int nvt_chk_uboot(ulong addr, unsigned int size, unsigned int pat_id)
{
	unsigned int is_secure = 0;
	#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT

		#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
			NVT_SMC_EFUSE_DATA efuse_data = {0};
			efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;
			is_secure = nvt_ivot_optee_efuse_operation(&efuse_data);
		#endif
	#endif
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	extern ulong __image_copy_start;
	extern HEADINFO gHeadInfo;
	ulong tag_offset;
	char *tag;
	NVTPACK_BFC_HDR *pbfc = (NVTPACK_BFC_HDR *)addr;
	gHeadInfo.CodeEntry = (ulong)&__image_copy_start;
	tag_offset = (unsigned long)gHeadInfo.BinInfo_1 - (unsigned long)gHeadInfo.CodeEntry;
	tag = (char*)(addr + tag_offset);

	/* This is for compression check */
	if(is_secure == 0)
	{
		if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
		{
			printf("uboot pat%d, check sum fail.\r\n",pat_id);
			return -1;
		}
		if(pbfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			// decode 0x200 size for get BinInfo_1
			const int preload_size = 0x200;
			// enlarge 10 times to avoid buffer overwrite on decompressing
			const int tmp_size = preload_size*10;
			unsigned char *p_tmp = (unsigned char *)malloc(tmp_size);
			if (p_tmp == NULL) {
				printf("no mem to decode uboot.lz, req size = 0x%08X\n", tmp_size);
				return -1;
			}

			if((cpu_to_be32(pbfc->uiAlgorithm) & 0xFF ) == 11) {
				printf("uboot.lzma used, skip tag check.\r\n");
			} else {
				lz_uncompress((unsigned char *)(addr + sizeof(NVTPACK_BFC_HDR)),
						(unsigned char *)p_tmp,
						preload_size);
				tag = (char*)(p_tmp + tag_offset);
				if(strncmp(tag, gHeadInfo.BinInfo_1, 8) !=0 ) {
					printf("uboot pat%d, tag not match %8s(expect) != %8s(bin).\r\n"
					    ,pat_id ,gHeadInfo.BinInfo_1, tag);
					return -1;
				}
			}
		}

	}
	else
	{
		if(MemCheck_CalcCheckSum16Bit(addr, size)!=0)
		{
			printf("uboot pat%d, res check sum fail.\r\n",pat_id);
			return -1;
		}
		return 0;
	}
#endif /* CONFIG_NVT_BIN_CHKSUM_SUPPORT */
	return 0;
}

#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
static void get_bininfo_size_offset(unsigned char* bininfo,unsigned int* size,unsigned int *offset)
{

	*size = bininfo[4] | bininfo[5]<<8 | bininfo[6] << 16 | bininfo[7] << 24;
	*offset = bininfo[0] | bininfo[1] << 8 | bininfo[2] << 16 | bininfo[3] <<24;
}

static void fill_data_r(UINT8 * s, UINT8 * target, int len)
{
	int   i;

	for(i=0; i<len; i++)
		target[len-i-1] = s[i];
}

static int nvt_chk_key(ulong addr, unsigned int size)
{
	HEADINFO *head_info = (HEADINFO *)addr;
	unsigned int key_size = 0;
	unsigned int i=0;
	unsigned int key_offset =0;
	unsigned char * key_buf = NULL;
	unsigned char sha256[32]={0};
	unsigned char key_buf_reverse[256]={0};
	int ret =0;


	get_bininfo_size_offset(head_info->BinInfo_1, &key_size, &key_offset);
	key_buf = (unsigned char *)(addr + key_offset);

	#if 1
		printf("only check N key-> key_size:%d key_offset:%d\n",key_size,key_offset);
	#endif
	fill_data_r(key_buf,key_buf_reverse,256);
	#if 0
	printf("N key reverse:\n");
	for(i=0; i< 256;i++)
	{
		printf("%x ",key_buf_reverse[i]);
	}
	printf("\n");
	#endif
	printf("check rsa 2048 key\n");
	NVT_SMC_SHA_DATA sha_data={0};
	sha_data.input_data = key_buf_reverse;
	sha_data.output_data = sha256;
	sha_data.input_size = 256;
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return ret;
	}
	#if 1
	printf(" rsa key reverse and sha256:\n");
	for(i=0; i< 32; i++)
	{
		printf("%x ",*(char *)(sha256+i));
	}
	printf("\n");
	#endif
	#if 1
	/*************
		compare key hash from efuse
	**************/

	/*default set sha256 in 1,2 efuse key field,   0 efuse key field for aes key*/

	//check low part-->sha256[0] ~ sha256[15]

	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_COMPARE_KEY;
	efuse_data.key_data.field = EFUSE_OTP_2ND_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data, &sha256[0], 16);
	if(nvt_ivot_optee_efuse_operation(&efuse_data) !=1)
	{
		printf("compare key2 fail!, please check key in efuse or partition\n");
		return -1;
	}
	efuse_data.key_data.field = EFUSE_OTP_3RD_KEY_SET_FIELD;
	memcpy(efuse_data.key_data.data, &sha256[16], 16);
	if(nvt_ivot_optee_efuse_operation(&efuse_data) !=1)
	{
		printf("compare key3 fail!, please check key in efuse or partition\n");
		return -1;
	}

	printf("check rsa key ok\n");
	#endif
	return 0;
}


static int nvt_chk_signature(ulong addr, unsigned int size)
{
	int ret=0;
	HEADINFO *head_info = (HEADINFO *)addr;
	unsigned int key_size = 0;
	unsigned int key_offset =0;
	unsigned char * key_buf = NULL;
	unsigned int signature_size = 0;
	unsigned int signature_offset = 0;
	unsigned char * signature_buf = NULL;
	unsigned int crypto_size = 0;
	unsigned int crypto_offset = 0;
	unsigned char * crypto_buf = NULL;
	unsigned int *sha256 = NULL;
	unsigned int rsa_output[64]={0};   //usging unsigned int for address alignment 4
	unsigned int current_sha256[64]={0}; //usging unsigned int for address alignment 4
	unsigned int output_size = sizeof(rsa_output);
	unsigned int data_size =0;
	unsigned int sha256_align_size =0;
	#if 1
	NVT_SMC_EFUSE_DATA efuse_data = {0};
	efuse_data.cmd = NVT_SMC_EFUSE_IS_RSA_KEY_CHECK;
	if(1 == nvt_ivot_optee_efuse_operation(&efuse_data))
	{
		ret = nvt_chk_key( addr, size);
		if(ret != 0)
		{
			printf("nvt_chk_public_key fail ret:%x\n",ret);
			return -1;
		}
	}
	else
	{
		printf("rsa key check not enable\n");
	}
	#else
		ret = nvt_chk_key( addr, size);
		if(ret != 0)
		{
			printf("nvt_chk_public_key fail ret:%x\n",ret);
			return -1;
		}
	#endif
	get_bininfo_size_offset(head_info->BinInfo_1, &key_size, &key_offset);
	key_buf = (unsigned char *)(addr + key_offset);


	get_bininfo_size_offset(head_info->BinInfo_2, &signature_size, &signature_offset);
	signature_buf = (unsigned char *)(addr + signature_offset);


	get_bininfo_size_offset(head_info->BinInfo_3, &crypto_size, &crypto_offset);
	crypto_buf = (unsigned char *)(addr + crypto_offset);


	NVT_SMC_RSA_DATA rsa_data={0};
	//rsa input data size should equal to n key size
	unsigned char signature_buf_tmp[256]={0};
	memcpy(signature_buf_tmp,signature_buf,signature_size);
	rsa_data.rsa_mode = NVT_SMC_RSA_MODE_2048;
	rsa_data.n_key = key_buf;
	rsa_data.n_key_size = 256;
	rsa_data.ed_key = (unsigned char *)((unsigned int)key_buf + 256);
	rsa_data.ed_key_size = key_size - rsa_data.n_key_size;
	rsa_data.input_data = signature_buf_tmp;
	rsa_data.input_size = 256;
	rsa_data.output_data = (unsigned char *)rsa_output;

	ret = nvt_ivot_optee_rsa_operation(&rsa_data);

	if(ret != 0){
		printf("nvt_ivot_optee_rsa_decrypt fail ret:%d\n",ret);
		return -1;
	}

	#if 1
	unsigned int i=0;
	printf("\nrsa decrypt size (%d) :\n", output_size);
	for(i=0;i< (output_size/4);i++)
	{
		printf("%08x ",rsa_output[i]);
	}
	printf("\n");
	#endif

	#if 1
	printf("crypto_buf:%x offset:%x addr:%x\n",crypto_buf,crypto_offset,addr);
	printf("data size:%x\n",crypto_size);
	printf("data :%x %x %x %x %x %x\n",crypto_buf[0],crypto_buf[1],crypto_buf[2],crypto_buf[3],crypto_buf[4],crypto_buf[5]);

	#endif

	/**
	  	------
		header
		-------
		key
		-----
		signature
		----
		encrypt bin

		encrypt bin length = total size - encrypt bin offset

	*/

	data_size = head_info->BinLength - crypto_offset;
	#if 1
	if((data_size & 0x3f) != 0)
	{
		sha256_align_size = ((data_size/0x40) + 1)*0x40;
		memset((unsigned char *)(crypto_buf + data_size),0, sha256_align_size - data_size);
		printf("sha256 data not align 64 ,need add pending data\n");
	}
	else
	{
		sha256_align_size = data_size;
	}

	#endif
	NVT_SMC_SHA_DATA sha_data={0};
	sha_data.input_data = crypto_buf;
	sha_data.output_data = (unsigned char *)current_sha256;
	sha_data.input_size = sha256_align_size;
	sha_data.sha_mode = NVT_SMC_SHA_MODE_SHA256;
	ret = nvt_ivot_optee_sha_operation(&sha_data);
	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha_operation fail,ret:%d\n",ret);
		return ret;
	}


	if(ret != 0)
	{
		printf("nvt_ivot_optee_sha256_compute fail,ret:%d\n",ret);
		return -1;
	}

	sha256 = (unsigned int *)((unsigned int)rsa_output + 256 - 32);// the last 32 bytes are signature
	#if 1
	printf("\n hash img:\n");
	for(i=0;i< 8;i++)
	{
		printf("%x ",current_sha256[i]);
	}
	printf("\n");
	printf("\n signature data:\n");
	for(i=0;i< 8;i++)
	{
		printf("%x ",sha256[i]);
	}
	printf("\n");
	#endif

	if(memcmp(&current_sha256[0], sha256, 32)!= 0)
	{
		printf("compare hash fail!\n");
		return -1;
	}

	return 0;
}
#endif
#endif
static int nvt_chk_linux(ulong addr, unsigned int size, unsigned int pat_id)
{
	unsigned int is_secure = 0;
	#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
		#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
			NVT_SMC_EFUSE_DATA efuse_data = {0};
			efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;
			is_secure = nvt_ivot_optee_efuse_operation(&efuse_data);
		#endif
	#endif

	if(is_secure == 0)
	{
		if(*(unsigned int *)(addr) == MAKEFOURCC('B', 'C', 'L', '1')) {
			if(MemCheck_CalcCheckSum16Bit(addr, size) !=0) {
				printf("uImage checksum fail\n");
				return -1;
			}
		} else if(image_check_dcrc((const image_header_t*)(addr))!=1) {
			printf("linux-kernel pat%d, res check sum fail.\r\n",pat_id);
			return -1;
		}
	}
	else
	{
		printf("%s uImage is encryped , check by checksum\r\n",ANSI_COLOR_YELLOW);
		if(MemCheck_CalcCheckSum16Bit(addr, size) !=0)
		{
			printf("uImage checksum fail\n");
			return -1;
		}

	}


	return 0;
}

static int nvt_chk_rootfs(ulong addr, unsigned int size, unsigned int pat_id)
{
#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	ulong uiContextBuf = addr;
	if(*(unsigned int*)uiContextBuf == MAKEFOURCC('C','K','S','M'))
	{
		NVTPACK_CHKSUM_HDR* pHdr = (NVTPACK_CHKSUM_HDR*)uiContextBuf;
		if(pHdr->uiVersion != NVTPACK_CHKSUM_HDR_VERSION)
		{
			printf("Wrong HEADER_CHKSUM_VERSION %08X(uboot) %08X(root-fs).\r\n",NVTPACK_CHKSUM_HDR_VERSION,pHdr->uiVersion);
			return -1;
		}
		UINT32 uiLen = pHdr->uiDataOffset + pHdr->uiDataSize + pHdr->uiPaddingSize;
		if(MemCheck_CalcCheckSum16Bit(uiContextBuf,uiLen)!=0)
		{
			printf("pat%d, res check sum fail.\r\n",pat_id);
			return -1;
		}
	}
	else
	{
		printf("root-fs has no CKSM header\r\n");
		return -1;
	}
#endif /* CONFIG_NVT_BIN_CHKSUM_SUPPORT */
	return 0;
}

int nvt_chk_all_in_one_valid(unsigned short EmbType, unsigned int addr, unsigned int size, unsigned int id)
{
	switch(EmbType)
	{
		case EMBTYPE_LOADER:
			if(nvt_chk_loader((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_FDT:
			if(nvt_chk_modelext((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_UITRON:
			if(nvt_chk_uitron((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_ECOS:
			if(nvt_chk_ecos((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_UBOOT:
			if(nvt_chk_uboot((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_ATF:
			if(nvt_chk_atf((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_LINUX:
			if(nvt_chk_linux((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_ROOTFS:
			if(nvt_chk_rootfs((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_RTOS:
			if(nvt_chk_rtos((ulong)addr,size,id)!=0)
				return -1;
			break;
		case EMBTYPE_APP:
			if(nvt_chk_app((ulong)addr,size,id)!=0){
				return -1;
			}
			break;
		case EMBTYPE_TEEOS:
			if(nvt_chk_teeos((ulong)addr,size,id)!=0){
				return -1;
			}
			break;
		case EMBTYPE_AI:
			if(nvt_chk_ai((ulong)addr,size,id)!=0){
				return -1;
			}
			break;
		default:
			break;
	}

	return 0;
}

static int nvt_on_partition_enum_sanity(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	int ret = 0;
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	u64 PartitionSize = pEmb[id].PartitionSize;
	#elif defined(CONFIG_NVT_IVOT_EMMC)
	u64 PartitionSize = pEmb[id].PartitionSize;
	#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
	u64 PartitionSize = pEmb[id].PartitionSize;
	#endif
	unsigned int size = p_mem->len;

	#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	// Skip rootfs nvt head info
	if ((pEmb[id].EmbType == EMBTYPE_ROOTFS) || (pEmb[id].EmbType == EMBTYPE_USER) || (pEmb[id].EmbType == EMBTYPE_USERRAW)) {
		size -= 64;
	}
	#endif
	//check partition size
	if(size > PartitionSize)
	{
		printf("Partition[%d] Size is too smaller than that you wanna update.(0x%08X > 0x%08llX)\r\n"
			,id
			,size
			,PartitionSize);
		return -1;
	}

	ret = nvt_chk_all_in_one_valid(pEmb[id].EmbType, (ulong)p_mem->p_data, p_mem->len, id);
	#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT || CONFIG_NVT_LINUX_SPINOR_BOOT || CONFIG_NVT_LINUX_EMMC_BOOT*/
	return ret;
}

static int nvt_on_partition_enum_update(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	u64 PartitionOffset = pEmb[id].PartitionOffset;
	u64 PartitionSize = pEmb[id].PartitionSize;
	int ret = 0;

	unsigned int is_secure = 0;
	#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
		#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
			NVT_SMC_EFUSE_DATA efuse_data = {0};
			efuse_data.cmd = NVT_SMC_EFUSE_IS_SECURE;
			is_secure = nvt_ivot_optee_efuse_operation(&efuse_data);
		#endif
	#endif

	switch(pEmb[id].EmbType)
	{
		case EMBTYPE_MBR:
			printf("%s Update: MBR(EMMC only) %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			nvt_chk_last_ebr((ulong)p_mem->p_data, PartitionOffset, pEmb);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_LOADER:
			printf("%s Update: loader %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_FDT:
			printf("%s Update: fdt %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_UITRON:
			printf("%s Update: uitron %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_ECOS:
			printf("%s Update: ecos %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_UBOOT:
			printf("%s Update: uboot %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			#if 1
			if(is_secure == 1){
				#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
					#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
					//// check signature
					if(nvt_chk_signature((ulong)p_mem->p_data, p_mem->len))
					{
						printf("check signature fail\n");
						return -1;

					}
					#endif
				#endif
			}
			#endif
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_ATF:
			printf("%s Update: ATF %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			#if 1
			if(is_secure == 1){
				#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
					#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
					//// check signature
					if(nvt_chk_signature((ulong)p_mem->p_data, p_mem->len))
					{
						printf("check signature fail\n");
						return -1;

					}
					#endif
				#endif
			}
			#endif
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_APP:
			printf("%s Update: App %u %s \r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_update_fs_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize,&pEmb[id]);
			break;
		case EMBTYPE_LINUX:
			printf("%s Update: linux %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			#if 1
			if(is_secure == 1){
				#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
					#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
					//// check signature
					if(nvt_chk_signature((ulong)p_mem->p_data, p_mem->len))
					{
						printf("check signature fail\n");
						return -1;

					}
					#endif
				#endif
			}
			#endif
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_ROOTFS:
			printf("%s Update: rootfs%u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_update_fs_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize,&pEmb[id]);
			break;
		case EMBTYPE_ROOTFSL:
			printf("%s Update: rootfs logical %u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_update_rootfsl_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize, &pEmb[id]);
			break;
		case EMBTYPE_RTOS:
			printf("%s Update: rtos %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_TEEOS:
			printf("%s Update: tee os  %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			#if 1
			if(is_secure == 1){
				#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
					#ifdef CONFIG_NVT_IVOT_OPTEE_SECBOOT_SUPPORT
					//// check signature
					if(nvt_chk_signature((ulong)p_mem->p_data, p_mem->len))
					{
						printf("check signature fail\n");
						return -1;

					}
					#endif
				#endif
			}
			#endif
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_AI:
			printf("%s Update: ai %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		case EMBTYPE_USER:
			printf("%s Update: user partition %u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_update_user_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize, &pEmb[id]);
			break;
		case EMBTYPE_USERRAW:
			printf("%s Update: user raw partition %u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_update_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize);
			break;
		default:
			printf("%s Update: skip partition %d, embtype=%d %s\r\n", ANSI_COLOR_YELLOW, id, pEmb[id].EmbType, ANSI_COLOR_RESET);
			break;
	}

	#else
		int ret = 0;
		printf("nvt_on_partition_enum_update, not support boot type\n");
	#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT || CONFIG_NVT_LINUX_SPINOR_BOOT || CONFIG_NVT_LINUX_EMMC_BOOT */
	return ret;
}

static int nvt_on_partition_enum_mtd_readback(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	int ret = 0;
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	u64 PartitionOffset = pEmb[id].PartitionOffset;
	u64 PartitionSize = pEmb[id].PartitionSize;
	#elif defined(CONFIG_NVT_IVOT_EMMC)
	u64 PartitionOffset = pEmb[id].PartitionOffset;
	u64 PartitionSize = pEmb[id].PartitionSize;
	#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
	u64 PartitionOffset = pEmb[id].PartitionOffset;
	u64 PartitionSize = pEmb[id].PartitionSize;
	#else
	u64 PartitionOffset = pEmb[id].PartitionOffset;
	u64 PartitionSize = pEmb[id].PartitionSize;
	#endif

	nvt_dbg(IND, "Read back check callback\n");
	switch(pEmb[id].EmbType)
	{
		case EMBTYPE_MBR:
			printf("%s Read: MBR %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_LOADER:
			printf("%s Read: loader %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_FDT:
			printf("%s Read: fdt %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_UITRON:
			printf("%s Read: uitron %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_ECOS:
			printf("%s Read: ecos %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_UBOOT:
			printf("%s Read: uboot %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_ATF:
			printf("%s Read: ATF %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_APP:
			printf("%s Read: App partition %u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_read_fs_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize, id, &pEmb[id]);
			break;
		case EMBTYPE_LINUX:
			printf("%s Read: linux %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_ROOTFS:
			printf("%s Read: rootfs%u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_read_fs_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize, id, &pEmb[id]);
			break;
		case EMBTYPE_ROOTFSL:
			printf("%s Read: rootfs%u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_read_rootfsl_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize, id, &pEmb[id]);
			break;
		case EMBTYPE_RTOS:
			printf("%s Read: rtos %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_TEEOS:
			printf("%s Read: tee os %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_AI:
			printf("%s Read: ai %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		case EMBTYPE_USER:
			printf("%s Read: user partition %u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			nvt_read_user_partition((ulong)p_mem->p_data, p_mem->len, PartitionOffset, PartitionSize, id, &pEmb[id]);
			break;
		case EMBTYPE_USERRAW:
			printf("%s Read: user raw partition %u %s\r\n", ANSI_COLOR_YELLOW, pEmb[id].OrderIdx, ANSI_COLOR_RESET);
			ret = nvt_read_partitions((ulong)p_mem->p_data, p_mem->len, PartitionOffset, id, pEmb[id].EmbType);
			break;
		default:
			printf("%s Read: skip partition %d, embtype=%d %s\r\n", ANSI_COLOR_YELLOW, id, pEmb[id].EmbType, ANSI_COLOR_RESET);
			break;
	}

	#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT || CONFIG_NVT_LINUX_SPINOR_BOOT || CONFIG_NVT_LINUX_EMMC_BOOT */
	return ret;
}

static int nvt_getfdt_emb_addr_size(ulong addr, u32 *fdt_emb_addr, u32 *fdt_emb_len)
{
	const char *ptr = NULL;
	const struct fdt_property* fdt_property = NULL;
	int  nodeoffset, nextoffset;

	#if defined(CONFIG_NVT_IVOT_EMMC)
	char path[128] = {0};
	sprintf(path, "/mmc@%x", IOADDR_SDIO3_REG_BASE);
	nodeoffset = fdt_path_offset((const void*)addr, path);
	#elif defined(CONFIG_NVT_SPI_NOR)
	nodeoffset = fdt_path_offset((const void*)addr, "/nor");
	#else
	nodeoffset = fdt_path_offset((const void*)addr, "/nand");
	#endif
	nextoffset = fdt_first_subnode((const void*)addr, nodeoffset);
	if (nextoffset < 0 || nodeoffset < 0) {
		*fdt_emb_addr = 0;
		return -1;
	}
	*fdt_emb_addr = addr + nextoffset;
	nodeoffset = nextoffset;
	while(strcmp(ptr, "nvtpack") != 0) {
		nextoffset = fdt_next_node((const void*)addr, nodeoffset, NULL);
		if (nextoffset < 0) {
			return -1;
		}
		ptr = fdt_get_name((const void*)addr, nextoffset, NULL);
		nodeoffset = nextoffset;
	}
	nextoffset = fdt_first_property_offset((const void*)addr, nodeoffset);
	nodeoffset = nextoffset;
	if (nextoffset < 0) {
		return -1;
	}
	while(nextoffset >= 0) {
		nextoffset = fdt_next_property_offset((const void*)addr, nodeoffset);
		fdt_property = fdt_get_property_by_offset((const void*)addr, nodeoffset, NULL);
		nodeoffset = nextoffset;
	}

	if (fdt_emb_len != NULL)
		*fdt_emb_len = (ulong)fdt_property->data + strlen(fdt_property->data) - *fdt_emb_addr;

	return 0;
}

static int nvt_checkfdt_part_is_match(const void *fdt_old, const void *fdt_new)
{
	int i, len;
	const void *fdt[2] = {fdt_old, fdt_new};
	const char *name[2] = {0};
	unsigned long long ofs[2] = {0};
	unsigned long long size[2] = {0};
	int  nodeoffset[2] = {0};
#if defined(CONFIG_NVT_IVOT_EMMC)
	char path[128] = {0};
#endif

	for (i = 0; i < 2; i++) {
		#if defined(CONFIG_NVT_IVOT_EMMC)
		sprintf(path, "/mmc@%x", IOADDR_SDIO3_REG_BASE);
		nodeoffset[i] = fdt_path_offset((const void*)fdt[i], path);
		#elif defined(CONFIG_NVT_SPI_NOR)
		nodeoffset[i] = fdt_path_offset((const void*)fdt[i], "/nor");
		#else
		nodeoffset[i] = fdt_path_offset((const void*)fdt[i], "/nand");
		#endif
		nodeoffset[i] = fdt_first_subnode((const void*)fdt[i], nodeoffset[i]);
		if (nodeoffset[i] < 0 || nodeoffset[i] < 0) {
			printf(ANSI_COLOR_RED "cannot find /mmc@f0510000 or /nor or /nand from fdt.\r\n");
			return -1;
		}
	}

	while (nodeoffset[0] >= 0 || nodeoffset[1] >= 0) {
		//get next nodes of partition_
		for (i = 0; i < 2; i++) {
			ofs[i] = 0;
			size[i] = 0;
			while(nodeoffset[i] >= 0) {
				unsigned long long *nodep;
				name[i] = fdt_get_name(fdt[i], nodeoffset[i], NULL);
				if (strncmp(name[i], "partition_", 10) == 0) {
					nodep = (unsigned long long *)fdt_getprop(fdt[i], nodeoffset[i], "reg", &len);
					if (nodep == NULL || len != sizeof(unsigned long long)*2) {
						printf(ANSI_COLOR_RED "unable to get 'reg' form %s\r\n", name[i]);
						return -1;
					}
					ofs[i] = be64_to_cpu(nodep[0]);
					size[i] = be64_to_cpu(nodep[1]);
					nodeoffset[i] = fdt_next_subnode(fdt[i], nodeoffset[i]);
					break;
				}
				nodeoffset[i] = fdt_next_subnode(fdt[i], nodeoffset[i]);
				name[i] = NULL; //clean name for indicate if partition_xxx has found
			}
		}
		if (name[0] == NULL && name[1] == NULL) {
			break;
		}
		if (strcmp(name[0], name[1]) != 0) {
			printf(ANSI_COLOR_RED "partition name not matched old:%s new:%s\r\n", name[0], name[1]);
			return -1;
		}
		if (ofs[0] != ofs[1]) {
			printf(ANSI_COLOR_RED "%s offset not matched old:%llx new:%llx\r\n", name[0], ofs[0], ofs[1]);
			return -1;
		}
		if (size[0] != size[1]) {
			printf(ANSI_COLOR_RED "%s size not matched old:%llx new:%llx\r\n", name[0], size[0], size[1]);
			return -1;
		}
	}
	return 0;
}

int nvt_process_all_in_one(ulong addr, unsigned int size, int firm_upd_firm)
{
	u32 i;
	int ret = 0;
	unsigned char md5_output_orig[16] = {0};
	unsigned char md5_output_cur[16] = {0};
	u32 need_chk_all_partition_exist = 0;
	NVTPACK_VERIFY_OUTPUT np_verify = {0};
	NVTPACK_GET_PARTITION_INPUT np_get_input;
	NVTPACK_ENUM_PARTITION_INPUT np_enum_input;
	NVTPACK_MEM mem_in = {(void*)addr, (unsigned int)size};
	NVTPACK_MEM mem_out = {0};

	memset(&np_get_input, 0, sizeof(np_get_input));
	memset(&np_enum_input, 0, sizeof(np_enum_input));

	if(nvtpack_verify(&mem_in, &np_verify) != NVTPACK_ER_SUCCESS)
	{
		printf("verify failed.\r\n");
		return -1;
	}
	if(np_verify.ver != NVTPACK_VER_16072017)
	{
		printf("wrong all-in-one bin version\r\n");
		return -1;
	}

	//check if modelext exists.
	EMB_PARTITION* pEmbNew = emb_partition_info_data_new;
	u32 emb_addr_new = 0;
	u32 emb_size_new = 0;
	np_get_input.id = 1; // modelext must always put in partition[1]
	np_get_input.mem = mem_in;
	if(nvtpack_get_partition(&np_get_input,&mem_out) == NVTPACK_ER_SUCCESS)
	{
		if(nvt_check_isfdt((ulong)mem_out.p_data) != 0) {
			printf("partition[1] is not fdt.\r\n");
			return -1;
		}

		/* extract new partition from new modelext */
		ret = nvt_getfdt_emb_addr_size((ulong)mem_out.p_data, &emb_addr_new, &emb_size_new);
		if(ret < 0) {
			printf("failed to get new partition address and size.\r\n");
			return -1;
		}

		ret = nvt_getfdt_emb((ulong)mem_out.p_data, pEmbNew);
		if(ret < 0) {
			printf("failed to get new partition table.\r\n");
			return -1;
		}
	}

	// get current partition in embbed storage
	EMB_PARTITION* pEmbCurr = emb_partition_info_data_curr;
	u32 emb_addr_curr = 0;
	u32 emb_size_curr = 0;
	if (nvt_chk_mtd_fdt_is_null()) {
		printf("%s modelext is empty %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
		need_chk_all_partition_exist = 1;
	} else if (nvt_check_isfdt((ulong)nvt_fdt_buffer) == 0) {
		//To do current modelext init.
		ret = nvt_fdt_init(true);
		if (ret < 0)
			printf("fdt init fail\n");

		ret = nvt_getfdt_emb_addr_size((ulong)nvt_fdt_buffer, &emb_addr_curr, &emb_size_curr);
		if(ret < 0) {
			printf("failed to get new partition resource.\r\n");
			return -1;
		}

		ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, pEmbCurr);
		if(ret < 0) {
			printf("failed to get current partition resource.\r\n");
			return -1;
		}
		// check if partition changes.
		if(emb_size_new != 0 && nvt_checkfdt_part_is_match(nvt_fdt_buffer, mem_out.p_data) != 0) {
			//must be fully all-in-one-bin
			printf("%s detected partition changed. %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			debug("%s fdt in mtd: 0x%08x@0x%08x fdt in all-in-one:0x%08x@0x%08x %s\r\n",
																		ANSI_COLOR_RED,
																		emb_size_curr,
																		emb_addr_curr,
																		emb_size_new,
																		emb_addr_new,
																		ANSI_COLOR_RESET);
			need_chk_all_partition_exist = 1;
		} else {
			printf("%s Partition is not changed %s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
		}
	} else {
		if (emb_size_new == 0) {
			//also pEmbCurr is NULL
			printf("%s FDT mtd partition doesn't exist in neither embedded nor all-in-one %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			return -1;
		} else {
			//the first burn, must be fully all-in-one-bin
			printf("%s detected partition changed. %s\r\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
			need_chk_all_partition_exist = 1;
		}
	}
	//read fdt from loader to get fdt info
	//need get some fdt information, like nvt_memory_cfg nsmem in nvt_on_partition_enum_sanity
	// when run the nvt_fdt_init(false); the nvt_fdt_buffer will become new fdt.
	ret = nvt_fdt_init(false);
	if(ret < 0)
	{
		printf("fdt init fail\n");
		return ret;
	}

	//check all-in-one firmware sanity
	EMB_PARTITION* pEmb = (emb_size_new != 0)? pEmbNew : pEmbCurr;
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	np_enum_input.fp_enum = nvt_on_partition_enum_sanity;
	if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS)
	{
		printf("failed sanity.\r\n");
		return -1;
	}

	if(need_chk_all_partition_exist)
	{
		for(i=1; i<EMB_PARTITION_INFO_COUNT; i++)
		{
			if(pEmbNew[i].PartitionSize!=0 && (pEmbNew[i].OrderIdx==0 || pEmbNew[i].EmbType==EMBTYPE_MBR))
			{
				switch(pEmbNew[i].EmbType)
				{
					case EMBTYPE_MBR:
					case EMBTYPE_FDT:
					case EMBTYPE_UITRON:
					case EMBTYPE_ECOS:
					case EMBTYPE_UBOOT:
					case EMBTYPE_ATF:
					//case EMBTYPE_DSP:
					case EMBTYPE_LINUX:
					case EMBTYPE_ROOTFS:
					case EMBTYPE_RTOS:
					case EMBTYPE_APP:
					case EMBTYPE_TEEOS:
					case EMBTYPE_AI:
					{
						//check this type exist in all-in-one
						np_get_input.id = i;
						np_get_input.mem = mem_in;
						if(nvtpack_check_partition(pEmbNew[i], &np_get_input,&mem_out) != NVTPACK_ER_SUCCESS)
						{
							printf("partition changed, need partition[%d]\n",i);
							return -1;
						}
					}break;
				}
			}
		}
	}

	/* partition table env init */
	ret = nvt_part_config(NULL, pEmb);
	if (ret < 0)
		return ret;

	/*
	 * start to update each partition of all-in-one
	 */

	//multi-bin
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	np_enum_input.fp_enum = nvt_on_partition_enum_update;

	if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS)
	{
		printf("failed to run nvt_update_partitions.\r\n");
		return -1;
	}
	/* To calculate original buffer the all-in-one image md5 sum */
	md5_wd((unsigned char *) addr, size, md5_output_orig, CHUNKSZ_MD5);

	/*
	 * Read back updated image from mtd to check if valid or not
	 */
	/*if (firm_upd_firm)
		nvt_disable_mem_protect();*/

	//multi-bin
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	np_enum_input.fp_enum = nvt_on_partition_enum_mtd_readback;
	if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
		printf("failed to run nvt_read_partitions.\r\n");
		return -1;
	}

	//check all-in-one firmware sanity
	np_enum_input.mem = mem_in;
	np_enum_input.p_user_data = pEmb;
	np_enum_input.fp_enum = nvt_on_partition_enum_sanity;

	if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS)
	{
		printf("failed sanity.\r\n");
#ifndef CONFIG_NVT_SPI_NONE
		ret = nvt_flash_mark_bad();
		if (ret)
			printf("flash mark bad process failed\n");
#endif
		return -1;
	} else {
		printf("%s Read back check sanity successfully. %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	}

	/* To calculate the all-in-one image md5 sum after read back */
	md5_wd((unsigned char *) addr, size, md5_output_cur, CHUNKSZ_MD5);
	if (memcmp(md5_output_orig, md5_output_cur, sizeof(md5_output_cur)) != 0) {
		printf("%s All-in-one image MD5 sum is not match %s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
#ifndef CONFIG_NVT_SPI_NONE
		ret = nvt_flash_mark_bad();
		if (ret)
			printf("flash mark bad process failed\n");
#endif
		return -1;
	} else {
		printf("%s All-in-one image MD5 sum is match %s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	}

	if(emb_size_new!=0)
	{
		int ret = 0;
		//reload new modelext but must keep loader information
		printf("Reload new modelext partition after updating\r\n");
		ret = nvt_fdt_init(true);
		if (ret < 0)
			printf("modelext init fail\n");
	}
	return 0;
}

/*
 * This function can be used to do rootfs format
 * Two conditions will do format:
 *				iTron cmd: key restore_rootfs
 *				Linux: uctrl usys -rootfs_broken => Send event to itron, itron can decide what to do
 */
int nvt_process_rootfs_format(void)
{
	EMB_PARTITION *p_emb_partition = NULL;
	int ret = 0;
	loff_t part_off=0, part_size=0;
	char command[128];
	unsigned long val = 0;

	memset(command, 0, sizeof(command));
	/* get partition table */
	ret = nvt_getfdt_emb((ulong)nvt_fdt_buffer, p_emb_partition);
	if (ret < 0) {
		printf("FDT: emb_partition setting is null\n");
		goto err_fmt_rootfs;
	}

	/* partition table env init */
	ret = nvt_part_config(NULL, p_emb_partition);
	if (ret < 0)
		goto err_fmt_rootfs;

	/* Check if rootfs1 is existed */
	ret = get_part("rootfs1", &part_off, &part_size);
	if (ret < 0)
		goto err_fmt_rootfs;

	printf("Starting to format R/W rootfs partition\n");
	sprintf(command, "nand erase.part rootfs1");
	run_command(command, 0);

#if defined(CONFIG_NVT_UBIFS_SUPPORT)
	sprintf(command, "ubi part rootfs1");
	run_command(command, 0);

	sprintf(command, "ubi create rootfs1");
	run_command(command, 0);
#endif

	val = (nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & ~COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK) | COMM_UBOOT_BOOT_FUNC_BOOT_DONE;
	nvt_writel(val, (ulong)nvt_shminfo_comm_uboot_boot_func);
	flush_dcache_all();
	while(1) {
		// Waiting for itron trigger reboot.
		printf(".");
		mdelay(1000);
	}

err_fmt_rootfs:
	val = (nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & ~COMM_UBOOT_BOOT_FUNC_BOOT_DONE_MASK) | COMM_UBOOT_BOOT_FUNC_BOOT_NG;
	nvt_writel(val, (ulong)nvt_shminfo_comm_uboot_boot_func);
	flush_dcache_all();
	return -1;
}

/*
 * This function can be used to do system recovery in emmc boot mode
 * Below condition will do system recovery:
 *				Loader send event
 */
int nvt_process_sys_recovery(void)
{
	int ret = 0;

#if defined(CONFIG_NVT_IVOT_EMMC_T)
	char dev_part_str[10];
	loff_t size = 0;
	u32 i;
	ulong addr = 0;
	MODELEXT_HEADER* p_resource_curr = NULL;
	EMB_PARTITION* pEmbCurr = NULL;
	NVTPACK_GET_PARTITION_INPUT np_get_input;

	memset(&np_get_input, 0, sizeof(np_get_input));

	printf("%sStarting to do EMMC boot recovery %s\r\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
	/* Switch to emmc bus partition 3 (cache partition) */

	sprintf(dev_part_str, "%d:3", CONFIG_NVT_IVOT_EMMC);
	if (fs_set_blk_dev("mmc", dev_part_str, FS_TYPE_FAT)) {
		return ERR_NVT_UPDATE_OPENFAILED;
	} else {
		if (!fs_exists(get_nvt_bin_name(NVT_BIN_NAME_TYPE_RECOVERY_FW))) {
			return ERR_NVT_UPDATE_NO_NEED;
		}
	}
	if (fs_set_blk_dev("mmc", dev_part_str, FS_TYPE_FAT))
		return ERR_NVT_UPDATE_OPENFAILED;
	else {
		ret = fs_read(get_nvt_bin_name(NVT_BIN_NAME_TYPE_RECOVERY_FW), (ulong)CONFIG_NVT_RUNFW_SDRAM_BASE, 0, 0, &size);
		if (size <= 0 || ret < 0) {
			printf("Read %s at 0x%x failed ret=%d\n", get_nvt_bin_name(NVT_BIN_NAME_TYPE_RECOVERY_FW), CONFIG_NVT_RUNFW_SDRAM_BASE, size);
			return ERR_NVT_UPDATE_READ_FAILED;
		} else {
			printf("Read %s at 0x%x successfully, size=%d\n", get_nvt_bin_name(NVT_BIN_NAME_TYPE_RECOVERY_FW), CONFIG_NVT_RUNFW_SDRAM_BASE, size);
			NVTPACK_MEM mem_in = {(void*)CONFIG_NVT_RUNFW_SDRAM_BASE, (unsigned int)size};
			NVTPACK_MEM mem_out = {0};
			pEmbCurr = (EMB_PARTITION*)modelext_get_cfg((unsigned char*)CONFIG_SMEM_SDRAM_BASE,MODELEXT_TYPE_EMB_PARTITION,&p_resource_curr);
			if(pEmbCurr==NULL) {
				printf("failed to get current partition resource.\r\n");
				return -1;
			}
			for(i=1; i<EMB_PARTITION_INFO_COUNT; i++)
			{
				if(pEmbCurr[i].PartitionSize!=0 && (pEmbCurr[i].OrderIdx==0 || pEmbCurr[i].EmbType==EMBTYPE_MBR || pEmbCurr[i].EmbType==EMBTYPE_DSP))
				{
					switch(pEmbCurr[i].EmbType)
					{
						case EMBTYPE_MBR:
						case EMBTYPE_FDT:
						case EMBTYPE_UITRON:
						case EMBTYPE_ATF:
						case EMBTYPE_UBOOT:
						#if defined(CONFIG_DSP1_FREERTOS) || defined(CONFIG_DSP2_FREERTOS)
						case EMBTYPE_DSP:
						#endif
						case EMBTYPE_LINUX:
						case EMBTYPE_ROOTFS:
						case EMBTYPE_RTOS:
						case EMBTYPE_TEEOS:
						case EMBTYPE_AI:
						{
							//check this type exist in all-in-one
							np_get_input.id = i;
							np_get_input.mem = mem_in;
							if(nvtpack_check_partition(pEmbCurr[i], &np_get_input,&mem_out) != NVTPACK_ER_SUCCESS)
							{
								printf("Recovery procedure: need partition[%d]\n",i);
								return -1;
							}
						}break;
					}
				}
			}
			ret = nvt_process_all_in_one((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE ,size, 0);
			if (ret < 0)
				return ERR_NVT_UPDATE_FAILED;
		}
	}
#endif

	return ret;
}

int nvt_fw_update(bool firm_upd_firm)
{
	int ret = 0, retry = 0;
	SHMINFO *p_shminfo;
	loff_t size = 0;
	char command[128];

	p_shminfo = (SHMINFO *)CONFIG_SMEM_SDRAM_BASE;
	if (strncmp(p_shminfo->boot.LdInfo_1, "LD_NVT", 6) != 0) {
		printf("can't get right bin info in %s \n", __func__);
		return ERR_NVT_UPDATE_FAILED;
	}

	if (firm_upd_firm) {
		unsigned long firm_addr = nvt_readl((ulong)nvt_shminfo_comm_fw_update_addr);
		unsigned long firm_size = nvt_readl((ulong)nvt_shminfo_comm_fw_update_len);

		printf("%s\tfirmware image at: 0x%08lx@0x%08lx %s\r\n", ANSI_COLOR_YELLOW, firm_size, firm_addr, ANSI_COLOR_RESET);
		ret = nvt_process_all_in_one(firm_addr, firm_size, firm_upd_firm);
		if (ret < 0)
			return ERR_NVT_UPDATE_FAILED;
	} else if (p_shminfo->boot.LdCtrl2 & LDCF_BOOT_FLASH) {
		return ERR_NVT_UPDATE_NO_NEED;
	} else {
		if (nvt_fs_set_blk_dev())
			return ERR_NVT_UPDATE_OPENFAILED;
		else {
			if (fs_size(get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW), &size) < 0) {
				return ERR_NVT_UPDATE_NO_NEED;
			}
		}

		while (retry < UPDATE_ALL_IN_ONE_RETRY_TIMES) {
		if (nvt_fs_set_blk_dev())
			return ERR_NVT_UPDATE_OPENFAILED;
		else {
			/* To check if A.bin is larger than all-in-one temp buffer size */
			if (size > CONFIG_NVT_ALL_IN_ONE_IMG_SIZE) {
				printf("%sYour image size %lld is larger than all-in-one size with %d %s\r\n", ANSI_COLOR_RED, size, CONFIG_NVT_ALL_IN_ONE_IMG_SIZE, ANSI_COLOR_RESET);
				return ERR_NVT_UPDATE_READ_FAILED;
			}

			ret = fs_read(get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW), (ulong)CONFIG_NVT_RUNFW_SDRAM_BASE, 0, 0, &size);
			if (size <= 0 || ret < 0) {
				printf("Read %s at 0x%x failed ret=%lld\n", get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW), CONFIG_NVT_RUNFW_SDRAM_BASE, size);
				return ERR_NVT_UPDATE_READ_FAILED;
			} else {
				printf("Read %s at 0x%x successfully, size=%lld\n", get_nvt_bin_name(NVT_BIN_NAME_TYPE_FW), CONFIG_NVT_RUNFW_SDRAM_BASE, size);
				ret = nvt_process_all_in_one((ulong)CONFIG_NVT_RUNFW_SDRAM_BASE ,size, 0);
				if (ret < 0) {
					retry++;
				} else {
					/* This flag is for the first boot after update A.bin finished. */
					p_shminfo->boot.LdCtrl2 &= ~LDCF_BOOT_CARD;
					break;
				}
			}
		}
		if (retry >= UPDATE_ALL_IN_ONE_RETRY_TIMES)
			return ERR_NVT_UPDATE_FAILED;
		}
	}

	return 0;
}
