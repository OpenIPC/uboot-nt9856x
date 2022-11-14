/*
 *  board/novatek/evb670b/nvt_emmc_partition.c
 *
 *  Author:	Alvin lin
 *  Created:	May 21, 2013
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <asm/global_data.h>
#include <nvt_stbc.h>
#include <nvt_emmc_partition.h>
#include <nvt_emmc_fwupdate.h>
#include <nvt_emmc_xbootdat.h>
#include <nvt_vtbl_check.h>

DECLARE_GLOBAL_DATA_PTR;

static struct gpt_dram_part nvt_gpt_dram_tbl[GPT_ENTRY_COUNT];

static int emmc_part_inited = 0;

#if defined(CONFIG_EVB_172_A32_EMMC)
#include "../nt72172_a32_evb/nt72172_emmc_nvt_img.c"
#elif defined(CONFIG_EVB_172_A64_EMMC)
#include "../nt72172_a64_evb/nt72172_emmc_nvt_img.c"
#elif defined(CONFIG_TK_172_A64_ANDROID)
#include "../nt72172_a64_tk_android/nt72172_emmc_tk_android_img.c"
#elif defined(CONFIG_TK_172_A32_ANDROID)
#include "../nt72172_a32_tk_android/nt72172_emmc_tk_android_img.c"
#elif defined(CONFIG_TK_172_A32_ANDROID_OPERA)
#include "../nt72172_a32_tk_android_opera/nt72172_emmc_tk_android_opera_img.c"
#elif defined(CONFIG_TK_172_A32_ANDROID_OREO)
#include "../nt72172_a32_tk_android_oreo/nt72172_emmc_tk_android_oreo_img.c"
#elif defined(CONFIG_EVB_673_A32_EMMC)
#include "../nt72673_a32_emmc_evb/nt72673_emmc_nvt_img.c"
#elif defined(CONFIG_EVB_673_A32_EMMC_CHINA_DRM)
#include "../nt72673_a32_emmc_china_drm/nt72673_emmc_nvt_img.c"
#elif defined(CONFIG_EVB_673_A64_EMMC)
#include "../nt72673_a64_emmc_evb/nt72673_emmc_nvt_img.c"
#elif defined(CONFIG_TK_673_A32_ANDROID_OREO)
#include "../nt72673_a32_tk_android_oreo/nt72673_emmc_tk_android_oreo_img.c"
#elif defined(CONFIG_TK_673_A32_ANDROID_OREO_SEC)
#include "../nt72673_a32_tk_android_oreo_sec/nt72673_emmc_tk_android_oreo_sec_img.c"
#elif defined(CONFIG_N69004_A32_ANDROID_OREO)
#include "../n69004_a32_android_oreo/n69004_emmc_android_oreo_img.c"
#elif defined(CONFIG_N62002_A32_ANDROID_OREO)
#include "../n62002_a32_android_oreo/n62002_emmc_android_oreo_img.c"
#elif defined(CONFIG_N62002_A32_ANDROID_OREO_SEC)
#include "../n62002_a32_android_oreo_sec/n62002_emmc_android_oreo_sec_img.c"
#elif defined(CONFIG_TK_673_A64_ANDROID_OREO)
#include "../nt72673_a64_tk_android_oreo/nt72673_emmc_tk_android_oreo_img.c"
#elif defined(CONFIG_EVB_671_A32_EMMC)
#include "../nt72671_a32_emmc_evb/nt72671_emmc_nvt_img.c"
#elif defined(CONFIG_TK_671_A32_ANDROID_OREO)
#include "../nt72671_a32_tk_android_oreo/nt72671_emmc_tk_android_oreo_img.c"
#else
#error "You should define your product partition and image first !"
#endif


unsigned int nvt_emmc_get_image_cnt(void)
{
	return sizeof(image_info) / sizeof(nvt_emmc_image_info);
}

nvt_emmc_image_info* nvt_emmc_get_img_by_index(unsigned int index)
{
	int total_cnt = sizeof(image_info) / sizeof(nvt_emmc_image_info);

	if(index >= total_cnt)
		return NULL;
	else
		return (&(image_info[index]));
}

nvt_emmc_image_info* nvt_emmc_get_img_by_name(char *name)
{
	int i, total_cnt = sizeof(image_info) / sizeof(nvt_emmc_image_info);

	for(i=0;i < total_cnt;i++) {
		if(strcmp(image_info[i].image_name, name) == 0)
			break;
	}

	if(i >= total_cnt)
		return NULL;
	else
		return (&(image_info[i]));
}

unsigned char* nvt_emmc_get_partition_name_by_name(char *img_name)
{
	int i, total_cnt = sizeof(image_info) / sizeof(nvt_emmc_image_info);
	for(i=0;i < total_cnt;i++) {
		if(strcmp(image_info[i].image_name, img_name) == 0)
			break;
	}

	if(i >= total_cnt)
		return NULL;
	else
		return image_info[i].user_part_name;

}

unsigned long long nvt_emmc_get_part_start_sector(unsigned char* part_name)
{
	unsigned long long start_lba = 0;
	int i;

	for(i=0;i < GPT_ENTRY_COUNT;i++) {
		if(nvt_gpt_dram_tbl[i].part_name == NULL)
			break;

		if(strcmp(part_name, nvt_gpt_dram_tbl[i].part_name) == 0) {
			start_lba = nvt_gpt_dram_tbl[i].part_start_lba;
			break;
		}
	}

	return start_lba;
}

unsigned long long nvt_emmc_get_part_size(unsigned char* part_name)
{
	unsigned long long size_in_sector = 0;
	int i;

	for(i=0; i < GPT_ENTRY_COUNT; i++) {
		if(nvt_gpt_dram_tbl[i].part_name == NULL)
			break;

		if(strcmp(nvt_gpt_dram_tbl[i].part_name, part_name) == 0) {
			size_in_sector = nvt_gpt_dram_tbl[i].part_end_lba -
				nvt_gpt_dram_tbl[i].part_start_lba + 1;
			break;
		}
	}

	return size_in_sector;
}

int nvt_emmc_get_partition_id_by_name(char *name)
{
	int i, total_cnt = sizeof(image_info) / sizeof(nvt_emmc_image_info);
	int part_id = 0;
	unsigned char *part_name = NULL;

	for(i=0;i < total_cnt;i++) {
		if(strcmp(image_info[i].image_name, name) == 0) {
			part_name = image_info[i].user_part_name;
			break;
		}
	}

	if(i >= total_cnt)
		return INVAILD_USER_PART_ID;
	else {
		for(i=0; i < GPT_ENTRY_COUNT; i++) {
			if(nvt_gpt_dram_tbl[i].part_name == NULL)
				break;
			if(strcmp(nvt_gpt_dram_tbl[i].part_name, part_name) == 0) {
				part_id = i+1;
				break;
			}
		}
	}
out:
	return part_id;

}

void nvt_emmc_part_init(void)
{
	int i, sz = sizeof(image_info) / sizeof(nvt_emmc_image_info);
	unsigned int raddr;
	nvt_emmc_image_info *pimg;

	if(!emmc_part_inited) {
		unsigned long long start_sector;
		nvt_gen_gpt_dram_tbl(nvt_part_parm_tbl, nvt_gpt_dram_tbl);
		start_sector = nvt_emmc_get_part_start_sector(nvt_emmc_get_partition_name_by_name("xbootdat"));
		pimg = nvt_emmc_get_img_by_name("xbootdat");
		xbootdat_init((pimg->part_offset + start_sector));

		for(i=0; i < sz; i++) {
			if(image_info[i].user_part_name == NULL)
				break;

			if(image_info[i].part_type == PART_TYPE_FS && image_info[i].size == 0)
				image_info[i].size = nvt_emmc_get_part_size(image_info[i].user_part_name);
		}

		emmc_part_inited = 1;
	}

out:
	return;
}

int do_nvt_emmc_part(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i,ret = 0;
	if (argc < 2)
		return CMD_RET_USAGE;

	nvt_emmc_part_init();

	if(strcmp(argv[1],"info") == 0) {
		for(i=0;i < GPT_ENTRY_COUNT;i++) {
			if(nvt_gpt_dram_tbl[i].part_name == NULL)
				break;
			else {
				printf("PART:%s start:%lld end:%lld len:%lld\n",nvt_gpt_dram_tbl[i].part_name,
						nvt_gpt_dram_tbl[i].part_start_lba, nvt_gpt_dram_tbl[i].part_end_lba,
						(nvt_gpt_dram_tbl[i].part_end_lba - nvt_gpt_dram_tbl[i].part_start_lba + 1));
			}
		}
	} else if (strcmp(argv[1],"read") == 0) {
		ret = nvtgpt_dump_gpt();
	} else if(strcmp(argv[1],"write") == 0) {
		ret = nvtgpt_write_gpt(nvt_part_parm_tbl, GPT_ENTRY_COUNT);
	} else
		ret = CMD_RET_USAGE;

	return ret;
}

U_BOOT_CMD(
	epart, 2, 1, do_nvt_emmc_part,
	"EMMC partition tool",
	"epart info - print current firmware partition setting\n"
	"epart read - read current emmc partition\n"
	"epart write - write partition table to emmc (this may destroy all data on emmc chip, make sure you know what you are doing)\n"
);


