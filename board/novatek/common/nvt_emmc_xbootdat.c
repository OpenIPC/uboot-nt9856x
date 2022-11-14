/*
 *  board/novatek/nt72670btk/nvt_emmc_xbootdat.c
 *
 *  Author:	Alvin lin
 *  Created:	Jun 5, 2015
 *  Copyright:	Novatek Inc.
 *
 */
#include <asm-generic/errno.h>
#include <common.h>
#include <asm/global_data.h>
#include <asm/arch/hardware.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <nvt_emmc_xbootdat.h>
#include <nvt_emmc_utils.h>
#include <nvt_emmc_partition.h>
#include <nvt_emmc_fwupdate.h>

#if defined(CONFIG_EVB_172_A32_EMMC)
#include "../nt72172_a32_evb/nt72172_emmc_nvt_xbootdat.c"
#elif defined(CONFIG_EVB_172_A64_EMMC)
#include "../nt72172_a64_evb/nt72172_emmc_nvt_xbootdat.c"
#elif defined(CONFIG_TK_172_A64_ANDROID)
#include "../nt72172_a64_tk_android/nt72172_emmc_tk_android_xbootdat.c"
#elif defined(CONFIG_TK_172_A32_ANDROID)
#include "../nt72172_a32_tk_android/nt72172_emmc_tk_android_xbootdat.c"
#elif defined(CONFIG_TK_172_A32_ANDROID_OPERA)
#include "../nt72172_a32_tk_android_opera/nt72172_emmc_tk_android_opera_xbootdat.c"
#elif defined(CONFIG_TK_172_A32_ANDROID_OREO)
#include "../nt72172_a32_tk_android_oreo/nt72172_emmc_tk_android_oreo_xbootdat.c"
#elif defined(CONFIG_EVB_673_A32_EMMC)
#include "../nt72673_a32_emmc_evb/nt72673_emmc_nvt_xbootdat.c"
#elif defined(CONFIG_EVB_673_A32_EMMC_CHINA_DRM)
#include "../nt72673_a32_emmc_china_drm/nt72673_emmc_nvt_xbootdat.c"
#elif defined(CONFIG_EVB_673_A64_EMMC)
#include "../nt72673_a64_emmc_evb/nt72673_emmc_nvt_xbootdat.c"
#elif defined(CONFIG_TK_673_A32_ANDROID_OREO)
#include "../nt72673_a32_tk_android_oreo/nt72673_emmc_tk_android_oreo_xbootdat.c"
#elif defined(CONFIG_TK_673_A32_ANDROID_OREO_SEC)
#include "../nt72673_a32_tk_android_oreo_sec/nt72673_emmc_tk_android_oreo_sec_xbootdat.c"
#elif defined(CONFIG_N69004_A32_ANDROID_OREO)
#include "../n69004_a32_android_oreo/n69004_emmc_android_oreo_xbootdat.c"
#elif defined(CONFIG_N62002_A32_ANDROID_OREO)
#include "../n62002_a32_android_oreo/n62002_emmc_android_oreo_xbootdat.c"
#elif defined(CONFIG_N62002_A32_ANDROID_OREO_SEC)
#include "../n62002_a32_android_oreo_sec/n62002_emmc_android_oreo_sec_xbootdat.c"
#elif defined(CONFIG_TK_673_A64_ANDROID_OREO)
#include "../nt72673_a64_tk_android_oreo/nt72673_emmc_tk_android_oreo_xbootdat.c"
#elif defined(CONFIG_EVB_671_A32_EMMC)
#include "../nt72671_a32_emmc_evb/nt72671_emmc_nvt_xbootdat.c"
#elif defined(CONFIG_TK_671_A32_ANDROID_OREO)
#include "../nt72671_a32_tk_android_oreo/nt72671_emmc_tk_android_oreo_xbootdat.c"
#else
#error "You should define your production xbootdat info first !"
#endif

nvt_emmc_xbootdat_info* nvt_emmc_get_xdat_by_name(char *name)
{
	int i, total_cnt = sizeof(xdat_info) / sizeof(nvt_emmc_xbootdat_info);

	for(i=0;i < total_cnt;i++) {
		if(strcmp(xdat_info[i].xdat_name, name) == 0)
			break;
	}

	if(i >= total_cnt)
		return NULL;
	else
		return (&(xdat_info[i]));
}

