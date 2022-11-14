/**
    NVT update handling api

    @file       nvt_ivot_fw_update_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <jffs2/jffs2.h>
#include <mmc.h>
#include <nand.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <linux/ctype.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/IOAddress.h>
#include "nvt_ivot_pack.h"
#include "nvt_ivot_soc_utils.h"

unsigned long nvt_ramdisk_addr = 0;
unsigned long nvt_ramdisk_size = 0;
unsigned long nvt_ker_img_addr = 0;
unsigned long nvt_ker_img_size = 0;

char *nvt_bin_name = NULL;
char *nvt_bin_name_t = NULL;
char *nvt_bin_name_r = NULL;
char *nvt_bin_name_fdt = NULL;

EMB_PARTITION_FDT_TRANSLATE_TABLE emb_part_fdt_map[EMBTYPE_TOTAL_SIZE] = {
	{"partition_mbr", EMBTYPE_MBR},
	{"partition_loader", EMBTYPE_LOADER},
	{"partition_fdt", EMBTYPE_FDT},
	{"partition_fdt.app", EMBTYPE_FDT},
	{"partition_teeos", EMBTYPE_TEEOS},
	{"partition_uboot", EMBTYPE_UBOOT},
	{"partition_uenv", EMBTYPE_UENV},
	{"partition_kernel", EMBTYPE_LINUX},
	{"partition_rootfs", EMBTYPE_ROOTFS},
	{"partition_rootfsl", EMBTYPE_ROOTFSL},
	{"partition_usr", EMBTYPE_USER},
	{"partition_usrraw", EMBTYPE_USERRAW},
	{"partition_rtos", EMBTYPE_RTOS},
	{"partition_app", EMBTYPE_APP},
	{"partition_ai", EMBTYPE_AI},
	{"partition_atf", EMBTYPE_ATF},
};

EMB_PARTITION emb_partition_info_data_curr[EMB_PARTITION_INFO_COUNT];
EMB_PARTITION emb_partition_info_data_new[EMB_PARTITION_INFO_COUNT];

int nvt_chk_mtd_fdt_is_null(void)
{
	int ret;
	char command[128] = {0};

	void *tmp_model_ptr = memalign(ARCH_DMA_MINALIGN, CONFIG_FDT_SDRAM_SIZE);
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
		sprintf(command, "nand read %x %x %x", (unsigned long)tmp_model_ptr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_FDT_SDRAM_SIZE);
	#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
		sprintf(command, "mmc read 0x%x 0x%x 0x%x", (unsigned long)tmp_model_ptr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_FDT_SDRAM_SIZE/MMC_MAX_BLOCK_LEN);
	#else
		#if defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT)
			#if defined(CONFIG_NVT_SPI_NAND)
				sprintf(command, "nand read %x %x %x", (unsigned long)tmp_model_ptr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_FDT_SDRAM_SIZE);

			#elif defined(CONFIG_NVT_SPI_NOR)
				sprintf(command, "sf read 0x%x 0x%x 0x%x", (unsigned long)tmp_model_ptr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_FDT_SDRAM_SIZE);

			#elif defined(CONFIG_NVT_IVOT_EMMC)
				sprintf(command, "mmc read 0x%x 0x%x 0x%x", (unsigned long)tmp_model_ptr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_FDT_SDRAM_SIZE/MMC_MAX_BLOCK_LEN);
			#else
				printf("nvt_chk_mtd_fdt_is_null, no flash or emmc for ramdisk boot\n");
			#endif
		#else
			sprintf(command, "sf read 0x%x 0x%x 0x%x", (unsigned long)tmp_model_ptr, CONFIG_MODELEXT_FLASH_BASE, CONFIG_FDT_SDRAM_SIZE);

		#endif
	#endif
	printf("nvt_chk_mtd_fdt_is_null cmd:%s\n",command);
	ret = run_command(command, 0);
	if (ret < 0)
		return ret;

	ret = nvt_check_isfdt((unsigned long)tmp_model_ptr);
	free(tmp_model_ptr);
	if(ret != 0) {
		return 1;
	} else {
		return 0;
	}
}

int nvt_fw_load_tbin(void)
{
	loff_t size = 0;
	int ret = 0;

	if (nvt_fs_set_blk_dev())
		return ERR_NVT_UPDATE_OPENFAILED;
	else {
		if (!fs_exists(get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW))) {
			return ERR_NVT_UPDATE_NO_NEED;
		}
	}

	if (nvt_fs_set_blk_dev())
		return ERR_NVT_UPDATE_OPENFAILED;
	else {
		ret = fs_read(get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), (ulong)CONFIG_NVT_RUNFW_SDRAM_BASE, 0, 0, &size);
		if (size <= 0 || ret < 0) {
			nvt_dbg(ERR, "Read %s at 0x%x failed ret=%lld\n", get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), CONFIG_NVT_RUNFW_SDRAM_BASE, size);
			return ERR_NVT_UPDATE_READ_FAILED;
		} else {
			nvt_dbg(IND, "Read %s at 0x%x successfully, size=%lld\n", get_nvt_bin_name(NVT_BIN_NAME_TYPE_RUNFW), CONFIG_NVT_RUNFW_SDRAM_BASE, size);
		}
	}

	return 0;
}

int nvt_detect_fw_tbin(void)
{
	SHMINFO *p_shminfo;

	p_shminfo = (SHMINFO *)CONFIG_SMEM_SDRAM_BASE;
	if (strncmp(p_shminfo->boot.LdInfo_1, "LD_NVT", 6) != 0) {
		return -1;
	} else {
		if (p_shminfo->boot.LdCtrl2 & LDCF_BOOT_CARD) {
			/* T.bin */
			nvt_dbg(IND, "Boot from SD card\n");
			return 1;
		} else {
			/* A.bin */
			nvt_dbg(IND, "Boot from flash or emmc\n");
			return 0;
		}
	}
}


