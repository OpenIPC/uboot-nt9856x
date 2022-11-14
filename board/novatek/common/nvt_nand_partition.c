/*
 *  Author:	Will Fu
 *  Created:	Aug, 26, 2015
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <linux/mtd/nand.h>
#include <linux/crc32.h>
#include <nvt_nand_partition.h>
#include <nvt_npt.h>
#include <nand.h>
#include <nvt_nand_xbootdat.h>

static struct nand_partition_table g_npt;
static int g_nvt_ptn_init_done = 0;
static int32_t ptn_num = -1;
static int32_t vol_num = -1;


/*** NAND Util ***/
uint32_t nand_get_page_size(void)
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	return mtd->writesize;
}

uint32_t nand_get_block_size(void)
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	return mtd->erasesize;
}

uint32_t nand_total_blocks(void)
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	printf("NAND : total blocks number: %d\n", (uint32_t)(mtd->size >> mtd->erasesize_shift));
	return (uint32_t)(mtd->size >> mtd->erasesize_shift);
}

uint32_t nand_calc_block_addr(uint32_t start_block)
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	return start_block << mtd->erasesize_shift;
}

uint32_t nand_get_bad_block_size(uint32_t offset, uint32_t length)
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	u32 offend, i ,bbcnt;

	if ((offset + length) > mtd->size) {
		printf("NVT WARNING: %s exceed NAND chip size!", __func__);
		offend = mtd->size;
	} else {
		offend = offset+length;
	}

	bbcnt = 0;

	for(i=offset; i < offend;i += mtd->erasesize) {
		if (nand_block_isbad(mtd, i))
			bbcnt++;
	}
	return (bbcnt << mtd->size >> mtd->erasesize_shift);
}

static int32_t _convert_to_big_unit(char *buf, uint64_t size, const char *name)
{
	uint64_t mask_M = (1<<20) -1;
	uint64_t mask_K = (1<<10) -1;

	if ((size & mask_M) == 0) {
		sprintf(buf, "%lluM(%s)", size >> 20, name);
		return 0;
	}
	if ((size & mask_K) == 0) {
		sprintf(buf, "%lluK(%s)", size >> 10, name);
		return 0;
	}
	sprintf(buf, "%llu(%s)", size , name);
	return 0;
}

static int32_t _nvt_nand_get_ptn_env(char *msg)
{
	int32_t i = 0;
	char buf[128] = {0};

	for (i=0; i < ptn_num; i++) {
		_convert_to_big_unit(buf, g_npt.npt_part[i].size, g_npt.npt_part[i].name);
		strcat(msg, buf);
		if (i < ptn_num - 1) {
			strcat(msg, ",");
		}
	}
	return 0;
}

static void _nvt_nand_part_env_set(void)
{
	char data[1024]= {0};
	char msg[1024] = {0};
	int ret = 0;
	//env_set mtdids 'nand0=atmel_nand'
	//env_set mtdparts 'mtdparts=atmel_nand:8M(bootstrap)ro,-(rootfs)'

	/* set mtdids */
	ret = run_command("env_set mtdids 'nand0=nvt_nand'", 0);
	if(ret) {
		printf("env_set mtdids fail!\n");
		return;
	}

	/* set mtdparts */
	_nvt_nand_get_ptn_env(msg);
	sprintf(data, "env_set mtdparts 'mtdparts=nvt_nand:%s'", msg);
	ret = run_command(data, 0);
	if(ret) {
		printf("env_set mtdparts fail!\n");
		return;
	}
}

int32_t nvt_nand_ptn_get_num_of_table(void)
{
	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return -1;
	}

	return ptn_num;
}

char* nvt_nand_get_ptn_name(int32_t ptn_idx)
{
	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return 0;
	}

	if (ptn_idx < 0 || ptn_idx >= ptn_num) {
		printf("%s:[Error] out of partition idx range!\n", __func__);
		return 0;
	}
	return (char *)(g_npt.npt_part[ptn_idx].name);
}

