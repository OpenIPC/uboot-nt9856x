/*
 *  driver/mtd/spiflash/spinor.c
 *
 *  Author:	Howard Chang
 *  Created:	Jan 16, 2017
 *  Copyright:	Novatek Inc.
 *
 */
#include "config.h"
#include <common.h>
#include <malloc.h>
#include <linux/mtd/mtd.h>
#include <spi_flash.h>
#include "../nvt_flash_spi/nvt_flash_spi_nor_int.h"
#include "../nvt_flash_spi/nvt_flash_spi_int.h"
#include "ext_flash_table.h"
#include <asm/nvt-common/nvt_storage.h>

#define NOR_VERSION "1.0.8"

#ifdef CONFIG_SPI_FLASH_MTD
static struct mtd_info sf_mtd_info;
static char sf_mtd_name[8];
#endif

struct drv_nand_dev_info *info;
#define SIZE_4K  (4*1024)
#define SIZE_64K (64*1024)

const struct spi_flash_params spi_flash_params_table[] = {
	{"AT45DB011D",     0x1f2200, 0x0,       64 * 1024,     4,       0, SECT_4K},
	{"AT45DB021D",     0x1f2300, 0x0,       64 * 1024,     8,       0, SECT_4K},
	{"AT45DB041D",     0x1f2400, 0x0,       64 * 1024,     8,       0, SECT_4K},
	{"AT45DB081D",     0x1f2500, 0x0,       64 * 1024,    16,       0, SECT_4K},
	{"AT45DB161D",     0x1f2600, 0x0,       64 * 1024,    32,       0, SECT_4K},
	{"AT45DB321D",     0x1f2700, 0x0,       64 * 1024,    64,       0, SECT_4K},
	{"AT45DB641D",     0x1f2800, 0x0,       64 * 1024,   128,       0, SECT_4K},
	{"AT25DF321",      0x1f4701, 0x0,       64 * 1024,    64,       0, SECT_4K},
	{"EN25QH16",       0x1c7015, 0x0,       64 * 1024,    32,       0, SECT_4K},
	{"EN25Q32B",       0x1c3016, 0x0,       64 * 1024,    64,       0, 0},
	{"EN25Q64",        0x1c3017, 0x0,       64 * 1024,   128,       0, SECT_4K},
	{"EN25Q128B",      0x1c3018, 0x0,       64 * 1024,   256,       0, 0},
	{"EN25S64",        0x1c3817, 0x0,       64 * 1024,   128,       0, 0},
	{"EN25QH64A",      0x1c7017, 0x0,       64 * 1024,   128, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE5},
	{"EN25QH128",      0x1c7018, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE5},
	{"GD25Q64B",       0xc84017, 0x0,       64 * 1024,   128, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"GD25LQ32",       0xc86016, 0x0,       64 * 1024,    64,       0, SECT_4K},
	{"GD25Q127C",      0xc84018, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"GD25Q256C",      0xc84019, 0x0,       64 * 1024,   512, RD_EXTN, 0},
	{"MX25L2006E",     0xc22012, 0x0,       64 * 1024,     4,       0, 0},
	{"MX25L4005",      0xc22013, 0x0,       64 * 1024,     8,       0, 0},
	{"MX25L8005",      0xc22014, 0x0,       64 * 1024,    16,       0, 0},
	{"MX25L1605D",     0xc22015, 0x0,       64 * 1024,    32,       0, 0},
	{"MX25L3205D",     0xc22016, 0x0,       64 * 1024,    64,       0, 0},
	{"MX25L6405D",     0xc22017, 0x0,       64 * 1024,   128, RD_EXTN, 0},
	{"MX25L12835F",    0xc22018, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE1 | SPI_FLASH_NORMAL_WRITE},
	{"MX25L25635F",    0xc22019, 0x0,       64 * 1024,   512, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE1 | SPI_FLASH_NORMAL_WRITE},
	{"MX25L51235F",    0xc2201a, 0x0,       64 * 1024,  1024, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE1},
	{"MX25L12855E",    0xc22618, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE1},
	{"S25FL008A",      0x010213, 0x0,       64 * 1024,    16,       0, 0},
	{"S25FL016A",      0x010214, 0x0,       64 * 1024,    32,       0, 0},
	{"S25FL032A",      0x010215, 0x0,       64 * 1024,    64,       0, 0},
	{"S25FL064A",      0x010216, 0x0,       64 * 1024,   128,       0, 0},
	{"S25FL128P_256K", 0x012018, 0x0300,   256 * 1024,    64,       0, 0},
	{"S25FL128P_64K",  0x012018, 0x0301,    64 * 1024,   256,       0, 0},
	{"S25FL032P",      0x010215, 0x4d00,    64 * 1024,    64,       0, 0},
	{"S25FL064P",      0x010216, 0x4d00,    64 * 1024,   128,       0, 0},
	{"S25FL128S_256K", 0x012018, 0x4d00,   256 * 1024,    64,       0, SECT_4K},
	{"S25FL128S_64K",  0x012018, 0x4d01,    64 * 1024,   256,       0, 0},
	{"S25FL256S_256K", 0x010219, 0x4d00,   256 * 1024,   128,       0, 0},
	{"S25FL256S_64K",  0x010219, 0x4d01,    64 * 1024,   512,       0, 0},
	{"S25FL512S_256K", 0x010220, 0x4d00,   256 * 1024,   256,       0, 0},
	{"S25FL512S_64K",  0x010220, 0x4d01,    64 * 1024,  1024,       0, 0},
	{"S25FL512S_512K", 0x010220, 0x4f00,   256 * 1024,   256,       0, 0},
	{"M25P10",         0x202011, 0x0,       32 * 1024,     4,       0, 0},
	{"M25P20",         0x202012, 0x0,       64 * 1024,     4,       0, 0},
	{"M25P40",         0x202013, 0x0,       64 * 1024,     8,       0, 0},
	{"M25P80",         0x202014, 0x0,       64 * 1024,    16,       0, 0},
	{"M25P16",         0x202015, 0x0,       64 * 1024,    32,       0, 0},
	{"M25P32",         0x202016, 0x0,       64 * 1024,    64,       0, 0},
	{"M25P64",         0x202017, 0x0,       64 * 1024,   128,       0, 0},
	{"M25P128",        0x202018, 0x0,      256 * 1024,    64,       0, 0},
	{"N25Q32",         0x20ba16, 0x0,       64 * 1024,    64, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q32A",        0x20bb16, 0x0,       64 * 1024,    64, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q64",         0x20ba17, 0x0,       64 * 1024,   128, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q64A",        0x20bb17, 0x0,       64 * 1024,   128, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q128",        0x20ba18, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q128A",       0x20bb18, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q256",        0x20ba19, 0x0,       64 * 1024,   512, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q256A",       0x20bb19, 0x0,       64 * 1024,   512, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q512",        0x20ba20, 0x0,       64 * 1024,  1024, RD_FULL, E_FSR | SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q512A",       0x20bb20, 0x0,       64 * 1024,  1024, RD_FULL, E_FSR | SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q1024",       0x20ba21, 0x0,       64 * 1024,  2048, RD_FULL, E_FSR | SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"N25Q1024A",      0x20bb21, 0x0,       64 * 1024,  2048, RD_FULL, E_FSR | SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"XM25QH128A",     0x207018, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE3},
	{"SST25VF040B",    0xbf258d, 0x0,       64 * 1024,     8,       0, SECT_4K},
	{"SST25VF080B",    0xbf258e, 0x0,       64 * 1024,    16,       0, SECT_4K},
	{"SST25VF016B",    0xbf2541, 0x0,       64 * 1024,    32,       0, SECT_4K},
	{"SST25VF032B",    0xbf254a, 0x0,       64 * 1024,    64,       0, SECT_4K},
	{"SST25VF064C",    0xbf254b, 0x0,       64 * 1024,   128,       0, SECT_4K},
	{"SST25WF512",     0xbf2501, 0x0,       64 * 1024,     1,       0, SECT_4K},
	{"SST25WF010",     0xbf2502, 0x0,       64 * 1024,     2,       0, SECT_4K},
	{"SST25WF020",     0xbf2503, 0x0,       64 * 1024,     4,       0, SECT_4K},
	{"SST25WF040",     0xbf2504, 0x0,       64 * 1024,     8,       0, SECT_4K},
	{"SST25WF080",     0xbf2505, 0x0,       64 * 1024,    16,       0, SECT_4K},
	{"W25P80",         0xef2014, 0x0,       64 * 1024,    16,       0, 0},
	{"W25P16",         0xef2015, 0x0,       64 * 1024,    32,       0, 0},
	{"W25P32",         0xef2016, 0x0,       64 * 1024,    64,       0, 0},
	{"W25X40",         0xef3013, 0x0,       64 * 1024,     8,       0, SECT_4K},
	{"W25X16",         0xef3015, 0x0,       64 * 1024,    32,       0, SECT_4K},
	{"W25X32",         0xef3016, 0x0,       64 * 1024,    64,       0, SECT_4K},
	{"W25X64",         0xef3017, 0x0,       64 * 1024,   128,       0, SECT_4K},
	{"W25Q40CL",       0xef4013, 0x0,       64 * 1024,     8, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q40BV",       0xef4013, 0x0,       64 * 1024,    16, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q80BL",       0xef4014, 0x0,       64 * 1024,    16, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q16CL",       0xef4015, 0x0,       64 * 1024,    32, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q32BV",       0xef4016, 0x0,       64 * 1024,    64, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q64FV",       0xef4017, 0x0,       64 * 1024,   128, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q128BV",      0xef4018, 0x0,       64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q256",        0xef4019, 0x0,       64 * 1024,   512, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q512",        0xef4020, 0x0,       64 * 1024,  1024, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q80BW",       0xef5014, 0x0,       64 * 1024,    16, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q16DW",       0xef6015, 0x0,       64 * 1024,    32, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q32DW",       0xef6016, 0x0,       64 * 1024,    64, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q64DW",       0xef6017, 0x0,       64 * 1024,   128, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q128FW",      0xef6018, 0x0,       64 * 1024,   256, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"W25Q256FW",      0xef7019, 0x0,       64 * 1024,   512, RD_FULL, SECT_4K | SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
	{"XT25F128A",      0x207018, 0x0,       64 * 1024,    32, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE5},
	/*
	 * Note:
	 * Below paired flash devices has similar spi_flash params.
	 * (S25FL129P_64K, S25FL128S_64K)
	 * (W25Q80BL, W25Q80BV)
	 * (W25Q16CL, W25Q16DV)
	 * (W25Q32BV, W25Q32FV_SPI)
	 * (W25Q64CV, W25Q64FV_SPI)
	 * (W25Q128BV, W25Q128FV_SPI)
	 * (W25Q32DW, W25Q32FV_QPI)
	 * (W25Q64DW, W25Q64FV_QPI)
	 * (W25Q128FW, W25Q128FV_QPI)
	 */
};

const struct spi_flash_params spi_flash_std_params_table[] = {
	{"STDR04FW",       0xffff13, 0x0,       64 * 1024,     8, RD_EXTN, 0},
	{"STDR08FW",       0xffff14, 0x0,       64 * 1024,    16, RD_EXTN, 0},
	{"STDR16FW",       0xffff15, 0x0,       64 * 1024,    32, RD_EXTN, 0},
	{"STDR32FW",       0xffff16, 0x0,       64 * 1024,    64, RD_EXTN, 0},
	{"STDR64FW",       0xffff17, 0x0,       64 * 1024,   128, RD_EXTN, 0},
	{"STDR128FW",      0xffff18, 0x0,       64 * 1024,   256, RD_EXTN, 0},
	{"STDR256FW",      0xffff19, 0x0,       64 * 1024,   512, RD_EXTN, 0},
	{"STDR512FW",      0xffff1A, 0x0,       64 * 1024,  1024, RD_EXTN, 0},
};


/* Read commands array */
static u8 spi_read_cmds_array[] = {
	CMD_READ_ARRAY_SLOW,
	CMD_READ_ARRAY_FAST,
	CMD_READ_DUAL_OUTPUT_FAST,
	CMD_READ_QUAD_OUTPUT_FAST,
	CMD_READ_DUAL_IO_FAST,
	CMD_READ_QUAD_IO_FAST
};

u32 nvt_get_flash_erasesize(void)
{
	if (info->flash_info->block_size)
		return info->flash_info->block_size;
	else
		return 0;
}

static void nvt_parse_frequency(u32 *freq)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};

	sprintf(path,"/nor@%x",IOADDR_NAND_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "clock-frequency", NULL);

	if (cell > 0)
		*freq = __be32_to_cpu(cell[0]);
	else
		*freq = 12000000;
}

