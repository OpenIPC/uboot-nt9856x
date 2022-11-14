/*
 *  Author:	Will Fu
 *  Created:	Jul 25, 2015
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <command.h>
#include <nand.h>
#include <libfdt.h>
#include <nvt_nand_fwupdate.h>
#include <nvt_nand_partition.h>
#include <nvt_tzone.h>
#include <nvt_npt.h>
#include <nvt_nand_xbootdat.h>
#include <nvt_nand_misc_ptn.h>
#include <nt72_nand_api.h>

#if defined(CONFIG_NVT_TURNKEY_FWUPDATE)
#include <nvt_tk_fwupdate.h>
#endif

extern char *source_cmd;
#if defined(CONFIG_NVT_FASTETH_MAC)
static int do_update_source(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{

	if (argc==2){
		if (!strcmp(argv[1],"eth"))
			source_cmd = "tftp";
		else
			source_cmd = "nvtload";
		return 0;
	}
	else
		return cmd_usage(cmdtp);
}

U_BOOT_CMD(
        update_source, 2,      0,      do_update_source,
        "change file source(default : usb)",
        "usb or eth"
);
#endif

/* Will: Return value define.
 *  1: No such file or Update Fail
 *  0: update Success
*/
int nvt_nand_update_bin(char *ptn_name, char *fname)
{
	char cmd_buf[256] = {0};
	u32 page_size;
	u32 write_size;
	int cmd_ret;
	int ptn_idx = -1;

	ptn_idx = nvt_nand_get_ptn_index(ptn_name);
        if (ptn_idx < 0) {
                printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, ptn_name);
                goto err_out;
        }

	memset((void *)CONFIG_FWUP_RAM_START, 0xff, nand_calc_partition_size(ptn_idx));
	//load image to ram
	sprintf(cmd_buf,"%s 0x%x %s",source_cmd,CONFIG_FWUP_RAM_START,fname);
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else	
			printf("Load %s from usb fail !\n",fname);
		goto err_out;
	}

	page_size = nand_get_page_size();

	write_size = simple_strtoul(env_get("filesize"), NULL, 16);

	if((write_size %page_size) ){
		write_size = (write_size & ~(page_size - 1)) + page_size;
	}

	if(write_size > (nand_calc_max_image_size(ptn_idx))) {
		printf("Error ! nand bad block too much, can't update\n");
		goto err_out;
	}

	//erase nand partition
	sprintf(cmd_buf,"nand erase 0x%x 0x%x",nand_calc_partition_addr(ptn_idx),
	nand_calc_partition_size(ptn_idx));
	cmd_ret = run_command(cmd_buf, 0);

	if(cmd_ret == -1) {
		printf("erase nand partition fail !\n");
		goto err_out;
	}

	//write image to nand partition
	sprintf(cmd_buf,"nand write 0x%x 0x%x 0x%x",CONFIG_FWUP_RAM_START,
		nand_calc_partition_addr(ptn_idx),
		write_size);

	cmd_ret = run_command(cmd_buf,0);

	if(cmd_ret == -1) {
		printf("write file %s to nand fail !\n",fname);
		goto err_out;
	}

	return 0;

err_out:
	return -1;
}

int nvt_nand_update_ubi(char *ptn_name, char *fname)
{
	char cmd_buf[256] = {0};
	int cmd_ret;
	u32 page_size;
	int write_size;
	u32 img_nand_start;
	int ptn_idx = -1;

	ptn_idx = nvt_nand_get_ptn_index(ptn_name);
	if (ptn_idx < 0) {
		printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, ptn_name);
		goto err_out;
	}

	memset((void *)CONFIG_FWUP_RAM_START, 0xff, nand_calc_partition_size(ptn_idx));
	//load image to ram
	sprintf(cmd_buf,"%s 0x%x %s",source_cmd,CONFIG_FWUP_RAM_START,fname);
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else	
			printf("Load %s from usb fail !\n",fname);
		goto err_out;
	}

	page_size = nand_get_page_size();

	write_size = simple_strtoul(env_get("filesize"), NULL, 16);
	if((write_size % page_size))
		write_size = (write_size & ~(page_size - 1)) + page_size;

	img_nand_start = nand_calc_partition_addr(ptn_idx);

	if(write_size > (nand_calc_max_image_size(ptn_idx))) {
		printf("Error ! nand bad block too much, can't update\n");
		goto err_out;
	}

	//erase nand partition
	sprintf(cmd_buf,"nand erase 0x%x 0x%x",nand_calc_partition_addr(ptn_idx),
			nand_calc_partition_size(ptn_idx));
	cmd_ret = run_command(cmd_buf, 0);

	if(cmd_ret == -1) {
		printf("erase nand partition fail !\n");
		goto err_out;
	}

#ifdef CONFIG_NVT_BY_PART_ECC_INV
	cmd_ret = run_command("nset inv 1",0);
	if(cmd_ret == -1) {
		printf("error configuring ECC inv mode\n");
		goto err_out;
	}