/*************************************************************************
* _LZ_ReadVarSize() - Read unsigned integer with variable number of
* bytes depending on value.
*************************************************************************/

static unsigned int _lz_read_var_size(unsigned int *x, unsigned char *buf)
{
	unsigned int y, b, num_bytes;

	/* Read complete value (stop when byte contains zero in 8:th bit) */
	y = 0;
	num_bytes = 0;
	do
	{
		b = (unsigned int) (*buf ++);
		y = (y << 7) | (b & 0x0000007f);
		++ num_bytes;
	}
	while( b & 0x00000080 );

	/* Store value in x */
	*x = y;

	/* Return number of bytes read */
	return num_bytes;
}

/**
	LZ Uncompress

	Uncompress/decompress by LZ77 decoder

	@param[in] in       Input buffer of compressed data
	@param[out] out     Output buffer to store decompressed data (Caller should maintain large enough buffer size to hold decompressed data)
	@param[in] insize   Length of input buffer in

	@return void
*/
void lz_uncompress(unsigned char *in, unsigned char *out, unsigned int insize)
{
	unsigned char marker, symbol;
	unsigned int  i, inpos, outpos, length, offset;

	/* Do we have anything to uncompress? */
	if( insize < 1 )
	{
		return;
	}

	/* Get marker symbol from input stream */
	marker = in[ 0 ];
	inpos = 1;

	/* Main decompression loop */
	outpos = 0;
	do
	{
		symbol = in[ inpos ++ ];
		if( symbol == marker )
		{
			/* We had a marker byte */
			if( in[ inpos ] == 0 )
			{
				/* It was a single occurrence of the marker byte */
				out[ outpos ++ ] = marker;
				++ inpos;
			}
			else
			{
				/* Extract true length and offset */
				inpos += _lz_read_var_size( &length, &in[ inpos ] );
				inpos += _lz_read_var_size( &offset, &in[ inpos ] );

				/* Copy corresponding data from history window */
				for( i = 0; i < length; ++ i )
				{
					out[ outpos ] = out[ outpos - offset ];
					++ outpos;
				}
			}
		}
		else
		{
			/* No marker, plain copy */
			out[ outpos ++ ] = symbol;
		}
	}
	while( inpos < insize );
}

