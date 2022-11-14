/*
 *  board/novatek/nt72668tk/nvt_eburner.h
 *
 *  Author:	Alvin lin
 *  Created:	Mar 22, 2016
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_EMPTY_BURNER_H__
#define __NVT_EMPTY_BURNER_H__
#include <nvt_emmc_xbootdat.h>

#define NVT_EBURNER_WHOLE_IMG_HDR_SIGN   	"TIMG"
#define NVT_EBURNER_PER_IMG_HDR_SIGN   	"PIMG"

#define NVT_EBURNER_BOOT_FLAG_SIGN 		"BTFG"

#define NVT_EBURNER_IMAGE_NAME_LEN 		16
#define NVT_EBURNER_IMAGE_PATH_LEN 		64
#define NVT_EBURNER_TXT_PARAM_LEN 		1024

#ifndef NVT_EBURNER_IMG_FNAME
    #define NVT_EBURNER_IMG_FNAME 			"nvt_tv_firmware.img"
#endif

#define  DTB_MAGIC 0xedfe0dd0

typedef struct _whl_hdr {
	unsigned char sign[4];
	unsigned long long flen;
	unsigned char md5sum[16];
	unsigned char rsa_sign[256];
} nvt_eburner_whole_img_hdr;

typedef enum {
	NVT_EBURNER_DEV_TYPE_UNKNOWN,
	NVT_EBURNER_DEV_TYPE_SPI,
	NVT_EBURNER_DEV_TYPE_NAND,
	NVT_EBURNER_DEV_TYPE_EMMC,
	NVT_EBURNER_DEV_TYPE_MAX
} nvt_eburner_dev_type;

typedef struct _pimg_hdr {
	unsigned char sign[4];
	unsigned long long flen;
	unsigned char md5sum[16];
	unsigned char img_name[NVT_EBURNER_IMAGE_NAME_LEN];
	unsigned char dev_path[NVT_EBURNER_IMAGE_PATH_LEN];
	unsigned char compress[16];
	unsigned int dev_ofs;
	unsigned char txt_param[NVT_EBURNER_TXT_PARAM_LEN];
	nvt_eburner_dev_type dev_type;
} nvt_eburner_per_img_hdr;

int nvt_eburner_panel_param_init(int *pindex, panel_param_blk *pparam);
int nvt_eburner_check_trigger(unsigned int adc_reg_base, unsigned int adc_val, unsigned int adc_range);
int nvt_eburner_load_img(void* dest_addr, char* img_name, char* update_img_fname);
int nvt_eburner_run_updater(void);

#endif
