/*
 *  nvt_eburner.c
 *
 *  Author:	Alvin lin
 *  Created:	Mar 21, 2016
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <config.h>
#include <fdt_support.h>
#include <asm/types.h>
#include <asm/arch/hardware.h>
#include <nvt_eburner.h>
#include <nvt_emmc_xbootdat.h>
#include <nvt_emmc_partition.h>
#include <nvt_emmc_utils.h>
#include <nvt_common_utils.h>
#include <nvt_panel_init.h>
#include <nvt_tzone.h>


#if defined(CONFIG_NVTTK_EBURNER_V2)

static panel_param_blk panel_cfgs[PANEL_PARM_NUM+1];

int nvt_eburner_load_img(void* dest_addr, char* img_name, char* update_img_fname)
{
	int ret = 0;
	char cmd[512] = {0};
	unsigned long long offset = 0; 
	unsigned int align_shift = 0;
	unsigned long long flen;
	unsigned long long tmp_len;
	nvt_eburner_whole_img_hdr *pwhdr;
	nvt_eburner_per_img_hdr *pphdr;

	//read whole image header
	sprintf(cmd, "fatload usb 0:auto 0x%x %s 0x%x", 
			CONFIG_SYS_FWUPDATE_BUF, update_img_fname, sizeof(nvt_eburner_whole_img_hdr));
	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("%s read whole image header fail !\n", __func__);
		goto out;
	}

	//do little sanity check
	pwhdr = (nvt_eburner_whole_img_hdr*)CONFIG_SYS_FWUPDATE_BUF;
	if(memcmp(pwhdr->sign, NVT_EBURNER_WHOLE_IMG_HDR_SIGN, strlen(NVT_EBURNER_WHOLE_IMG_HDR_SIGN))) {
		printf("%s invalid firmware image !\n", __func__);
		ret = -1;
		goto out;
	}

	offset += sizeof(nvt_eburner_whole_img_hdr);

	//find specified image
	do {
		memset(cmd, 0, sizeof(cmd));
		memset((void*)CONFIG_SYS_FWUPDATE_BUF, 0, sizeof(nvt_eburner_per_img_hdr));

		//load pimg hdr
		sprintf(cmd, "fatload usb 0:auto 0x%x %s 0x%x 0x%llx", 
				CONFIG_SYS_FWUPDATE_BUF, update_img_fname, sizeof(nvt_eburner_per_img_hdr), offset);
		ret = run_command(cmd, 0);

		if(ret < 0){
			printf("%s read per image fail  !\n", __func__);
			goto out;
		}

		pphdr = (nvt_eburner_per_img_hdr*)CONFIG_SYS_FWUPDATE_BUF;

		//do little sanity check
		if(memcmp(pphdr->sign, NVT_EBURNER_PER_IMG_HDR_SIGN, strlen(NVT_EBURNER_PER_IMG_HDR_SIGN))) {
			printf("%s invalid per image header !\n", __func__);
			ret = -1;
			goto out;
		}

		if(strcmp(pphdr->img_name, img_name)) {
			offset += sizeof(nvt_eburner_per_img_hdr);
			offset += pphdr->flen;
			continue;
		} else {
			//load image to user buffer
			memset(cmd, 0, sizeof(cmd));
			offset += sizeof(nvt_eburner_per_img_hdr);
			align_shift = offset & (ARCH_DMA_MINALIGN - 1);
			tmp_len = pphdr->flen;
			sprintf(cmd, "fatload usb 0:auto 0x%x %s 0x%llx 0x%llx", 
					CONFIG_SYS_FWUPDATE_BUF,
					update_img_fname,
					tmp_len + align_shift,
					offset - align_shift);
			ret = run_command(cmd, 0);
			if(ret < 0) {
				printf("%s read fdt fail !\n", __func__);
				goto out;
			}
			memcpy(dest_addr,
				(const void*)(CONFIG_SYS_FWUPDATE_BUF + align_shift),
				tmp_len);
			break;
		}
	} while (1);

out:
	return ret;

}

int nvt_eburner_prepare_boot_cmd(void)
{
	char cmd[512] = {0};
	char buf[255] = {0};
	int ret = 0,len;
	unsigned char *ker_load_addr = 0;
	unsigned char *ramdisk_load_addr = 0;
	int block_cnt = 0;
	struct boot_address addr;
	int cpus;

	memset(buf, 0, sizeof(buf));

	if(NULL != env_get("console"))
		sprintf(cmd, "console=%s,115200 ", env_get("console"));
	else
		sprintf(cmd, "%s ", "console=ttyS0,115200");

	if (env_get("memstr") != NULL)
		sprintf(buf,"%s ",env_get("memstr"));
	else
		sprintf(buf,"%s ","mem=512M");

	strcat(cmd,buf);

	if(env_get("usb") != NULL){
		sprintf(buf,"root=%s rw ",env_get("usb"));
		env_set("ramdisk_loadaddr", NULL);
	}else {
		sprintf(buf,"%s ","root=/dev/ram0 rw ");
	}


	strcat(buf,"rootfstype=ext4 ");
	strcat(cmd,buf);

	if(env_get_yesno("quiet"))
		strcat(cmd,"quiet ");

	env_set("no_secos", "y");

	if(env_get("cpus") != NULL) {
		cpus = simple_strtoul(env_get("cpus"),NULL,10);
		if(cpus > 4 || cpus < 1)
			cpus = 4;
	} else {
		cpus = 4;
	}
	sprintf(buf, "maxcpus=%d ",cpus);
	strcat(cmd,buf);

	if(env_get("kparam_addon") != NULL) {
		sprintf(buf," %s ", env_get("kparam_addon"));
		strcat(cmd, buf);
	}


	ker_load_addr = (unsigned char*)simple_strtoul(env_get("kernel_loadaddr"),NULL,16);


	if(env_get("ramdisk_loadaddr") != NULL)
		ramdisk_load_addr = (unsigned char*)simple_strtoul(env_get("ramdisk_loadaddr"),NULL,16);
	else
		ramdisk_load_addr = 0;

	addr.kernel_addr = ker_load_addr;
	addr.ramdisk_addr = ramdisk_load_addr;

	env_set("bootargs",cmd);

	return ret;
}

int __attribute__((weak)) nvt_eburner_panel_param_init(int *pindex, panel_param_blk *pparam) 
{
	int ret = 0;
	int index;
	panel_index_blk *ppidx;

	if(pindex == NULL || pparam == NULL) {
		printf("%s index or param buffer invalid !\n", __func__);
		goto out;
	}

	ret = nvt_eburner_load_img(panel_cfgs, "xbootdat", NVT_EBURNER_IMG_FNAME);

	if(ret < 0) {
		printf("%s load panel config fail !\n", __func__);
		goto out;
	}

	memcpy(pparam, panel_cfgs, sizeof(panel_index_blk));
	ppidx = (panel_index_blk*)pparam;
	index = ppidx->idx.index;

	memcpy(pparam, (panel_cfgs + index + 1), sizeof(panel_param_blk));

out:
	return ret;
}

int nvt_eburner_run_updater(void)
{
	char cmd[512] = {0};
	unsigned int fdt_high = 0;
	int len;
	int ret = 0;
	panel_index_blk pidx;
	panel_param_blk pparm;
	nvt_emmc_image_info *pimg;
	unsigned long *dtb_magic;
	image_header_t *phdr;
	unsigned int fdt_size;

#if defined(CONFIG_NVTTK_EBURNER_V2_PACK_EBURNER)
	unsigned int kernel_loadaddr = simple_strtoul(env_get("kernel_loadaddr"), NULL, 16);
	ret = nvt_eburner_load_img((void*)kernel_loadaddr, "eburner", NVT_EBURNER_IMG_FNAME);
	if(ret < 0) {
		printf("%s load eburner fail !\n", __func__);
		ret = -1;
		goto out;
	}
#else
	sprintf(cmd, "fatload usb 0:auto %s nvttk_eburner", env_get("kernel_loadaddr"));
	run_command(cmd, 0);
#endif
	fdt_high = simple_strtoul(env_get("kernel_loadaddr"),NULL,16) + 0x40 + 0x2000000;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "0x%x", fdt_high);
	env_set("fdt_high", cmd);

	env_set("no_secos", "y");

	//load fdt from image to buffer
	ret = nvt_eburner_load_img((void*)fdt_high, "fdt0", NVT_EBURNER_IMG_FNAME);
	if(ret < 0) {
		printf("%s load fdt from firmware fail, load from device instead !\n", __func__);

		pimg = nvt_emmc_get_img_by_name("fdt0");
		if(pimg == NULL) {
			printf("%s get fdt0 from device fail !\n", __func__);
			goto out;
		}
		pimg->read_img(pimg, (unsigned char*)fdt_high, &len, NULL);

	}

	phdr = (image_header_t*)fdt_high;
	if(image_get_magic(phdr) == IH_MAGIC) {
		fdt_size = image_get_size(phdr);
		memmove((void*)(fdt_high - sizeof(image_header_t)), (const void*)fdt_high, fdt_size + sizeof(image_header_t));
	}

	dtb_magic = (unsigned long*)fdt_high;

	if(*dtb_magic != DTB_MAGIC) {
		printf("%s invalid fdt0 data !\n", __func__);
		goto out;
	}

	if(nvt_in_empty_burner_mode()) {
		_VID_PowerON_Sequence();
	}

	panel_init_late();

	//prepare boot argument
	nvt_eburner_prepare_boot_cmd();

	//boot to kernel
	sprintf(cmd, "bootm %s - %s", env_get("kernel_loadaddr"), env_get("fdt_high"));
	run_command(cmd, 0);

out:
	return ret;

}


int nvt_eburner_check_trigger(unsigned int adc_reg_base, unsigned int adc_val, unsigned int adc_range)
{
	unsigned int reg_val = __REG(adc_reg_base);
	int ret = 0;

	if(reg_val >= (adc_val - adc_range) && reg_val <= (adc_val + adc_range))
		ret = 1;
	else
		ret = 0;

	return ret;
}
#endif//CONFIG_NVTTK_EBURNER_V2