int get_part(const char *partname, loff_t *off, loff_t *maxsize)
{
#if defined(CONFIG_NVT_LINUX_EMMC_BOOT) || (defined(CONFIG_NVT_LINUX_RAMDISK_SUPPORT) && defined(CONFIG_NVT_IVOT_EMMC))
	char cmdline[1024] = {0}, nvtemmcpart[32] = {0};
	char *nvtemmcpart_off = NULL, *nvtemmcpart_off_next = NULL, *nvtemmcpart_off_end = NULL, *tmp = NULL;
	u32 nvtemmcpart_sz = 0;

	sprintf(cmdline,"%s ",env_get("bootargs"));
	nvtemmcpart_off = strstr((char *)cmdline, "nvtemmcpart=") + strlen("nvtemmcpart=") - 1;
	nvtemmcpart_off_end = strstr((char *)nvtemmcpart_off, " ");
	nvtemmcpart_off_next = strstr((char *)nvtemmcpart_off, ",");
	*maxsize = 0;
	*off = 0;

	if (nvtemmcpart_off == NULL || ((unsigned long)nvtemmcpart_off_end - (unsigned long)nvtemmcpart_off) < 20)
		return -1;
	do
	{
		memset(nvtemmcpart, 0, sizeof(nvtemmcpart));
		nvtemmcpart_sz = (u32)(nvtemmcpart_off_next - nvtemmcpart_off - 1);
		strncpy(nvtemmcpart, nvtemmcpart_off+1, nvtemmcpart_sz);
		nvtemmcpart_off = nvtemmcpart_off_next;
		nvtemmcpart_off_next = strstr((char *)nvtemmcpart_off_next+1, ",");

		if (strstr(nvtemmcpart, partname) != NULL) {
			*maxsize = simple_strtoul(nvtemmcpart, &tmp, 0);
			*off = simple_strtoul((tmp + 1), NULL, 0);
			break;
		}
	} while((unsigned long)nvtemmcpart_off_next < (unsigned long)nvtemmcpart_off_end && (unsigned long)nvtemmcpart_off_next != 0);

	return 0;
#elif defined(CONFIG_CMD_MTDPARTS)
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
		return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part);
	if (ret)
		return ret;

	*off = part->offset;
	*maxsize = part->size;

	return 0;
#else
	puts("offset is not a number\n");
	return -1;
#endif
}

/*
 * get from bin info.
 * select:
 *		0: FW_NAME
 *		1: RUNFW_NAME
 *		2: MODELEXT_NAME
 */
char *get_nvt_bin_name(NVT_BIN_NAME_TYPE type)
{
	int  nodeoffset, len = 0;
	const void *nodep;      /* property node pointer */

	nodeoffset = fdt_path_offset((const void*)nvt_fdt_buffer, "/nvt_info");

	if (nodeoffset < 0) {
		printf("failed to fdt_path_offset() for /nvt_info, er = %d\n", nodeoffset);
		return NULL;
	}

	nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "BIN_NAME", &len);

	if (nvt_bin_name == NULL) {
		nvt_bin_name = calloc(sizeof(char),15);
		if (!nvt_bin_name) {
			printf( "%s: allocation failure \n", __FUNCTION__);
			return NULL;
		}
		strcpy(nvt_bin_name, nodep);
		strcat(nvt_bin_name, ".bin");
	}

	if ( nvt_bin_name_t == NULL) {
		nvt_bin_name_t = calloc(sizeof(char),15);
		if (!nvt_bin_name_t) {
			printf( "%s: allocation failure \n", __FUNCTION__);
			free(nvt_bin_name);
			return NULL;
		}
		strcpy(nvt_bin_name_t, nodep);
		nvt_bin_name_t[strlen(nvt_bin_name_t) - 1] = '\0';
		strcat(nvt_bin_name_t, "T.bin");
	}

	if ( nvt_bin_name_r == NULL) {
		nvt_bin_name_r = calloc(sizeof(char),15);
		if (!nvt_bin_name_r) {
			printf( "%s: allocation failure \n", __FUNCTION__);
			free(nvt_bin_name);
			free(nvt_bin_name_t);
			return NULL;
		}
		strcpy(nvt_bin_name_r, nodep);
		nvt_bin_name_r[strlen(nvt_bin_name_r) - 1] = '\0';
		strcat(nvt_bin_name_r, "R.bin");
	}

	if ( nvt_bin_name_fdt == NULL) {
		nvt_bin_name_fdt = calloc(sizeof(char),15);
		if (!nvt_bin_name_fdt) {
			printf( "%s: allocation failure \n", __FUNCTION__);
			free(nvt_bin_name);
			free(nvt_bin_name_t);
			free(nvt_bin_name_r);
			return NULL;
		}
		strcpy(nvt_bin_name_fdt, nodep);
		strcat(nvt_bin_name_fdt, ".fdt.bin");
	}

	if (type == NVT_BIN_NAME_TYPE_FW) {
		return nvt_bin_name;
	} else if (type == NVT_BIN_NAME_TYPE_RUNFW) {
		return nvt_bin_name_t;
	} else if (type == NVT_BIN_NAME_TYPE_RECOVERY_FW) {
		return nvt_bin_name_r;
	} else {
		return nvt_bin_name_fdt;
	}
}