static void nvt_parse_parameter(u32 *trace_stdtable)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};

	sprintf(path,"/nor@%x",IOADDR_NAND_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "trace-stdtable", NULL);

	if (cell > 0)
		*trace_stdtable = __be32_to_cpu(cell[0]);
	else
		*trace_stdtable = 1;
}

#ifdef CONFIG_SPI_FLASH_MTD

static void spi_flash_mtd_sync(struct mtd_info *mtd)
{
}

static int spi_flash_mtd_number(void)
{
#ifdef CONFIG_SYS_MAX_FLASH_BANKS
	return CONFIG_SYS_MAX_FLASH_BANKS;
#else
	return 0;
#endif
}


static int spi_flash_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct spi_flash *flash = mtd->priv;
	int err;

	instr->state = MTD_ERASING;

	err = spi_flash_erase(flash, instr->addr, instr->len);

	if (err) {
		instr->state = MTD_ERASE_FAILED;
		instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
		return -EIO;
	}

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int spi_flash_mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct spi_flash *flash = mtd->priv;
	int err;

	err = spi_flash_read(flash, from, len, buf);

	if (!err)
		*retlen = len;

	return err;
}

static int spi_flash_mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct spi_flash *flash = mtd->priv;
	int err;

	err = spi_flash_write(flash, to, len, buf);

	if (!err)
		*retlen = len;

	return err;
}
#endif