#endif

	sprintf(cmd_buf,"nand write.trimffs 0x%x 0x%x 0x%x",CONFIG_FWUP_RAM_START,nand_calc_partition_addr(ptn_idx),
			write_size);
	cmd_ret = run_command(cmd_buf,0);

	if(cmd_ret == -1) {
		printf("write file %s to nand fail!\n",fname);
		goto err_out;
	}

#ifdef CONFIG_NVT_BY_PART_ECC_INV
	cmd_ret = run_command("nset inv 0",0);
	if(cmd_ret == -1) {
		printf("error disabling ECC inv mode\n");
		goto err_out;
	}
#endif

	return 0;

err_out:
	return -1;
}

int nvt_nand_update_ubifs(char *vol_name, char *fname)
{
	char cmd_buf[256] = {0};
	int cmd_ret, ret;
	unsigned int leb_size;
	unsigned int img_leb_size;
	int vol_idx = -1;
	struct nvt_ubi_vol vol;

	ret = nvt_ubi_get_vol_by_name(&vol, vol_name);
	if (ret) {
		printf("%s:[Error] Unknown nand volume name '%s'\n", __func__, vol_name);
		goto err_out;
	}

	memset((void *)CONFIG_FWUP_RAM_START, 0xff, nand_calc_partition_size(vol.mtd_idx));
	//load image to ram
	sprintf(cmd_buf,"%s 0x%x %s",source_cmd,CONFIG_FWUP_RAM_START,fname);
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else
			printf("Load %s from usb fail !\n",fname);
		goto err_out;
	}

	//Check image size
	img_leb_size = simple_strtoul(env_get("filesize"), NULL, 16);
	if (img_leb_size > vol.size) {
		printf("Error! image size too large, can not update\n");
		goto err_out;
	}

	/* UBI select part (attaching mtdN to part) */
	sprintf(cmd_buf, "ubi part %s", nvt_nand_get_ptn_name(vol.mtd_idx));
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("Error: ubi part %s not found!\n", nvt_nand_get_ptn_name(vol.mtd_idx));
		goto err_out;
	}

	/* Remove vol */
	sprintf(cmd_buf, "ubi remove %s", vol.name);
	run_command(cmd_buf, 0);

	/* Create vol */
	sprintf(cmd_buf, "ubi nvt_create %s 0x%llx d %d", vol.name, vol.size, vol.vol_idx);
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("[ERROR] Can not create vol! ret=%d\n", cmd_ret);
		goto err_out;
	}

	printf("Start updating ...\n");
	sprintf(cmd_buf,"ubi write 0x%x %s 0x%x",CONFIG_FWUP_RAM_START, vol.name, img_leb_size);
	cmd_ret = run_command(cmd_buf,0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if(cmd_ret != 0) {
		printf("write file %s to nand fail!\n",fname);
		goto err_out;
	}
	return 0;

err_out:
	return -1;
}

int nvt_nand_update_stbc(char *ptn_name, char *fname)
{
	char cmd_buf[256] = {0};
	u32 page_size;
	u32 write_size;
	int cmd_ret;
	struct mtd_info *info = nand_info[nand_curr_device];
	u_char stbc_type;
	size_t ooblen = 1;
        int ptn_idx = -1;

        ptn_idx = nvt_nand_get_ptn_index(ptn_name);
        if (ptn_idx < 0) {
                printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, ptn_name);
                goto err_out;
        }

	memset((void *)CONFIG_FWUP_RAM_START, 0xff, nand_calc_partition_size(ptn_idx));
	//load image to ram
	sprintf(cmd_buf,"%s 0x%x %s",source_cmd,CONFIG_FWUP_RAM_START,fname);
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else	
			printf("Load %s from usb fail !\n",fname);
		goto err_out;
	}

	page_size = nand_get_page_size();

	write_size = simple_strtoul(env_get("filesize"), NULL, 16);

	if((write_size %page_size)){
		write_size = (write_size & ~(page_size - 1)) + page_size;
	}

	if(write_size > (nand_calc_max_image_size(ptn_idx))) {
		printf("Error ! nand bad block too much, can't update\n");
		goto err_out;
	}

	//erase nand partition
	sprintf(cmd_buf,"nand erase 0x%x 0x%x",nand_calc_partition_addr(ptn_idx),
	nand_calc_partition_size(ptn_idx));
	cmd_ret = run_command(cmd_buf, 0);

	if(cmd_ret == -1) {
		printf("erase nand partition fail !\n");
		goto err_out;
	}

	/* Rom code read stbc magic in oob first 3 bytes, But first 2 bytes
	 * be used for bad block mark. So we just need set one byte.
	 * And nand_write_skip_bad_oob change to using MTD_OPS_AUTO_OOB mode.
	 * We dont care oob layout.
	 */
	stbc_type = 0x0;

	cmd_ret = nand_write_skip_bad_oob(info, nand_calc_partition_addr(ptn_idx), &write_size, (u_char*)(CONFIG_FWUP_RAM_START), &ooblen, &stbc_type, 0);
	

	if(cmd_ret) {
		printf("write file %s to nand fail !\n",fname);
		goto err_out;
	}
	printf("Write STBC to nand success!\n");
	return 0;

err_out:
	return -1;
	
}