int nvt_check_is_fw_update_fw(void)
{
	unsigned long      boot_reason;

	boot_reason = nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & COMM_UBOOT_BOOT_FUNC_BOOT_REASON_MASK;

	return (boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_UPDFIRM);
}

int nvt_check_is_fomat_rootfs(void)
{
	unsigned long      boot_reason;
	static int         is_format = -1;

	if(is_format != -1)
		return is_format;

	boot_reason = nvt_readl((ulong)nvt_shminfo_comm_uboot_boot_func) & COMM_UBOOT_BOOT_FUNC_BOOT_REASON_MASK;
	is_format = ((boot_reason == COMM_UBOOT_BOOT_FUNC_BOOT_FORMAT_ROOTFS) ? 1 : 0);

	return is_format;
}

int nvt_fs_set_blk_dev(void)
{
	#if 0
	if (fs_set_blk_dev("mmc", "0:1", FS_TYPE_FAT))
		if (fs_set_blk_dev("mmc", "0:1", FS_TYPE_EXFAT))
			if (fs_set_blk_dev("mmc", "0:0", FS_TYPE_FAT))
				if (fs_set_blk_dev("mmc", "0:0", FS_TYPE_EXFAT)){
			                printf("MMC interface configure failed\n");
			                return -1;
				}
	#else
	if (fs_set_blk_dev("mmc", "0:1", FS_TYPE_FAT))
			if (fs_set_blk_dev("mmc", "0:0", FS_TYPE_FAT)) {
			                printf("MMC interface configure failed\n");
			                return -1;
				}
	#endif

	return 0;
}

#ifndef CONFIG_NVT_SPI_NONE
int nvt_flash_mark_bad(void)
{
	#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	int i = 0, ret = 0;
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);

	for (i = 0; i < 100; i++) {
		if (nvt_flash_ecc_err_pages[i] != 0) {
			ret = mtd_block_markbad(mtd, (loff_t)nvt_flash_ecc_err_pages[i]);
			if (ret) {
				printf("cannot mark nand flash block %d as bad, error %d", nvt_flash_ecc_err_pages[i], ret);
				return ret;
			}
		}
	}
	#endif
	return 0;
}
#endif

