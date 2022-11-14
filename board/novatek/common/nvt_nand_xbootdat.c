#include <asm-generic/errno.h>
//#include <asm/global_data.h>
#include <nvt_nand_partition.h>
#include <nvt_nand_xbootdat.h>
#include <common.h>

#if defined(CONFIG_NT72673_A32_NAND_EVB)
#include "../nt72673_a32_nand_evb/nt72673_nand_nvt_xbootdat.c"
#elif defined(CONFIG_NT72673_N33005_NAND)
#include "../nt72673_n33005_nand/n33005_nand_nvt_xbootdat.c"
#else
#error "You should define your production xbootdat info first !"
#endif

nvt_nand_xbootdat_info* nvt_nand_get_xdat_by_name(char *name)
{
	int i, total_cnt = sizeof(xdat_info) / sizeof(nvt_nand_xbootdat_info);

	for(i=0;i < total_cnt;i++) {
		if(strcmp(xdat_info[i].xdat_name, name) == 0)
			break;
	}

	if(i >= total_cnt)
		return NULL;
	else
		return (&(xdat_info[i]));
}

static u32 nvt_nand_get_xdat_size(void)
{
	return sizeof(xdat_info) / sizeof(nvt_nand_xbootdat_info);
}

static nvt_nand_xbootdat_info* nvt_nand_get_xdat_by_index(u32 idx)
{
	int total_cnt = sizeof(xdat_info) / sizeof(nvt_nand_xbootdat_info);

	if (idx >= total_cnt)
		return NULL;
	else
		return(&(xdat_info[idx]));
}

void nvt_nand_xbootdat_init(void)
{
	int i;
	unsigned int xbootdat_start_block;
	xbootdat_start_block=nand_calc_partition_block(nvt_nand_get_ptn_index("xbootdat"));

	for(i=0; i < (sizeof(xdat_info) / sizeof(nvt_nand_xbootdat_info)); i++)
		xdat_info[i].start_block += xbootdat_start_block;
}

int do_nvt_xsave(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_nand_xbootdat_info *pxdat_info;
	int ret = 0;

	if(argc < 2) { 
		ret = CMD_RET_USAGE;
		goto out;
	}

	if(strcmp(argv[1], "help") == 0) {
		int i, total_cnt;

		total_cnt = nvt_nand_get_xdat_size();

		for(i = 0; i < total_cnt;i++) {
			pxdat_info = nvt_nand_get_xdat_by_index(i);

			if(pxdat_info != NULL)
				pxdat_info->xdathelp(pxdat_info, XDAT_HELP_SAVE);
		}

		goto out;
	}


	pxdat_info = nvt_nand_get_xdat_by_name(argv[1]);

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
		"xsave panel config config_file_name\n"
);

int do_nvt_xdump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_nand_xbootdat_info *pxdat_info;
	int ret = 0;

	if(argc != 2) { 
		ret = CMD_RET_USAGE;
		goto out;
	}

	if(strcmp(argv[1], "help") == 0) {
		int i, total_cnt;

		total_cnt = nvt_nand_get_xdat_size();

		for(i = 0; i < total_cnt;i++) {
			pxdat_info = nvt_nand_get_xdat_by_index(i);

			if(pxdat_info != NULL)
				pxdat_info->xdathelp(pxdat_info, XDAT_HELP_DUMP);
		}

		goto out;
	}


	pxdat_info = nvt_nand_get_xdat_by_name(argv[1]);

	if(pxdat_info == NULL) {
		ret = CMD_RET_USAGE;
		goto out;
	}


	ret = pxdat_info->xdatdump(pxdat_info, argc, argv);


out:
	return ret;
}

U_BOOT_CMD(
		xdump, 2, 1, do_nvt_xdump,
		"dump xbootdat parameter",
		"xdump panel - dump panel parameters in xbootdat\n"
		"xdump kercmd - dump kernel command in xbootdat\n"
);

int do_nvt_xerase(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nvt_nand_xbootdat_info *pxdat_info;
	int ret = 0;

	if(argc != 2) { 
		ret = CMD_RET_USAGE;
		goto out;
	}

	if(strcmp(argv[1], "help") == 0) {
		int i, total_cnt;

		total_cnt = nvt_nand_get_xdat_size();

		for(i = 0; i < total_cnt;i++) {
			pxdat_info = nvt_nand_get_xdat_by_index(i);

			if(pxdat_info != NULL)
				pxdat_info->xdathelp(pxdat_info, XDAT_HELP_ERASE);
		}

		goto out;
	}

	pxdat_info = nvt_nand_get_xdat_by_name(argv[1]);

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