int nvt_nand_update_loader(char *ptn_name, char *fname)
{
	char cmd_buf[256] = {0};
	u32 page_size;
	u32 write_size;
	u32 fst_write_size;
	u32 bad_size;
	int cmd_ret;
	struct mtd_info *info = nand_info[nand_curr_device];
	u_char loader_type;
	size_t ooblen = 1;
	u32 fstblk_wlen;//first block write length
	loff_t offset;
	size_t        Actual;
        int ptn_idx = -1;

        ptn_idx = nvt_nand_get_ptn_index(ptn_name);
        if (ptn_idx < 0) {
                printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, ptn_name);
                goto err_out;
        }

	memset((void *)CONFIG_FWUP_RAM_START, 0xff, nand_calc_partition_size(ptn_idx));
	//load image to ram
	sprintf(cmd_buf,"%s 0x%x %s",source_cmd,CONFIG_FWUP_RAM_START,fname);
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else	
			printf("Load %s from usb fail !\n",fname);
		goto err_out;
	}

	page_size = nand_get_page_size();
	
	write_size = simple_strtoul(env_get("filesize"), NULL, 16);
	if((write_size % page_size))
		write_size = (write_size & ~(page_size - 1)) + page_size;
	fstblk_wlen = info->erasesize;

	if(write_size > (nand_calc_max_image_size(ptn_idx))) {
		printf("Error ! nand bad block too much, can't update\n");
		goto err_out;
	}

	//erase nand partition
	sprintf(cmd_buf,"nand erase 0x%x 0x%x",nand_calc_partition_addr(ptn_idx)
			,nand_calc_partition_size(ptn_idx));
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret == -1) {
		printf("erase nand partition fail !\n");
		goto err_out;
	}

	/* Rom code read xboot magic in oob first 3 bytes, But first 2 bytes
	 * be used for bad block mark. So we just need set one byte.
	 * And nand_write_skip_bad_oob change to using MTD_OPS_AUTO_OOB mode.
	 * We dont care oob layout.
	 */
	loader_type = 0x01;

	fst_write_size = (write_size > fstblk_wlen) ? fstblk_wlen:write_size;
	cmd_ret = nand_write_skip_bad_oob(info, nand_calc_partition_addr(ptn_idx), &fst_write_size, (u_char*)(CONFIG_FWUP_RAM_START), &ooblen, &loader_type, 0);
	

	if(cmd_ret) {
		printf("write file %s to nand first block fail !\n",fname);
		goto err_out;
	}

	//check bad block size for first block
	offset = nand_calc_partition_addr(ptn_idx);
	bad_size = 0;
	while(nand_block_isbad(info, offset)) {
		bad_size += info->erasesize;
		offset += info->erasesize;
	}

	//write remains
	if(write_size > fstblk_wlen){
		write_size -= fstblk_wlen;
		//	printf(" 2. write par name=%d, addr = %x\n", ptn_idx,nand_calc_partition_addr(ptn_idx));
		//	printf(" 2. fstblk_wlen=%x, write_size=%x\n", fstblk_wlen,write_size);

		cmd_ret = nand_write_skip_bad(info, nand_calc_partition_addr(ptn_idx) + bad_size + info->erasesize, &write_size, &Actual, write_size, (u_char*)(CONFIG_FWUP_RAM_START + fstblk_wlen), 0);

		if(cmd_ret) {
			printf("write file %s remain parts nand first block fail !\n",fname);
			goto err_out;
		}
	}
	printf("Write loader to nand success!\n");

	return 0;

err_out:
	return -1;
}


int nvt_nand_update_fdt(char *ptn_name, char *fname)
{
	char cmd_buf[256] = {0};
	u32 page_size;
	u32 write_size = 0;
	int cmd_ret;
	int ret =0;
	unsigned char *buf = (unsigned char *)CONFIG_SYS_FDT_BUF;
	unsigned char *fdt_buf = NULL;
        int ptn_idx = -1;
	int with_common_hdr = 0;
	image_header_t *phdr= NULL;

        ptn_idx = nvt_nand_get_ptn_index(ptn_name);
        if (ptn_idx < 0) {
                printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, ptn_name);
                goto err_out;
        }

#if defined(CONFIG_NT72563_NAND_NSV)
	if (!strncmp(ptn_name, "fdt1", strlen("fdt1"))) {
		// set rootfs_mtd ++
		int rootfs_mtd_num = simple_strtoul(env_get("rootfs_mtd"), NULL, 10);
		rootfs_mtd_num ++;
		sprintf(cmd_buf, "%d", rootfs_mtd_num );
		printf("Get rootfs_mtd_num : %s\n", cmd_buf);
		env_set("rootfs_mtd", cmd_buf);
	}