int nvt_enum_fdt_props(ulong fdt_addr, char *part_node_name, void *pdata,
					int (*enum_funcs)(ulong fdt_addr, const char *node_name, const char *prop_name, char *prop_val, void *pdata))
{
	int nodeoffset = 0, nextsubnodeoffset = 0, nextnodeoffset = 0, ret = 0, len = 0, part_num = 0;
	const char *list_node_name = NULL;
	char path[100];

	nodeoffset = fdt_path_offset((const void*)fdt_addr, part_node_name);
	if (nodeoffset < 0)	{
		return -1;
	}

	/* To check if the next subnode is existed */
	nextsubnodeoffset = fdt_first_subnode((const void*)fdt_addr, nodeoffset);
	if (nextsubnodeoffset < 0) {
		goto loop_prop_parse;
	}
	for (nextnodeoffset = nextsubnodeoffset;
		(nextnodeoffset > 0);
		(nextnodeoffset = fdt_next_node((const void*)fdt_addr, nextnodeoffset, &len))) {
		nodeoffset = nextnodeoffset;
		list_node_name = fdt_get_name((const void*)fdt_addr, nextnodeoffset, NULL);
		if (list_node_name == NULL) {
			return 0;
		}

		/* get node path to decide if this loop is end */
		fdt_get_path((const void*)fdt_addr, nodeoffset, path, sizeof(path));
		if (strncmp(part_node_name, path, strlen(part_node_name)) != 0) {
			if (part_num > EMB_PARTITION_INFO_COUNT) {
				printf("%s Your partition table total number is more than %d %s\r\n", ANSI_COLOR_RED, EMB_PARTITION_INFO_COUNT, ANSI_COLOR_RESET);
			}
			return 0;
		}
loop_prop_parse:
		for (nodeoffset = fdt_first_property_offset((const void*)fdt_addr, nodeoffset);
			(nodeoffset >= 0);
			(nodeoffset = fdt_next_property_offset((const void*)fdt_addr, nodeoffset))) {
			const struct fdt_property *prop;

			if (!(prop = fdt_get_property_by_offset((const void*)fdt_addr, nodeoffset, &len))) {
				break;
			}

			const char *p_id_name = fdt_string((const void*)fdt_addr, fdt32_to_cpu(prop->nameoff));
			debug("%s: node_name=%s prop_name=%s prop_data=%s\n", __func__, list_node_name, p_id_name, (char*)&prop->data);
			ret = (*enum_funcs)((ulong)fdt_addr, list_node_name, p_id_name, (char*)&prop->data, pdata);
			if (ret < 0) {
				return -1;
			}
		}
		part_num ++;
	}
	return 0;
}

int nvt_fdt_embpart_lookup(char *part_node_name, unsigned short *embtype)
{
	int i = 0, j = 0, ret = -1;
	size_t len = 0, len_tbl = 0;

	len = strlen(part_node_name);
	while (!isdigit(part_node_name[i]) && i < len) {
		i++;
	}

	len = (size_t)i;
	*embtype = 0;
	for (i = 0; i < EMBTYPE_TOTAL_SIZE; i++) {
		len_tbl = strlen(emb_part_fdt_map[i].fdt_node_name);
		if (strncmp(emb_part_fdt_map[i].fdt_node_name, part_node_name, len) == 0 && len_tbl == len) {
			*embtype = emb_part_fdt_map[i].emb_type;
			ret = len;
			break;
		} else {
			*embtype = EMBTYPE_UNKNOWN;
		}
	}

	return ret;
}

static int nvt_getfdt_emb_part_info(ulong fdt_addr, char *part_node_name, EMB_PARTITION *pemb, unsigned int part_id)
{
	int  nodeoffset, nextoffset, ret;
	const char *ptr = NULL;
	char *endptr = NULL;
	u64 addr, size;
	//const unsigned long long *val = NULL;
	const fdt64_t *val;
	unsigned int idx = 0;

	#if defined(CONFIG_NVT_IVOT_EMMC)
	char path[128] = {0};
	sprintf(path, "/mmc@%x", IOADDR_SDIO3_REG_BASE);
	nodeoffset = fdt_path_offset((void*)fdt_addr, path);
	#elif defined(CONFIG_NVT_SPI_NOR)
	nodeoffset = fdt_path_offset((void*)fdt_addr, "/nor");
	#else
	nodeoffset = fdt_path_offset((void*)fdt_addr, "/nand");
	#endif
	/* Got partition_loader node */
	nextoffset = fdt_first_subnode((void*)fdt_addr, nodeoffset);
	if (nextoffset < 0) {
		return -1;
	}

	nodeoffset = nextoffset;
	ptr = fdt_get_name((const void*)fdt_addr, nextoffset, NULL);
	while(strcmp(ptr, part_node_name) != 0) {
		nextoffset = fdt_next_node((void*)fdt_addr, nodeoffset, NULL);
		if (nextoffset < 0)
			return -1;
		ptr = fdt_get_name((const void*)fdt_addr, nextoffset, NULL);
		nodeoffset = nextoffset;
	}

	val = fdt_getprop((const void*)fdt_addr, nodeoffset, "reg", NULL);
	if (!val)
	    printf("fdt_getprop fail\n");
	addr = be64_to_cpu(val[0]);
	size = be64_to_cpu(val[1]);
	debug("addr = 0x%x, size = 0x%x\n", addr, size);
	ret = nvt_fdt_embpart_lookup(part_node_name, &pemb[part_id].EmbType);
	if (ret < 0) {
		nvt_dbg(ERR, "Skip this partition node: %s \n", part_node_name);
		return 0;
	} else {
		if (!isdigit(part_node_name[(u32)ret])) {
			idx = 0;
		} else {
			idx = simple_strtoul(&part_node_name[(u32)ret], &endptr, 10);
			if (*endptr != '\0') {
				printf("%s: get partition table index error with %s\n", __func__, part_node_name);
				return -1;
			}
		}
		pemb[part_id].OrderIdx = idx;
		pemb[part_id].PartitionOffset = addr;
		pemb[part_id].PartitionSize = size;
	}
	debug("EmbType=%u PartitionOffset=%llx PartitionSize=%llx Node_name=%s idx=%u\n",
											pemb[part_id].EmbType,
											pemb[part_id].PartitionOffset,
											pemb[part_id].PartitionSize,
											part_node_name,
											idx);
	return 0;
}

