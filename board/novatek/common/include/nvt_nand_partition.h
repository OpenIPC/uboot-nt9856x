#ifndef __NVT_NAND_PARTITION_H__
#define __NVT_NAND_PARTITION_H__

#include <linux/types.h>
#include <nvt_npt.h>

#define BBT_MAX_BLOCK_NUM	(4)

/*** NAND Util ***/
uint32_t nand_get_page_size(void);
uint32_t nand_get_block_size(void);
uint32_t nand_total_blocks(void);
uint32_t nand_calc_block_addr(uint32_t start_block);
uint32_t nand_get_bad_block_size(uint32_t offset, uint32_t length);

/*** PTN Util ***/
int32_t nvt_nand_ptn_get_num_of_table(void);
char* nvt_nand_get_ptn_name(int32_t ptn_idx);
int32_t nvt_nand_get_ptn_index(char *name);
uint32_t nand_calc_partition_addr(int32_t ptn_idx);
uint32_t nand_calc_partition_size(int32_t ptn_idx);
uint32_t nand_calc_partition_block(int32_t ptn_idx);
uint32_t nand_calc_max_image_size(int32_t ptn_idx);
int32_t nvt_nand_show_npt(void);
int32_t nvt_nand_ptn_init(void);

/*** UBI Vol Util ***/
int32_t nvt_ubi_get_vol_by_name(struct nvt_ubi_vol *vol, const char *vol_name);
#endif