#endif

	ret = nvt_prepare_kercmd();
	if(ret < 0) 
		printf("%s:prepare kernel cmdline fail !, it's OK when run empty burner",__func__);

	memset((void *)buf, 0xff, nand_calc_partition_size(ptn_idx));
	/* Load iamge to ram */
	sprintf(cmd_buf,"%s 0x%x %s",source_cmd, buf,fname);
	cmd_ret = run_command(cmd_buf, 0);
	if(cmd_ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else	
			printf("Load %s from usb fail !\n",fname);
		goto err_out;
	}

	/* Check common hdr */
	if (image_get_magic((image_header_t *)buf) == IH_MAGIC) {
		printf("%s: With Common Header\n", __func__);
		with_common_hdr = 1;
	}

	if (with_common_hdr) {
		fdt_buf = buf + sizeof(image_header_t);
		phdr = (image_header_t *)buf;
	} else {
		fdt_buf = buf;
	}

	if(fdt_check_header(fdt_buf) < 0) {
		printf("%s: Invaild fdt image, run nwrite fdt first !\n", __func__);
		goto err_out;
	}

	/* Update FDT */
	fdt_chosen(fdt_buf);

	/* Update Common Hdr And setting write_size */
	if (with_common_hdr) {
		phdr->ih_size = cpu_to_be32(fdt_totalsize(fdt_buf));
		phdr->ih_dcrc = cpu_to_be32(crc32(0, fdt_buf, be32_to_cpu(phdr->ih_size)));
		phdr->ih_hcrc = 0;
		phdr->ih_hcrc = cpu_to_be32(crc32(0, (const unsigned char *)phdr, sizeof(image_header_t)));
		write_size = fdt_totalsize(fdt_buf) + sizeof(image_header_t);
	} else {
		write_size = fdt_totalsize(fdt_buf);
	}

	printf("Write size : %u, %u\n", write_size, simple_strtoul(env_get("filesize"), NULL, 16));

	page_size = nand_get_page_size();

	if((write_size % page_size))
		write_size = (write_size & ~(page_size - 1)) + page_size;

	if(write_size > (nand_calc_max_image_size(ptn_idx))) {
		printf("Error ! nand bad block too much, can't update\n");
		goto err_out;
	}
	//erase nand partition
	sprintf(cmd_buf,"nand erase 0x%x 0x%x",nand_calc_partition_addr(ptn_idx),
		nand_calc_partition_size(ptn_idx));
	cmd_ret = run_command(cmd_buf, 0);

	if(cmd_ret == -1) {
		printf("erase nand partition fail !\n");
		goto err_out;
	}
	
	sprintf(cmd_buf,"nand write 0x%x 0x%x 0x%x",buf,
		nand_calc_partition_addr(ptn_idx),
		write_size);
	cmd_ret = run_command(cmd_buf,0);

	if(cmd_ret == -1) {
		printf("write file %s to nand fail !\n",fname);
		goto err_out;
	}

	return 0;

err_out:
	return -1;
}

int nvt_nand_update_npt(char *ptn_name, char *fname)
{
	int ret;
	char cmd_buf[256] = {0};
	struct nt72_npt_info *npt_info;
	uint32_t page_size;
	uint32_t write_size;
	uint32_t npt_partition_sz;
	uint32_t npt_partition_addr;
	struct nand_partition_table npt;

	npt_partition_sz = nt72_npt_get_store_blk_nr() * nand_get_block_size();
	npt_partition_addr = nt72_npt_get_store_blk_offset() * nand_get_block_size();
	write_size = sizeof(struct nand_partition_table);
	page_size = nand_get_page_size();

	npt_info = memalign(4096, sizeof(struct nt72_npt_info));
	if (!npt_info) {
		printf("%s: npt_info malloc fail!\n", __func__);
		return -ENOMEM;
	}
	memset(npt_info, 0, sizeof(struct nt72_npt_info));

	ret = nt72_npt_if_load(npt_info, fname);
	if (ret != 0) {
		printf("%s:[Error] Load NAND partition table fail!\n", __func__);
		goto err_out;
	}

	/* Check block size is match or not */
	if (((npt_info->unit_kb) << 10) != nand_get_block_size()) {
		printf("%s:[Error] Load NPT block size is not match with flash block size\n",
		       __func__);
		ret = -1;
		goto err_out;
	}

	/* Get NPT */
	nt72_npt_gen_npt(npt_info, &npt);

	/* Copy to Buffer */
	memset((void *)CONFIG_FWUP_RAM_START, 0xff, npt_partition_sz);
	memcpy((void *)CONFIG_FWUP_RAM_START, &npt, write_size);

	/* ROUND UP page alignment */
	if ((write_size %page_size)) {
		write_size = (write_size & ~(page_size - 1)) + page_size;
	}

	if (write_size > npt_partition_sz) {
		printf("Error ! nand bad block too much, can't update\n");
		ret = -1;
		goto err_out;
	}

	//erase nand partition
	sprintf(cmd_buf,"nand erase 0x%x 0x%x", npt_partition_addr, npt_partition_sz);
	ret = run_command(cmd_buf, 0);
	if(ret == -1) {
		printf("erase nand partition fail !\n");
		goto err_out;
	}

	//write image to nand partition
	sprintf(cmd_buf,"nand write 0x%x 0x%x 0x%x",CONFIG_FWUP_RAM_START,
		npt_partition_addr, write_size);

	ret = run_command(cmd_buf,0);
	if(ret == -1) {
		printf("write file %s to nand fail !\n",fname);
		goto err_out;
	}

	/* 2. Re-init partition */
	ret = nvt_nand_ptn_init();
	if (ret != 0) {
		printf("Init partition fail!\n");
		ret = -1;
		goto err_out;
	}

	printf("NPT update success!\n");

err_out:
	free(npt_info);
	return ret;

}


