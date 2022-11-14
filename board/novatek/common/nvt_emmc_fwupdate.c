/*
 *  board/novatek/evb670b/nvt_emmc_fwupdate.c
 *
 *  Author:	Alvin lin
 *  Created:	May 21, 2013
 *  Copyright:	Novatek Inc.
 *
 */

#include <common.h>
#include <asm-generic/errno.h>
#include <fdt_support.h>
#include <nvt_emmc_fwupdate.h>
#include <nvt_emmc_utils.h>
#include <nvt_emmc_partition.h>
#include <nvt_emmc_xbootdat.h>
#include <nvt_tk_gpt.h>
#include <mmc.h>
#if defined(CONFIG_FASTBOOT_FLASH_MMC_DEV)
#include <fb_mmc.h>
#endif

#include <nvt_tzone.h>
#include <crypto_core.h>
#include <asm/arch/hardware.h>

#define ANDROID_BOOT_HEADER 4


#ifndef CONFIG_SYS_DCACHE_OFF
u32 v7_dcache_get_line_len(void);
#endif

int nvt_emmc_image_help(struct _nvt_emmc_image_info *this, NVT_EMMC_IMG_HELP_TYPE help_type)
{
	int ret = 0;
	int part_id;
	char tmp_str[24] = {0};

	if (this == NULL) {
		ret = -EINVAL;
		printf("%s : invalid image !\n",__func__);
		goto out;
	}

	switch(help_type) {
		case IMG_HELP_READ:
			printf("Usage: eread buffer_addr %s\n",this->image_name);
			break;

		case IMG_HELP_WRITE:
			part_id =  nvt_emmc_get_partition_id_by_name(this->image_name);
			if(part_id == 0)
				sprintf(tmp_str,"%s", "mmcblk0boot0");
			else
				sprintf(tmp_str,"%s%d", "mmcblk0p", part_id);

			printf("Usage: ewrite %s [%s] - %s | %s\n", this->image_name, this->default_name, 
					nvt_emmc_get_partition_name_by_name(this->image_name), tmp_str);

			break;

		case IMG_HELP_ERASE:
			printf("Usage: eerase %s\n",this->image_name);
			break;

		default:
			break;
	}
out:
	return ret;
}

int nvt_emmc_read_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args)
{
	int ret = 0;
	char cmd[128] = {0};
	unsigned int block_cnt = 0;
	unsigned long long start_lba;

	if (this == NULL) {
		ret = -EINVAL;
		printf("%s : invalid image !\n",__func__);
		goto out;
	}

	//For some situation, we don't need read all the partition
	if(args != NULL)
	{
		block_cnt = *(int *)args;
	}
	else
	{
		block_cnt = this->size;
	}

	//switch to boot partition, if needed
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
			ret = run_command("mmc dev 0 1", 0);
			if (ret < 0) {
				printf("%s: switch to boot partition 1 fail !\n",__func__);
				goto out;
			}
			break;

		case PART_TYPE_BOOT2:
			ret = run_command("mmc dev 0 2", 0);
			if (ret < 0) {
				printf("%s: switch to boot partition 2 fail !\n", __func__);
				goto out;
			}
			break;

		default:
			break;
	}

	//read image into buffer
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
		case PART_TYPE_BOOT2:
			sprintf(cmd, "mmc read 0x%lx 0x%x 0x%x", (unsigned long)buf,
					(unsigned int)this->part_offset, block_cnt);
			break;

		case PART_TYPE_BIN:
		default:
			start_lba = nvt_emmc_get_part_start_sector(this->user_part_name);
			sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", (unsigned  long)buf,
					(start_lba + this->part_offset), block_cnt);
			break;
	}

	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("%s read image fail !\n",__func__);
		run_command("mmc dev 0 0", 0);
		*rdlen = 0;
		goto out;
	}

	*rdlen = block_cnt << BYTE_TO_SECTOR_SFT;

	//switch back to user partition, if needed
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
		case PART_TYPE_BOOT2:
			ret = run_command("mmc dev 0 0", 0);
			if (ret < 0) {
				printf("%s: switch to user fail !\n", __func__);
				goto out;
			}
			break;

		default:
			break;
	}

out:
	return ret;
}

int nvt_emmc_read_android_boot_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args)
{
	int ret;
	char cmd[128] = {0};
	unsigned long long start_lba;

	struct boot_address *addr = (struct boot_address *)args;
	struct boot_img_hdr *boot_img_hdr = (struct boot_img_hdr*)addr->kernel_addr;

	unsigned int kernel_size;
	unsigned int ramdisk_size;
	unsigned int ramdisk_offset;
	unsigned int block_cnt;

	start_lba = nvt_emmc_get_part_start_sector(this->user_part_name);
	sprintf(cmd, "mmc read 0x%lx 0x%x 0x%x", (unsigned long)boot_img_hdr,
			(unsigned int)(start_lba + this->part_offset), ANDROID_BOOT_HEADER);

	ret = run_command(cmd, 0);
	if(strncmp ((char*)(boot_img_hdr->magic), "ANDROID!", 8) != 0) {
		printf("%s incorrect android boot.img !\n",__func__);
		ret = -1;
		goto out;
	}

	kernel_size = boot_img_hdr->kernel_size;
	ramdisk_size = boot_img_hdr->ramdisk_size;

	//Move ramdisk to right address
	if(addr->ramdisk_addr != NULL)
	{
		block_cnt = (((kernel_size + boot_img_hdr->page_size - 1) / boot_img_hdr->page_size) + ((ramdisk_size + boot_img_hdr->page_size - 1) / boot_img_hdr->page_size));
		ramdisk_offset = ((kernel_size + boot_img_hdr->page_size - 1) / boot_img_hdr->page_size) * boot_img_hdr->page_size;
	}
	else
	{
		block_cnt = (kernel_size + boot_img_hdr->page_size - 1) / boot_img_hdr->page_size;
	}

	//Normally the page minimum size is 512 bytes, so it is safe now
	block_cnt = block_cnt * (boot_img_hdr->page_size / SECTOR_IN_BYTES);

	sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", (unsigned long)addr->kernel_addr,
			(start_lba + this->part_offset + ANDROID_BOOT_HEADER), block_cnt);

	*rdlen = block_cnt << BYTE_TO_SECTOR_SFT;

	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("%s read image fail !\n",__func__);
		ret = -EIO;
		*rdlen = 0;

		goto out;
	}

	if(addr->ramdisk_addr != NULL)
	{
		/* Align to 4 bytes */
		ramdisk_size = (ramdisk_size + 3) & ~3;
		memmove_4byte_align(addr->ramdisk_addr, addr->kernel_addr + ramdisk_offset, ramdisk_size);
	}

out:

	return ret;

}

