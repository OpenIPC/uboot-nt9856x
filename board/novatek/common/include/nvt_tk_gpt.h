/*
 *  nvt_tk_gpt.h
 *
 *  Author:	Alvin lin
 *  Created:	Jun 11, 2016
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_TK_GPT_H__
#define __NVT_TK_GPT_H__

#define SECTOR_IN_BYTES 	512
#define BYTE_TO_SECTOR_SFT 	9
#define BOOT_CODE_SIZE 		446
#define MAX_PART_ENTRY_PER_MBR 	4
#define GPT_ENTRY_COUNT 	128

#define GPT_HDR_SIGN 		"EFI PART"
#define GPT_HDR_VER 		0x10000
#define GPT_UUID_SZ 		16
#define GPT_PART_NAME_SZ 	72
#define GPT_PART_START_SECTOR 	34
#define GPT_1ST_PART_START_PAD  (2048 - GPT_PART_START_SECTOR)
//#define GPT_1ST_PART_START_PAD  0
#define PTYPE_EFI_GPT           0xee
#define MAX_ALLOW_PART_NUM 	30

#define NVT_GPT_RESERVED_SECTORS 	0
struct dos_part_entry{
	unsigned char boot_flag;//1:means this partion can use to boot
	unsigned char start_chs[3];//start chs
	unsigned char part_type;//partition type
	unsigned char end_chs[3];//end chs
	unsigned int start_lba;//start LBA address
	unsigned int size_in_sector;//size in sector
}__attribute__((packed));

struct mbr {
	unsigned char boot_code[BOOT_CODE_SIZE];//0-445 bytes are boot code
	struct dos_part_entry part_entrys[MAX_PART_ENTRY_PER_MBR];//partition entrys
	unsigned char signature[2];//0xaa55
} __attribute__((packed));

struct gpt_header {
	unsigned char signature[8];
	unsigned int version;
	unsigned int gpt_hdr_sz;//count in byte
	unsigned int gpt_hdr_crc;
	unsigned int reserve;
	unsigned long long gpt_lba;
	unsigned long long backup_gpt_lba;
	unsigned long long part_area_start_lba;
	unsigned long long part_area_end_lba;
	unsigned char disk_guid[GPT_UUID_SZ];
	unsigned long long part_tbl_start_lba;
	unsigned int num_of_part_entry;
	unsigned int part_entry_sz;
	unsigned int part_tbl_crc;
} __attribute__((packed));

struct gpt_header_sector {
	struct gpt_header gpt_hdr;
	unsigned char reserved[SECTOR_IN_BYTES - sizeof(struct gpt_header)];
} __attribute__((packed));

struct gpt_part_entry {
	unsigned char part_type_guid[GPT_UUID_SZ];
	unsigned char part_unique_guid[GPT_UUID_SZ];
	unsigned long long part_start_lba;
	unsigned long long part_end_lba;
	unsigned long long part_attr;
	unsigned char part_name[GPT_PART_NAME_SZ];
} __attribute__((packed));

struct gpt_dram_part {
	const unsigned char *part_name;
	unsigned long long part_start_lba;
	unsigned long long part_end_lba;
};

struct part_parm {
	const char *part_name;//partition name in ascii,max length is 35 bytes
	unsigned int sectors;//partition size in sectors
};

static char nvt_gpt_part_type_guid[] = {
	0xAF, 0x3D, 0xC6, 0x0F, 0x83, 0x84, 0x72, 0x47,
	0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4
};

int nvtgpt_dump_gpt(void);
int nvtgpt_write_gpt(struct part_parm *nvt_part_parm_tbl, unsigned int entry_cnt);
int nvt_gen_gpt_dram_tbl(struct part_parm *pparm_tbl, struct gpt_dram_part *pgpt_dram_tbl);
int nvtgpt_ascii2unicode(unsigned char* inbuf, unsigned char* outbuf, unsigned int out_buflen);
int nvtgpt_unicode2ascii(unsigned char* inbuf, unsigned char* outbuf, unsigned int out_buflen);
unsigned long long nvtgpt_get_dev_max_sectors(void);
int _nvt_tk_gpt_get_part_info(unsigned char* part_name, unsigned long *pstart_lba, unsigned long *psize_in_sector);
#endif
