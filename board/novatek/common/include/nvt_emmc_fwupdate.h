/*
 *  board/novatek/evb670b/nvt_emmc_fwupdate.h
 *
 *  Author:	Alvin lin
 *  Created:	May 21, 2013
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_EMMC_FWUPDATE_H__
#define __NVT_EMMC_FWUPDATE_H__
#include <u-boot/sha256.h>
#include "nvt_emmc_partition.h"

typedef struct boot_img_hdr boot_img_hdr;

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024

struct boot_img_hdr
{
    uint8_t magic[BOOT_MAGIC_SIZE];

    uint32_t kernel_size;  /* size in bytes */
    uint32_t kernel_addr;  /* physical load addr */

    uint32_t ramdisk_size; /* size in bytes */
    uint32_t ramdisk_addr; /* physical load addr */

    uint32_t second_size;  /* size in bytes */
    uint32_t second_addr;  /* physical load addr */

    uint32_t tags_addr;    /* physical addr for kernel tags */
    uint32_t page_size;    /* flash page size we assume */
    uint32_t unused;       /* reserved for future expansion: MUST be 0 */

    /* operating system version and security patch level; for
     * version "A.B.C" and patch level "Y-M-D":
     * ver = A << 14 | B << 7 | C         (7 bits for each of A, B, C)
     * lvl = ((Y - 2000) & 127) << 4 | M  (7 bits for Y, 4 bits for M)
     * os_version = ver << 11 | lvl */
    uint32_t os_version;

    uint8_t name[BOOT_NAME_SIZE]; /* asciiz product name */

    uint8_t cmdline[BOOT_ARGS_SIZE];

    uint32_t id[8]; /* timestamp / checksum / sha1 / etc */

    /* Supplemental command line data; kept here to maintain
     * binary compatibility with older versions of mkbootimg */
    uint8_t extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
} __attribute__((packed));

typedef struct _vtbl_hint_hdr {                                                                                                                                                                                
	unsigned char sign[4];//VTBL
	unsigned int flen;
} nvtfw_vtbl_hint_hdr;

#define NVTFW_VTBL_HINT_HDR_SIGN 	"VTBL"

typedef struct _vhash_val {
	unsigned char vhash[SHA256_SUM_LEN];
	unsigned char reserved[EMMC_RPMB_SECTOR_SIZE - SHA256_SUM_LEN];
} vhash_val;

#define RSA_OUT_LEN                             256

typedef struct _nvt_bin_rsa_info {
	unsigned char rsa_buff[RSA_OUT_LEN];
	unsigned int file_len;
} nvt_bin_rsa_info;

#define NVT_TKFM_RSA_PADDING_LEN 	60

typedef struct {
	unsigned char rsa_buff[RSA_OUT_LEN];
	unsigned int file_len;
	unsigned char padding[NVT_TKFM_RSA_PADDING_LEN];
} nvt_tkfm_rsa_info;

#define EMMC_SECTOR_SIZE                	512
#define EMMC_BYTE_TO_SECTOR_SFT 		9

int nvt_emmc_image_help(struct _nvt_emmc_image_info *this, NVT_EMMC_IMG_HELP_TYPE help_type);
int nvt_emmc_read_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args);
int nvt_emmc_write_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args);
int nvt_emmc_write_img_and_update_bootarg(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args);
int nvt_emmc_write_fdt_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args);
int nvt_emmc_erase_img(struct _nvt_emmc_image_info *this, void* args);
int nvt_emmc_read_android_boot_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args);
int nvt_emmc_check_common_header_exist(nvt_emmc_image_info *pimg);
int nvt_emmc_read_secos_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args);
void nvt_setup_emmc_boot_config(void);
int nvt_write_vhash_val(vhash_val *pval);

#if defined(CONFIG_FASTBOOT_FLASH_MMC_DEV)
int nvt_emmc_write_sparse_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args);
#endif

#if defined(CONFIG_NVT_TURNKEY_FWUPDATE)
#include <nvt_tk_fwupdate.h>
#endif

#endif