int nvt_emmc_read_secos_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int *rdlen, void* args)
{
	int ret = 0;
	char cmd[128] = {0};
	unsigned int Image_blk = 0;
	secos_header_t * header = 0;
	int encrypt = 0;
	unsigned long long start_lba;
	image_header_t *phdr;
	int with_common_header = 0;
	unsigned int secos_sz;

	start_lba = nvt_emmc_get_part_start_sector(this->user_part_name);

	sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", (unsigned long)buf, (start_lba + this->part_offset), 1);

	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("read secos image fail !\n");
		goto out;
	}

	phdr = (image_header_t*)buf;
	if(image_get_magic(phdr) == IH_MAGIC)
		with_common_header = 1;

	if (with_common_header) {
		secos_sz = image_get_size(phdr) + sizeof(image_header_t);
		secos_sz >>= BYTE_TO_SECTOR_SFT;
		secos_sz += 1;
		buf = (unsigned char*)(image_get_load(phdr) - sizeof(image_header_t));
		sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", (unsigned long)buf, this->part_offset + start_lba, secos_sz);

		ret = run_command(cmd, 0);

		if(ret >= 0)
			*rdlen = secos_sz << BYTE_TO_SECTOR_SFT;
		else
			*rdlen = 0;
	} else {

		encrypt = dec_secos_header(SECOS_ENC_BUFFER_BLOCK, SECOS_BUFFER_BLOCK, SECOS_HEADER_MAGIC_CODE);

		header = (secos_header_t * )(SECOS_BUFFER_START - sizeof(secos_header_t));

		if(encrypt == -1) {
			printf("header of secure OS is error !\n");
			ret = -EINVAL;
			goto out;
		} else if (encrypt == 1){ //encrypted secos.bin 
			Image_blk = this->size;
			buf = (unsigned char*)SECOS_ENC_BUFFER_BLOCK;
		} else {//non-encrypted secos.bin
			Image_blk = (header->BinSize + sizeof(secos_header_t)) >> BYTE_TO_SECTOR_SFT;
			buf = (unsigned char*)SECOS_BUFFER_BLOCK;
		}

		sprintf(cmd, "mmc read 0x%lx 0x%llx 0x%x", (unsigned long)buf, this->part_offset + start_lba, Image_blk + 2);

		ret = run_command(cmd, 0);

		if(ret >= 0)
			*rdlen = Image_blk << BYTE_TO_SECTOR_SFT;
		else
			*rdlen = 0;
	}

out:
	return ret;	
}

int nvt_emmc_write_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args)
{
	int ret = 0;
	char cmd[128] = {0};
	unsigned int write_sectors = 0;
	unsigned int offset_blks = 0;
	unsigned long long start_lba;

	if (this == NULL) {
		ret = -EINVAL;
		printf("%s : invalid image !\n",__func__);
		goto out;
	}
	//kwinyee debug
	//printf("ewrite: name:%s start:%d offset:%d size:%d\n",this->image_name,
	//		this->pfs_part->start_sector, this->part_offset,this->size);

	//check buffer size is less than or equal to partition size
	write_sectors = (buf_sz % SECTOR_IN_BYTES) ? ((buf_sz >> BYTE_TO_SECTOR_SFT) + 1) : (buf_sz >> BYTE_TO_SECTOR_SFT);

	if(write_sectors > this->size) {
		printf("%s error : image size(%d blks) large than partition size(%d blks) !\n",__func__,
				write_sectors, this->size);
		ret = -EIO;
		goto out;
	}

	if(args != NULL)
		offset_blks = *((unsigned int*)args);

	//switch to boot partition, if needed
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
			//for emmc boot mode enable
			run_command("mmc rescan", 0);

			ret = run_command("mmc dev 0 1", 0);
			if (ret < 0) {
				printf("%s: switch to boot partition 1 fail !\n", __func__);
				goto out;
			}
			break;

		case PART_TYPE_BOOT2:
			//for emmc boot mode enable
			run_command("mmc rescan", 0);

			ret = run_command("mmc dev 0 2", 0);
			if (ret < 0) {
				printf("%s: switch to boot partition 2 fail !\n", __func__);
				goto out;
			}
			break;

		default:
			break;
	}

	//write image from buffer
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
		case PART_TYPE_BOOT2:
			sprintf(cmd, "mmc write 0x%lx 0x%x 0x%x", (unsigned long)buf,
					this->part_offset + offset_blks, write_sectors);
			break;

		case PART_TYPE_BIN:
		default:
			start_lba = nvt_emmc_get_part_start_sector(this->user_part_name);
			sprintf(cmd, "mmc write 0x%lx 0x%llx 0x%x", (unsigned long)buf, 
					(start_lba + this->part_offset + offset_blks), 
					write_sectors);
			break;
	}
	//printf("emmc write cmd : %s\n", cmd);
	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("%s write image fail !\n",__func__);
		run_command("mmc dev 0 0", 0);
		goto out;
	}


	//switch back to user partition, if needed
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
		case PART_TYPE_BOOT2:
			ret = run_command("mmc dev 0 0", 0);
			if (ret < 0) {
				printf("%s: switch to user fail !\n", __func__);
				goto out;
			}
			break;

		default:
			break;
	}


out:
	return ret;
}

int nvt_emmc_write_img_and_update_bootarg(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args)
{
	int ret = 0;

	ret = nvt_emmc_write_img(this, buf, buf_sz, args);

	if(ret < 0)
		goto out;

	if(strcmp(this->image_name,"ker0") == 0)
		ret = run_command("xsave kercmd", 0);
	else
		ret = run_command("xsave rcvcmd", 0);

out:
	return ret;
}

int nvt_emmc_write_fdt_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args)
{
	int ret = 0;
	unsigned char *fdt_buf = (unsigned char*)CONFIG_SYS_FDT_BUF;
	image_header_t *phdr;
	int with_common_hdr = 0;

	phdr = (image_header_t*)buf;

	if(image_get_magic(phdr) == IH_MAGIC)
		with_common_hdr = 1;

	if(buf != fdt_buf) {
		if(with_common_hdr) {
			memcpy(fdt_buf - sizeof(image_header_t), buf, buf_sz);
			phdr = (image_header_t*)(fdt_buf - sizeof(image_header_t));
		} else
			memcpy(fdt_buf, buf, buf_sz);
	}

	if(strcmp(this->image_name,"fdt0") == 0)
		ret = nvt_prepare_kercmd();
	else
		ret = nvt_prepare_rcvcmd();

	if(ret < 0) 
		printf("%s:read kernel or recovery image fail !, it's OK when run empty burner",__func__);


	if(fdt_check_header(fdt_buf) < 0) {
		printf("%s: Invaild fdt image, run ewrite fdt0 first !\n", __func__);
		ret = -1;
		goto out;
	}

	fdt_chosen(fdt_buf);

#if defined(CONFIG_ARM64)	
	ret = nvt_set_fdt_ker_mem_range(fdt_buf);
	if(ret != 0)
		printf("%s set kernel memory range fail !\n", __func__);
#endif

	if(with_common_hdr) {
		//recalc crc32
		phdr->ih_size = cpu_to_be32(fdt_totalsize(fdt_buf));
		phdr->ih_dcrc = cpu_to_be32(crc32(0, fdt_buf, be32_to_cpu(phdr->ih_size)));
		phdr->ih_hcrc = 0;
		phdr->ih_hcrc = cpu_to_be32(crc32(0, (unsigned char*)phdr, sizeof(image_header_t)));
		ret = nvt_emmc_write_img(this, fdt_buf - sizeof(image_header_t), fdt_totalsize(fdt_buf) + sizeof(image_header_t), args);
	} else {
		ret = nvt_emmc_write_img(this, fdt_buf, fdt_totalsize(fdt_buf), args);
	}
out:
	return ret;
}

#if defined(CONFIG_FASTBOOT_FLASH_MMC_DEV)
int nvt_emmc_write_sparse_img(struct _nvt_emmc_image_info *this, unsigned char *buf, unsigned int buf_sz, void *args)
{
	int ret = 0;

	fb_mmc_flash_write(this->user_part_name, buf, buf_sz);
out:
	return ret;
}

void fastboot_fail(const char *reason)
{
	printf("flash image fail : %s\n", reason);
}

