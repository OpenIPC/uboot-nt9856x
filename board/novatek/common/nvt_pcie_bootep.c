/**
    NVT utilities for pice customization

    @file       nvt_pcie_bootep.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <linux/libfdt.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/IOAddress.h>
#include "nvt_ivot_pack.h"
#include <malloc.h>
#include <mapmem.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <fs.h>
#include <nand.h>
#include <mmc.h>
#include <spi_flash.h>


#define EP_LOADER_SIZE 0x10000
/* This buffer is a tmperoary buffer for put uboot + eploader */
#define TMP_BUFFER_SIZE 0x100000

/**************************************************************************
 *
 *
 * Novatek RC boot EP Flow
 *
 *
 **************************************************************************/

static int nvt_checkfdt_part_is_match(const void *fdt_old, const void *fdt_new)
{
	int i, len;
	const void *fdt[2] = {fdt_old, fdt_new};
	const char *name[2] = {0};
	unsigned long ofs[2] = {0};
	unsigned long size[2] = {0};
	int  nodeoffset[2] = {0};

	for (i = 0; i < 2; i++) {
		nodeoffset[i] = fdt_path_offset((const void*)fdt[i], "/nvt_memory_cfg");
		nodeoffset[i] = fdt_first_subnode((const void*)fdt[i], nodeoffset[i]);
		if (nodeoffset[i] < 0 || nodeoffset[i] < 0) {
			printf(ANSI_COLOR_RED "cannot find /nvt_memory_cfg.\r\n");
			return -1;
		}
	}

	while (nodeoffset[0] >= 0 && nodeoffset[1] >= 0) {
		for (i = 0; i < 2; i++) {
			ofs[i] = 0;
			size[i] = 0;
			while(nodeoffset[i] >= 0) {
				unsigned int *nodep;
				name[i] = fdt_get_name(fdt[i], nodeoffset[i], NULL);

				nodep = (unsigned int *)fdt_getprop(fdt[i], nodeoffset[i], "reg", &len);
				if (nodep == NULL || len != (sizeof(unsigned int)*2)) {
					printf(ANSI_COLOR_RED "unable to get 'reg' form %s len %d\r\n", name[i], len);
					return -1;
				}
				ofs[i] = be32_to_cpu(nodep[0]);
				size[i] = be32_to_cpu(nodep[1]);
				nodeoffset[i] = fdt_next_subnode(fdt[i], nodeoffset[i]);
				break;
			}
		}
		if (name[0] == NULL && name[1] == NULL) {
			break;
		}
		if (strcmp(name[0], name[1]) != 0) {
			printf(ANSI_COLOR_RED "/nvt_memory_cfg not matched old:%s new:%s\r\n", name[0], name[1]);
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

static bool nvt_runfw_bin_eprootfs_chk_exist(void *fdt_buf, unsigned int id, char * label_val)
{
	char root[255];
	int ret = 0;

#if defined(CONFIG_NVT_IVOT_EMMC)
	sprintf(root, "/mmc@%x/nvtpack/index", IOADDR_SDIO3_REG_BASE);
#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(root, "/nor@%x/nvtpack/index", IOADDR_NAND_REG_BASE);
#else
	sprintf(root, "/nand@%x/nvtpack/index", IOADDR_NAND_REG_BASE);
#endif
	ret = nvt_get_nvtpack_label_from_id(nvt_fdt_buffer, root, id, label_val);
	if (ret < 0) {
		return false;
	}
	if (strncmp(label_val, "ep-rootfs", 9) == 0) {
		return true;
	} else {
		return false;
	}
}


/** TODO
 * nvt_rc_boot_copy_image_to_ep - Get partition address, size from storage
 * @addr: a temporary buffer as the address after copying
 * @fdt_addr: a parition address to get from storage
 * @size: a partition size to get from storage
 * @label: a string used to switch difference flow
 *
 * If label is FDT or uboot, using NVT header to get real image size
 * If label is kernel or rootfs, using uboot header to get real image size
 *
 * Returns 0 on success, < 0 otherwise
 */
#if 1
static int nvt_rc_boot_copy_image_to_ep(unsigned long addr, unsigned long offset, unsigned long size)
{
	char command[128];
	unsigned long align_off;
	image_header_t *hdr;
	size = round_up(size, _EMBMEM_BLK_SIZE_);

#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT) || (defined(CONFIG_NVT_SPI_NAND) && defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT))
	sprintf(command, "nand read 0x%lx 0x%lx 0x%lx", addr, fdt_addr, size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT) || (defined(CONFIG_NVT_SPI_NOR) && defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT))
	size = ALIGN_CEIL(size, ARCH_DMA_MINALIGN);
	sprintf(command, "sf read 0x%lx 0x%lx 0x%lx", addr, fdt_addr, size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_IVOT_EMMC) && defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT))
	/* MMC read should use block number unit */
	size = ALIGN_CEIL(size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	align_off = ALIGN_CEIL(fdt_addr, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
	sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", addr, align_off, size);
#endif

	if (run_command(command, 0) != 0) {
		nvt_dbg(ERR, "command : \"%s\" fail", command);
		return -1;
	}
	return 0;
}
#endif

static int nvt_rc_boot_get_epfdt(unsigned long *rcfdt_addr, unsigned long *epfdt_addr, unsigned long *epfdt_size)
{
	int ret;
	ulong rc_size;

	/* 1. Verify RC FDT and Get Size firstly */
	if ((ret = fdt_check_header((void *)*rcfdt_addr) != 0)) {
		nvt_dbg(ERR, "invalid fdt header, addr=0x%08lx er = %d \n", (unsigned long)_BOARD_LINUXTMP_ADDR_, ret);
		return ret;
	}
	rc_size = (ulong)fdt_totalsize((void *)*rcfdt_addr);
	if (rc_size <= 0) {
		nvt_dbg(ERR, "invalid fdt size, addr=0x%08lx er = %d \n", (unsigned long)_BOARD_LINUXTMP_ADDR_, ret);
		return ret;
	}

	/* 2. Verify EP FDT (fdt address: RC FDT + RC FDT Size) and Get Size */
	*epfdt_addr = (unsigned long)((unsigned long)*rcfdt_addr + (unsigned long)rc_size);
	if ((ret = fdt_check_header((void *)*epfdt_addr) != 0)) {
		nvt_dbg(ERR, "invalid ep fdt header, addr=0x%08lx er = %d\n", epfdt_addr, ret);
		return ret;
	}

	*epfdt_size = (ulong)fdt_totalsize((void *)*epfdt_addr);
	if (*epfdt_size <= 0) {
		nvt_dbg(ERR, "invalid ep fdt size, addr=0x%08lx er = %d \n", (unsigned long)epfdt_addr, ret);
		return ret;
	}

	nvt_dbg(MSG, "epfdt addr : 0x%lx\n", *epfdt_addr);

	return ret;

}

static int nvt_rc_boot_eploader_copy(unsigned long *src_addr, unsigned size)
{
	int ret;
	HEADINFO *head_info;
	char cmd[255];
	unsigned long ep_addr;
	char *buffer = malloc(TMP_BUFFER_SIZE);
	if (buffer == NULL) {
		nvt_dbg(ERR, ": malloc fail\n");
		return -1;
	}

	/* 1. Copy to tmp buffer */
	memcpy((void *)buffer, src_addr, size);

	/* 2. Parsing NVT header to get real image size */
	head_info = (HEADINFO *)(buffer + 0x300);
	ep_addr = (unsigned long)buffer + head_info->BinLength;
	nvt_dbg(MSG, ": EP loader addr = 0x%lx\n", ep_addr);

	/* 3. Copy EP Loader to EP SRAM */
	sprintf(cmd, "nvt_pcie_copy_sram 0x%lx", ep_addr);
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		goto out;
	}
out:
	if (buffer) {
		free(buffer);
	}
	return ret;
}

static int nvt_rc_boot_ep_loader_process(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	unsigned long *src_addr = p_mem->p_data;
	unsigned long size = p_mem->len;
	int ret = 0;

	switch(pEmb[id].EmbType)
	{
		case EMBTYPE_UBOOT:
			ret = nvt_rc_boot_eploader_copy(src_addr, size);
			break;
		default:
			break;
	}
	return ret;
}


static int nvt_rc_boot_ep_partition_enum_copy(unsigned int id, NVTPACK_MEM* p_mem, void* p_user_data)
{
	EMB_PARTITION* pEmb = (EMB_PARTITION*)p_user_data;
	u64 PartitionOffset = pEmb[id].PartitionOffset;
	u64 PartitionSize = pEmb[id].PartitionSize;
	unsigned long dest_addr;
	unsigned long src_addr = (unsigned long)p_mem->p_data;
	unsigned long size = p_mem->len;
	unsigned long epfdt_addr;

	int ret = 0;
	char label[255];
	char cmd[255];

	switch(pEmb[id].EmbType)
	{
		case EMBTYPE_FDT:
			nvt_dbg(MSG, "Copy EP DTS : 0x%lx\n",src_addr);
			if (nvt_rc_boot_get_epfdt(&src_addr, &epfdt_addr, &size) < 0)
				return -1;
			/* RC and EP memtbl.dtsi nvt_memory_cfg node declarations should be matched */
			if (nvt_checkfdt_part_is_match((void*)src_addr, (void*)epfdt_addr) < 0)
				return -1;
			dest_addr = (unsigned long)_BOARD_FDT_ADDR_;
			src_addr = epfdt_addr;
			break;

		case EMBTYPE_ATF:
			nvt_dbg(MSG, "Copy EP ATF : \n");
			dest_addr = (unsigned long)_BOARD_ATF_ADDR_;
			break;
#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
		case EMBTYPE_TEEOS:
			nvt_dbg(MSG, "Copy EP TEEOS : \n");
			dest_addr = (unsigned long)_BOARD_TEEOS_ADDR_;
//			size = p_mem->len;
			break;
#endif
		case EMBTYPE_UBOOT:
			nvt_dbg(MSG, "Copy EP UBOOT : \n");
			dest_addr = (unsigned long)_BOARD_UBOOT_ADDR_;
			break;
		case EMBTYPE_LINUX:
			nvt_dbg(MSG, "Copy EP LINUX : \n");
			dest_addr = (unsigned long)CONFIG_LINUX_SDRAM_START;
			break;
		case EMBTYPE_ROOTFS:
			if (nvt_runfw_bin_eprootfs_chk_exist(nvt_fdt_buffer, id, label) == true) {
				nvt_dbg(MSG, "Copy EP-ROOTFS : \n");
				dest_addr = (unsigned long)(_BOARD_LINUX_ADDR_ + _BOARD_LINUX_SIZE_ - nvt_ramdisk_size - 0x10000);
				break;
			} else {
				return 0;
			}
		default:
			return 0;
	}

	/* Copy to EP DRAM */
	sprintf(cmd, "nvt_pcie_copy 0x%lx 0x%lx 0x%lx", src_addr, dest_addr, size);
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}
}

