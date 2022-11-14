/*
 *  nvt_tk_gpt.c
 *
 *  Author:	Alvin lin
 *  Created:	Jun 11, 2016
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <mmc.h>
#include <uuid.h>
#include <nvt_tk_gpt.h>

static struct mbr nvt_gpt_protect_mbr __attribute__((aligned(64)));
static struct gpt_header_sector nvt_gpt_hdr_sector __attribute__((aligned(64)));
static struct gpt_header_sector nvt_gpt_hdr_backup_sector __attribute__((aligned(64)));
static struct gpt_part_entry nvt_gpt_part_tbl[GPT_ENTRY_COUNT] __attribute__((aligned(64)));
static struct gpt_dram_part *pnvt_gpt_dram_part_tbl;


int nvtgpt_ascii2unicode(unsigned char* inbuf, unsigned char* outbuf, unsigned int out_buflen)
{
	int ret = 0;
	int i;
	int conv_len;
	unsigned short *ptr = (unsigned short*)outbuf;

	if((size_t)out_buflen < (strlen((const char*)inbuf) << 1)) {
		printf("outbuf length not enough ! \n");
		ret = -1;
		goto out;
	}

	memset(outbuf, 0, out_buflen);

	conv_len = (int)strlen((const char*)inbuf);

	for(i=0; i < conv_len; i++) {
		*ptr = inbuf[i];
		ptr++;
	}

out:
	return ret;
}

int nvtgpt_unicode2ascii(unsigned char* inbuf, unsigned char* outbuf, unsigned int out_buflen)
{
	int i,j;

	memset(outbuf, 0, out_buflen);

	for(i=0,j=0; i < out_buflen;i+=2) {
		if(inbuf[i] == '\0')
			break;
		else
			outbuf[j++] = inbuf[i];
	}

	return 0;
}


unsigned long long nvtgpt_get_dev_max_sectors(void)
{
	unsigned long long dev_sectors = 0;

	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_SYS_DEV);

	mmc_init(mmc);

	dev_sectors =  ((mmc->capacity) >> BYTE_TO_SECTOR_SFT);


	if(dev_sectors > NVT_GPT_RESERVED_SECTORS)
		dev_sectors -= NVT_GPT_RESERVED_SECTORS;
	else
		dev_sectors = 0;

	return dev_sectors;
}

static int nvtgpt_init_headers(struct mbr *prot_mbr, struct gpt_header_sector* gpt_hdr_sector, struct gpt_part_entry *gpt_part_tbl, unsigned long long max_sectors)
{
	int ret = 0;
	unsigned char uuid[16];

	if (prot_mbr == NULL || gpt_hdr_sector == NULL || gpt_part_tbl == NULL || max_sectors == 0) {
		ret = -1;
		goto out;
	}

	//init protect mbr
	memset(prot_mbr, 0, sizeof(struct mbr));
	prot_mbr->signature[0] = 0x55;
	prot_mbr->signature[1] = 0xaa;

	prot_mbr->part_entrys[0].boot_flag = 0;

	prot_mbr->part_entrys[0].start_chs[0] = 0xfe;
	prot_mbr->part_entrys[0].start_chs[1] = 0xff;
	prot_mbr->part_entrys[0].start_chs[2] = 0xff;

	prot_mbr->part_entrys[0].end_chs[0] = 0xfe;
	prot_mbr->part_entrys[0].end_chs[1] = 0xff;
	prot_mbr->part_entrys[0].end_chs[2] = 0xff;

	prot_mbr->part_entrys[0].part_type = PTYPE_EFI_GPT;

	prot_mbr->part_entrys[0].start_lba = 1;
	prot_mbr->part_entrys[0].size_in_sector = max_sectors - 1;

	//init gpt header
	memset(gpt_hdr_sector, 0, sizeof(struct gpt_header_sector));

	memcpy(gpt_hdr_sector->gpt_hdr.signature, GPT_HDR_SIGN, strlen(GPT_HDR_SIGN));
	gpt_hdr_sector->gpt_hdr.version = GPT_HDR_VER;
	gpt_hdr_sector->gpt_hdr.gpt_hdr_sz = sizeof(struct gpt_header);
	gpt_hdr_sector->gpt_hdr.gpt_hdr_crc = 0;
	gpt_hdr_sector->gpt_hdr.gpt_lba = 1;
	gpt_hdr_sector->gpt_hdr.backup_gpt_lba = max_sectors - 1;
	gpt_hdr_sector->gpt_hdr.part_area_start_lba = GPT_PART_START_SECTOR;
	gpt_hdr_sector->gpt_hdr.part_area_end_lba = max_sectors - GPT_PART_START_SECTOR;
	gen_rand_uuid(uuid);
	memcpy(gpt_hdr_sector->gpt_hdr.disk_guid, uuid, sizeof(uuid));
	gpt_hdr_sector->gpt_hdr.part_tbl_start_lba = 2;
	gpt_hdr_sector->gpt_hdr.num_of_part_entry = GPT_ENTRY_COUNT;
	gpt_hdr_sector->gpt_hdr.part_entry_sz = sizeof(struct gpt_part_entry);
	gpt_hdr_sector->gpt_hdr.part_tbl_crc = 0;

	//init gpt part tbl
	memset(gpt_part_tbl, 0, sizeof(struct gpt_part_entry) * GPT_ENTRY_COUNT);


out:
	return ret;
}

int nvtgpt_dump_gpt(void)
{
	int ret = 0;
	struct mbr mbr_entry  __attribute__((aligned(64)));
	struct gpt_header_sector gpt_hdr_sec __attribute__((aligned(64)));
	int rdlen, i;
	unsigned int hdr_crc;
	unsigned int j;
	struct gpt_part_entry gpt_part_array[GPT_ENTRY_COUNT] __attribute__((aligned(64)));
	char empty_uuid[GPT_UUID_SZ] = {};
	unsigned char tmp_part_name[GPT_PART_NAME_SZ] = {};
	char cmd[64] = {0};
	

	//read MBR
	sprintf(cmd, "mmc read 0x%x 0 1", &mbr_entry);
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s read protect mbr fail\n", __func__);
		goto out;
	}
	//kwinyee debug
	invalidate_dcache_range((unsigned long)(&mbr_entry), (unsigned long)(&mbr_entry) + SECTOR_IN_BYTES);
	

	if(mbr_entry.signature[0] != 0x55 && mbr_entry.signature[1] != 0xaa) {
		printf("invalid mbr entry\n");
		ret = -1;
		goto out;
	}

	//read gpt header
	sprintf(cmd, "mmc read 0x%x 1 1", &gpt_hdr_sec);
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s read gpt header fail !\n", __func__);
		goto out;
	}
	//kwinyee debug
	invalidate_dcache_range((unsigned long)(&gpt_hdr_sec), (unsigned long)(&gpt_hdr_sec) + SECTOR_IN_BYTES);

	printf("=====protect MBR=====\n");
	printf("boot flag : 0x%x\n", mbr_entry.part_entrys[0].boot_flag);
	printf("start_chs : 0x%02x 0x%02x 0x%02x\n", mbr_entry.part_entrys[0].start_chs[0],
			mbr_entry.part_entrys[0].start_chs[1], mbr_entry.part_entrys[0].start_chs[2]);
	printf("part type : 0x%02x\n", mbr_entry.part_entrys[0].part_type);
	printf("end_chs : 0x%02x 0x%02x 0x%02x\n", mbr_entry.part_entrys[0].end_chs[0],
			mbr_entry.part_entrys[0].end_chs[1], mbr_entry.part_entrys[0].end_chs[2]);
	printf("start lba : 0x%x\n", mbr_entry.part_entrys[0].start_lba);
	printf("sz in sector : %d\n", mbr_entry.part_entrys[0].size_in_sector);

	printf("=====gpt header=====\n");
	printf("gpt hdr sign : ");
	for(i=0;i < 8;i++)
		printf("%c", gpt_hdr_sec.gpt_hdr.signature[i]);
	printf("\n");
	printf("version : 0x%x\n", gpt_hdr_sec.gpt_hdr.version);
	printf("gpt_hdr_sz : 0x%x\n", gpt_hdr_sec.gpt_hdr.gpt_hdr_sz);
	printf("gpt_hdr_crc : 0x%x\n", gpt_hdr_sec.gpt_hdr.gpt_hdr_crc);
	printf("gpt_lba : %llu\n", gpt_hdr_sec.gpt_hdr.gpt_lba);
	printf("backup gpt_lba : %llu\n", gpt_hdr_sec.gpt_hdr.backup_gpt_lba);
	printf("part_area_start_lba : %llu\n", gpt_hdr_sec.gpt_hdr.part_area_start_lba);
	printf("part_area_end_lba : %llu\n", gpt_hdr_sec.gpt_hdr.part_area_end_lba);
	printf("disk_guid:");
	for(i=0;i<16;i++)
		printf("%02X", gpt_hdr_sec.gpt_hdr.disk_guid[i]);
	printf("\n");
	printf("part_tbl_start_lba : %llu\n", gpt_hdr_sec.gpt_hdr.part_tbl_start_lba);
	printf("num_of_part_entry : %d\n", gpt_hdr_sec.gpt_hdr.num_of_part_entry);
	printf("part_entry_sz : %d\n", gpt_hdr_sec.gpt_hdr.part_entry_sz);
	printf("part_tbl_crc : 0x%x\n", gpt_hdr_sec.gpt_hdr.part_tbl_crc);


	gpt_hdr_sec.gpt_hdr.gpt_hdr_crc = 0;

	hdr_crc = crc32(0, (const char *)(&(gpt_hdr_sec.gpt_hdr)), sizeof(gpt_hdr_sec.gpt_hdr));

	printf("calc hdr crc = 0x%x\n", hdr_crc);

	//read gpt part entrys
	rdlen = sizeof(gpt_part_array);
	
	sprintf(cmd, "mmc read 0x%x 2 0x%x", gpt_part_array, rdlen >> BYTE_TO_SECTOR_SFT );
	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("%s read gpt part entrys fail !\n",__func__);
		goto out;
	}


	for(i=0; i < GPT_ENTRY_COUNT; i++) {
		if(memcmp(empty_uuid, gpt_part_array[i].part_type_guid, GPT_UUID_SZ) == 0)
			break;
		printf("===========part entry %d===============\n", (i+1));
		printf("part type guid:");
		for(j=0;j<GPT_UUID_SZ;j++)
			printf("%02X", gpt_part_array[i].part_type_guid[j]);
		printf("\n");
		printf("unique part guid:");
		for(j=0; j<GPT_UUID_SZ; j++)
			printf("%02X", gpt_part_array[i].part_unique_guid[j]);
		printf("\n");
		printf("start lba : %llu\n", gpt_part_array[i].part_start_lba);
		printf("end lba : %llu\n", gpt_part_array[i].part_end_lba);
		printf("part attribute : 0x%llx\n", gpt_part_array[i].part_attr);
		nvtgpt_unicode2ascii((unsigned char*)gpt_part_array[i].part_name, tmp_part_name, GPT_PART_NAME_SZ);
		printf("part name : %s\n", tmp_part_name);
		printf("=======================================\n");
	}

	hdr_crc = crc32(0, (const char*)gpt_part_array, rdlen);
	printf("part table crc = 0x%x\n", hdr_crc);

out:
	return ret;
}

int nvtgpt_write_gpt(struct part_parm *nvt_part_parm_tbl, unsigned int entry_cnt)
{
	unsigned long long dev_max_sectors = 0;
	unsigned long long part_avail_sectors = 0;
	unsigned long long curr_sector = 0;
	long backup_part_tbl_ofs = 0;
	int ret = 0;
	int i;
	unsigned char uuid[16];
	unsigned char tmp_part_name[GPT_PART_NAME_SZ] = {};
	size_t rdlen;
	char cmd[64] = {0};

	if(entry_cnt > GPT_ENTRY_COUNT) {
		printf("%s entry count %lu large than max gpt entry count !\n", __func__, entry_cnt);
		ret = -1;
		goto out;
	}

	dev_max_sectors = nvtgpt_get_dev_max_sectors();


	if(dev_max_sectors == 0) {
		printf("%s get max sectors fail !\n", __func__);
		ret = -1;
		goto out;
	}

	part_avail_sectors = dev_max_sectors - 1 - 2 - ((2 * sizeof(nvt_gpt_part_tbl)) >> BYTE_TO_SECTOR_SFT)  - GPT_1ST_PART_START_PAD;

	//check all partitions can fit into storage
	for(i=0;i < GPT_ENTRY_COUNT;i++) {
		if(nvt_part_parm_tbl[i].part_name == NULL)
			break;
		else
			curr_sector += nvt_part_parm_tbl[i].sectors;
	}

	if(curr_sector > part_avail_sectors) {
		printf("%s partition table large than device, abort update partition table !\n", __func__);
		ret = -1;
		goto out;
	}

	//init header
	ret = nvtgpt_init_headers(&nvt_gpt_protect_mbr, &nvt_gpt_hdr_sector, nvt_gpt_part_tbl, dev_max_sectors);
	if(ret != 0) {
		printf("%s init gpt hdr fail !\n", __func__);
		goto out;
	}

	memcpy(&nvt_gpt_hdr_backup_sector, &nvt_gpt_hdr_sector, sizeof(nvt_gpt_hdr_backup_sector));

	curr_sector = nvt_gpt_hdr_sector.gpt_hdr.part_area_start_lba + GPT_1ST_PART_START_PAD;

	//create part tbl
	for(i=0;i < GPT_ENTRY_COUNT;i++) {
		if(nvt_part_parm_tbl[i].part_name == NULL)
			break;
		//set part type guid
		memcpy(nvt_gpt_part_tbl[i].part_type_guid, nvt_gpt_part_type_guid, sizeof(nvt_gpt_part_type_guid));

		//set part uuid
		gen_rand_uuid(uuid);
		memcpy(nvt_gpt_part_tbl[i].part_unique_guid, uuid, sizeof(uuid));

		//set part start lba
		nvt_gpt_part_tbl[i].part_start_lba = curr_sector;

		//set part attr
		nvt_gpt_part_tbl[i].part_attr = 0;

		//set part name
		memset(tmp_part_name, 0, GPT_PART_NAME_SZ);
		ret = nvtgpt_ascii2unicode((unsigned char*)nvt_part_parm_tbl[i].part_name, tmp_part_name, GPT_PART_NAME_SZ);

		if(ret != 0) {
			printf("%s cover part name %s fail !\n", __func__, nvt_part_parm_tbl[i].part_name);
			goto out;
		}

		memcpy(nvt_gpt_part_tbl[i].part_name, tmp_part_name, GPT_PART_NAME_SZ);


		//set part end lba
		//0 means rest all for this partition
		if(nvt_part_parm_tbl[i].sectors == 0) {
			nvt_gpt_part_tbl[i].part_end_lba = curr_sector + part_avail_sectors - 1;
			break;
		} 

		nvt_gpt_part_tbl[i].part_end_lba = curr_sector + nvt_part_parm_tbl[i].sectors - 1;
		curr_sector += nvt_part_parm_tbl[i].sectors;
		part_avail_sectors -= nvt_part_parm_tbl[i].sectors;
	}

	//calculate crcs
	nvt_gpt_hdr_sector.gpt_hdr.part_tbl_crc = crc32(0, (const char*)nvt_gpt_part_tbl, sizeof(nvt_gpt_part_tbl));
	nvt_gpt_hdr_sector.gpt_hdr.gpt_hdr_crc = 0;
	nvt_gpt_hdr_sector.gpt_hdr.gpt_hdr_crc  = crc32(0, (const char*)(&(nvt_gpt_hdr_sector.gpt_hdr)), sizeof(nvt_gpt_hdr_sector.gpt_hdr));

	nvt_gpt_hdr_backup_sector.gpt_hdr.part_tbl_crc = nvt_gpt_hdr_sector.gpt_hdr.part_tbl_crc;


	//write protect mbr
	sprintf(cmd, "mmc write 0x%lx 0x0 1", (unsigned long)(&(nvt_gpt_protect_mbr)));
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s write protect mbr fail !\n", __func__);
		goto out;
	}

	//write gpt hdr
	sprintf(cmd, "mmc write 0x%lx 0x1 1", (unsigned long)(&nvt_gpt_hdr_sector));
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s write gpt hdr fail !\n", __func__);
		goto out;
	}

	//write part tbl
	sprintf(cmd, "mmc write 0x%lx 0x2 0x%x", (unsigned long)(&nvt_gpt_part_tbl), sizeof(nvt_gpt_part_tbl) >> BYTE_TO_SECTOR_SFT);
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s write part tbl fail !\n", __func__);
		goto out;
	}

	//write backup tbl
	backup_part_tbl_ofs = dev_max_sectors - GPT_PART_START_SECTOR;
	sprintf(cmd, "mmc write 0x%lx 0x%x 0x%x", (unsigned long)(&nvt_gpt_part_tbl), backup_part_tbl_ofs,
			sizeof(nvt_gpt_part_tbl) >> BYTE_TO_SECTOR_SFT);
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s write back up part tbl fail !\n", __func__);
		goto out;
	}

	//update backup gpt header field
	nvt_gpt_hdr_backup_sector.gpt_hdr.gpt_hdr_crc = 0;
	nvt_gpt_hdr_backup_sector.gpt_hdr.gpt_lba = dev_max_sectors - 1;
	nvt_gpt_hdr_backup_sector.gpt_hdr.backup_gpt_lba = 1;
	nvt_gpt_hdr_backup_sector.gpt_hdr.part_tbl_start_lba = dev_max_sectors - GPT_PART_START_SECTOR;

	nvt_gpt_hdr_backup_sector.gpt_hdr.gpt_hdr_crc  = crc32(0, (const char*)(&(nvt_gpt_hdr_backup_sector.gpt_hdr)), 
			sizeof(nvt_gpt_hdr_backup_sector.gpt_hdr));


	//write backup gpt hdr
	sprintf(cmd, "mmc write 0x%lx 0x%llx 1", (unsigned long)(&nvt_gpt_hdr_backup_sector), 
			nvt_gpt_hdr_backup_sector.gpt_hdr.gpt_lba);
	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s write backup gpt hdr fail !\n", __func__);
		goto out;
	}

out:
	return ret;
}

int nvt_gen_gpt_dram_tbl(struct part_parm *pparm_tbl, struct gpt_dram_part *pgpt_dram_tbl)
{
	unsigned long long dev_max_sectors = 0;
	unsigned long long part_avail_sectors = 0;
	unsigned long long part_start_sector = GPT_PART_START_SECTOR + GPT_1ST_PART_START_PAD;
	int i;

	dev_max_sectors = nvtgpt_get_dev_max_sectors();

	part_avail_sectors = dev_max_sectors - 1 - 2 - ((2 * sizeof(nvt_gpt_part_tbl)) >> BYTE_TO_SECTOR_SFT)  - GPT_1ST_PART_START_PAD;

	for(i=0; i < GPT_ENTRY_COUNT; i++) {
		if(pparm_tbl[i].part_name == NULL)
			break;

		pgpt_dram_tbl[i].part_name = pparm_tbl[i].part_name;
		pgpt_dram_tbl[i].part_start_lba = part_start_sector;
		if(pparm_tbl[i].sectors == 0)
			pparm_tbl[i].sectors = part_avail_sectors;
		part_avail_sectors -= pparm_tbl[i].sectors;
		part_start_sector += pparm_tbl[i].sectors;
		pgpt_dram_tbl[i].part_end_lba = part_start_sector - 1;
	}

	pnvt_gpt_dram_part_tbl = pgpt_dram_tbl;

	return 0;
}

int _nvt_tk_gpt_get_part_info(unsigned char* part_name, unsigned long *pstart_lba, unsigned long *psize_in_sector)
{
	int ret = 0;
	int i;

	*pstart_lba = 0;
	*psize_in_sector = 0;

	for(i=0; i < GPT_ENTRY_COUNT;i++) {
		if(pnvt_gpt_dram_part_tbl[i].part_start_lba == 0)
			break;
		if(strcmp((const char*)part_name, (const char*)(pnvt_gpt_dram_part_tbl[i].part_name)) == 0) {
			*pstart_lba = (unsigned long)pnvt_gpt_dram_part_tbl[i].part_start_lba;
			*psize_in_sector = (unsigned long)(pnvt_gpt_dram_part_tbl[i].part_end_lba - 
				pnvt_gpt_dram_part_tbl[i].part_start_lba);
			break;
		}
	}

	return ret;
}