void fastboot_okay(const char *reason)
{
	printf("flash image okay : %s\n", reason);
}
#endif

int nvt_emmc_erase_img(struct _nvt_emmc_image_info *this, void* args)
{
	int ret = 0;
	char cmd[128] = {0};
	unsigned long long start_lba;

	if (this == NULL) {
		ret = -EINVAL;
		printf("%s : invalid image !\n",__func__);
		goto out;
	}

	if (this->part_type == PART_TYPE_FS) {
		ret = -EINVAL;
		printf("DO NOT use eerase command to erase file system partition, try emark_format_db instead !\n");
		goto out;
	}

	//switch to boot partition, if needed
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
			ret = run_command("mmc dev 0 1", 0);
			if (ret < 0) {
				printf("%s: switch to boot partition 1 fail !\n", __func__);
				goto out;
			}
			break;

		case PART_TYPE_BOOT2:
			ret = run_command("mmc dev 0 2", 0);
			if (ret < 0) {
				printf("%s: switch to boot partition 2 fail !\n", __func__);
				goto out;
			}
			break;

		default:
			break;
	}

	//erase partition
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
		case PART_TYPE_BOOT2:
			memset((void*)CONFIG_SYS_FWUPDATE_BUF, 0x0, (this->size) << BYTE_TO_SECTOR_SFT);
			sprintf(cmd, "mmc write 0x%x 0x%x 0x%x", CONFIG_SYS_FWUPDATE_BUF, this->part_offset,
					this->size);
			break;
		case PART_TYPE_BIN:
			memset((void*)CONFIG_SYS_FWUPDATE_BUF, 0x0, (this->size) << BYTE_TO_SECTOR_SFT);
			start_lba = nvt_emmc_get_part_start_sector(this->user_part_name);
			sprintf(cmd, "mmc write 0x%x 0x%llx 0x%x", CONFIG_SYS_FWUPDATE_BUF,
					(start_lba + this->part_offset),
					this->size);
			break;

		default:
			memset((void*)CONFIG_SYS_FWUPDATE_BUF, 0x0, (FS_ERASE_BLK_CNT << BYTE_TO_SECTOR_SFT));
			start_lba = nvt_emmc_get_part_start_sector(this->user_part_name);
			sprintf(cmd, "mmc write 0x%x 0x%llx 0x%x", CONFIG_SYS_FWUPDATE_BUF,
					(start_lba + this->part_offset), (FS_ERASE_BLK_CNT));
	}

	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("%s erase image fail !\n",__func__);
		run_command("mmc dev 0 0", 0);
		goto out;
	}

	//switch back to user partition, if needed
	switch(this->part_type) {
		case PART_TYPE_BOOT1:
		case PART_TYPE_BOOT2:
			ret = run_command("mmc dev 0 0", 0);
			if (ret < 0) {
				printf("%s: switch to user fail !\n", __func__);
				goto out;
			}
			break;

		default:
			break;
	}


out:
	return ret;
}

#define NVT_FWUPDATE_MAX_WRITE_BYTES 		(CONFIG_NVT_FW_UPDATE_BUF_MAX * 1024 *1024)
#define NVT_FWUPDATE_MAX_WRITE_BLKS 		(NVT_FWUPDATE_MAX_WRITE_BYTES >> BYTE_TO_SECTOR_SFT)

int do_nvt_emmc_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	char cmd[128] = {0};
	char img_name[64] = {0};
	unsigned int flen;
	unsigned int read_blks;
	unsigned int read_len;
	int cnt,i;
	nvt_emmc_image_info *pimg;

	if(argc != 2 && argc != 3)
		return CMD_RET_USAGE;

	cnt = nvt_emmc_get_image_cnt();

	if(strcmp(argv[1],"help") == 0) {
		for(i=0;i < cnt;i++) {
			pimg = (nvt_emmc_image_info*)nvt_emmc_get_img_by_index(i);
			if(pimg->image_help != NULL)
				pimg->image_help(pimg, IMG_HELP_WRITE);
		}

		goto out;
	}

	pimg = (nvt_emmc_image_info*)nvt_emmc_get_img_by_name(argv[1]);

	if(pimg == NULL) {
		printf("%s: unknown image name %s\n",__func__,argv[1]);
		ret = -EINVAL;
		goto out;
	}

	if(argc == 3)
		strcpy(img_name, argv[2]);
	else
		strcpy(img_name, pimg->default_name);

	ret = run_command("usb start", 0);
	if(ret != 0) {
		printf("%s init usb fail !\n",__func__);
		ret = -EIO;
		goto out;
	}

	read_blks = 0;

	//initial partition table first be write emmc to get correct offset	
	nvt_emmc_part_init();

	do {
		sprintf(cmd, "fatload usb 0:auto 0x%x %s 0x%x 0x%x", CONFIG_SYS_FWUPDATE_BUF,
				 img_name, NVT_FWUPDATE_MAX_WRITE_BYTES, (read_blks << BYTE_TO_SECTOR_SFT));
		//printf("ewrite fatcmd: %s\n", cmd);
		ret = run_command(cmd, 0);
		if(ret != 0) {
			printf("read image file %s error !\n", img_name);
			ret = -EIO;
			goto out;
		}

		flen = simple_strtoul(env_get("filesize"), NULL, 16);

		if(pimg->size < (read_blks + (flen >> BYTE_TO_SECTOR_SFT))) {
			printf("%s image size large than partition size, update fail !\n", img_name);
			ret = -EINVAL;
			goto out;
		}

		if(pimg->write_img != NULL)
			ret = pimg->write_img(pimg, (unsigned char*)CONFIG_SYS_FWUPDATE_BUF, flen, &read_blks);

		if(ret != 0) {
			printf("write image file %s error !\n", img_name);
			ret = -EIO;
			goto out;
		}

		if(flen < NVT_FWUPDATE_MAX_WRITE_BYTES)
			break;
		else
			read_blks += NVT_FWUPDATE_MAX_WRITE_BLKS;
	}while(1);

out:
	return ret;
}

U_BOOT_CMD(
	ewrite, 3, 1, do_nvt_emmc_write,
	"ewrite - write image to emmc partition",
	"       use \"ewrite help\" to get more information\n"
);


int do_nvt_emmc_erase(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	int ret = 0;
	int cnt,i;
	nvt_emmc_image_info *pimg;

	if(argc != 2)
		return CMD_RET_USAGE;

	cnt = nvt_emmc_get_image_cnt();

	if(strcmp(argv[1],"help") == 0) {
		for(i=0;i < cnt;i++) {
			pimg = (nvt_emmc_image_info*)nvt_emmc_get_img_by_index(i);
			if(pimg->image_help != NULL)
				pimg->image_help(pimg, IMG_HELP_ERASE);
		}

		goto out;
	}

	pimg = nvt_emmc_get_img_by_name(argv[1]);

	if(pimg == NULL) {
		printf("%s: unknown image name %s\n",__func__,argv[1]);
		ret = -EINVAL;
		goto out;
	}

	if(pimg->erase_img != NULL)
		ret = pimg->erase_img(pimg, NULL);

out:
	return ret;
}

U_BOOT_CMD(
	eerase, 2, 1, do_nvt_emmc_erase,
	"eerase - erase image from emmc partition",
	"       use \"eerase help\" to get more information\n"
);

