/*
 *  spinand driver
 *
 *  Author:	Howard Chang
 *  Created:	Aug 2, 2018
 *  Copyright:	Novatek Inc.
 *
 */
#include "config.h"
#include <common.h>
#include <malloc.h>
#include <nand.h>
#include "../nvt_flash_spi/nvt_flash_spi_int.h"

#define NAND_VERSION "1.0.15"

#ifdef CONFIG_CMD_NAND
typedef struct nand_2plane_id {
	char    *name;
	int     mfr_id;
	int     dev_id;
} NAND_2PLANE_ID;

static NAND_2PLANE_ID nand_2plane_list[] = {
	// flash name / Manufacturer ID / Device ID
	{ "MXIC_MX35LF2G14AC",		0xC2,	0x20	},
	{ "MICRON_MT29F2G01AB",		0x2C,	0x24	}
};
#define NUM_OF_2PLANE_ID (sizeof(nand_2plane_list) / sizeof(NAND_2PLANE_ID))

/* error code and state */
enum {
	ERR_NONE	= 0,
	ERR_DMABUSERR	= -1,
	ERR_SENDCMD	= -2,
	ERR_DBERR	= -3,
	ERR_BBERR	= -4,
	ERR_ECC_FAIL	= -5,
	ERR_ECC_UNCLEAN = -6,
};

enum {
	STATE_READY = 0,
	STATE_CMD_HANDLE,
	STATE_DMA_READING,
	STATE_DMA_WRITING,
	STATE_DMA_DONE,
	STATE_PIO_READING,
	STATE_PIO_WRITING,
};

static struct nand_ecclayout hw_smallpage_ecclayout = {
	.eccbytes = 12,
	.eccpos = {0, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	.oobfree = { {1, 3}, {5, 1} }
};

static struct nand_ecclayout spinand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {
		8, 9, 10, 11, 12, 13, 14, 15,
		24, 25, 26, 27, 28, 29, 30, 31,
		40, 41, 42, 43, 44, 45, 46, 47,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobavail = 12,
	.oobfree = {
		{.offset = 16,
			.length = 4},
		{.offset = 32,
			.length = 4},
		{.offset = 48,
			.length = 4},
	}
};

static void drv_nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{

}

static void nvt_parse_frequency(u32 *freq)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};

	sprintf(path,"/nand@%lx",(unsigned long)IOADDR_NAND_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "clock-frequency", NULL);

	if (cell > 0)
		*freq = __be32_to_cpu(cell[0]);
	else
		*freq = 12000000;
}



static int nvt_scan_nand_type(struct drv_nand_dev_info *info, int32_t* id)
{
	int maf_id, dev_id;
	uint32_t reg;
	struct nand_flash_dev *type = nvt_nand_ids;

	maf_id = *id & 0xFF;
	dev_id = (*id >> 8) & 0xFF;

	debug("the maf_id is 0x%x, the dev_id is 0x%x\n", maf_id, dev_id);

	for (; type->name != NULL; type++) {
		if (type->dev_id == dev_id)
			break;
	}

	if (!type->name) {
		return -ENODEV;
	}

	info->flash_info->chip_id = *id;
	info->flash_info->chip_sel = 0;
	info->flash_info->page_size = type->pagesize;
	info->flash_info->block_size = type->erasesize;
	info->flash_info->device_size = type->chipsize;
	info->flash_info->page_per_block = (type->erasesize / type->pagesize);

	debug("page_size %d, block_size %d, device_size %d page_per_block %d\n", \
		info->flash_info->page_size, \
		info->flash_info->block_size, \
		info->flash_info->device_size, \
		info->flash_info->page_per_block);

	if (type->pagesize == 4096) {
		info->flash_info->phy_page_ratio = 8;
		info->flash_info->oob_size = 128;
		info->flash_info->device_size = 512;
		info->flash_info->module_config = NAND_PAGE4K | NAND_2COL_ADDR;
		nand_hostSetupPageSize(0, NAND_PAGE_SIZE_4096);
	} else {
		info->flash_info->phy_page_ratio = 4;
		info->flash_info->oob_size = 64;
		info->flash_info->module_config = NAND_PAGE2K | NAND_2COL_ADDR;
		nand_hostSetupPageSize(0, NAND_PAGE_SIZE_2048);
	}

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
		info->flash_info->module_config |= NAND_PRI_ECC_RS;
	} else {
		info->flash_info->module_config |= NAND_PRI_ECC_SPI_ON_DIE;
		info->flash_info->ecc_strength = type->ecc.strength_ds;
	}

	reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
	reg &= ~(0x7FFFF);

	NAND_SETREG(info, NAND_MODULE0_REG_OFS, reg | info->flash_info->module_config);

	return E_OK;
}