static u32 nvt_emmc_get_xdat_size(void)
{
	return sizeof(xdat_info) / sizeof(nvt_emmc_xbootdat_info);
}

static nvt_emmc_xbootdat_info* nvt_emmc_get_xdat_by_index(u32 idx)
{
	int total_cnt = sizeof(xdat_info) / sizeof(nvt_emmc_xbootdat_info);

	if (idx >= total_cnt)
		return NULL;
	else
		return(&(xdat_info[idx]));
}

void xbootdat_init(unsigned int xbootdat_start_offset)
{
	int i;

	for(i=0; i < (sizeof(xdat_info) / sizeof(nvt_emmc_xbootdat_info)); i++)
		xdat_info[i].start_sector += xbootdat_start_offset;
}

int do_nvt_xsave(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_emmc_xbootdat_info *pxdat_info;
	int ret = 0;

	if(argc < 2) { 
		ret = CMD_RET_USAGE;
		goto out;
	}

	if(strcmp(argv[1], "help") == 0) {
		int i, total_cnt;

		total_cnt = nvt_emmc_get_xdat_size();

		for(i = 0; i < total_cnt;i++) {
			pxdat_info = nvt_emmc_get_xdat_by_index(i);

			if(pxdat_info != NULL)
				pxdat_info->xdathelp(pxdat_info, XDAT_HELP_SAVE);
		}

		goto out;
	}


	pxdat_info = nvt_emmc_get_xdat_by_name(argv[1]);

	if(pxdat_info == NULL) {
		ret = CMD_RET_USAGE;
		goto out;
	}


	ret = pxdat_info->xdatsave(pxdat_info, argc, argv);
out:
	return ret;
}

U_BOOT_CMD(
	xsave, 4, 1, do_nvt_xsave,
	"save parameter to xbootdat",
	"xsave panel index idx_id - save panel index for xboot use\n"
	"xsave kercmd - save kernel command line for xboot use\n"
);

int do_nvt_xdump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_emmc_xbootdat_info *pxdat_info;
	int ret = 0;

	if(argc != 2 && argc != 3) { 
		ret = CMD_RET_USAGE;
		goto out;
	}

	if(strcmp(argv[1], "help") == 0) {
		int i, total_cnt;

		total_cnt = nvt_emmc_get_xdat_size();

		for(i = 0; i < total_cnt;i++) {
			pxdat_info = nvt_emmc_get_xdat_by_index(i);

			if(pxdat_info != NULL)
				pxdat_info->xdathelp(pxdat_info, XDAT_HELP_DUMP);
		}

		goto out;
	}


	pxdat_info = nvt_emmc_get_xdat_by_name(argv[1]);

	if(pxdat_info == NULL) {
		ret = CMD_RET_USAGE;
		goto out;
	}


	ret = pxdat_info->xdatdump(pxdat_info, argc, argv);


out:
	return ret;
}

U_BOOT_CMD(
	xdump, 3, 1, do_nvt_xdump,
	"dump xbootdat parameter",
	"xdump panel - dump panel parameters in xbootdat\n"
	"xdump kercmd - dump kernel command in xbootdat\n"
);

int do_nvt_xerase(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_emmc_xbootdat_info *pxdat_info;
	int ret = 0;

	if(argc != 2) { 
		ret = CMD_RET_USAGE;
		goto out;
	}

	if(strcmp(argv[1], "help") == 0) {
		int i, total_cnt;

		total_cnt = nvt_emmc_get_xdat_size();

		for(i = 0; i < total_cnt;i++) {
			pxdat_info = nvt_emmc_get_xdat_by_index(i);

			if(pxdat_info != NULL)
				pxdat_info->xdathelp(pxdat_info, XDAT_HELP_ERASE);
		}

		goto out;
	}

	pxdat_info = nvt_emmc_get_xdat_by_name(argv[1]);

	if(pxdat_info == NULL) {
		ret = CMD_RET_USAGE;
		goto out;
	}


	ret = pxdat_info->xdaterase(pxdat_info, argc, argv);


out:
	return ret;
}


U_BOOT_CMD(
	xerase, 2, 1, do_nvt_xerase,
	"erase xbootdat parameter",
	"xerase panel - erase panel parameters in xbootdat\n"
	"xerase kercmd -erase kernel command line in xbootdat\n"
);