int do_nvt_emmc_fast_erase(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	int ret = 0;
	char cmd[128] = {};
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_SYS_DEV);
	int boot_sector_cnt = (int)(mmc->capacity_boot / EMMC_SECTOR_SIZE);

	run_command("mmc erase0all", 0);

	printf("erase boot0....\n");
	run_command("mmc dev 0 1", 0);
	memset((void*)CONFIG_SYS_FWUPDATE_BUF, 0, (size_t)mmc->capacity_boot);
	sprintf(cmd, "mmc write 0x%x 0 0x%x", CONFIG_SYS_FWUPDATE_BUF, boot_sector_cnt);
	run_command(cmd, 0);

	run_command("mmc dev 0 2", 0);
	printf("erase boot1.....\n");
	run_command(cmd, 0);

	run_command("mmc dev 0 0", 0);
	printf("erase data.....\n");

	run_command("mmc en_egroup", 0);
	run_command("mmc rescan", 0);

	sprintf(cmd, "mmc erase 0 0x" LBAF, nvtgpt_get_dev_max_sectors());
	ret = run_command(cmd, 0);

	run_command("mmc dis_egroup", 0);
	run_command("mmc rescan", 0);

	return ret;
}

U_BOOT_CMD(
	efast_erase, 1, 0, do_nvt_emmc_fast_erase,
	"efast_erase - use mmc erase command to erase whole emmc data partition",
	""
);

int do_nvt_emmc_erase_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, cnt;

	if(argc != 2)
		return CMD_RET_USAGE;

	cnt = simple_strtoul(argv[1], NULL, 10);

	for(i=0;i < cnt;i++) {
		printf("==============erase count %d ================\n", (i + 1));
		run_command("efast_erase ", 0);
	}

	printf("erase loop finished !\n");

	return 0;
}

U_BOOT_CMD(
	eerase_loop, 2, 1, do_nvt_emmc_erase_loop,
	"eerase_loop - erase loop times",
	""
);

#if defined(CONFIG_EMMC_WRITE_RELIABILITY_TEST)


wr_header_t wr_test_entry;
int do_nvt_emmc_write_rel_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	int ret = 0;
	char cmd[128] = {};
	void *addr;
	unsigned char *write_pattern = (unsigned char *)CONFIG_SYS_FWUPDATE_BUF;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_SYS_DEV);

	run_command("mmc dev 0 0", 0);
        memset(&wr_test_entry,0x0, sizeof(wr_header_t));
        sprintf(cmd, "mmc read 0x%x 0x%x 0x%x",CONFIG_SYS_FWUPDATE_BUF,WR_HEADER_ADDR,1);
        ret = run_command(cmd, 0);
        memcpy( &wr_test_entry,(unsigned char*)CONFIG_SYS_FWUPDATE_BUF, sizeof(wr_header_t));
        printf("This eMMC write reliability test %d times\n", wr_test_entry.wr_test_cnt);
        mmc->wr_test_cnt = wr_test_entry.wr_test_cnt;
        mmc->wr_total_test_cnt = wr_test_entry.wr_total_test_cnt;
        wr_test_entry.wr_test_cnt++;
        memcpy((unsigned char*)CONFIG_SYS_FWUPDATE_BUF, &wr_test_entry, sizeof(wr_header_t));
        sprintf(cmd, "mmc write 0x%x 0x%x 0x%x",CONFIG_SYS_FWUPDATE_BUF, (wr_test_entry.wr_header_start),1);
        ret = run_command(cmd, 0);

	addr = write_pattern;
	memset(write_pattern, 0x5a5aa5a5,WR_PAT_SIZE);

	run_command("mmc dev 0 2", 0);
        mmc->en_wr = true;
	sprintf(cmd, "mmc write 0x%x 0x%x 0x%x", addr,WR_PAT_SIZE/512*3, WR_PAT_SIZE/512);
	ret = run_command(cmd, 0);

	return ret;
}


int do_nvt_emmc_write_rel_test_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    int ret = 0;
    char cmd[128] = {};
    u32 blk, cnt, i,j;
    struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_SYS_DEV);
    int boot_sector_cnt = (int)(mmc->capacity_boot / EMMC_SECTOR_SIZE);
    unsigned char *write_pattern = (unsigned char *)CONFIG_SYS_FWUPDATE_BUF;
    memset(&wr_test_entry,0x0, sizeof(wr_header_t));
    wr_test_entry.wr_start = 0 ;
    wr_test_entry.wr_size = mmc->capacity_boot - WR_PAT_SIZE;
    wr_test_entry.wr_test_pat_start = boot_sector_cnt - WR_PAT_SIZE/EMMC_SECTOR_SIZE;
    wr_test_entry.wr_test_pat_size = WR_PAT_SIZE;
    wr_test_entry.wr_header_start =  WR_HEADER_ADDR;//100MB
    if(mmc->wr_total_test_cnt != 0)
        wr_test_entry.wr_total_test_cnt = mmc->wr_total_test_cnt;

#if defined(CONFIG_EMMC_WRITE_RELIABILITY_DBG)
    printf("This eMMC write reliability test header before \n");
    printf("wr_test_cnt: %d\n times", wr_test_entry.wr_test_cnt);
    printf("wr_dcrc: %x\n", wr_test_entry.wr_dcrc);
    printf("wr_pcrc: %x \n", wr_test_entry.wr_pcrc);
    printf("wr_wr_start: %x sectors\n", wr_test_entry.wr_start);
    printf("wr_size: %d bytes\n", wr_test_entry.wr_size);
    printf("wr_test_pat_start: %x sectors\n", wr_test_entry.wr_test_pat_start);
    printf("wr_test_pat_size: %d bytes\n", wr_test_entry.wr_test_pat_size);
    printf("wr_header_start: %x sectors\n", wr_test_entry.wr_header_start);
#endif

    run_command("mmc dev 0 2", 0);

    if(check_write_reliability_setting() == 0)
    {
        printf("This eMMC need to configure for Write Reliability \n");
        return 1;
    }
    printf("This eMMC initialization for Write Reliability \n");

    printf("reset mmc test data to 0x0.....\n");
    run_command("mmc erase0boot", 0);
    sprintf(cmd, "mmc read 0x%x 0x%x 0x%x",CONFIG_SYS_FWUPDATE_BUF,0, boot_sector_cnt);
    ret = run_command(cmd, 0);
    wr_test_entry.wr_dcrc = cpu_to_be32(crc32(0, (unsigned char*)CONFIG_SYS_FWUPDATE_BUF, be32_to_cpu(wr_test_entry.wr_size)));
    wr_test_entry.wr_pcrc = cpu_to_be32(crc32(0, (unsigned char*)(CONFIG_SYS_FWUPDATE_BUF + wr_test_entry.wr_size), be32_to_cpu(wr_test_entry.wr_test_pat_size)));

    run_command("mmc dev 0 0", 0);
    memcpy((unsigned char*)CONFIG_SYS_FWUPDATE_BUF, &wr_test_entry, sizeof(wr_header_t));
    sprintf(cmd, "mmc write 0x%x 0x%x 0x%x",CONFIG_SYS_FWUPDATE_BUF, (wr_test_entry.wr_header_start),1);
    ret = run_command(cmd, 0);