static int drv_nand_readpage(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	return nand_cmd_read_operation(info, (int8_t *)info->data_buff,
			page_addr * info->flash_info->page_size, 1);
}

static int drv_nand_write_page(struct drv_nand_dev_info *info,
				int column, int page_addr)
{
	if (column != info->flash_info->page_size)
		nand_cmd_write_spare_sram(info);

	return nand_cmd_write_operation_single(info, (int8_t *)info->data_buff,
				page_addr * info->flash_info->page_size, column);
}

static int drv_nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;
	struct drv_nand_dev_info *info = this->priv;
	return nand_host_check_ready(info);
}


static int drv_nand_read_page_ecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)

{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;
	u8 status = 0, chip_id = info->flash_info->chip_id & 0xFF;
	u32 full_id = info->flash_info->chip_id;
	int ret = 0, count = 0, i = 0;

	chip->read_buf(mtd, buf, eccsize * eccsteps);

	if (info->nand_chip.oob_poi)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	if (info->retcode == ERR_ECC_UNCLEAN) {
		mtd->ecc_stats.failed++;
	} else if (info->retcode == ECC_CORRECTED) {
		if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
				mtd->ecc_stats.corrected += \
					nand_cmd_read_ecc_corrected(info);

				ret = status;
		} else {
			if (chip_id == NAND_MXIC) {
				ret = nand_cmd_read_flash_ecc_corrected(info);
				mtd->ecc_stats.corrected += ret;
			} else if ((chip_id == NAND_TOSHIBA) || \
					(full_id== WINBOND_W25N02KV)) {
				status = nand_cmd_read_status(info, \
					NAND_SPI_STS_FEATURE_4);

				for (i = 0; i < 4; i++)
					count += ((status >> i) & 0x1) ? 1 : 0;

				mtd->ecc_stats.corrected += count;
				ret = count;
			} else if (chip_id == SPI_NAND_XTX) {
				status = nand_cmd_read_status(info, \
					NAND_SPI_STS_FEATURE_3);

				count = (status >> 2) & 0xF;
				mtd->ecc_stats.corrected += count;
				ret = count;
			} else {
				mtd->ecc_stats.corrected++;
				ret = 1;
			}
		}
	}

	return ret;

}

static int drv_nand_write_page_ecc(struct mtd_info *mtd, struct nand_chip *chip,
				  const uint8_t *buf, int oob_required, int page)
{
	/* this function will write page data and oob into nand */
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;

	chip->write_buf(mtd, buf, eccsize * eccsteps);

	return 0;
}

static int drv_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = nand_chip->priv;

	int ret = info->retcode;

	info->retcode = ERR_NONE;

	if (ret < 0)
		return NAND_STATUS_FAIL;
	else
		return E_OK;
}

static uint8_t drv_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	char retval = 0xFF;

	if (info->buf_start < info->buf_count)
		retval = info->data_buff[info->buf_start++];

	return retval;
}

static u16 drv_nand_read_word(struct mtd_info *mtd)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	u16 retval = 0xFFFF;

	if (!(info->buf_start & 0x01) && info->buf_start < info->buf_count) {
		retval = *((u16 *)(info->data_buff+info->buf_start));
		info->buf_start += 2;
	}
	return retval;
}

static void drv_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(buf, info->data_buff + info->buf_start, real_len);

	info->buf_start += real_len;
}

static void drv_nand_write_buf(struct mtd_info *mtd,
	const uint8_t *buf, int len)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(info->data_buff + info->buf_start, buf, real_len);

	info->buf_start += real_len;
}

