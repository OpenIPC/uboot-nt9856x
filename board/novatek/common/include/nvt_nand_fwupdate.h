#ifndef __NVT_NAND_FWUPDATE_H__
#define __NVT_NAND_FWUPDATE_H__

#include <common.h>
#include <command.h>

struct cmd_mapping {
        char *cmd;
        char *fname;
        int (*cmd_update_handler)(char *ptn_name, char *fname);
        int (*cmd_erase_handler)(char *ptn_name);
        char *description;
};

void _nvt_fwupdate_help_init(struct cmd_mapping *cmdmap);
int nvt_nand_update_npt(char *ptn_name, char *fname);
int nvt_nand_update_bin(char *ptn_name, char *fname);
int nvt_nand_update_ubi(char *ptn_name, char *fname);
int nvt_nand_update_ubifs(char *vol_name, char *fname);
int nvt_nand_update_stbc(char *ptn_name, char *fname);
int nvt_nand_update_stbc_by_ECC_mode(char *ptn_name, char *fname, int ECC_mode);
int nvt_nand_update_stbc_with_RS(char *ptn_name, char *fname);
int nvt_nand_update_stbc_with_BCH8(char *ptn_name, char *fname);
int nvt_nand_update_loader(char *ptn_name, char *fname);
int nvt_nand_update_fdt(char *ptn_name, char *fname);
int nvt_nand_update_cmdx(char *ptn_name, char *fname);

int nvt_nand_read_secos_image(unsigned char* buf);
int nvt_nand_read_binary_image(char* part_name, unsigned char* buf);

int nvt_nand_erase_common(char *ptn_name);
int nvt_nand_erase_ubifs(char *vol_name);

#if defined(CONFIG_NVT_TURNKEY_FWUPDATE)
int nvt_nand_update_pptbl(char *ptn_name, char *fname);
int nvt_nand_erase_pptbl(char *ptn_name);
#endif


#endif