int nvt_nand_read_secos_image(unsigned char* buf)
{
	int ret = 0;
	char cmd[128] = {0};
	secos_header_t * header = 0;
	int encrypt = 0;
        int ptn_idx = -1;
	unsigned int Image_blk;

        ptn_idx = nvt_nand_get_ptn_index("secos");
        if (ptn_idx < 0) {
                printf("%s:[Error] Unknown nand partition name secos\n", __func__);
		ret = -EINVAL;
                goto out;
        }

	sprintf(cmd, "nand read 0x%x 0x%x 0x%x", (unsigned int)buf, nand_calc_partition_addr(ptn_idx) , nand_get_page_size());
	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("read secos image fail !\n");
		goto out;
	}

	encrypt = dec_secos_header(SECOS_ENC_BUFFER_BLOCK, SECOS_BUFFER_BLOCK, SECOS_HEADER_MAGIC_CODE);

	header = (secos_header_t * )(SECOS_BUFFER_START - sizeof(secos_header_t));

	if(encrypt == -1) {
		printf("header of secure OS is error !\n");
		ret = -EINVAL;
		goto out;
	} else if (encrypt == 1){ //encrypted secos.bin 
		Image_blk = nand_calc_partition_size(ptn_idx);
		buf = (unsigned char*)SECOS_ENC_BUFFER_BLOCK;
	} else {//non-encrypted secos.bin
		Image_blk = (header->BinSize + sizeof(secos_header_t));
		buf = (unsigned char*)SECOS_BUFFER_BLOCK;
	}

	sprintf(cmd, "nand read 0x%x 0x%x 0x%x", (unsigned int)buf, nand_calc_partition_addr(ptn_idx), Image_blk + nand_get_page_size());

	ret = run_command(cmd, 0);
	
out:
	return ret;	

}

int nvt_nand_read_binary_image(char* part_name, unsigned char* buf)
{
	int ret = 0;
	char cmd[128] = {0};
        int ptn_idx = -1;

        ptn_idx = nvt_nand_get_ptn_index(part_name);
        if (ptn_idx < 0) {
                printf("%s:[Error] Unknown nand partition name %s\n", __func__, part_name);
		ret = -EINVAL;
                goto out;
        }

	sprintf(cmd, "nand read 0x%x 0x%x 0x%x", (unsigned int)buf, nand_calc_partition_addr(ptn_idx) , nand_calc_partition_size(ptn_idx));
	ret = run_command(cmd, 0);

	if(ret < 0) {
		printf("read %s image fail !\n", part_name);
		goto out;
	}

out:
	return ret;
}

int nvt_nand_erase_common(char *ptn_name)
{
	int ret=-1;
	char cmd[128];
	int ptn_idx = -1;

	ptn_idx = nvt_nand_get_ptn_index(ptn_name);
	if (ptn_idx < 0) {
		printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, ptn_name);
		goto err_out;
	}

	//Erase partition
	sprintf(cmd,"nand erase 0x%x 0x%x",nand_calc_partition_addr(ptn_idx),nand_calc_partition_size(ptn_idx));
	ret = run_command(cmd, 0);
	if(ret < 0)
		printf("%s erase partition fail !\n",__func__);

err_out:
	return ret;
}

int nvt_nand_erase_ubifs(char *vol_name)
{
	char cmd_buf[256] = {0};
	int cmd_ret, ret;
	struct nvt_ubi_vol vol;

	ret = nvt_ubi_get_vol_by_name(&vol, vol_name);
	if (ret) {
		printf("%s:[Error] Unknown nand volume name '%s'\n", __func__, vol_name);
		goto err_out;
	}

	/* UBI select part (attaching mtdN to part) */
	sprintf(cmd_buf, "ubi part %s", nvt_nand_get_ptn_name(vol.mtd_idx));
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("Error: ubi part %s not found!\n", nvt_nand_get_ptn_name(vol.mtd_idx));
		goto err_out;
	}

	/* Remove vol */
	sprintf(cmd_buf, "ubi remove %s", vol.name);
	run_command(cmd_buf, 0);

	/* Create vol */
	sprintf(cmd_buf, "ubi nvt_create %s 0x%llx d %d", vol.name, vol.size, vol.vol_idx);
	cmd_ret = run_command(cmd_buf, 0);
	printf("'%s' return: %d\n",cmd_buf, cmd_ret);
	if (cmd_ret != 0) {
		printf("[ERROR] Can not create vol! ret=%d\n", cmd_ret);
		goto err_out;
	}

	return 0;

err_out:
	return -1;
}

#if defined(CONFIG_NVT_TURNKEY_FWUPDATE)
#define OP_MISC_READ		(0)
#define OP_MISC_WRITE		(1)