static void drv_nand_cmdfunc(struct mtd_info *mtd, unsigned command,
		int column, int page_addr)
{
	struct nand_chip *this = mtd_to_nand(mtd);
	struct drv_nand_dev_info *info = this->priv;

	int ret;
	uint8_t *ptr;

	info->data_size = 0;
	info->state = STATE_READY;
	info->retcode = ERR_NONE;

	switch (command) {
	case NAND_CMD_READOOB:
		info->buf_count = mtd->writesize + mtd->oobsize;
		info->buf_start = mtd->writesize + column;

		ptr = (uint8_t *)info->data_buff;
		ptr = info->data_buff + info->buf_start;
		if (info->buf_start != info->flash_info->page_size) {
			dev_err(&info->pdev->dev,
			"info->buf_start = %d, != 0\n", info->buf_start);
		}

		nand_cmd_read_page_spare_data(info, (int8_t *)ptr,
				info->flash_info->page_size * page_addr);
		/* We only are OOB, so if the data has error, does not matter */
	break;

	case NAND_CMD_READ0:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			printf("NAND_CMD_READ0 : is not Cache_Line_Size alignment!\n");

		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;

		ret = drv_nand_readpage(info, column, page_addr);
		if (ret == E_CTX)
			info->retcode = ERR_ECC_UNCLEAN;
		else if (ret < 0)
			info->retcode = ERR_SENDCMD;
		else if (ret == ECC_CORRECTED)
			info->retcode = ECC_CORRECTED;
		else
			info->retcode = ERR_NONE;
	break;

	case NAND_CMD_SEQIN:
		info->buf_start = column;
		info->buf_count = mtd->writesize + mtd->oobsize;
		memset(info->data_buff, 0xff, info->buf_count);

		/* save column/page_addr for next CMD_PAGEPROG */
		info->seqin_column = column;
		info->seqin_page_addr = page_addr;
	break;

	case NAND_CMD_PAGEPROG:
		if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
			printf("not MIPS_Cache_Line_Size alignment!\n");

		ret = drv_nand_write_page(info, info->seqin_column, info->seqin_page_addr);
		if (ret)
			info->retcode = ERR_SENDCMD;
	break;

	case NAND_CMD_ERASE1:
		ret = nand_cmd_erase_block(info, page_addr);
		if (ret)
			info->retcode = ERR_BBERR;
	break;

	case NAND_CMD_ERASE2:
	break;

	case NAND_CMD_READID:
		info->buf_start = 0;
		info->buf_count = 4;
		if(info->flash_info->chip_id)
			memcpy((uint32_t *)info->data_buff, &info->flash_info->chip_id, 4);
		else {
			ret = nvt_nand_read_id(info, (uint32_t *)info->data_buff);
			if (ret)
				info->retcode = ERR_SENDCMD;
		}
	break;
	case NAND_CMD_STATUS:
		info->buf_start = 0;
		info->buf_count = 1;
		nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);
		if (!(info->data_buff[0] & 0x80))
			info->data_buff[0] = 0x80;
	break;

	case NAND_CMD_RESET:
	break;

	default:
		printf("non-supported command.\n");
	break;
	}
}

static int drv_nand_reset(struct drv_nand_dev_info *info)
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

	nand_hostSetNandType(info, NANDCTRL_SPI_NAND_TYPE);

	nand_host_settiming2(info, 0x9F51);

	info->ops_freq = freq;

	return nand_cmd_reset(info);

}


int nvt_nand_read_id(struct drv_nand_dev_info *info, uint32_t *id)
{
	uint8_t  card_id[8];

	if (nand_cmd_read_id(card_id, info) != 0) {
		printf("NAND cmd timeout\r\n");
		return -1;
	} else {
		printf("id =  0x%02x 0x%02x 0x%02x 0x%02x\n",
			card_id[0], card_id[1], card_id[2], card_id[3]);

		*id = card_id[0] | (card_id[1] << 8) | (card_id[2] << 16) | \
			(card_id[3] << 24);
		return 0;
	}

}

int nvt_nand_scan_2plane_id(uint32_t id)
{
	int32_t i, read_id,flash_id;

	read_id = (id & 0x0000FFFF);

	for (i = 0 ; i < NUM_OF_2PLANE_ID; i++) {
		flash_id = (nand_2plane_list[i].mfr_id + (nand_2plane_list[i].dev_id << 8));
		if (read_id == flash_id)
			return 1;
	}
	return 0;
}

