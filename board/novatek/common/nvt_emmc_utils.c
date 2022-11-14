/*
 *  board/novatek/evb670btk/nvt_utils.c
 *
 *  Author:	Alvin lin
 *  Created:	June 21, 2013
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <nvt_stbc.h>
#include <fdt_support.h>
#include <nvt_emmc_fwupdate.h>
#include <nvt_emmc_utils.h>
#include <nvt_emmc_partition.h>
#include <nvt_tzone.h>
#include <asm/arch/hardware.h>


#if defined(CONFIG_EVB_670B_EMMC)
int do_nvt_boot_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char cmd[512] = {0};
	char buf[255] = {0};
	int ret = 0,len;
	unsigned char *ker_load_addr = 0;
	unsigned char *ramdisk_load_addr = 0;
	unsigned char *fdt_load_addr = 0;
	nvt_emmc_image_info *pimg;
	int block_cnt = 0;
	int cpus;

	if(NULL != env_get("console"))
		sprintf(cmd, "console=%s,115200 ", env_get("console"));
	else
		sprintf(cmd, "%s ", "console=ttyS0,115200");

	if (env_get("memstr") != NULL)
		sprintf(buf,"%s ",env_get("memstr"));
	else
		sprintf(buf,"%s ","mem=512M");

	strcat(cmd,buf);

	if(env_get("lpj") != NULL) {
		sprintf(buf, "lpj=%s ",env_get("lpj"));
		strcat(cmd, buf);
	}

	if(env_get("dtv_log_addr") != NULL) {
		sprintf(buf, "dtv_log_addr=%s ",env_get("dtv_log_addr"));
		strcat(cmd,buf);
	}


	if (env_get("ethaddr") != NULL ) {
		sprintf(buf,"ethaddr=%s ",env_get("ethaddr"));
		strcat(cmd,buf);
	}

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


	if(env_get("cpus") != NULL) {
		cpus = simple_strtoul(env_get("cpus"),NULL,10);
		if(cpus > 4 || cpus < 1)
			cpus = 4;
	} else {
		cpus = 4;
	}
	sprintf(buf, "maxcpus=%d ",cpus);
	strcat(cmd,buf);

	if(env_get("dump_restore") != NULL)
	{
		strcat(cmd,"rdinit=/linuxrc rootwait ");
	}
	else
	{
		strcat(cmd,"init=/init rootwait ");
	}

	if(env_get("kparam_addon") != NULL) {
		sprintf(buf," %s ", env_get("kparam_addon"));
		strcat(cmd, buf);
	}

	env_set("bootargs",cmd);

#if defined(CONFIG_NVT_TK_TZONE)
	pimg = nvt_emmc_get_img_by_name("secos");
	if(pimg == NULL) {	
		printf("%s get secos image fail !\n",__func__);
		goto loop;
	}

	ret = pimg->read_img(pimg, SECOS_BUFFER_BLOCK, &len, NULL);
#endif	


	ker_load_addr = (unsigned char*)simple_strtoul(env_get("kernel_loadaddr"),NULL,16);

	if(env_get("p3") != NULL) {
		run_command("usb start", 0);
		env_set("ramdisk_loadaddr", NULL);
		sprintf(cmd,"fatload usb 0:auto 0x%x p3.img",(unsigned int)ker_load_addr);
		ret = run_command(cmd, 0);
		goto out;
	}



	if(env_get("ramdisk_loadaddr") != NULL)
		ramdisk_load_addr = (unsigned char*)simple_strtoul(env_get("ramdisk_loadaddr"),NULL,16);
	else {
		ramdisk_load_addr = 0;
		env_set("ramdisk_loadaddr", "-");
	}


	pimg = nvt_emmc_get_img_by_name("ker0");

	if(pimg == NULL) {
		printf("%s get normal kernel image fail !\n",__func__);
		goto loop;
	}


	ret = pimg->read_img(pimg, ker_load_addr, &len, NULL);
	if(ret < 0) {
		printf("load kernel 0 from emmc fail !\n");
		goto loop;
	}


	if(len > 0 && ramdisk_load_addr != 0) {
		const image_header_t *hdr = (const image_header_t *)ramdisk_load_addr;
		sprintf(buf,"initrd=0x%x,0x%x", (unsigned int)(ramdisk_load_addr + sizeof(image_header_t)), image_get_data_size(hdr));
		strcat(cmd, buf);
	}

	//load dtb
	pimg = nvt_emmc_get_img_by_name("fdt0");
	if(pimg == NULL) {
		printf("%s get dtb image fail !\n",__func__);
		goto loop;
	}

	fdt_load_addr = (unsigned char*)simple_strtoul(env_get("fdt_high"),NULL,16);

	ret = pimg->read_img(pimg, fdt_load_addr, &len, NULL);

	if(ret < 0) {
		printf("load dtb from emmc fail !\n");
		goto loop;
	}

	env_set("bootargs",cmd);

out:
	usb_power_off();

	return ret;

loop:
	while(1);
}

U_BOOT_CMD(
		nvt_boot,	2,	0,	do_nvt_boot_cmd,
		"",""
	  );

#endif

#if defined(CONFIG_NVT_EUPM)
static int do_nvt_eupm(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char cmd[512] = {0};
	unsigned long fdt_high = 0;
	unsigned long kernel_loadaddr = 0;
	nvt_emmc_image_info *pimg;
	image_header_t *phdr;
	unsigned int fdt_size;
	int len;

	if(!env_get_yesno("no_secos")) {

		pimg = nvt_emmc_get_img_by_name("secos");
		if(pimg == NULL) {
			printf("%s get secos image fail !\n",__func__);
			goto out;
		}

		pimg->read_img(pimg, (unsigned char*)SECOS_ENC_BUFFER_BLOCK, &len, NULL);
	}

	pimg = nvt_emmc_get_img_by_name("kerupm");
	
	if(pimg == NULL) {
		printf("%s get kerupm image fail !\n", __func__);
		goto out;
	}

	kernel_loadaddr = simple_strtoul(env_get("kernel_loadaddr"), NULL, 16);
	pimg->read_img(pimg, (unsigned char*)kernel_loadaddr, &len, NULL);

	fdt_high = simple_strtoul(env_get("kernel_loadaddr"),NULL,16) + 0x40 + 0x2000000;
	sprintf(cmd, "0x%x", fdt_high);
	env_set("fdt_high", cmd);

	pimg = nvt_emmc_get_img_by_name("fdt0");
	if(pimg == NULL) {
		printf("%s get fdt0 image fail !\n", __func__);
		goto out;
	}
	pimg->read_img(pimg, (unsigned char*)fdt_high, &len, NULL);

	phdr = (image_header_t*)fdt_high;
	if(image_get_magic(phdr) == IH_MAGIC) {
		fdt_size = image_get_size(phdr);
		memmove((void*)(fdt_high - sizeof(image_header_t)), (const void*)fdt_high, fdt_size + sizeof(image_header_t));
	}

	sprintf(cmd, "console=ttyS0,115200 %s maxcpus=2 init=/init quiet rootwait earlyprintk", env_get("memstr"));

	if(env_get("eupm_param") != NULL)
		strcat(cmd, env_get("eupm_param"));

	env_set("bootargs", cmd);

	sprintf(cmd, "bootm %s - %s", env_get("kernel_loadaddr"), env_get("fdt_high"));
	run_command(cmd, 0);
out:	
	return 0;
}
U_BOOT_CMD(
	eupm,	1,	0,	do_nvt_eupm,
	"eupm",
	""
);

#endif //CONFIG_NVT_EUPM

#if defined(CONFIG_NVT_TK_LED_BLINK)
void nvt_tk_led_init(void)
{
}

void nvt_tk_led_ctrl(int on_off)
{
}

void nvt_tk_led_blink(void)
{
}
#endif


#if (1)
static int do_nvt_mark_format_db(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	nvt_emmc_image_info *pimg;

	if(argc != 2) {
		ret = CMD_RET_USAGE;
		goto out;
	}

	pimg = nvt_emmc_get_img_by_name(argv[1]);
	if(pimg == NULL) {
		printf("can not find database partition %s !\n", argv[1]);
		ret = CMD_RET_USAGE;
		goto out;
	}

	strcpy((char*)CONFIG_SYS_FWUPDATE_BUF, "ERASEDB");

	ret = pimg->write_img(pimg, (unsigned char*)CONFIG_SYS_FWUPDATE_BUF, strlen("ERASEDB"), NULL);
out:
	return ret;
}
U_BOOT_CMD(
	emark_format_db,	2,	1,	do_nvt_mark_format_db,
	"emark_format_db db_part_name",
	""
);

#endif

#if defined(CONFIG_ARM64)

static int nvt_parse_mem_str(char* memstr, u64 *start, u64 *size)
{
	char *ptr1;
	char *ptr2;
	int ret = 0;
	int shift_bits;
	u64 tmp_val;

	ptr1 = memstr;
	ptr2 = strchr(ptr1, '@');
	if(ptr2 == NULL) {
		printf("%s invalid memstr format !\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	switch(*(ptr2-1)) {
		case 'k':
		case 'K':
			shift_bits = 10;
			break;

		case 'm':
		case 'M':
			shift_bits = 20;
			break;

		case 'g':
		case 'G':
			shift_bits = 30;
			break;

		default:
			printf("%s invalid size !\n", __func__);
			ret = -EINVAL;
			goto out;
	}

	//get size
	*(ptr2 - 1) = '\0';
	tmp_val = simple_strtoull(ptr1, NULL, 10);
	*size = tmp_val << shift_bits;

	//get start 
	ptr1 = (ptr2+1);
	ptr2 = ptr1 + strlen(ptr1);

	switch(*(ptr2-1)) {
		case 'k':
		case 'K':
			shift_bits = 10;
			break;

		case 'm':
		case 'M':
			shift_bits = 20;
			break;

		case 'g':
		case 'G':
			shift_bits = 30;
			break;

		default:
			printf("%s invalid start !\n", __func__);
			ret = -EINVAL;
			goto out;
	}

	tmp_val = simple_strtoull(ptr1, NULL, 10);
	*start = tmp_val << shift_bits;

out:
	return ret;
}

int nvt_set_fdt_ker_mem_range(unsigned char* fdt_buf)
{
	int ret = 0;
	char kermem_str[1024] = {};//the format is "128M@64M,256M@2048M...", max 4 banks
	char *ptr;
	char *tmp_ptr;
	char tmp_str[64] = {};
	u64 start_addr[CONFIG_NR_DRAM_BANKS] = {};
	u64 mem_size[CONFIG_NR_DRAM_BANKS] = {};
	u64 tmp_start;
	u64 tmp_size;
	int total_bank = 0;

	//check fdt sanity
	if(fdt_check_header(fdt_buf) < 0) {
		printf("%s invalid fdt format !\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	//get kernel mem string setting
	if(env_get("kermem") == NULL) {
		printf("%s no kermem assigned !\n", __func__);
		ret = -EINVAL;
		goto out;
	}

	//parse bank setting
	strncpy(kermem_str, env_get("kermem"), sizeof(kermem_str));
	ptr = kermem_str;

	while(1) {
		memset(tmp_str, 0, sizeof(tmp_str));

		tmp_ptr = strchr(ptr, ',');

		if(tmp_ptr == NULL) 
			memcpy(tmp_str, ptr, strlen(ptr));
		else 
			memcpy(tmp_str, ptr, (tmp_ptr - ptr));

		ret = nvt_parse_mem_str(tmp_str, &tmp_start, &tmp_size);

		if(ret != 0)
			break;

		start_addr[total_bank] = tmp_start;
		mem_size[total_bank] = tmp_size;
		total_bank++;

		if(total_bank >= CONFIG_NR_DRAM_BANKS)
			break;

		ptr = strchr(ptr, ',');

		if(ptr == NULL)
			break;
		else 
			ptr++;
	}

	if(ret == 0)
		ret = fdt_fixup_memory_banks(fdt_buf, start_addr, mem_size, total_bank);

out:
	return ret;
}

#if defined(DEBUG)

int do_nvt_kermem(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_set_fdt_ker_mem_range((unsigned char*)simple_strtoull(argv[1], NULL, 16));
	return 0;
}

U_BOOT_CMD(
	nvt_kermem, 3, 1, do_nvt_kermem,
	"nvt_kermem - arm64 kermem debug command",
	"       \n"
);
#endif

#endif /*CONFIG_ARM64*/

