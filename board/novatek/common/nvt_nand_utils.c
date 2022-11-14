#include <common.h>
#include <nvt_nand_utils.h>
#include <nand.h>
#include <nvt_nand_partition.h>
#include <linux/mtd/mtd.h>

int nvt_run_cmd(const char *cmd, int flag, const char *description)
{
	int ret = 0;
	printf("**** %s ****\n", description);
	ret = run_command(cmd, flag);
	if (ret != 0) {
		printf("**** [Fail/Skip] %s ****\n", description);
	} else {
		printf("**** [Done] %s ****\n", description);
	}
	return ret;
}

static int nvt_nand_access(struct mtd_info *mtd, ulong addr, loff_t off,
		ulong count, int read, int is_raw)
{
	int ret = 0;

	while (count--) {
		/* Raw access */
		mtd_oob_ops_t ops = {
			.datbuf = (u8 *)addr,
			.oobbuf = ((u8 *)addr) + mtd->writesize,
			.len = mtd->writesize,
			.ooblen = mtd->oobsize,
			.mode = MTD_OPS_PLACE_OOB
		};

		if (is_raw)
			ops.mode = MTD_OPS_RAW;

		if (read) {
			ret = mtd_read_oob(mtd, off, &ops);
		} else {
			ret = mtd_write_oob(mtd, off, &ops);
			if (!ret)
				ret = nand_verify_page_oob(mtd, &ops, off);
		}

		if (ret) {
			printf("%s: error at offset %llx, ret %d\n",
					__func__, (long long)off, ret);
			break;
		}

		addr += mtd->writesize + mtd->oobsize;
		off += mtd->writesize;
	}

	return ret;
}

static int nvt_nand_dump_noskipbad(struct mtd_info *mtd, loff_t read_off, ulong count,
		char *dump_name, int is_raw)
{
	char cmd_buf[256] = {0};
	unsigned int page_size;
	int cmd_ret;
	int file_idx = 0;
	unsigned int total_size;
	unsigned int read_size;
	unsigned int cnt, write_size;

	total_size = count * mtd->writesize;
	while (total_size > 0) {
		read_size = (total_size > NDUMP_MAX_READ)? NDUMP_MAX_READ:total_size;

		page_size = nand_get_page_size();
		if((read_size % page_size) ){
			read_size = (read_size & ~(page_size - 1)) + page_size;
		}
		cnt = read_size / page_size;
		write_size = cnt * (page_size + mtd->oobsize);
		//Reset MEM
		memset((void *)CONFIG_FWUP_RAM_START, 0xFF, write_size);
		//Read to mem
		cmd_ret = nvt_nand_access(mtd, CONFIG_FWUP_RAM_START, read_off, cnt, 1, is_raw);
		if(cmd_ret == -1) {
			printf("%s: nvt_nand_access Error !\n", __func__);
			goto err_out;
		}

		//Write to file
		sprintf(cmd_buf, "nvtwrite 0x%x %s.%03d 0x%x", CONFIG_FWUP_RAM_START, dump_name, file_idx, write_size);
		printf("cmd: '%s'\n", cmd_buf);
		cmd_ret = run_command(cmd_buf, 0);
		if(cmd_ret == -1) {
			printf("Error ! '%s' fail !\n", __func__, cmd_buf);
			goto err_out;
		}
		read_off += read_size;
		total_size -= read_size;
		file_idx ++;
	}
	return 0;

err_out:
	return 1;
}

static int nvt_nand_ndumpchip(struct mtd_info *mtd, int is_raw)
{
	unsigned int chip_cnt;
	struct nand_chip *chip = mtd->priv;

	chip_cnt = chip->chipsize / nand_get_page_size();
	if (is_raw)
		return nvt_nand_dump_noskipbad(mtd, 0, chip_cnt, "chipdump_raw", is_raw);
	else
		return nvt_nand_dump_noskipbad(mtd, 0, chip_cnt, "chipdump", is_raw);
}

#define TOKEN_KEY "nova.no1"
static int token_pass = 0;
static int do_nvt_nand_utils(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	struct mtd_info *mtd = nand_info[nand_curr_device];
	char *cmd = NULL;
	char *s = NULL;
	ulong addr = 0;
	ulong off = 0;
	ulong count = 0;
	int raw = 0;

	if (argc < 2)
		return cmd_usage(cmdtp);

	cmd = argv[1];

	if (strncmp(cmd, "token", strlen("token")) == 0) {
		if (argc == 3) {
			if (strcmp(argv[2], TOKEN_KEY) == 0) {
				token_pass = 1;
				printf("PASS, you are right!\n");
				return 0;
			}
		}
	} else if (strncmp(cmd, "read_noskipbad", strlen("read_noskipbad")) == 0) {
		if (token_pass == 0) return 0;
		if (argc != 5)
			return cmd_usage(cmdtp);
		s = strchr(cmd, '.');
		if (s && !strcmp(s, ".raw")) {
			raw = 1;
		}

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		off = (ulong)simple_strtoul(argv[3], NULL, 16);
		count = (ulong)simple_strtoul(argv[4], NULL, 16);
		count = (count + mtd->writesize - 1) / mtd->writesize;

		printf("read_noskipbad raw: %d\n", raw);
		return nvt_nand_access(mtd, addr, off, count, 1, raw);
	} else if (strncmp(cmd, "dump_noskipbad", strlen("dump_noskipbad")) == 0) {
		if (token_pass == 0) return 0;
		if (argc != 5)
			return cmd_usage(cmdtp);
		s = strchr(cmd, '.');
		if (s && !strcmp(s, ".raw")) {
			raw = 1;
		}
		off = (ulong)simple_strtoul(argv[2], NULL, 16);
		count = (ulong)simple_strtoul(argv[3], NULL, 16);
		count = (count + mtd->writesize - 1) / mtd->writesize;

		return nvt_nand_dump_noskipbad(mtd, off, count, argv[4], raw);

	} else if (strncmp(cmd, "dumpchip", strlen("dumpchip")) == 0) {
		if (token_pass == 0) return 0;
		if (argc != 2)
			return cmd_usage(cmdtp);

		s = strchr(cmd, '.');
		if (s && !strcmp(s, ".raw")) {
			raw = 1;
		}
		printf("nvt_nand_ndumpchip raw: %d\n", raw);
		return nvt_nand_ndumpchip(mtd, raw);
	} else {
		return cmd_usage(cmdtp);
	}
	return 0;
}

static char nvt_nand_help_text[] =
	"token - you know\n"
	"nvt_nand read_noskipbad[.raw] - <addr> <offset> <size>\n"
	"		Read NAND data to DRAM addr.\n"
	"			(with OOB) (no skip BAD) (w/wo ECC)\n"
	"nvt_nand dump_noskipbad[.raw] - <offset> <size> <prefix file name>\n"
	"		Dump NAND data to USB.\n"
	"			(with OOB) (no skip BAD) (w/wo ECC)\n"
	"nvt_nand dumpchip[.raw]\n"
	"		Dump NAND Chip data to USB.\n"
	"			(with OOB) (no skip BAD) (w/wo ECC)\n"
	"";

U_BOOT_CMD(
	nvt_nand, CONFIG_SYS_MAXARGS, 1, do_nvt_nand_utils,
	"nvt_nand utils", nvt_nand_help_text
);

