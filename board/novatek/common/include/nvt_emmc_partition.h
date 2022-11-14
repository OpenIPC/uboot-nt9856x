/*
 *  board/novatek/evb670b/nt72670b_emmc_partition.h
 *
 *  Author:	Alvin lin
 *  Created:	May 21, 2013
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_EMMC_PARTITION_H__
#define __NVT_EMMC_PARTITION_H__
#include "nvt_tk_gpt.h"

typedef enum {
	PART_TYPE_BOOT1 = 1,
	PART_TYPE_BOOT2,
	PART_TYPE_BIN,
	PART_TYPE_FS,
	PART_TYPE_MAX
}NVT_EMMC_PART_TYPE;

typedef enum {
	PHY_PART_TYPE_BOOT1 = 1,
	PHY_PART_TYPE_BOOT2,
	PHY_PART_TYPE_RPMB,
	PHY_PART_TYPE_USER,
	PHY_PART_TYPE_MAX
} NVT_EMMC_PHY_PART_TYPE;

typedef enum {
	IMG_HELP_READ,
	IMG_HELP_WRITE,
	IMG_HELP_ERASE,
	IMG_HELP_MAX,
}NVT_EMMC_IMG_HELP_TYPE;

struct _nvt_emmc_image_info;

#define INVAILD_USER_PART_ID 		-1
#define FS_ERASE_BLK_CNT 		4096
#define EMMC_BOOT0_PART_NAME 		"emmc_boot0"
#define EMMC_BOOT1_PART_NAME 		"emmc_boot1"
#define EMMC_RPMB_SECTOR_SIZE	256

typedef struct _nvt_emmc_image_info {
	char *user_part_name;
	char *image_name;
	char *default_name;
	NVT_EMMC_PART_TYPE part_type;
	int (*image_help)(struct _nvt_emmc_image_info *this, NVT_EMMC_IMG_HELP_TYPE help_type);
	int (*read_img)(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args);
	int (*write_img)(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void* args);
	int (*erase_img)(struct _nvt_emmc_image_info *this, void* args);
	unsigned int part_offset;//count in sector, only vaild if partition type is boot or bin, others are set to 0
	unsigned int size;//count in sector
} nvt_emmc_image_info;


unsigned int nvt_emmc_get_image_cnt(void);
unsigned char* nvt_emmc_get_partition_name_by_name(char *img_name);
unsigned long long nvt_emmc_get_part_start_sector(unsigned char* part_name);
unsigned long long nvt_emmc_get_part_size(unsigned char* part_name);
nvt_emmc_image_info* nvt_emmc_get_img_by_name(char *name);
int nvt_emmc_get_partition_id_by_name(char *name);
nvt_emmc_image_info* nvt_emmc_get_img_by_index(unsigned int index);

void nvt_emmc_part_init(void);
int nvt_ewritepart(unsigned int ptableaddr,int ptablelen);
#endif