/**
 * nvt_rc_boot_rc_boot_flow - The main flow for RC boot EP
 * @tbin: a flag for T.bin
 *
 * Init PCIE and create mapping table
 * Set communication register to idle
 * Get EP loader and copy to EP DRAM
 * Boot up EP CPU
 * Wait EP DRAM config done
 * Get all partition and copy to EP DRAM
 * Release EP CPU
 *
 * Returns 0 on success, < 0 otherwise
 */
static int nvt_rc_boot_flow(bool tbin)
{
	int ret;
	int i;
	char cmd[255];
        EMB_PARTITION *pEmbCurr = emb_partition_info_data_curr;
	NVTPACK_ENUM_PARTITION_INPUT np_enum_input;

	/* Init PCIE and create mapping table */
	sprintf(cmd, "nvt_pcie_init");
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}

	/* Set communication register to idle */
	sprintf(cmd, "nvt_pcie_epstatus set idle");
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}

	/* 1. Get EP loader and copy to EP SRAM */
	memset(&np_enum_input, 0, sizeof(np_enum_input));
	np_enum_input.mem = (NVTPACK_MEM){(void*)CONFIG_NVT_RUNFW_SDRAM_BASE, 0};;
	np_enum_input.p_user_data = pEmbCurr;
	np_enum_input.fp_enum = nvt_rc_boot_ep_loader_process;
	if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
		printf("failed with rc copy ep loader process.\r\n");
		return -1;
	}

	/* 2. Boot up EP CPU */
	sprintf(cmd, "nvt_pcie_boot");
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}

	/* 3. Wait EP DRAM config done */
	i = 0;
	sprintf(cmd, "nvt_pcie_epstatus get");
	ret = run_command(cmd, 0);
	while ((ret != 1) && (i < 10)) {   // 0 = ep not boot, so we should copy ep loader and boot ep cpu
		nvt_dbg(MSG, "Wait EP DRAM config done ret = %d\n");
		udelay(50000);
		ret = run_command(cmd, 0);
		i++;
	}
	if (i >= 10) {
		nvt_dbg(ERR, "Wait EP DRAM config done Time out !!!");
		return -1;
	}

	/* 4. Copy all EP image to EP DRAM */

	memset(&np_enum_input, 0, sizeof(np_enum_input));
	np_enum_input.mem = (NVTPACK_MEM){(void*)CONFIG_NVT_RUNFW_SDRAM_BASE, 0};;
	np_enum_input.p_user_data = pEmbCurr;
	np_enum_input.fp_enum = nvt_rc_boot_ep_partition_enum_copy;
	if(nvtpack_enum_partition(&np_enum_input) != NVTPACK_ER_SUCCESS) {
		printf("failed with copy other images.\r\n");
		return -1;
	}

	/* 5. Issue cmd to EP CPU starting boot */
	sprintf(cmd, "nvt_pcie_epstatus set start 0x%lx", _BOARD_FDT_ADDR_);
	ret = run_command(cmd, 0);
	if (ret < 0) {
		nvt_dbg(ERR, "Run command : %s fail\n", cmd);
		return ret;
	}
	return 0;
}