#if defined(CONFIG_EMMC_WRITE_RELIABILITY_DBG)
    printf("This eMMC write reliability test header after \n");
    printf("wr_test_cnt: %d\n times", wr_test_entry.wr_test_cnt);
    printf("wr_dcrc: %x\n", wr_test_entry.wr_dcrc);
    printf("wr_pcrc: %x \n", wr_test_entry.wr_pcrc);
    printf("wr_wr_start: %x sectors\n", wr_test_entry.wr_start);
    printf("wr_size: %d bytes\n", wr_test_entry.wr_size);
    printf("wr_test_pat_start: %x sectors\n", wr_test_entry.wr_test_pat_start);
    printf("wr_test_pat_size: %d bytes\n", wr_test_entry.wr_test_pat_size);
    printf("wr_header_start: %x sectors\n", wr_test_entry.wr_header_start);
#endif



    return ret;
}

int do_nvt_emmc_write_rel_test_check(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	int ret = 0;
	char cmd[128] = {};
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_SYS_DEV);
	int boot_sector_cnt = (int)(mmc->capacity_boot / EMMC_SECTOR_SIZE);
	__be32		wr_dcrc;	/* Image Data CRC Checksum	*/

	run_command("mmc dev 0 0", 0);
        memset(&wr_test_entry,0x0, sizeof(wr_header_t));
        sprintf(cmd, "mmc read 0x%x 0x%x 0x%x",CONFIG_SYS_FWUPDATE_BUF,WR_HEADER_ADDR,1);
        printf("cmd=%s\n", cmd);
        ret = run_command(cmd, 0);
        memcpy( &wr_test_entry,(unsigned char*)CONFIG_SYS_FWUPDATE_BUF, sizeof(wr_header_t));
#if defined(CONFIG_EMMC_WRITE_RELIABILITY_DBG)
    printf("This eMMC write reliability test header read back \n");
    printf("wr_test_cnt: %d times\n", wr_test_entry.wr_test_cnt);
    printf("wr_dcrc: %x\n", wr_test_entry.wr_dcrc);
    printf("wr_pcrc: %x \n", wr_test_entry.wr_pcrc);
    printf("wr_wr_start: %x sectors\n", wr_test_entry.wr_start);
    printf("wr_size: %d bytes\n", wr_test_entry.wr_size);
    printf("wr_test_pat_start: %x sectors\n", wr_test_entry.wr_test_pat_start);
    printf("wr_test_pat_size: %d bytes\n", wr_test_entry.wr_test_pat_size);
    printf("wr_header_start: %x sectors\n", wr_test_entry.wr_header_start);
#endif

    run_command("mmc dev 0 2", 0);
    sprintf(cmd, "mmc read 0x%x 0x%x 0x%x",CONFIG_SYS_FWUPDATE_BUF,0, boot_sector_cnt);
    ret = run_command(cmd, 0);
    wr_dcrc = cpu_to_be32(crc32(0, (unsigned char*)CONFIG_SYS_FWUPDATE_BUF, be32_to_cpu(wr_test_entry.wr_size)));

    printf("\n\n========This eMMC Write Reliability Test Result========\n");
    printf("This eMMC write reliability tatal test %d times\n", wr_test_entry.wr_test_cnt);
    if(wr_test_entry.wr_dcrc == wr_dcrc) {
        printf("eMMC Write Reliability Test Pass\n");
        printf("=======================================================\n");
    } else {
        printf("eMMC Write Reliability Test Fail\n");
        printf("=======================================================\n");
        while(1);
    }

	return ret;
}

int do_nvt_emmc_write_rel_test_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, cnt;

	if(argc != 2)
		return CMD_RET_USAGE;

	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_SYS_DEV);
	mmc->wr_total_test_cnt = simple_strtoul(argv[1], NULL, 10);

	run_command("mmc_wr_init", 0);

        printf("This eMMC initialization for Write Reliability Done\n");
	run_command("reboot", 0);

	return 0;
}
U_BOOT_CMD(
	mmc_wr_init, 2, 1, do_nvt_emmc_write_rel_test_init,
	"mmc_wr_init - emmc initialization for writereliability test",
	""
);
U_BOOT_CMD(
	mmc_wr_test, 2, 1, do_nvt_emmc_write_rel_test,
	"mmc_wr_test - emmc write reliability test",
	""
);
U_BOOT_CMD(
	mmc_wr_check, 2, 1, do_nvt_emmc_write_rel_test_check,
	"mmc_wr_check - emmc write reliability test check",
	""
);
U_BOOT_CMD(
	mmc_wr_test_loop, 2, 1, do_nvt_emmc_write_rel_test_loop,
	"mmc_wr_test_loop - emmc write reliability test loop times",
	""
);

#endif


int do_nvt_emmc_get_ddrinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	char cmd[128] = {0};
	char *out_fname;
	unsigned int ddr_len;
	nvt_emmc_image_info *pimg;


	pimg = nvt_emmc_get_img_by_name("ddrcfg");

	if(pimg == NULL || pimg->read_img == NULL) {
		ret = -EINVAL;
		printf("ddr config partition not exist !\n");
		goto out;
	}

	ret = pimg->read_img(pimg, (unsigned char*)CONFIG_SYS_FWUPDATE_BUF, &ddr_len, NULL);

	if(ret < 0) {
		ret = -EIO;
		printf("read ddr config from emmc error !\n");
		goto out;
	}


	if(argc >= 2)
		out_fname = argv[1];
	else
		out_fname = "658_ddr.ddr";

	ret = run_command("usb reset\0",0);

	if(ret != 0) {
		printf("usb storage not ready !\n");
		goto out;
	}

	sprintf(cmd,"fatwrite usb 0:1 0x%x %s 0x%x",CONFIG_SYS_FWUPDATE_BUF, out_fname, ddr_len);

	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("write ddr config to usb fail !\n");
		goto out;
	}

out:
	return ret;
}

U_BOOT_CMD(
	egetddr, 2, 1, do_nvt_emmc_get_ddrinfo,
	"egetddr - get ddr setting from board",
	"      fname - add fname parameter to specify saved file name, or else default name is 658_ddr.ddr \n"
	);

int do_nvt_emmc_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	int ret = 0;
	int i,cnt;
	nvt_emmc_image_info *pimg;
	unsigned char *addr;
	unsigned int rdlen;

	if(argc != 2 && argc !=  3)
		return CMD_RET_USAGE;

	cnt = nvt_emmc_get_image_cnt();

	if(strcmp(argv[1],"help") == 0) {
		for(i=0;i < cnt;i++) {
			pimg = (nvt_emmc_image_info*)nvt_emmc_get_img_by_index(i);
			if(pimg->image_help != NULL)
				pimg->image_help(pimg, IMG_HELP_READ);
		}

		goto out;
	}

	addr = (unsigned char*)simple_strtoul(argv[1],NULL,16);

	pimg = nvt_emmc_get_img_by_name(argv[2]);

	if(pimg == NULL) {
		printf("%s invalid image name : %s\n",__func__,argv[2]);
		ret = -EINVAL;
		goto out;
	}

	if(pimg->read_img != NULL)
		ret = pimg->read_img(pimg, addr, &rdlen, NULL);

	if (ret != 0) {
		printf("eread image error !\n");
		goto out;
	} else
		printf("image read to 0x%lx, len = 0x%x\n",(unsigned long)addr, rdlen);

out:
	return ret;
}

U_BOOT_CMD(
	eread, 3, 1, do_nvt_emmc_read,
	"eread - read image from emmc partition to dram",
	"       use \"eread help\" to get more information\n"
);