#ifdef CONFIG_OF_CONTROL
int spi_flash_decode_fdt(const void *blob, struct spi_flash *flash)
{
	fdt_addr_t addr;
	fdt_size_t size;
	int node;

	/* If there is no node, do nothing */
	node = fdtdec_next_compatible(blob, 0, COMPAT_GENERIC_SPI_FLASH);
	if (node < 0)
		return 0;

	addr = fdtdec_get_addr_size(blob, node, "memory-map", &size);
	if (addr == FDT_ADDR_T_NONE) {
		debug("%s: Cannot decode address\n", __func__);
		return 0;
	}

	if (flash->size != size) {
		debug("%s: Memory map must cover entire device\n", __func__);
		return -1;
	}

	return 0;
}
#endif /* CONFIG_OF_CONTROL */

static int spi_flash_set_qeb_mxic(void)
{
	u8 qeb_status;
	u32 data;
	int ret;

	ret =  spinor_read_status(info, NAND_SPI_NOR_STS_RDSR_1, &qeb_status);
	if (ret < 0) {
		return ret;
	}

	if (qeb_status & STATUS_QEB_MXIC) {
		debug("SF: mxic: QEB is already set\n");
	} else {
		data = qeb_status | STATUS_QEB_MXIC;

		ret = spinor_write_status(info, NAND_SPI_NOR_STS_WRSR_1, data);
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

static int spi_flash_set_qeb_winspan(void)
{
	u8 qeb_status;
	u32 data;
	int ret;

	ret = spinor_read_status(info, NAND_SPI_NOR_STS_RDSR_2, &qeb_status);
	if (ret < 0) {
		return ret;
	}

	if (qeb_status & STATUS_QEB_WINSPAN) {
		debug("SF: winspan: QEB is already set\n");
	} else {
		data = qeb_status | STATUS_QEB_WINSPAN;

		ret = spinor_write_status(info, NAND_SPI_NOR_STS_WRSR_2, data);
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

static int spi_flash_set_qeb_gdxtx(void)
{
	u8 qeb_status;
	u32 data;
	int ret;

	ret =  spinor_read_status(info, NAND_SPI_NOR_STS_RDSR_1, &qeb_status);
	if (ret < 0) {
		return ret;
	}

	if (qeb_status & STATUS_QEB_WINSPAN) {
		debug("SF: gdxtx: QEB is already set\n");
	} else {
		data = qeb_status | STATUS_QEB_WINSPAN;

		ret = spinor_write_status(info, NAND_SPI_NOR_STS_WRSR_1, data);
		if (ret < 0) {
			return ret;
		}
	}

	return ret;
}

static int spi_flash_set_qeb(void)
{
	u32 flags = info->flash_info->qe_flags;

	if (flags & SPI_FLASH_BUSWIDTH_QUAD_TYPE1)
		return spi_flash_set_qeb_mxic();
	else if (flags & SPI_FLASH_BUSWIDTH_QUAD_TYPE2) {
		if (flags & SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31)
			return spi_flash_set_qeb_winspan();
		else {
			printf("not support this type 0x%x\n", flags);
			return -1;
		}
	} else if (flags & SPI_FLASH_BUSWIDTH_QUAD_TYPE4)
		return spi_flash_set_qeb_gdxtx();

	printk("not support any QE configuration\n");
	return 0;
}

int nvt_spinor_write_status(struct spi_flash *flash, u8 ws)
{
	int ret = -1;

	ret = spinor_write_status(info, NAND_SPI_NOR_STS_WRSR_1, ws);

	return ret;
}

static int nvt_spinor_status_ops(struct spi_flash *flash, u8 *status)
{
	int ret = -1;

	ret = spinor_read_status(info, NAND_SPI_NOR_STS_RDSR_1, status);

	return ret;
}


static ssize_t nvt_spinor_read_ops(struct spi_nor *flash, loff_t offset,
		size_t len, u_char *data)
{
	size_t align_size = roundup(len, ARCH_DMA_MINALIGN);

	if ((uintptr_t)data & (CONFIG_SYS_CACHELINE_SIZE-1))
		debug("SF: Read buffer is not cacheline alignment\n");

	flush_dcache_range((unsigned long)data, (unsigned long)((unsigned long)data + align_size));

	spinor_read_operation(info, offset, len, (u8 *)data);

	return len;
}

static int nvt_spinor_erase_ops(struct spi_nor *flash, loff_t offset)
{
	return spinor_erase_sector(info, flash, offset);
}


static ssize_t nvt_spinor_write_ops(struct spi_nor *flash, loff_t offset,
		size_t len, const u_char *buf)
{
	size_t align_size = roundup(len, ARCH_DMA_MINALIGN);

	if ((uintptr_t)buf & (CONFIG_SYS_CACHELINE_SIZE-1))
		debug("SF: write buffer is not cacheline alignment\n");

	flush_dcache_range((unsigned long)buf, (unsigned long)((unsigned long)buf + align_size));

	spinor_program_operation(info, offset, len, (u8 *)buf);

	return len;
}

int nvt_spinor_erase_range(struct spi_flash *flash, u32 offset, size_t len)
{
	u32 erase_size, erase_addr;
	int ret = -1;

	if (len == flash->size) {
		return spinor_erase_chip(info);
	} else if (((offset % SIZE_64K) == 0) && ((len % SIZE_64K) == 0)) {
		erase_size = SIZE_64K;
		if (flash->size > SPI_FLASH_16MB_BOUN) {
			if (info->enter_4byte_addr_mode == 1) {
				flash->erase_opcode = CMD_ERASE_64K;
			} else {
				flash->erase_opcode = CMD_ERASE_64K_4BYTE;
			}
		} else {
			flash->erase_opcode = CMD_ERASE_64K;
		}
	} else {
		erase_size = flash->erase_size;
		if (flash->size > SPI_FLASH_16MB_BOUN) {
			if (info->enter_4byte_addr_mode == 1) {
				flash->erase_opcode = CMD_ERASE_4K;
			} else {
				flash->erase_opcode = CMD_ERASE_4K_4BYTE;
			}
		} else {
			flash->erase_opcode = CMD_ERASE_4K;
		}
	}

	if (offset % erase_size || len % erase_size) {
		debug("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	while (len) {
		erase_addr = offset;

		ret = spinor_erase_sector(info, flash, erase_addr);

		if (ret)
			break;

		offset += erase_size;
		len -= erase_size;
	}

	return ret;
}

static int spi_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct spi_flash *flash = mtd->priv;
	int err;

	instr->state = MTD_ERASING;

	err = nvt_spinor_erase_range(flash, instr->addr, instr->len);

	if (err) {
		instr->state = MTD_ERASE_FAILED;
		instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
		return -EIO;
	}

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int spi_mtd_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	int err;
	u32 p_flush_buf = (uintptr_t)buf;
	size_t align_size = roundup(len, ARCH_DMA_MINALIGN);
	u32 cacheline_mask = CONFIG_SYS_CACHELINE_SIZE-1;

	if (p_flush_buf & (CONFIG_SYS_CACHELINE_SIZE-1)) {
		p_flush_buf &= ~cacheline_mask;
		flush_dcache_range(p_flush_buf, p_flush_buf + align_size + CONFIG_SYS_CACHELINE_SIZE);
	} else
		flush_dcache_range(p_flush_buf, p_flush_buf + align_size);

	err = spinor_read_operation(info, from, len, (u8 *)buf);

	if (!err)
		*retlen = len;

	return err;
}

static int spi_mtd_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	int err;

	err = spinor_program_operation(info, to, len, (u8 *)buf);

	if (!err)
		*retlen = len;

	return err;
}

static int nvt_spinor_read_id(struct drv_nand_dev_info *info, uint32_t *id)
{
	uint8_t  card_id[8] = {0};

	if (nand_cmd_read_id(card_id, info) != 0) {
		printf("NOR cmd timeout\r\n");
		return -1;
	} else {
		printf("id =  0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			card_id[0], card_id[1], card_id[2], card_id[3], card_id[4]);

		*id = card_id[0] | (card_id[1] << 8) | (card_id[2] << 16) | \
			(card_id[3] << 24);

		*(id+1) = card_id[4];

		return 0;
	}
}

static int nvt_spinor_validate_params(struct spi_flash *flash, u8 *idcode)
{
	const struct spi_flash_params *params;
	struct mtd_info *mtd = &flash->mtd;
	u8 cmd;
	u16 jedec = idcode[1] << 8 | idcode[2];
	u16 ext_jedec = idcode[3] << 8 | idcode[4];
	u8 std_path = 0;

	if (info->trace_stdtable) {
		params = spi_flash_std_params_table;
		for (; params->name != NULL; params++) {
			if ((params->jedec & 0xFF) == idcode[2]) {
				std_path = 1;
				break;
			}
		}
	}

	if (std_path == 0) {
		params = ext_spi_flash_params_table;
		for (; params->name != NULL; params++) {
			if ((params->jedec >> 16) == idcode[0]) {
				if ((params->jedec & 0xFFFF) == jedec) {
					if (params->ext_jedec == 0)
						break;
					else if (params->ext_jedec == ext_jedec)
						break;
				}
			}
		}

		if (!params->name) {
			printf("Use default table\n");
			params = spi_flash_params_table;
			for (; params->name != NULL; params++) {
				if ((params->jedec >> 16) == idcode[0]) {
					if ((params->jedec & 0xFFFF) == jedec) {
						if (params->ext_jedec == 0)
							break;
						else if (params->ext_jedec == ext_jedec)
							break;
					}
				}
			}

			if (!params->name) {
				printf("SF: Unsupported flash IDs: ");
				printf("manuf %02x, jedec %04x, ext_jedec %04x\n",
					idcode[0], jedec, ext_jedec);
					return -1;
			}
		} else
			printf("Use external table\n");
	}

	/* Assign flash data */
	flash->name = params->name;

	/* Assign spi_flash ops */
	flash->write = nvt_spinor_write_ops;
	flash->erase = nvt_spinor_erase_ops;
	flash->read = nvt_spinor_read_ops;
	flash->status = nvt_spinor_status_ops;
	flash->wstatus = nvt_spinor_write_status;

	/*
	 * The Spansion S25FL032P and S25FL064P have 256b pages, yet use the
	 * 0x4d00 Extended JEDEC code. The rest of the Spansion flashes with
	 * the 0x4d00 Extended JEDEC code have 512b pages. All of the others
	 * have 256b pages.
	 */
	if (ext_jedec == 0x4d00) {
		if ((jedec == 0x0215) || (jedec == 0x216))
			flash->page_size = 256;
		else
			flash->page_size = 512;
	} else {
		flash->page_size = 256;
	}
	flash->sector_size = params->sector_size;
	flash->size = flash->sector_size * params->nr_sectors;
	flash->erase_size = SIZE_4K;

	/* Look for the fastest read cmd */
	cmd = fls(params->e_rd_cmd);
	if (cmd) {
		cmd = spi_read_cmds_array[cmd - 1];
		flash->read_opcode = cmd;
		if ((cmd == CMD_READ_QUAD_OUTPUT_FAST) || \
				(cmd == CMD_READ_QUAD_IO_FAST))
			info->flash_info->nor_read_mode = SPI_NOR_QUAD_READ;
		else
			info->flash_info->nor_read_mode = SPI_NOR_DUAL_READ;
	} else {
		/* Go for default supported read cmd */
		flash->read_opcode = CMD_READ_ARRAY_FAST;
		info->flash_info->nor_read_mode = SPI_NOR_NORMAL_READ;
	}

#ifndef CONFIG_FLASH_ONLY_DUAL
	/* Go for default supported write cmd */
	if ((params->flags & QUAD_PROGRAM) && !(params->flags & SPI_FLASH_NORMAL_WRITE))
		flash->program_opcode = CMD_QUAD_PAGE_PROGRAM;
	else
		flash->program_opcode = CMD_PAGE_PROGRAM;
#else
	if (info->flash_info->nor_read_mode == SPI_NOR_QUAD_READ)
		info->flash_info->nor_read_mode = SPI_NOR_DUAL_READ;

	if ((cmd == CMD_READ_QUAD_OUTPUT_FAST) || \
		(cmd == CMD_READ_QUAD_IO_FAST))
		flash->read_opcode = CMD_READ_DUAL_IO_FAST;

	flash->program_opcode = CMD_PAGE_PROGRAM;

	info->flash_info->nor_quad_support = DISABLE;
#endif

	/* Read read_dummy: dummy byte is determined based on the
	 * dummy cycles of a particular command.
	 * Fast commands - read_dummy = dummy_cycles/8
	 * I/O commands- read_dummy = (dummy_cycles * no.of lines)/8
	 * For I/O commands except cmd[0] everything goes on no.of lines
	 * based on particular command but incase of fast commands except
	 * data all go on single line irrespective of command.
	 */
	switch (flash->read_opcode) {
	case CMD_READ_QUAD_IO_FAST:
		flash->read_dummy = 2;
		break;
	case CMD_READ_ARRAY_SLOW:
		flash->read_dummy = 0;
		break;
	default:
		flash->read_dummy = 1;
	}

	/* Flash powers up read-only, so clear BP# bits  ATMEL/MICRONIX/SST*/
	if ((idcode[0] == 0x20) || (idcode[0] == 0xBF) || (idcode[0] == 0x1F))
		nvt_spinor_write_status(flash, 0);


	/*Copy flash info*/
	info->flash_info->page_size = flash->page_size;
	info->flash_info->device_size = flash->size;
	info->flash_info->block_size = flash->sector_size;
	info->flash_info->chip_sel = 0;
	info->flash_info->phy_page_ratio = 0;
	info->flash_info->qe_flags = params->flags;


	if (!mtd->name) {
		sprintf(sf_mtd_name, "nor%d", spi_flash_mtd_number());
		mtd->name = sf_mtd_name;
	}
	mtd->priv = flash;
	mtd->type = MTD_NORFLASH;
	mtd->writesize = 1;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->size = flash->size;
	mtd->_erase = spi_mtd_erase;
	mtd->_read = spi_mtd_read;
	mtd->_write = spi_mtd_write;
	mtd->writebufsize = flash->page_size;
	mtd->erasesize = SIZE_4K;

	return 0;
}

#ifdef CONFIG_SPI_FLASH_MTD
int spi_flash_mtd_register(struct spi_flash *flash)
{
	memset(&sf_mtd_info, 0, sizeof(sf_mtd_info));
	sprintf(sf_mtd_name, "nor%d", spi_flash_mtd_number());

	sf_mtd_info.name = sf_mtd_name;
	sf_mtd_info.type = MTD_NORFLASH;
	sf_mtd_info.flags = MTD_CAP_NORFLASH;
	sf_mtd_info.writesize = 1;
	sf_mtd_info.writebufsize = flash->page_size;

	sf_mtd_info._erase = spi_flash_mtd_erase;
	sf_mtd_info._read = spi_flash_mtd_read;
	sf_mtd_info._write = spi_flash_mtd_write;
	sf_mtd_info._sync = spi_flash_mtd_sync;

	sf_mtd_info.size = flash->size;
	sf_mtd_info.priv = flash;

	/* Only uniform flash devices for now */
	sf_mtd_info.numeraseregions = 0;
	sf_mtd_info.erasesize = SIZE_4K;

	return add_mtd_device(&sf_mtd_info);
}
#endif

static int nvt_spinor_reset(struct drv_nand_dev_info *info)
{
	uint32_t reg, clk_div, freq = 0x0;

	/* Config NAND module clock */
	reg = INW(IOADDR_CG_REG_BASE + 0x74);
	if (nvt_get_chip_id() == CHIP_NA51090) {
		reg |= (0x400);
	} else {	// 52x, 560
		reg |= (0x1);
	}
	OUTW(IOADDR_CG_REG_BASE + 0x74, reg);

	if (nvt_get_chip_id() == CHIP_NA51090) {
		reg = INW(IOADDR_CG_REG_BASE + 0x98);
		reg |= (0x2);
		OUTW(IOADDR_CG_REG_BASE + 0x98, reg);
	} else {
		reg = INW(IOADDR_CG_REG_BASE + 0x84);
		reg |= (0x1);
		OUTW(IOADDR_CG_REG_BASE + 0x84, reg);
	}

	NAND_SETREG(info, NAND_TIME0_REG_OFS, 0x06002222);
	NAND_SETREG(info, NAND_TIME1_REG_OFS, 0x7f0f);

	/* Need use PinCtrl framework */
	reg = INW(IOADDR_TOP_REG_BASE + 0x4);
	if (nvt_get_chip_id() == CHIP_NA51090) {
		reg |= 0x3;
	} else {
		reg |= 0x00002000;
	}
	OUTW(IOADDR_TOP_REG_BASE + 0x4, reg);

	reg = INW(IOADDR_TOP_REG_BASE + 0xA0);
	if (nvt_get_chip_id() == CHIP_NA51090) {
		reg &= ~(0x3F);
	} else {
		reg &= ~(0x0000050F);
	}
	OUTW(IOADDR_TOP_REG_BASE + 0xA0, reg);
	/* reset NAND Config NAND module configuration */

	/* Config clock div */
	nvt_parse_frequency(&freq);
	clk_div = NVT_FLASH_SOURCE_CLK/freq - 1;
	if (nvt_get_chip_id() == CHIP_NA51090) {
		reg = INW(IOADDR_CG_REG_BASE + 0x48);
		reg &= ~(0x3F0000);
		reg |= (clk_div << 16);
		OUTW(IOADDR_CG_REG_BASE + 0x48, reg);
	} else {
		reg = INW(IOADDR_CG_REG_BASE + 0x40);
		reg &= ~(0x3F000);
		reg |= (clk_div << 12);
		OUTW(IOADDR_CG_REG_BASE + 0x40, reg);
	}

	if (nvt_get_chip_id() == CHIP_NA51090) {
		/* Config driving */
		reg = INW(IOADDR_PAD_REG_BASE + 0x50);
		reg &= ~(0xFFFFFFF);
		reg |= 0x2222222;
		OUTW(IOADDR_PAD_REG_BASE + 0x40, reg);
	} else {
		/* Release SRAM */
		reg = INW(IOADDR_TOP_REG_BASE + 0x1000);
		reg &= ~(0x20000);
		OUTW(IOADDR_TOP_REG_BASE + 0x1000, reg);

		/* Config driving */
		reg = INW(IOADDR_PAD_REG_BASE + 0x40);
		reg &= ~(0x3F00FF);
		reg |= 0x150055;
		OUTW(IOADDR_PAD_REG_BASE + 0x40, reg);
	}

	nand_hostSetNandType(info, NANDCTRL_SPI_NOR_TYPE);

	nand_host_settiming2(info, 0x5F51);

	info->ops_freq = freq;

	return 0;

}

static int nvt_read_sfdp(struct drv_nand_dev_info *info)
{
	int ret;
	u8 sfdp_data;
	u32 address = 0x32;

	info->hspeed_dummy_cycle = 8;

	if (info->ops_freq > 96000000) {
		ret = spinor_read_sfdp(info, address, 1, &sfdp_data);
		if (ret)
			return ret;

		if (sfdp_data == MXIC25L25635F_SFDP)
			info->hspeed_dummy_cycle = 10;
	}

	return E_OK;
}


static int nvt_check_rdcr(struct drv_nand_dev_info *info)
{
	int ret;

	ret = spinor_read_status(info, NAND_SPI_NOR_STS_RDCR, &info->rdcr_status);
	if(ret != E_OK)
		return ret;

	/*Set correct dummy cycle configuration*/
	if (((info->rdcr_status & 0xC0) == 0xC0) && (info->ops_freq < 120000000)) {
		u8 rdsr_status;
		ret =  spinor_read_status(info, NAND_SPI_NOR_STS_RDSR_1, &rdsr_status);
		if (ret < 0) {
			return ret;
		}

		info->rdcr_status &= ~0xC0;

		ret = spi_nor_set_status_half_word(info, (rdsr_status<<8) | info->rdcr_status);
		if (ret < 0) {
			return ret;
		}
	}

	ret = spinor_read_status(info, NAND_SPI_NOR_STS_RDCR, &info->rdcr_status);

	return E_OK;
}

static int nvt_spinor_probe_slave(struct spi_flash *flash)
{
	u8 idcode[5] = {0};
	int ret;

	nvt_spinor_reset(info);
	nand_phy_config(info);
	nand_dll_reset();

	ret = nvt_check_rdcr(info);
	if (ret) {
		printf("SF: Failed to get status-3\n");
		goto err_read_id;
	}

	ret = nvt_read_sfdp(info);
	if (ret) {
		printf("SF: Failed to read sfdp\n");
	}

	nvt_parse_parameter(&info->trace_stdtable);

	/* Read the ID codes */
	ret = nvt_spinor_read_id(info, (uint32_t *)idcode);
	if (ret) {
		printf("SF: Failed to get idcodes\n");
		goto err_read_id;
	}

#ifdef DEBUG
	printf("SF: Got idcodes\n");
	print_buffer(0, idcode, 1, sizeof(idcode), 0);
#endif

	if ((idcode[0] | idcode[1]<<8 | idcode[2]<<16) == WINBOND_W25Q256FV) {
		info->enter_4byte_addr_mode = 1;
	} else {
		info->enter_4byte_addr_mode = 0;
	}

	/* Validate params from spi_flash_params table */
	if (nvt_spinor_validate_params(flash, idcode))
		goto err_read_id;

	if ( 0xc84019 == ((idcode[0] << 16) | (idcode[1] << 8) | idcode[2]))
		idcode[0] = 0xC2;

	/* Set the quad enable bit - only for quad commands */
	if ((flash->read_opcode == CMD_READ_QUAD_OUTPUT_FAST) ||
		(flash->read_opcode == CMD_READ_QUAD_IO_FAST) ||
		(flash->program_opcode == CMD_QUAD_PAGE_PROGRAM)) {
		if (spi_flash_set_qeb()) {
			debug("SF: Fail to set QEB for %02x\n", idcode[0]);
			return -1;
		} else {
			if (flash->program_opcode == CMD_QUAD_PAGE_PROGRAM)
				info->flash_info->nor_quad_support = ENABLE;
			else
				info->flash_info->nor_quad_support = DISABLE;
		}
	}

#ifdef CONFIG_OF_CONTROL
	if (spi_flash_decode_fdt(gd->fdt_blob, flash)) {
		debug("SF: FDT decode error\n");
		goto err_read_id;
	}
#endif
#ifndef CONFIG_SPL_BUILD
	printf("%s with page size ", flash->name);
	print_size(flash->page_size, ", erase size ");
	print_size(flash->erase_size, ", total ");
	print_size(flash->size, "");
	puts("\n");
#endif
	return 0;

err_read_id:
	return -1;
}

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_nor *flash;

	info = calloc(1, sizeof(struct drv_nand_dev_info));
	if (!info) {
		printf("alloc drv_nand_dev_info failed!\n");
		return NULL;
	}

	info->flash_info = calloc(1, sizeof(struct nvt_nand_flash));
	if (!info) {
		printf("alloc nvt_nand_flash failed!\n");
		return NULL;
	}

	info->mmio_base = (void *)(IOADDR_NAND_REG_BASE);

	/* Allocate space if needed (not used by sf-uclass */
	flash = calloc(1, sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate spi_flash\n");
		return NULL;
	}

	if (nvt_spinor_probe_slave(flash)) {
		free(flash);
		return NULL;
	}

	info->data_buff = malloc(info->flash_info->page_size + CACHE_LINE_SIZE);

#ifdef CONFIG_SPI_FLASH_MTD
	spi_flash_mtd_register(flash);
#endif

	printf("nvt spinor %d-bit mode @ %d Hz\n", \
		info->flash_info->nor_quad_support ? 4 : 1, \
		info->ops_freq);

	return flash;
}

void spi_flash_free(struct spi_flash *flash)
{
	free(flash);
}
