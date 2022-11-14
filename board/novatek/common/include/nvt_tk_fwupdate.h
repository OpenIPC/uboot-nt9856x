#ifndef __NVT_TK_FWUPDATE_H__
#define __NVT_TK_FWUPDATE_H__

#define NVTFW_UNIT_IN_BYTES		(512)
#define NVTFW_BOOT_FLAG_SIGN 		"BTFG"
#define NVTFW_UPDATE_FLAG_SIGN 		"UPFG"
#define NVTFW_PPCACHE_SIGN 		"PPCH"
#define NVTFW_IMAGE_NAME_LEN 		16
#define NVTFW_IMAGE_PATH_LEN 		32
#define NUM_OF_PPCACHE_ENTRY 		20
#define NVTFW_PPTBL_SIZE 		(NVTFW_UNIT_IN_BYTES * 3)

typedef struct _boot_flag {
	unsigned char sign[4];//BTFG
	unsigned int kernel_idx;
	unsigned int fs_idx;
	unsigned int ppcache_idx;
	unsigned char reserved[NVTFW_UNIT_IN_BYTES - 16];
} boot_flag;

typedef struct _update_flag {
	unsigned char sign[4];//UPFG
	unsigned char reserved[NVTFW_UNIT_IN_BYTES - 4];
}update_flag;

typedef struct _pp_cache_entry {
	unsigned char name[NVTFW_IMAGE_NAME_LEN];
	unsigned char path[NVTFW_IMAGE_PATH_LEN];//always fix to first path, dynamic calculat by index
	unsigned int offset;
	unsigned int valid;
	unsigned int index;
} pp_cache_entry;

typedef struct _pp_cache_tbl {
	unsigned char sign[4];//PPCH
	unsigned int crc32;
	pp_cache_entry entrys[NUM_OF_PPCACHE_ENTRY];
	unsigned char reserved[NVTFW_PPTBL_SIZE - 4 - 4 - sizeof(pp_cache_entry)*NUM_OF_PPCACHE_ENTRY];
} pp_cache_tbl;

#endif /*__NVT_TK_FWUPDATE_H__ */
