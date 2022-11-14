/*
 *  nvt_tkfm.c
 *
 *  Author:	Alvin lin
 *  Created:	Jun 9, 2017
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#include <config.h>
#include <nvt_emmc_partition.h>
#include <asm/arch/hardware.h>
#include <nvt_tzone.h>

#if defined(CONFIG_NVT_FM_TOOL)
static int do_nvt_tk_fm(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	char cmd[512] = {0};
	unsigned int fdt_high = 0;
	unsigned int drv_fdt_high = 0;
	nvt_emmc_image_info *pimg;
	int len;
	int ret;
	unsigned int ker_load_addr;
	image_header_t *phdr;
	int with_common_header = 0;

	env_set("no_secos", "n");

	pimg = nvt_emmc_get_img_by_name("secos");
	if(pimg == NULL) {
		printf("%s get secos image fail !\n",__func__);
		goto out;
	}

	pimg->read_img(pimg, (unsigned char*)SECOS_ENC_BUFFER_BLOCK, &len, NULL);

	run_command("usb reset", 0);

	ker_load_addr = simple_strtoul(env_get("kernel_loadaddr"), NULL, 16);

	sprintf(cmd, "fatload usb 0:auto 0x%x nvtfm_kernel", ker_load_addr);
	ret = run_command(cmd, 0);
	if(ret != 0) {
		printf("Cmd fail......\n");
		run_command("reboot", 0);
		while(1);
	}

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
		with_common_header = 1;
		memmove((void*)(fdt_high - sizeof(image_header_t)), (void*)fdt_high, image_get_size(phdr) + sizeof(image_header_t));
	}

	drv_fdt_high = fdt_high + 0x80000;
	pimg = nvt_emmc_get_img_by_name("memdtb");
	if(pimg == NULL) {
		printf("%s get memdtb image fail !\n", __func__);
		goto out;
	}
	pimg->read_img(pimg, (unsigned char*)drv_fdt_high, &len, NULL);


	sprintf(cmd, "console=ttyS1,115200 %s maxcpus=2 init=/init rootwait earlyprintk", env_get("memstr"));
	env_set("bootargs", cmd);

	//enable uart A for debug
	__REG(0xFD0F002C) &= ~((0xF << 20) | (0xF << 24));
	__REG(0xFD0F002C) |= ((4 << 20) | (4 << 24));

	sprintf(cmd, "bootm %s - %s", env_get("kernel_loadaddr"), env_get("fdt_high"));
	run_command(cmd, 0);
out:	
	return 0;
}
U_BOOT_CMD(
	nvttkfm,	1,	0,	do_nvt_tk_fm,
	"nvttkfm",
	""
);

#endif //CONFIG_NVT_FM_TOOL