int nvt_mmc_io_test(char* test_part_name, unsigned int run_count)
{
	int ret = 0;
	nvt_emmc_image_info *pimg;
	int i, cnt;
	unsigned int patt_size_in_sector = 10 * (1 << 11);
	unsigned int patt_size_in_byte = patt_size_in_sector << BYTE_TO_SECTOR_SFT;
	unsigned char *patt1, *patt2, *patt3;
	unsigned int *ptr;
	char cmd[64] = {};
	unsigned long long start_lba;


	pimg = nvt_emmc_get_img_by_name(test_part_name);
	if(pimg == NULL) {
		printf("%s invalid partition name %s\n", __func__, test_part_name);
		ret = -EINVAL;
		goto out;
	}

	start_lba = nvt_emmc_get_part_start_sector(test_part_name);

	//gen patten
	patt1 = (unsigned char*)CONFIG_SYS_FWUPDATE_BUF;
	patt2 = (unsigned char*)(patt1 + patt_size_in_byte);
	patt3 = (unsigned char*)(patt2 + patt_size_in_byte);

	srand(get_timer(0));
	ptr = (unsigned int*)patt1;
	cnt = patt_size_in_byte / sizeof(unsigned int);
	for(i=0;i<cnt;i++)
		ptr[i] = rand();
	memset(patt2, 0, patt_size_in_byte);

	for(i=0;i<run_count;i++) {
		printf("=========mmc io test round : %d=============\n", (i+1));

		//write patten 1
		sprintf(cmd , "mmc write 0x%x 0x%llx 0x%x", patt1, start_lba, patt_size_in_sector);
		ret = run_command(cmd, 0);
		if(ret < 0) {
			printf("%s write patten 1 fail !\n", __func__);
			ret = -EIO;
			goto out;
		}

		//read patten 1
		sprintf(cmd , "mmc read 0x%x 0x%llx 0x%x", patt3, start_lba, patt_size_in_sector);
		ret = run_command(cmd, 0);
		if(ret < 0) {
			printf("%s read patten 1 fail !\n", __func__);
			ret = -EIO;
			goto out;
		}

		//compare
		if(memcmp(patt3, patt1, patt_size_in_byte)) {
			printf("%s patten not match !\n", __func__);
			ret = -EIO;
			goto out;
		}

		//reset to 0
		sprintf(cmd, "mmc write 0x%x 0x%llx 0x%x", patt2, start_lba, patt_size_in_sector);
		ret = run_command(cmd, 0);
		if(ret < 0) {
			printf("%s reset partition fail !\n", __func__);
			ret = -EIO;
			goto out;
		}
	}

out:
	if(pimg != NULL) {
		if(pimg->part_type == PART_TYPE_FS) {
			sprintf(cmd, "emark_format_db %s", test_part_name);
			run_command(cmd, 0);
		} else {
			sprintf(cmd, "eerase %s", test_part_name);
			run_command(cmd, 0);
		}
			
	}
	return ret;
}