/**************************************************************************
 *
 *
 * Novatek EP Boot Flow
 *
 *
 **************************************************************************/

static int nvt_ep_boot_get_ep_info(const void *fdt_addr, u32 *data)
{
	int  nodeoffset, nextoffset;
	const char *ptr = NULL;
	u32 *val = NULL;

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/ep_info");
	if (nodeoffset > 0) {
		val = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "enable", NULL);
		if (val != NULL) {
			*data = (unsigned int)fdt32_to_cpu(val[0]);
		}
	} else {
		nvt_dbg(ERR, "can not find /epinfo on device tree\n");
		return -1;
	}
	return 0;
}

static int nvt_ep_boot_get_linux_mem(const void *fdt_addr, u64 *addr, u64 *size)
{
	int  nodeoffset, nextoffset;
	const char *ptr = NULL;
	u64 *val = NULL;

	nodeoffset = fdt_path_offset((void *)fdt_addr, "/memory");
	if (nodeoffset > 0) {
		val = (u64 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "reg", NULL);
		if (val != NULL) {
			*addr = fdt64_to_cpu(val[0]);
			*size = fdt64_to_cpu(val[1]);
		} else {
			nvt_dbg(ERR, "can not find reg on device tree\n");
			return -1;
		}
	}
	return 0;
}