int nvt_nand_board_nand_init(struct nand_chip *nand)
{
	int32_t id;
	struct drv_nand_dev_info *info;
	u32 status;

#ifdef CONFIG_NVT_SPI_NONE
	return -ENOMEM;
#endif

	info = malloc(sizeof(struct drv_nand_dev_info));
	if (!info) {
		printf("alloc drv_nand_dev_info failed!\n");
		return -ENOMEM;
	}

	info->flash_info = malloc(sizeof(struct nvt_nand_flash));
	if (!info) {
		printf("alloc nvt_nand_flash failed!\n");
		return -ENOMEM;
	}
	info->flash_info->spi_nand_status.bBlockUnlocked    = FALSE;
	info->flash_info->spi_nand_status.bQuadEnable       = FALSE;
	info->flash_info->spi_nand_status.bQuadProgram      = FALSE;
	info->flash_info->spi_nand_status.uiTimerRecord     = FALSE;

	nand->priv = info;
	info->mmio_base = (void *)(CONFIG_SYS_NAND_BASE);
	info->use_ecc = CONFIG_NVT_NAND_ECC;

	drv_nand_reset(info);
	nand_phy_config(info);
	nand_dll_reset();

	/*Delay 1 ms for spinand characteristic*/
	mdelay(1);

	nvt_nand_read_id(info, (uint32_t *)&id);

#ifndef CONFIG_FLASH_ONLY_DUAL
	if ((id & 0xFFFF) != TOSHIBA_TC58CVG)
		info->flash_info->spi_nand_status.bQuadProgram = TRUE;
#endif

	if (nvt_scan_nand_type(info, &id)) {
		printf("flash not support with id 0x%x\n", id);
		return -ENODEV;
	}

	info->data_buff = malloc(info->flash_info->page_size+info->flash_info->oob_size + CACHE_LINE_SIZE);

	if (((unsigned long)(info->data_buff)) % CACHE_LINE_SIZE)
		info->data_buff = (uint8_t *)((((unsigned long)info->data_buff + CACHE_LINE_SIZE - 1)) & 0xFFFFFFC0);

	if(info->data_buff == NULL) {
		printf("allocate nand buffer fail !\n");
		return 1;
	}

	status = nand_cmd_read_status(info, NAND_SPI_STS_FEATURE_2);

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC)
		status &= ~SPINAND_CONFIG_ECCEN;
	else
		status |= SPINAND_CONFIG_ECCEN;

#ifndef CONFIG_FLASH_ONLY_DUAL
	if (((id & 0xFF) == NAND_MXIC) || ((id & 0xFF) == SPI_NAND_GD) || \
		((id & 0xFF) == SPI_NAND_DOSILICON) || ((id & 0xFF) == SPI_NAND_XTX)) {
		status |= SPINAND_CONFIG_QUADEN;
	}
#endif

	nand_cmd_write_status(info, NAND_SPI_STS_FEATURE_2, status);

	info->use_dma = 1;
	info->data_size = 0;
	info->state = STATE_READY;

	if (info->flash_info->page_size == 512)
		nand->ecc.layout = &hw_smallpage_ecclayout;
	else
		nand->ecc.layout = &spinand_oob_64;
	nand->ecc.size = 0x200;
	nand->ecc.bytes = 0x8;
	nand->ecc.steps = 0x4;
	nand->ecc.total	= nand->ecc.steps * nand->ecc.bytes;
	nand->ecc.strength = 1;

	nand->cmd_ctrl = drv_nand_hwcontrol;
	nand->dev_ready = drv_nand_dev_ready;
	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.read_page = drv_nand_read_page_ecc;
	nand->ecc.write_page = drv_nand_write_page_ecc;
	nand->options = NAND_NO_SUBPAGE_WRITE;
	nand->waitfunc = drv_nand_waitfunc;
	nand->read_byte = drv_nand_read_byte;
	nand->read_word = drv_nand_read_word;
	nand->read_buf = drv_nand_read_buf;
	nand->write_buf = drv_nand_write_buf;
	nand->cmdfunc = drv_nand_cmdfunc;
	nand->chip_delay = 0;

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
		nand->ecc.strength = 4;
	} else {
		nand->ecc.strength = info->flash_info->ecc_strength;
	}

	if (nvt_nand_scan_2plane_id(id) == 1) {
		info->flash_info->plane_flags = 1;
		//printf("2 plane en\n");
	} else {
		info->flash_info->plane_flags = 0;
		//printf("2 plane dis\n");
	}

	printf("nvt spinand %d-bit mode @ %d Hz\n", \
		info->flash_info->spi_nand_status.bQuadProgram ? 4 : 1, \
		info->ops_freq);

	return 0;
}

#endif