static int nvt_nand_misc_op(unsigned int buf, unsigned int offset, unsigned int size, int op)
{
	int ret = -1;
	char cmd_buf[256] = {0};
	struct mtd_info *info = nand_info[nand_curr_device];
        int ptn_idx = -1;
	unsigned int read_off = 0;
	int bad_num = 0;

	if (buf >= CONFIG_FWUP_RAM_START 
			&& buf <= CONFIG_FWUP_RAM_START + info->erasesize) {
		printf("Error buf can not be in CONFIG_FWUP_RAM_START!\n");
		goto err_out;
	}

	if (size == 0) {
		printf("Error size is zero!\n");
		goto err_out;
	}

	if (offset + size > info->erasesize) {
		printf("Error out of one block!\n");
		goto err_out;
	}

	ptn_idx = nvt_nand_get_ptn_index("misc");
	if (ptn_idx < 0) {
		printf("%s:[Error] Unknown nand partition name '%s'\n", __func__, "misc");
		goto err_out;
	}

	/* Load Storage data to DRAM */
	read_off = nand_calc_partition_addr(ptn_idx);
	do {
		bad_num = nand_get_bad_number(info, read_off, info->erasesize);
		if (bad_num < 0) {
			printf("Error! nand_get_bad_number fail\n");
			goto err_out;
		}
		if (bad_num > 0) {
			printf("Skip bad block %d\n", bad_num);
			read_off += bad_num *  info->erasesize;
		}
	} while (bad_num > 0);

	if (read_off >= (nand_calc_partition_addr(ptn_idx) + nand_calc_partition_size(ptn_idx))) {
		printf("No valid block! Fail\n");
		goto err_out;
	}

	sprintf(cmd_buf, "nand read 0x%x 0x%x 0x%x", CONFIG_FWUP_RAM_START, read_off, info->erasesize);
	ret = run_command(cmd_buf, 0);
	if(ret == -1) {
		printf("Error ! %s nand read fail !\n", __func__);
		goto err_out;
	}

	if (op == OP_MISC_WRITE) {
		memcpy((void *)(CONFIG_FWUP_RAM_START + offset), (void *)buf, size);

		/* Write back to Storage */
		sprintf(cmd_buf,"nand erase 0x%x 0x%x", read_off, info->erasesize);
		ret = run_command(cmd_buf, 0);
		if(ret == -1) {
			printf("%s erase partition fail !\n",__func__);
			goto err_out;
		}

		sprintf(cmd_buf, "nand write 0x%x 0x%x 0x%x", CONFIG_FWUP_RAM_START, read_off, info->erasesize);
		ret = run_command(cmd_buf, 0);
		if(ret == -1) {
			printf("write file %s to nand fail !\n", "misc");
			goto err_out;
		}
	} else if (op == OP_MISC_READ) {
		memcpy((void *)buf, (const void *)(CONFIG_FWUP_RAM_START + offset), size);
	}
	ret = 0;
err_out:
	return ret;
}

int do_nvt_read_update_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	update_flag __attribute__((__aligned__(64))) uflag = {};
	int ret = -1;

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nvt_nand_misc_op((unsigned int)&uflag, UPDATE_FLAG_OFFSET, sizeof(update_flag), OP_MISC_READ);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
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
	nread_update_flag, 1, 1, do_nvt_read_update_flag,
	"nread_update_flag - read update flag to misc partition",
	""
);

int do_nvt_write_update_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	update_flag __attribute__((__aligned__(64))) uflag = {};
	int ret = -1;

	if(argc != 1)
		return CMD_RET_USAGE;

	memset((void *)&uflag, 0, sizeof(update_flag));
	memcpy(uflag.sign, NVTFW_UPDATE_FLAG_SIGN, strlen(NVTFW_UPDATE_FLAG_SIGN));
	ret = nvt_nand_misc_op((unsigned int)&uflag, UPDATE_FLAG_OFFSET, sizeof(update_flag), OP_MISC_WRITE);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}

	if(ret == 0)
		printf("write update flag done !\n");
	else
		printf("write update flag fail !\n");
out:
	return ret;
}

U_BOOT_CMD(
	nwrite_update_flag, 1, 1, do_nvt_write_update_flag,
	"nwrite_update_flag - write update flag to misc partition",
	""
);

int do_nvt_erase_update_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	update_flag __attribute__((__aligned__(64))) uflag = {};
	int ret = -1;

	if(argc != 1)
		return CMD_RET_USAGE;

	memset((void *)&uflag, 0, sizeof(update_flag));
	ret = nvt_nand_misc_op((unsigned int)&uflag, UPDATE_FLAG_OFFSET, sizeof(update_flag), OP_MISC_WRITE);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}

	if(ret == 0)
		printf("erase update flag done !\n");
	else
		printf("erase update flag fail !\n");
out:
	return ret;
}

U_BOOT_CMD(
	nerase_update_flag, 1, 1, do_nvt_erase_update_flag,
	"nerase_update_flag - erase update flag to misc partition",
	""
);

int do_nvt_write_boot_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	boot_flag __attribute__((__aligned__(64)))  bflag = {};
	int ret = -1;
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

	ret = nvt_nand_misc_op((unsigned int)&bflag, BOOT_FLAG_OFFSET, sizeof(boot_flag), OP_MISC_WRITE);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}
	if(ret == 0)
		printf("write boot flag done !\n");
	else
		printf("write boot flag fail !\n");
