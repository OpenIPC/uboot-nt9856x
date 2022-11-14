#ifndef __NVT_NPT_H__
#define __NVT_NPT_H__

#include <common.h>
#include <linux/mtd/partitions.h>

#define MAX_CSV_SIZE (4*1024)
#define MAX_NR_PART (32)
#define MAX_NR_PART_XBOOT (16)
#define LIMIT_NR_PART (MAX_NR_PART - 1) /* the last one needs to be null */

#define NVT_NPT_STORE_DEFAULT_BLK_OFF	(16)
#define NVT_NPT_STORE_DEFAULT_BLK_NR	(3)

struct nvt_partition {
	char name[16]; /* identifier string */
	uint64_t size; /* partition size */
	uint64_t offset; /* offset within the master MTD space */
};

struct nvt_ubi_vol {
	int mtd_idx;
	int vol_idx;
	char name[16]; /* identifier string */
	uint64_t size; /* partition size */
};

struct nt72_npt_info {
	/* the buffer should be the first field to be aligned for FAT read */
	unsigned char pt_buf[MAX_CSV_SIZE];

	unsigned int version;
	char name[32];
	unsigned int chip_id;
	unsigned int nand_id;
	unsigned int nand_sz;
	unsigned int unit_kb;
	unsigned int unit_vol_kb;
	unsigned int part_cnt;
	unsigned int vol_cnt;
	char img_name[MAX_NR_PART][32];

	struct nvt_partition part[MAX_NR_PART];
	struct nvt_ubi_vol vol[MAX_NR_PART];
};


struct nand_partition_table {
	u32 crc;
	u32 crc_xboot;
	struct nvt_partition npt_part[MAX_NR_PART];
	struct nvt_ubi_vol npt_vol[MAX_NR_PART];
};


int nt72_npt_if_load(struct nt72_npt_info *p, const char *csv_file);
void nt72_npt_parse_csv_field(char *next_field, char *f0, char *f1, char *f2,
			      char *f3, char *f4, char *f5);
void nt72_npt_parse_csv_line(char **next_line, char *f0, char *f1, char *f2,
			     char *f3, char *f4, char *f5);
int nt72_npt_parse(struct nt72_npt_info *p);

int nt72_npt_gen_mtd_parts(struct nt72_npt_info *p, struct mtd_partition *mtd_parts);
void nt72_npt_print_mtd_parts(struct nvt_partition *mtd_parts, int count);

int nt72_npt_gen_npt(struct nt72_npt_info *npt_info, struct nand_partition_table *npt);
int nt72_npt_gen_mtd_parts_by_npt(struct nand_partition_table *npt, struct mtd_partition *mtd_parts);

uint32_t nt72_npt_get_store_blk_offset();
uint32_t nt72_npt_get_store_blk_nr();

#endif /* __NVT_NPT_H__ */