int32_t nvt_nand_get_ptn_index(char *name)
{
	int32_t i = 0;
	int32_t idx = -1;
	char *search_name;
	search_name = name;
	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return -1;
	}

	for(i = 0; i < ptn_num; i++) {
		if(!strcmp(g_npt.npt_part[i].name, search_name)) {
			idx = i;
			break;
		}
	}
	if (idx == -1)
		printf("%s:[ERROR] '%s' ptn idx not found!\n", __func__, name);

	return idx;
}

uint32_t nand_calc_partition_addr(int32_t ptn_idx)
{
	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return 0;
	}

	if (ptn_idx < 0 || ptn_idx >= ptn_num) {
		printf("%s:[Error] out of partition idx range!\n", __func__);
		return 0;
	}
	return g_npt.npt_part[ptn_idx].offset;
}

uint32_t nand_calc_partition_size(int32_t ptn_idx)
{
	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return 0;
	}

	if (ptn_idx < 0 || ptn_idx >= ptn_num) {
		printf("%s:[Error] out of partition idx range!\n", __func__);
		return 0;
	}
	return g_npt.npt_part[ptn_idx].size;
}

uint32_t nand_calc_partition_block(int32_t ptn_idx)
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return 0;
	}

	if (ptn_idx < 0 || ptn_idx >= ptn_num) {
		printf("%s:[Error] out of partition idx range!\n", __func__);
		return 0;
	}
	return (uint32_t)(g_npt.npt_part[ptn_idx].offset) >> mtd->erasesize_shift;
}

uint32_t nand_calc_max_image_size(int32_t ptn_idx)
{
	uint32_t bad_len,max_len;

	if (g_nvt_ptn_init_done != 1) {
		printf("%s:[ERROR] no init NPT\n", __func__);
		return 0;
	}

	if (ptn_idx < 0 || ptn_idx >= ptn_num) {
		printf("%s:[Error] out of partition idx range!\n", __func__);
		return 0;
	}

	max_len = nand_calc_partition_size(ptn_idx);
	bad_len = nand_get_bad_block_size(nand_calc_partition_addr(ptn_idx), nand_calc_partition_size(ptn_idx));
	max_len -= bad_len;

	return max_len;
}

static int32_t _nvt_nand_read_npt(struct nand_partition_table* npt)
{
	int ret = 0;
	char cmd[128] = {0};
	uint32_t page_size;
	uint32_t read_size;
	uint32_t npt_partition_sz;
	uint32_t npt_partition_addr;
	uint32_t verify_crc;

	npt_partition_sz = nt72_npt_get_store_blk_nr() * nand_get_block_size();
	npt_partition_addr = nt72_npt_get_store_blk_offset() * nand_get_block_size();
	page_size = nand_get_page_size();
	read_size = sizeof(struct nand_partition_table);
	/* ROUND UP page alignment */
	if (read_size % page_size) {
		read_size = (read_size & ~(page_size - 1)) + page_size;
	}

	/* Read block */
	memset((void *)CONFIG_FWUP_RAM_START, 0xff, npt_partition_sz);
	sprintf(cmd, "nand read 0x%x 0x%x 0x%x", (unsigned int)CONFIG_FWUP_RAM_START,
		npt_partition_addr, read_size);
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("read NPT fail!\n");
		goto out;
	}

	/* Copy to struct */
	memcpy(npt, (void *)CONFIG_FWUP_RAM_START, sizeof(struct nand_partition_table));


	/* Verify struct */
	verify_crc = crc32(0, (const unsigned char *)(npt->npt_part), sizeof(struct nvt_partition)* MAX_NR_PART);
	verify_crc = crc32(verify_crc, (const unsigned char *)(npt->npt_vol), sizeof(struct nvt_ubi_vol)* MAX_NR_PART);
	if (npt->crc != verify_crc) {
		printf("Verify NPT fail!\n");
		ret = -1;
		goto out;
	}
	ret = 0;

out:
	return ret;
}