int do_nvt_emmc_miscmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	//unsigned int len;
	//char cmd[128] = {0};
	unsigned char *ker_load_addr = NULL;
	unsigned char *ramdisk_load_addr = NULL;
	nvt_emmc_image_info *pimg;

	/* eMMC write buffer must aligned in 64 bytes */
	struct bootloader_message __attribute__((__aligned__(64)))  misc_info ;

	if(argc != 2)
		return CMD_RET_USAGE;

	memset(&misc_info, 0, sizeof(struct bootloader_message));
	strncpy(misc_info.command, argv[1], strlen(argv[1]));

	pimg = nvt_emmc_get_img_by_name("misc");

	if(pimg == NULL) {
		printf("%s get misc partition fail !\n",__func__);
		return -EINVAL;
	}

	ret = pimg->write_img(pimg, (unsigned char*)(&misc_info), sizeof(struct bootloader_message), NULL);


	return ret;
}

U_BOOT_CMD(
	emiscmd, 3, 1, do_nvt_emmc_miscmd,
	"emiscmd - write misc command to misc partition",
	""
);


#if defined(CONFIG_NVT_TURNKEY_FWUPDATE)

int nt72658_read_update_flag(update_flag *pupdate)
{
	int ret = 0;
	unsigned int rdlen;
	nvt_emmc_image_info *pimg;

	if(pupdate == NULL) {
		ret = -1;
		goto out;
	}

	pimg = nvt_emmc_get_img_by_name("uflag");
	if(pimg == NULL) {
		ret = -1;
		goto out;
	}

	ret = pimg->read_img(pimg, (unsigned char*)pupdate, &rdlen, NULL);

	if(ret < 0)
		goto out;

	if(memcmp(pupdate->sign, NVTFW_UPDATE_FLAG_SIGN, 4)) {
		ret = -1;
		goto out;
	}

out:
	return ret;
}

int nt72658_write_update_flag(update_flag *pupdate)
{
	int ret = 0;
	unsigned int rdlen;
	nvt_emmc_image_info *pimg;

	if(pupdate == NULL) {
		ret = -1;
		goto out;
	}

	pimg = nvt_emmc_get_img_by_name("uflag");
	if(pimg == NULL) {
		ret = -1;
		goto out;
	}

	ret = pimg->write_img(pimg, (unsigned char*)pupdate, sizeof(update_flag), NULL);

	if(ret < 0)
		goto out;

out:
	return ret;
}


int nt72658_read_boot_flag(boot_flag *pboot)
{
	int ret = 0;
	unsigned int rdlen;
	nvt_emmc_image_info *pimg;

	if(pboot == NULL) {
		ret = -1;
		goto out;
	}

	pimg = nvt_emmc_get_img_by_name("bflag");
	if(pimg == NULL) {
		ret = -1;
		goto out;
	}

	ret = pimg->read_img(pimg, (unsigned char*)pboot, &rdlen, NULL);

	if(ret < 0)
		goto out;

	if(memcmp(pboot->sign, NVTFW_BOOT_FLAG_SIGN, 4)) {
		ret = -1;
		goto out;
	}

	if(pboot->kernel_idx != 0 && pboot->kernel_idx != 1) {
		ret = -1;
		goto out;
	}

	if(pboot->fs_idx != 0 && pboot->fs_idx != 1) {
		ret = -1;
		goto out;
	}

	if(pboot->ppcache_idx != 0 && pboot->ppcache_idx != 1) {
		ret = -1;
		goto out;
	}
out:
	return ret;
}

int nt72658_write_boot_flag(boot_flag *pboot)
{
	int ret = 0;
	unsigned int rdlen;
	nvt_emmc_image_info *pimg;

	if(pboot == NULL) {
		ret = -1;
		goto out;
	}

	pimg = nvt_emmc_get_img_by_name("bflag");
	if(pimg == NULL) {
		ret = -1;
		goto out;
	}

	ret = pimg->write_img(pimg, (unsigned char*)pboot, sizeof(boot_flag), NULL);

	if(ret < 0)
		goto out;

out:
	return ret;
}

int do_nvt_read_update_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	update_flag __attribute__((__aligned__(64)))  uflag = {};
	int ret = 0;

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nt72658_read_update_flag(&uflag);

	if(ret != 0) {
		printf("update flag not exist !\n");
		ret = 0;
		goto out;
	}

	if(memcmp(uflag.sign, NVTFW_UPDATE_FLAG_SIGN, strlen(NVTFW_UPDATE_FLAG_SIGN)) != 0)
		printf("update flag not exist !\n");
	else
		printf("update flag exist !\n");
out:
	return ret;
}
U_BOOT_CMD(
	eread_update_flag, 1, 1, do_nvt_read_update_flag,
	"eread_update_flag - read update flag from misc partition",
	""
);


int do_nvt_write_update_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	update_flag __attribute__((__aligned__(64)))  uflag = {};
	int ret = 0;

	memcpy(uflag.sign, NVTFW_UPDATE_FLAG_SIGN, strlen(NVTFW_UPDATE_FLAG_SIGN));

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nt72658_write_update_flag(&uflag);

	if(ret == 0)
		printf("write update flag done !\n");
	else
		printf("write update flag fail !\n");
out:
	return ret;
}

U_BOOT_CMD(
	ewrite_update_flag, 1, 1, do_nvt_write_update_flag,
	"ewrite_update_flag - write update flag to misc partition",
	""
);

int do_nvt_erase_update_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	update_flag __attribute__((__aligned__(64)))  uflag = {};
	int ret = 0;


	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nt72658_write_update_flag(&uflag);

	if(ret == 0)
		printf("erase update flag done !\n");
	else
		printf("erase update flag fail !\n");
out:
	return ret;
}

U_BOOT_CMD(
	eerase_update_flag, 1, 1, do_nvt_erase_update_flag,
	"eerase_update_flag - erase update flag to misc partition",
	""
);


int do_nvt_write_boot_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	boot_flag __attribute__((__aligned__(64)))  bflag = {};
	int ret = 0;
	int kernel_idx,fs_idx,ppcache_idx;


	if(argc != 4)
		return CMD_RET_USAGE;

	kernel_idx = simple_strtoul(argv[1], NULL, 10);
	fs_idx = simple_strtoul(argv[2], NULL, 10);
	ppcache_idx = simple_strtoul(argv[3], NULL, 10);
	memcpy(bflag.sign, NVTFW_BOOT_FLAG_SIGN, strlen(NVTFW_BOOT_FLAG_SIGN));

	if (kernel_idx == 0)
		bflag.kernel_idx = 0;
	else
		bflag.kernel_idx = 1;

	if (fs_idx == 0)
		bflag.fs_idx = 0;
	else
		bflag.fs_idx = 1;

	if (ppcache_idx == 0)
		bflag.ppcache_idx = 0;
	else
		bflag.ppcache_idx = 1;


	ret = nt72658_write_boot_flag(&bflag);
	if(ret == 0)
		printf("write boot flag done !\n");
	else
		printf("write boot flag fail !\n");
	return ret;
}

U_BOOT_CMD(
	ewrite_boot_flag, 4, 1, do_nvt_write_boot_flag,
	"ewrite_boot_flag - write boot flag to misc partition",
	"ewrite_boot_flag kernel_idx fs_idx ppcache_idx"
);


int do_nvt_read_boot_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	boot_flag __attribute__((__aligned__(64)))  bflag = {};
	int ret = 0;

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nt72658_read_boot_flag(&bflag);

	if(ret == 0)
		printf("boot flag - kernel_idx:%d fs_idx:%d ppcache_idx:%d\n", bflag.kernel_idx, 
				bflag.fs_idx, bflag.ppcache_idx);
	else {
		printf("boot flag not exist !\n");
		ret = 0;
	}

	return ret;
}