static int nvt_ep_boot_prepare_env_and_boot(u64 *linux_addr, u64 *linux_size)
{
	char buf[255], cmd[255];
	u64 kernel_size;
	unsigned long nvt_ep_boot_ramdisk_addr, nvt_ep_boot_ramdisk_size, size;
	image_header_t *hdr;

	/* Get Ramdisk size and address */
	nvt_ep_boot_ramdisk_addr = _BOARD_LINUX_ADDR_ + _BOARD_LINUX_SIZE_ - nvt_ep_boot_ramdisk_size - 0x10000;
	hdr = (image_header_t *)nvt_ep_boot_ramdisk_addr;
	size = image_get_data_size(hdr) + sizeof(image_header_t);
	nvt_ep_boot_ramdisk_size = ALIGN_CEIL(size, 4096);


	sprintf(buf, "0x%08x ", nvt_ep_boot_ramdisk_addr + nvt_ep_boot_ramdisk_size);
	env_set("initrd_high", buf);

	/* To assign relocated fdt address */
	sprintf(buf, "0x%08x ", *linux_addr + *linux_size);
	env_set("fdt_high", buf);

	/* The following will setup the lmb memory parameters for bootm cmd */
	sprintf(buf, "0x%08x ", *linux_addr + *linux_size);
	env_set("bootm_size", buf);
	env_set("bootm_mapsize", buf);

	sprintf(buf, "0x%08x ", *linux_addr);
	env_set("bootm_low", buf);
	env_set("kernel_comp_addr_r", buf);

	/* Get kernel size */
	hdr = (image_header_t *)CONFIG_LINUX_SDRAM_START;
	kernel_size = image_get_data_size(hdr);

	sprintf(buf, "0x%x", kernel_size);
	env_set("kernel_comp_size", buf);
	run_command("pri", 0);

	sprintf(cmd, "booti");

#ifdef CONFIG_NVT_BIN_CHKSUM_SUPPORT
	sprintf(cmd, "%s %lx %lx %lx", cmd, CONFIG_LINUX_SDRAM_START + sizeof(image_header_t), nvt_ep_boot_ramdisk_addr + 64, (unsigned long)nvt_fdt_buffer);
#else
	sprintf(cmd, "%s %lx %lx %lx", cmd, CONFIG_LINUX_SDRAM_START + sizeof(image_header_t), nvt_ep_boot_ramdisk_addr, (unsigned long)nvt_fdt_buffer);
#endif
	printf("%s\n", cmd);
	run_command(cmd, 0);

	return 0;
}