/* check filename if exist */
static int nvt_check_filename(char *part_node_name)
{
	int i = 0, ret = 0;
	size_t len = 0;

	len = strlen(part_node_name);
	if(len > 255) {
		printf("file name len %d error\n", len);
		return ret;
	}
	while (!isspace(part_node_name[i]) && len && i < len) {
		i++;
	}
	if (i)
	    ret = 1;

	return ret;
}

static int nvt_fdt_enum_fill_emb_parts(ulong fdt_addr, const char *node_name, const char *prop, char *prop_val, void *pdata)
{
	int ret = 0;
	char part_name[100], *endptr = NULL;
	unsigned int part_id = 0;
	char prop_partname[] = "partition_name";
	EMB_PARTITION *pemb = (EMB_PARTITION *)pdata;

	if (strncmp(node_name, "id", 2) != 0) {
		printf("nvtpack-index naming error: %s\n", prop);
		return -1;
	}

	part_id = simple_strtoul(&node_name[2], &endptr, 10);
	if (*endptr != '\0') {
		printf("nvtpack-index string transfer error: %s\n", prop);
		return -1;
	}

	/* Only check partition_name prop */
	if (strncmp(prop, prop_partname, strlen(prop_partname)) != 0) {
		pemb[part_id].FileExist = nvt_check_filename((char*)prop_val);
		debug("%s part_id: %d File Exist: %d %s \n", ANSI_COLOR_YELLOW, part_id, pemb[part_id].FileExist, ANSI_COLOR_RESET);

		return 0;
	}

	ret = sprintf(part_name, "partition_%s", prop_val);
	if (ret < 0)
		return -1;

	#if defined(CONFIG_NVT_IVOT_EMMC)
	debug("%s Got from /emmc/nvtpack/index node name: %s prop name: %s, prop value: %s %s \n", ANSI_COLOR_YELLOW, node_name, part_name, prop_val, ANSI_COLOR_RESET);
	#else
	debug("%s Got from /nand/nvtpack/index node name: %s prop name: %s, prop value: %s %s \n", ANSI_COLOR_YELLOW, node_name, part_name, prop_val, ANSI_COLOR_RESET);
	#endif
	ret = nvt_getfdt_emb_part_info(fdt_addr, part_name, pemb, part_id);
	if (ret < 0) {
		printf("%s get emb part info from nvtpack node is failed: %s %u %s\n", ANSI_COLOR_RED, part_name, part_id, ANSI_COLOR_RESET);
		return -1;
	}

	return 0;
}

int nvt_getfdt_emb(ulong fdt_addr, EMB_PARTITION *pemb)
{
	int ret = 0;
	char path[128] = {0};

	#if defined(CONFIG_NVT_IVOT_EMMC)
	sprintf(path, "/mmc@%x/nvtpack/index", IOADDR_SDIO3_REG_BASE);
	ret = nvt_enum_fdt_props(fdt_addr, path, pemb, nvt_fdt_enum_fill_emb_parts);
	#elif defined(CONFIG_NVT_SPI_NOR)
	sprintf(path, "/nor@%x/nvtpack/index", IOADDR_NAND_REG_BASE);
	ret = nvt_enum_fdt_props(fdt_addr, path, pemb, nvt_fdt_enum_fill_emb_parts);
	#else
	sprintf(path, "/nand@%x/nvtpack/index", IOADDR_NAND_REG_BASE);
	ret = nvt_enum_fdt_props(fdt_addr, path, pemb, nvt_fdt_enum_fill_emb_parts);
	#endif
	if (ret < 0)
		return -1;

	return 0;
}