U_BOOT_CMD(
	eread_boot_flag, 1, 1, do_nvt_read_boot_flag,
	"eread_boot_flag - read boot flag from misc partition",
	""
);

int do_nvt_fix_bootargs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	boot_flag __attribute__((__aligned__(64)))  bflag = {};
	int ret = 0;

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nt72658_read_boot_flag(&bflag);

	if(ret < 0) {
		printf("%s read boot flag fail !\n", __func__);
		goto out;
	}

	if(bflag.fs_idx == 0)
		sprintf(tmp_str, "/dev/mmcblk0p%d", nvt_emmc_get_partition_id_by_name("fs0"));
	else
		sprintf(tmp_str, "/dev/mmcblk0p%d", nvt_emmc_get_partition_id_by_name("fs1"));

	env_set("usb", tmp_str);

	if(bflag.kernel_idx == 0)
		run_command("xsave kercmd", 0);
	else
		run_command("xsave rcvcmd", 0);

out:

	return ret;
}

U_BOOT_CMD(
	efix_bootargs, 1, 1, do_nvt_fix_bootargs,
	"efix_bootargs - fix ftd bootargs depend on boot flag",
	""
);

static void nvt_show_pptbl(pp_cache_tbl *ptbl)
{
	int i, crc_val, crc_cal;

	if(ptbl == NULL)
		return;

	if(memcmp(ptbl->sign, NVTFW_PPCACHE_SIGN, strlen(NVTFW_PPCACHE_SIGN)) == 0) {
		crc_val = ptbl->crc32;
		ptbl->crc32 = 0;
		crc_cal = crc32(0, (const unsigned char *)ptbl, sizeof(pp_cache_tbl));
		if(crc_val != crc_cal)
			goto out;

		printf("========================================================\n");

		for(i=0;i < NUM_OF_PPCACHE_ENTRY;i++) {
			if(strlen(ptbl->entrys[i].name) <= 0)
				break;
			printf("------------------------------------------------\n");
			printf("entry[%d].name = %s\n", i, ptbl->entrys[i].name);
			printf("entry[%d].path = %s\n", i, ptbl->entrys[i].path);
			printf("entry[%d].offset = %d\n", i, ptbl->entrys[i].offset);
			printf("entry[%d].index = %d\n", i, ptbl->entrys[i].index);
		}
	}

	printf("========================================================\n");
out:
	return;
}

int do_nvt_dump_pp_table(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	pp_cache_tbl __attribute__((__aligned__(64)))  pp_tbl = {};
	nvt_emmc_image_info *pimg;
	int rdlen;
	int ret = 0;

	if(argc != 1)
		return CMD_RET_USAGE;

	pimg = nvt_emmc_get_img_by_name("pptbl0");

	if(pimg == NULL) {
		printf("You need setup pptbls in your partition first !\n");
		goto out;
	}

	ret = pimg->read_img(pimg, (unsigned char*)(&pp_tbl), &rdlen, NULL);
	if(ret != 0) {
		ret = 0;
		printf("%s read pptbl0 fail !\n", __func__);
		goto out;
	}

	printf("pp table 0 : \n");
	nvt_show_pptbl(&pp_tbl);

	memset(&pp_tbl, 0, sizeof(pp_cache_tbl));

	pimg = nvt_emmc_get_img_by_name("pptbl1");

	if(pimg == NULL) {
		printf("You need setup pptbls in your partition first !\n");
		goto out;
	}

	ret = pimg->read_img(pimg, (unsigned char*)(&pp_tbl), &rdlen, NULL);
	if(ret != 0) {
		ret = 0;
		printf("%s read pptbl1 fail !\n", __func__);
		goto out;
	}

	printf("pp table 1 : \n");
	nvt_show_pptbl(&pp_tbl);


out:
	return ret;
}

U_BOOT_CMD(
	edump_pptbl, 1, 1, do_nvt_dump_pp_table,
	"edump_pptbl - dump pp table from misc partition",
	""
);


#endif

void nvt_setup_emmc_boot_config(void)
{
	u32 reg_val_1_to_20, reg_val0, reg_bs, tmp_val;
	u32 ack, bus_width, latch;
	char cmd[128] = {0};

	//BS0
	reg_val0 = __REG(0xfd101800);
	reg_val0 = ((reg_val0 ^ 0x1) & 0x1);


	//BS1-20
	reg_val_1_to_20 = (__REG(0xfc040700) ^ ((1 << 5) | (1 << 19))) & 0x1ffffe;

	reg_bs = (reg_val0 | reg_val_1_to_20);

	//set ack mode
	tmp_val = reg_bs >> 15;
	tmp_val &= 1;

	if(tmp_val == 0)
		//boot mode with ack
		ack = 1;
	else
		//no boot ack
		ack = 0;

	//set bus width
	tmp_val = reg_bs >> 13;
	tmp_val &= 3;
	switch(tmp_val) {
		case 0:
		case 3:
			//x8
			bus_width = 2;
			break;

		case 1:
			//x4
			bus_width = 1;
			break;

		case 2:
			//x1
			bus_width = 0;
			break;
	}

	//set latch mode
	tmp_val = reg_bs >> 17;
	tmp_val &= 1;

	if(tmp_val == 0)
		//sdr
		latch = 0;
	else
		//ddr
		latch = 2;

	sprintf(cmd, "mmc partconf 0 %d 1 0",ack);
	run_command(cmd, 0);

	sprintf(cmd, "mmc bootbus 0 %d 0 %d", bus_width, latch);
	run_command(cmd, 0);
}

enum {
	STORAGE_NONE,
	STORAGE_NAND,
	STORAGE_EMMC,
};