static int nvt_ep_boot_flow()
{
	int ret;
	u64 linux_addr, linux_size;
	u32 data;

	/* Verify EP device tree */
	ret = nvt_ep_boot_get_ep_info(nvt_fdt_buffer, &data);
	if (ret < 0) {
		return ret;
	}

	if (data == 0) {
		nvt_dbg(ERR, "!!! Stop Boot EP !!!\n");
		while (1);
	} else {
		/* normal boot ep */
	}

	/* Get linux memory address and size from device tree*/
	ret = nvt_ep_boot_get_linux_mem(nvt_fdt_buffer, &linux_addr, &linux_size);
	if (ret != 0) {
		return ret;
	}

	/* Prepare env for boot to linux and boot */
	ret = nvt_ep_boot_prepare_env_and_boot(&linux_addr, &linux_size);

	return ret;
}

/**************************************************************************
 *
 *
 * Novatek RC Boot EP Entry
 *
 *
 **************************************************************************/

int do_nvt_pcie(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	ret = run_command("nvt_pcie_getid", 0);
	if (ret == 0) {
		ret = nvt_ep_boot_flow();
	} else {
		if (nvt_detect_fw_tbin()) {
			ret = nvt_rc_boot_flow(true);
		} else {
			ret = nvt_rc_boot_flow(false);
		}
	}
	return ret;
}

U_BOOT_CMD(
	nvt_pcie, 1,    1,  do_nvt_pcie,
	"nvt RC boot EP",
	""
);