static int32_t _nvt_nand_ptn_get_entry_number(struct nand_partition_table *npt)
{
	int num = 0;
	int i = 0;
	for (i = 0; i < MAX_NR_PART; i++) {
		if (strlen(npt->npt_part[i].name) == 0) {
			break;
		}
		num ++;
	}

	return num; 
}

static int32_t _nvt_nand_ptn_get_vol_number(struct nand_partition_table *npt)
{
	int num = 0;
	int i = 0;
	for (i = 0; i < MAX_NR_PART; i++) {
		if (strlen(npt->npt_vol[i].name) == 0) {
			break;
		}
		num ++;
	}

	return num;
}

static int32_t _nvt_nand_ptn_check_block_num(void)
{
	int32_t i;
	uint64_t maxblocks;
	uint64_t blocksnum;
	struct mtd_info *mtd = nand_info[nand_curr_device];

	maxblocks = mtd->size >> mtd->erasesize_shift;
	printf("maxblocks: %lld, size: %lld, shitf: %d\n", maxblocks, mtd->size, mtd->erasesize_shift);

	blocksnum = g_npt.npt_part[ptn_num-1].offset + g_npt.npt_part[ptn_num-1].size;
	blocksnum = blocksnum >> mtd->erasesize_shift;
	if (blocksnum < (maxblocks - BBT_MAX_BLOCK_NUM)) {
		printf("NVT PTN WARNING: Some Nand blocks not used!!! , use %llu blocks\n",blocksnum);
	}
	else if(blocksnum > (maxblocks - BBT_MAX_BLOCK_NUM)){
		for (i=0 ; i < 5; i++) {
			printf("NVT PTN ERROR: Over max Nand blocks!!! , use %llu blocks\n",blocksnum);
		}
	}
	return 0;
}

int32_t nvt_ubi_get_vol_by_name(struct nvt_ubi_vol *vol, const char *vol_name)
{
	int i = 0;
	for (i = 0; i < vol_num; i ++) {
		if (!strcmp(g_npt.npt_vol[i].name, vol_name)) {
			memcpy(vol , &(g_npt.npt_vol[i]), sizeof(struct nvt_ubi_vol));
			return 0;
		}
	}

	/* Not found */
	return -1;
}

static int32_t _nvt_ubi_vol_storage_sync_mtd(struct nand_partition_table *npt, int mtd_idx)
{
	char cmd_buf[256] = {0};
	int cmd_ret;
	int i;
	int vol_sum;
	int maybe_repartion = 0;

	vol_sum = 0;
	for (i = 0; i < vol_num; i ++) {
		if (npt->npt_vol[i].mtd_idx == mtd_idx)
			vol_sum ++;
	}
	if (vol_sum == 0)
		return 0;

sync_start:
	if (maybe_repartion == 1) {
		/* Do erase partition first */
		sprintf(cmd_buf, "nand erase 0x%x 0x%x",nand_calc_partition_addr(mtd_idx),nand_calc_partition_size(mtd_idx));
		cmd_ret = run_command(cmd_buf, 0);
		printf("'%s' return: %d\n",cmd_buf, cmd_ret);
		if (cmd_ret != 0) {
			printf("Error: Erase %s fail!\n", nvt_nand_get_ptn_name(mtd_idx));
			goto err;
		}
	}

	/* Check mtd_idx */
	/* Attach part */
	sprintf(cmd_buf, "ubi part %s", nvt_nand_get_ptn_name(mtd_idx));
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("Error: ubi part %s not found!\n", nvt_nand_get_ptn_name(mtd_idx));
		goto repart;
	}

	/* Check each vol with size and id */
	for (i = 0; i < vol_num; i ++) {
		if (npt->npt_vol[i].mtd_idx != mtd_idx)
			continue;

		sprintf(cmd_buf, "ubi nvt_checkvol %s 0x%llx %d",
			npt->npt_vol[i].name, npt->npt_vol[i].size, npt->npt_vol[i].vol_idx);
		cmd_ret = run_command(cmd_buf, 0);
		printf("'%s' return: %d\n",cmd_buf, cmd_ret);
		if (cmd_ret != 0) {
			printf("Error: check vol size/id fail!, Recreate\n");
			/* Remove */
			sprintf(cmd_buf, "ubi remove %s", npt->npt_vol[i].name);
			cmd_ret = run_command(cmd_buf, 0);
			printf("'%s' return: %d\n",cmd_buf, cmd_ret);

			/* Create */
			sprintf(cmd_buf, "ubi nvt_create %s 0x%llx d %d",
					npt->npt_vol[i].name, npt->npt_vol[i].size, npt->npt_vol[i].vol_idx);
			cmd_ret = run_command(cmd_buf, 0);
			printf("'%s' return: %d\n",cmd_buf, cmd_ret);
			if (cmd_ret != 0) {
				printf("[ERROR] Can not create vol! ret=%d\n", cmd_ret);
				return -1;
			}
		}
	}

	/* Check vol num */
	sprintf(cmd_buf, "ubi nvt_checkvolnum %d", vol_sum);
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("Error: check vol num fail!\n");
		goto repart;
	}

	/* sync pass */
	return 0;

