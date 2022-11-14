#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <nvt_common_utils.h>

char *source_cmd = "nvtload";
static int do_nvt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char cmd_buf[256] = {0};
	int cmd_ret;

	if((strncmp(argv[0], "nvtload", 7) == 0))
	{
		if (argc < 3) {
			printf("usage: <addr> <filename> [bytes]\n");
			return 1;
		}
	
		run_command("usb start", 0);
		if (argv[3] != NULL) {
			sprintf(cmd_buf,"fatload usb 0:auto %s %s %s", argv[1], argv[2], argv[3]);
		} else {
			sprintf(cmd_buf,"fatload usb 0:auto %s %s", argv[1], argv[2]);
		}
		cmd_ret = run_command(cmd_buf, 0);
		if (cmd_ret != 0)
			return 1;
	}
#ifdef CONFIG_FAT_WRITE
	else if((strncmp(argv[0], "nvtwrite", 8) == 0))
	{
		if (argc != 4) {
			printf("usage: <addr> <filename> <bytes(hex)>\n");
			return 1;
		}

		run_command("usb start", 0);
		sprintf(cmd_buf,"fatwrite usb 0:auto %s %s %s", argv[1], argv[2], argv[3]);
		cmd_ret = run_command(cmd_buf, 0);
		if (cmd_ret != 0)
			return 1;
	}
#endif /*End of CONFIG_FAT_WRITE*/

	return 0;

}

U_BOOT_CMD(
	nvtload,	6,	1,	do_nvt,
	"batch for usb, fatls and fatload commands",
	" - load binary file from a dos filesystem"
	"  <addr> <filename> [bytes]\n"
	"	 - load binary file 'filename' from 'dev' on 'interface'\n"
	"	   to address 'addr' from dos filesystem"
);

#ifdef CONFIG_FAT_WRITE
U_BOOT_CMD(
        nvtwrite,        6,      1,      do_nvt,
        "batch for usb, fatls and fatwrite commands",
        " - write binary file to a dos filesystem"
        "  <addr> <filename> [bytes]\n"
        "        - write binary file 'filename' to 'dev' on 'interface'\n"
        "          to address 'addr' to dos filesystem"
);
#endif

int nvt_in_empty_burner_mode(void)
{
	int ret = 0;
	unsigned int bootstrap;

	bootstrap = (*(volatile unsigned int *)(0xFC040210)) & 0xFF;
	if(bootstrap == 0x000000A0 || bootstrap == 0xB0 || bootstrap == 0x000000C0) 
		ret = 1;
	else
		ret = 0;

	return ret;
}