out:
	return ret;
}

U_BOOT_CMD(
	nwrite_boot_flag, 4, 1, do_nvt_write_boot_flag,
	"nwrite_boot_flag - write boot flag to misc partition",
	"nwrite_boot_flag kernel_idx fs_idx ppcache_idx"
);

int do_nvt_read_boot_flag(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	boot_flag __attribute__((__aligned__(64)))  bflag = {};
	int ret = -1;

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nvt_nand_misc_op((unsigned int)&bflag, BOOT_FLAG_OFFSET, sizeof(boot_flag), OP_MISC_READ);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}
	ret = 0;

	if(memcmp(bflag.sign, NVTFW_BOOT_FLAG_SIGN, 4))
		ret = -1;

	if(bflag.kernel_idx != 0 && bflag.kernel_idx != 1)
		ret = -1;

	if(bflag.fs_idx != 0 && bflag.fs_idx != 1)
		ret = -1;

	if(bflag.ppcache_idx != 0 && bflag.ppcache_idx != 1)
		ret = -1;

	if (ret == 0) {
		printf("boot flag - kernel_idx:%d fs_idx:%d ppcache_idx:%d\n",
			 bflag.kernel_idx, bflag.fs_idx, bflag.ppcache_idx);
	} else {
		printf("boot flag not exist !\n");
	}

	ret = 0;
out:
	return ret;
}

U_BOOT_CMD(
	nread_boot_flag, 1, 1, do_nvt_read_boot_flag,
	"nread_boot_flag - read boot flag from misc partition",
	""
);

int do_nvt_fix_bootargs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *ptr;
	char tmp_str[32] = {0};
	boot_flag __attribute__((__aligned__(64)))  bflag = {};
	int ret = -1;
	int ptn_idx = -1;

	if(argc != 1)
		return CMD_RET_USAGE;

	ret = nvt_nand_misc_op((unsigned int)&bflag, BOOT_FLAG_OFFSET, sizeof(boot_flag), OP_MISC_READ);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}

	if(bflag.fs_idx == 0)
		ptn_idx = nvt_nand_get_ptn_index("fs0");
	else
		ptn_idx = nvt_nand_get_ptn_index("fs1");
	if (ptn_idx < 0) {
		printf("%s: get nvt_nand_get_ptn_index fail\n", __func__);
		goto out;
	}

	sprintf(tmp_str, "%d", ptn_idx);
	env_set("rootfs_mtd", tmp_str);

	if(bflag.kernel_idx == 0)
		run_command("xsave kercmd", 0);
	else
		run_command("xsave rcvcmd", 0);

	ret = 0;
out:
	return ret;
}

U_BOOT_CMD(
	nfix_bootargs, 1, 1, do_nvt_fix_bootargs,
	"nfix_bootargs - fix ftd bootargs depend on boot flag",
	""
);

int nvt_nand_update_pptbl(char *ptn_name, char *fname)
{
	int ret = -1;
	unsigned int pptbl_offset;
	unsigned long file_size = 0;
	char cmd_buf[256] = {0};
	pp_cache_tbl __attribute__((__aligned__(64))) pp_tbl = {};
	memset(&pp_tbl, 0x0, sizeof(pp_cache_tbl));

	/* Load from USB file */
	sprintf(cmd_buf,"%s 0x%x %s", source_cmd, CONFIG_FWUP_RAM_START, fname);
	ret = run_command(cmd_buf, 0);
	if(ret != 0) {
		if(!strcmp(source_cmd,"tftp"))
			printf("Load %s from ethernet fail !\n",fname);
		else
			printf("Load %s from usb fail !\n",fname);
		goto out;
	}

	file_size = simple_strtoul(env_get("filesize"), NULL, 16);
	if (file_size > sizeof(pp_cache_tbl)) {
		printf("Error! write size over misc pp_cache_tbl entry size!\n");
		goto out;
	}

	memcpy(&pp_tbl, (const void *)CONFIG_FWUP_RAM_START, sizeof(pp_cache_tbl));

	/* Write to misc */
	if (strlen(ptn_name) == strlen("pptbl0")
		&& (!strcmp(ptn_name, "pptbl0"))) {
		pptbl_offset = PPTBL0_FLAG_OFFSET;
	} else {
		pptbl_offset = PPTBL1_FLAG_OFFSET;
	}

	ret = nvt_nand_misc_op((unsigned int)&pp_tbl, pptbl_offset, sizeof(pp_cache_tbl), OP_MISC_WRITE);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}
	ret = 0;

out:
	return ret;
}

int nvt_nand_erase_pptbl(char *ptn_name)
{
	int ret = -1;
	unsigned int pptbl_offset;
	pp_cache_tbl __attribute__((__aligned__(64))) pp_tbl = {};
	memset(&pp_tbl, 0x0, sizeof(pp_cache_tbl));

	/* Write to misc */
	if (strlen(ptn_name) == strlen("pptbl0")
		&& (!strcmp(ptn_name, "pptbl0"))) {
		pptbl_offset = PPTBL0_FLAG_OFFSET;
	} else {
		pptbl_offset = PPTBL1_FLAG_OFFSET;
	}

	ret = nvt_nand_misc_op((unsigned int)&pp_tbl, pptbl_offset, sizeof(pp_cache_tbl), OP_MISC_WRITE);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}

	ret = 0;