repart:
	/* Removeall */
	sprintf(cmd_buf, "ubi nvt_removeall");
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("Error: ubi removeall fail!\n");
		maybe_repartion = 1;
		goto sync_start;
	}

	/* Create each vol */
	for (i = 0; i < vol_num; i ++) {
		if (npt->npt_vol[i].mtd_idx != mtd_idx)
			continue;

		sprintf(cmd_buf, "ubi nvt_create %s 0x%llx d %d",
			npt->npt_vol[i].name, npt->npt_vol[i].size, npt->npt_vol[i].vol_idx);
		cmd_ret = run_command(cmd_buf, 0);
		printf("'%s' return: %d\n",cmd_buf, cmd_ret);
		if (cmd_ret != 0) {
			printf("[ERROR] Can not create vol! ret=%d\n", cmd_ret);
			goto err;
		}
	}

	return 0;
err:
	return -1;
}

static int32_t _nvt_ubi_vol_storage_sync(struct nand_partition_table *npt)
{
	int i, ret;
	for (i = 0 ; i < ptn_num ; i ++) {
		ret = _nvt_ubi_vol_storage_sync_mtd(npt, i);
		if (ret != 0)
			return -1;
	}
	return 0;
}

int32_t nvt_nand_show_npt(void)
{
	/* Dump MTD Part */
	nt72_npt_print_mtd_parts(g_npt.npt_part, ptn_num);
}

/* nvt_nand_ptn_init
 * Return value:
 *	 0: init done
 *	 1: init read npt fail, by-pass
 *	-1: something error
 */
int32_t nvt_nand_ptn_init(void)
{
	int32_t ret = -1;

	/* Reset ptn struct */
	g_nvt_ptn_init_done = 0;
	memset(&g_npt, 0, sizeof(struct nand_partition_table));

	/* Read ptn */
	ret = _nvt_nand_read_npt(&g_npt);
	if (ret != 0) {
		/* by-pass init and return 1 */
		printf("Read npt Fail, by-pass partition init!\n");
		ret = 1;
		goto out;
	}

	/* Get number of ptn */
	ptn_num = _nvt_nand_ptn_get_entry_number(&g_npt);

	/* Get number of vol */
	vol_num = _nvt_nand_ptn_get_vol_number(&g_npt);

	/* Print nand total blocks */
	nand_total_blocks();

	/* Check partition layout is OK or not */
	_nvt_nand_ptn_check_block_num();

	/* Set mtd ptn into mtdparts for prepare_kercmd */
	_nvt_nand_part_env_set();

	g_nvt_ptn_init_done = 1;

	/* Sync ubi volume in flash */
	ret = _nvt_ubi_vol_storage_sync(&g_npt);

out:
	return ret;
}