int nvt_getfdt_rootfs_mtd_num(ulong fdt_addr, unsigned int *mtd_num, uint32_t *ro_attr)
{
	int  nodeoffset, nextoffset;
	unsigned int i = 0;
	const char *ptr = NULL;
	int len = 0;
	const void * prop_val = NULL;

	#if defined(CONFIG_NVT_IVOT_EMMC)
	char path[128] = {0};
	sprintf(path, "/mmc@%x", IOADDR_SDIO3_REG_BASE);
	nodeoffset = fdt_path_offset((void*)fdt_addr, path);
	#elif defined(CONFIG_NVT_SPI_NOR)
	nodeoffset = fdt_path_offset((void*)fdt_addr, "/nor");
	#else
	nodeoffset = fdt_path_offset((void*)fdt_addr, "/nand");
	#endif
	/* Got partition_loader node */
	nextoffset = fdt_first_subnode((void*)fdt_addr, nodeoffset);
	if (nextoffset < 0) {
		return -1;
	}

	nodeoffset = nextoffset;
	ptr = fdt_get_name((const void*)fdt_addr, nextoffset, NULL);
	#if defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	while(strcmp(prop_val, "true") != 0) {
		nextoffset = fdt_next_node((void*)fdt_addr, nodeoffset, NULL);
		if (nextoffset < 0)
			return -1;
		prop_val = fdt_getprop((const void*)fdt_addr, nextoffset, "active", &len);
		ptr = fdt_get_name((const void*)fdt_addr, nextoffset, NULL);
		nodeoffset = nextoffset;
		if (strncmp(ptr, "partition_rootfs", 16) == 0)
			i++;
		if (strncmp(ptr, "partition_", 10) != 0)
			break;
	}
	#else
	while(strcmp(ptr, "partition_rootfs") != 0) {
		nextoffset = fdt_next_node((void*)fdt_addr, nodeoffset, NULL);
		if (nextoffset < 0)
			return -1;
		ptr = fdt_get_name((const void*)fdt_addr, nextoffset, NULL);
		nodeoffset = nextoffset;
		i++;
	}

	/* Check if we have two rootfs partitions. (ro + rw) */
	nextoffset = fdt_next_node((void*)fdt_addr, nodeoffset, NULL);
	if (nextoffset < 0)
		return -1;
	ptr = fdt_get_name((const void*)fdt_addr, nextoffset, NULL);
	if (strncmp(ptr, "partition_rootfs", 16) == 0)
		*ro_attr = 1;
	else
		*ro_attr = 0;
	#endif

	*mtd_num = i;
	return 0;
}

#ifdef CONFIG_NVT_PCIE_CASCADE
int nvt_get_nvtpack_label_from_id(void *fdt_addr, char *root, unsigned int id, char *label)
{
	int  nodeoffset, nextoffset;
	char *ptr = NULL;
	char name[255];

	nodeoffset = fdt_path_offset((void*)fdt_addr, root);
	if (nodeoffset < 0) {
		return -1;
	}

	/* To the pointer of the id offset */
	sprintf(name, "id%d", id);
	nextoffset = fdt_subnode_offset((const void *)fdt_addr, nodeoffset, name);
	if (nextoffset < 0) {
		return -1;
	}

	/* To get label name from id offset */
	ptr = (char *)fdt_getprop((const void*)fdt_addr, nextoffset, "label", NULL);
	if (ptr == NULL) {
		return -1;
	} else {
		sprintf(label, "%s", ptr);
		return 0;
	}
}
#endif /* CONFIG_NVT_PCIE_CASCADE */

/*** NAND Util ***/
uint32_t nand_get_page_size(void)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	return mtd->writesize;
}

uint32_t nand_get_block_size(void)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	return mtd->erasesize;
}

uint32_t nand_total_blocks(void)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	printf("NAND : total blocks number: %d\n", (uint32_t)(mtd->size >> mtd->erasesize_shift));
	return (uint32_t)(mtd->size >> mtd->erasesize_shift);
}