out:
	return ret;
}

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
	int ret = -1;
	unsigned int pptbl_offset;
	pp_cache_tbl __attribute__((__aligned__(64))) pp_tbl = {};

	memset(&pp_tbl, 0x0, sizeof(pp_cache_tbl));
	ret = nvt_nand_misc_op((unsigned int)&pp_tbl, PPTBL0_FLAG_OFFSET, sizeof(pp_cache_tbl), OP_MISC_READ);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}
	printf("pp table 0 :\n");
	nvt_show_pptbl(&pp_tbl);

	memset(&pp_tbl, 0x0, sizeof(pp_cache_tbl));
	ret = nvt_nand_misc_op((unsigned int)&pp_tbl, PPTBL1_FLAG_OFFSET, sizeof(pp_cache_tbl), OP_MISC_READ);
	if (ret != 0) {
		printf("%s: nvt_nand_misc_op fail\n", __func__);
		goto out;
	}
	printf("pp table 1 :\n");
	nvt_show_pptbl(&pp_tbl);

	ret = 0;

out:
	return ret;
}

U_BOOT_CMD(
	ndump_pptbl, 1, 1, do_nvt_dump_pp_table,
	"ndump_pptbl - dump pp table from misc partition",
	""
);
#endif

char nwrite_help[4096]= {0};
char nerase_help[4096]= {0};
static struct cmd_mapping *cmdmap;
void _nvt_fwupdate_help_init(struct cmd_mapping *_cmdmap)
{
	int i = 0;
	cmdmap = _cmdmap;
	sprintf(nwrite_help, "\n");
	char line[512];
	for(i=0; cmdmap[i].cmd != NULL; i++ ) {
		sprintf(line, "% 16s - write %s into nand flash [default: %s]\n", cmdmap[i].cmd, cmdmap[i].description, cmdmap[i].fname);
		strcat(nwrite_help, line);
	}

	sprintf(nerase_help, "");
	for(i=0; cmdmap[i].cmd != NULL; i++ ) {
		if (i == 0)
			sprintf(line, "% 9s - erase %s\n", cmdmap[i].cmd, cmdmap[i].description);
		else
			sprintf(line, "% 16s - erase %s\n", cmdmap[i].cmd, cmdmap[i].description);
		strcat(nerase_help, line);
	}

	return;
}

static int do_nvt_nwrite_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;
	int i = 0;
	int isRun = 0;
	if(argc != 2 && argc != 3) {
		return cmd_usage(cmdtp);
	}

	for(i=0; cmdmap[i].cmd != NULL; i++ ) {
		if(!strcmp(argv[1], cmdmap[i].cmd)) {
			isRun = 1;
			if(argc >= 3) {
				ret = cmdmap[i].cmd_update_handler(cmdmap[i].cmd, argv[2]);
			}else{
				ret = cmdmap[i].cmd_update_handler(cmdmap[i].cmd, cmdmap[i].fname);
			}

			if(ret < 0)
				goto err_out;

			break;
		}
	}

	if (isRun == 0) {
		printf("No found exection argument!!\n");
		ret = -1;
		goto err_out;
	}
	ret = 0;

err_out:
	return ret;

}
U_BOOT_CMD(
	nwrite,	3,	0,	do_nvt_nwrite_cmd,
	"nwrite - write image to nand flash",
	nwrite_help
);

static int do_nvt_erase_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;
	int i = 0;
	int isRun = 0;
	if(argc != 2) {
		return cmd_usage(cmdtp);
	}

	for(i=0; cmdmap[i].cmd != NULL; i++ ) {
		if(!strcmp(argv[1], cmdmap[i].cmd)) {
			isRun = 1;
			ret = cmdmap[i].cmd_erase_handler(argv[1]);
			if(ret < 0)
				goto err_out;
			break;
		}
	}

	if (isRun == 0) {
		printf("No found exection argument!!\n");
		ret = -1;
	}
	ret = 0;

err_out:
	return ret;
}

U_BOOT_CMD(
	nerase,	2,	0,	do_nvt_erase_cmd,
	"nerase partition - erase specific image partition in nand flash.",
	nerase_help
);

static int do_nvt_nset(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    int key_id;
    int key_count;
	char cmd_buf[256] = {0};

	if(argc != 3) {
		return cmd_usage(cmdtp);
	}

	if(!strcmp(argv[1], "inv")) {
		if(!strcmp(argv[2], "0")) {
			nt72_nand_set_inv(0);
		}
		else if(!strcmp(argv[2], "1")) {
			nt72_nand_set_inv(1);
		}
		else {
			goto err_out;
		}
	}
	else {
		goto err_out;
	}

	return 0;
err_out:
	printf("nset error parameters\n");
	return 1;
}
U_BOOT_CMD(
	nset,	3,	0,	do_nvt_nset,
	"nset - set NFC parameters",
	""
);
