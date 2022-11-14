/*
 *  board/novatek/common/nvt_kx_hash.h
 *
 *  Author:	Alvin lin
 *  Created:	Feb 27, 2018
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_KX_HASH_H__
#define __NVT_KX_HASH_H__
#include "nvt_emmc_fwupdate.h"

typedef enum {
	NVT_EMMC_BOOT0,
	NVT_EMMC_BOOT1,
	NVT_EMMC_RPMB,
	NVT_EMMC_USER,
	NVT_EMMC_NONE,
	NVT_EMMC_MAX,
} nvt_emmc_partition_info;

typedef enum {
	NVT_SIGN_FMT_XBOOT,
	NVT_SIGN_FMT_BINFO,
	NVT_SIGN_FMT_COMMON,
	NVT_SIGN_FMT_NFSB,
	NVT_SIGN_FMT_VTBL,
	NVT_SIGN_FMT_STBC,
	NVT_SIGN_FMT_ANDOID_BOOTIMG,
	NVT_SIGN_FMT_ANDOID_DM_VERITY,
	NVT_SIGN_FMT_UNKNOWN,
} nvt_sign_fmt;

typedef struct {
	char *img_name;
	char *dev_node;
	nvt_sign_fmt sign_fmt;
	nvt_emmc_partition_info part_info;
	unsigned short part_id;
	unsigned long start;//count in sector
	unsigned long size;//count int sector
} nvt_vhash_img_info;

#define BOOT0_START_POS                 (16*1024 - sizeof(nvt_bin_rsa_info))
#define BOOT0_SIZE                      (16*1024)
#define LVDS_START_POS 			(40*1024 - sizeof(nvt_bin_rsa_info))
#define DDRINIT_START_POS               (128*1024 - sizeof(nvt_bin_rsa_info))
#define BOOT1_START_POS                 (128*1024)
#define CONFIG_NVT_VHASH_HDR_SIZE       4096
#define NFSB_HDR_SECTOR_CNT 		(4096 / EMMC_SECTOR_SIZE)
#define NVT_SIGN_ANDROID_DM_VERITY_HDR_SZ 	4096

#define RPMB_AUTH_KEY_LEN               32

#define NVT_STBC_IMG_LEN 			(64*1024)
#define NVT_STBC_SIGN_IMG_LEN                   (0x10110)              /*64KB + 260 byte align 16 byte*/

extern  nvt_vhash_img_info nsv_checked_imgs[];
extern int vhash_image_count;
int nvt_calc_vhash(nvt_vhash_img_info *pimg_info, int pimg_info_cnt);
void nvt_vhash_overwrite(void);

#endif