static int do_nvt_bootstrap_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	u32 reg_val_1_to_20, reg_val0, reg_bs;
	int i,tmp_val;
	int storage_type = STORAGE_NONE;

	//BS0
	reg_val0 = __REG(0xfd101800);
	reg_val0 = ((reg_val0 ^ 0x1) & 0x1);


	//BS1-20
	reg_val_1_to_20 = (__REG(0xfc040700) ^ ((1 << 5) | (1 << 19))) & 0x1ffffe;

	reg_bs = (reg_val0 | reg_val_1_to_20);


	for(i = 0;i <= 20;i++) {
		printf("BS%d : %01x\n",i,(reg_bs & 0x1));
		reg_bs >>= 1;
	}

	//BS0
	tmp_val = reg_bs & 0x1;
	if(tmp_val & 0x1)
		printf("Bypass CPU MaskROM\n");
	else
		printf("Using CPU MaskROM\n");

	//BS1
	tmp_val = (reg_bs >> 1) & 0x1;
	if(tmp_val & 0x1)
		printf("Boot from CPU, bypass STBC\n");
	else
		printf("Bootfrom STBC\n");

	//BS2
	tmp_val = (reg_bs >> 2) & 0x1;

	if(tmp_val)
		printf("Bypass STBC MaskROM\n");
	else
		printf("Through STBC MaskROM\n");




	//interpret BS3,4
	tmp_val = (reg_bs >> 3) & 0x3;

	if(((reg_bs >> 12) & 1) == 1) {
		switch(tmp_val) {
			case 0:
				printf("STBC boot from SPI, CPU boot from NAND 2 Row\n");
				break;

			case 1: 
				printf("STBC & CPU both boot from NAND 2 Row\n");
				break;

			case 2:
				printf("STBC & CPU both boot from NAND 3 Row\n");
				break;

			case 3:
				printf("STBC boot from SPI, CPU from NAND 3 Row\n");
				break;
		}
	} else {
		switch(tmp_val) {
			case 0:
				printf("STBC boot from SPI, CPU boot from eMMC\n");
				break;

			case 1: 
				printf("STBC & CPU both boot from eMMC\n");
				break;

			case 2:
				printf("STBC & CPU both boot from SPI\n");
				break;

			case 3:
				printf("Reserved\n");
				break;
		}
	}

	//interpret BS 5
	tmp_val = (reg_bs >> 5) & 1;

	if(tmp_val == 0)
		printf("Internal bootstrap enable\n");
	else
		printf("internap bootstrap disable\n");

	//interpret BS 6,7
	tmp_val = (reg_bs >> 6) & 3;

	switch(tmp_val) {
		case 0:
			printf("DSP 200MHz(dsp_clk/2), AHB 62.5 MHz(cpu_clk/16), AXI 125MHz(axi_clk/2)\n");
			break;

		case 1:
			printf("DSP 250MHz(axi_clk), AHB 48MHz(spi_clk), AXI 125MHz(ddr_clk/2)\n");
			break;

		case 2:
			printf("DSP 125MHz(ddr_clk/2), AHB 96MHz(OCSI 16/2), AXI 125MHz(cpu_clk/8)\n");
			break;

		case 3:
			printf("DSP 400MHz(dsp_clk), AHB 100MHz(ahb_clk), AXI 250MHz(axi_clk)\n");
			break;

	}

	//interpret BS 8
	tmp_val = (reg_bs >> 8) & 1;
	if(tmp_val == 0)
		printf("Default disable STBC watchdog\n");
	else
		printf("Default enable STBC watchdog\n");

	//interpret BS 9
	tmp_val = (reg_bs >> 9) & 1;

	if(tmp_val == 0)
		printf("Secure boot disable\n");
	else
		printf("Secure boot enable\n");

	//interpret BS 10
	tmp_val = (reg_bs >> 10) & 1;

	if(tmp_val == 0)
		printf("SPI Flash scramble Disable\n");
	else
		printf("SPI Flash scramble Enable\n");

	//interpret BS 11
	tmp_val = (reg_bs >> 11) & 1;

	if(tmp_val == 0)
		printf("Turnkey secure boot Disable\n");
	else
		printf("Turnkey secure boot Enable\n");

	//interpret BS 12
	tmp_val = (reg_bs >> 12) & 1;

	if(tmp_val == 0) {
		storage_type = STORAGE_EMMC;
		printf("No support NAND\n");
	} else {
		storage_type = STORAGE_NAND;
		printf("Boot up on NAND\n");
		tmp_val = (reg_bs >> 3) & 3;
		switch(tmp_val) {
			case 0:
				printf("STBC boot from SPI, CPU boot from NAND 2 Row\n");
				break;

			case 1:
				printf("STBC & CPU both boot from NAND 2 Row\n");
				break;

			case 2:
				printf("STBC & CPU both boot from NAND 3 Row\n");
				break;

			case 3:
				printf("STBC boot from SPI, CPU from NAND 3 Row\n");
				break;
		}
	}

	//interpret BS 13,14

	tmp_val = (reg_bs >> 13) & 3;
	switch(tmp_val) {
		case 0:
			printf("eMMC Bus width is 8 bit\n");
			break;

		case 1:
			printf("eMMC Bus width is 4 bit\n");
			break;

		case 2:
			printf("eMMC Bus width is 1 bit\n");
			break;

		case 3:
			printf("eMMC Bus width is 1 bit\n");
			break;
	}


	//interpret BS 15
	tmp_val = (reg_bs >> 15) & 1;

	if(tmp_val == 0)
		printf("eMMC boot mode wait ACK\n");
	else
		printf("eMMC boot mode do not wait ACK\n");


	//interpret BS 16
	tmp_val = (reg_bs >> 16) & 1;
	if(tmp_val == 0)
		printf("eMMC REQ 2K mode \n");

	//interpret BS 17
	tmp_val = (reg_bs >> 17) & 1;
	if(tmp_val == 0)
		printf("eMMC data latch sdr\n");
	else
		printf("eMMC data latch ddr\n");

	//interpret BS 18
	tmp_val = (reg_bs >> 18) & 1;
	if(tmp_val == 0)
		printf("eMMC clock switch 26M\n");
	else
		printf("eMMC clock switch 13M\n");

	//interpret BS 19
	tmp_val = (reg_bs >> 19) & 1;
	if(tmp_val == 0)
		printf("NFC/eMMC type internal boot strap enable \n");
	else
		printf("NFC/eMMC type internal boot strap disable \n");

	return 0;

}

U_BOOT_CMD(
		bs,	3,	0,	do_nvt_bootstrap_cmd,
		"bs    - print nvt bootstrap info\n",
		""
	  );


int nvt_mmc_erase_part(NVT_EMMC_PHY_PART_TYPE part_type);

int do_nvt_emmc_eraseall(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	if(argc != 2 || (strcmp(argv[1],"fast") != 0 && strcmp(argv[1],"full") != 0))
		return CMD_RET_USAGE;

	if(strcmp(argv[1],"fast") == 0) {
		int i,cnt;
		nvt_emmc_image_info *pimg;
		cnt = nvt_emmc_get_image_cnt();
		for(i=0; i<cnt;i++) {
			pimg = (nvt_emmc_image_info*)nvt_emmc_get_img_by_index(i);
			if(pimg->erase_img != NULL)
				pimg->erase_img(pimg, NULL);
		}
	} else {
		nvt_mmc_erase_part(PHY_PART_TYPE_BOOT1);
		nvt_mmc_erase_part(PHY_PART_TYPE_BOOT2);
		nvt_mmc_erase_part(PHY_PART_TYPE_USER);
	}
	return 0;
}

U_BOOT_CMD(
	eeraseall, 2, 1, do_nvt_emmc_eraseall,
	"eeraseall - erase all images on emmc",
	"eeraseall fast || full"
);


int nvt_write_vhash_val(vhash_val *pval)
{
	int ret = 0;
	char cmd[128] = {0};
	nvt_emmc_image_info *pimg;

	if(pval == NULL) {
		ret = -1;
		goto out;
	}

	pimg = nvt_emmc_get_img_by_name("vhash");

	if(pimg == NULL) {
		ret = -1;
		goto out;
	}

	ret = pimg->write_img(pimg, (unsigned char*)pval, sizeof(vhash_val), NULL);

out:
	return ret;
}

static char __attribute__((__aligned__(64)))  comm_hdr_buff[512];

int nvt_emmc_check_common_header_exist(nvt_emmc_image_info *pimg)
{
	int ret = 0;
	unsigned long long start_lba;
	char cmd[128] = {0};
	image_header_t *phdr;

	start_lba = nvt_emmc_get_part_start_sector(pimg->user_part_name);
	sprintf(cmd, "mmc read 0x%lx 0x%x 1", (unsigned long)&comm_hdr_buff,
			(unsigned int)(start_lba + pimg->part_offset), 1);

	ret = run_command(cmd, 0);
	if(ret < 0) {
		printf("%s read header fail !\n", __func__);
		ret = -1;
		goto out;
	}

	phdr = (image_header_t*)comm_hdr_buff;

	if(image_get_magic(phdr) == IH_MAGIC)
		ret = 1;
	else
		ret = 0;
out:
	return ret;

}
