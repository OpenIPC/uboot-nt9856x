/*
 * External Flash table
 *
 * Copyright (C) Novatek Inc. 2017
 *
 */
#include <linux/mtd/mtd.h>
/*
 * {"Flash_name", jedec_id, ext_jedec, sector_size, nr_sectors, rd_cmd, flags},
 *
 * rd_cmd:
 *       1 bit : 0
 *       2 bits: RD_EXTN
 *       4 bits: RD_FULL
 *
 * flags:
 *       SECT_4K
 *       SPI_FLASH_BUSWIDTH_QUAD_TYPE1
 *       SPI_FLASH_BUSWIDTH_QUAD_TYPE2
 *       SPI_FLASH_BUSWIDTH_QUAD_TYPE3
 *       SPI_FLASH_BUSWIDTH_QUAD_TYPE4
 *       SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31
 *       SPI_FLASH_BUSWIDTH_QUAD_TYPE5
 *       SPI_FLASH_NORMAL_WRITE
 *
 *  Read: 2 bits, Write: 1 bit
 * {"MX25L6405D", 0xc22017, 0x0, 64 * 1024,   128, RD_EXTN, 0},
 *
 *  Read: 4 bits, Write: 1 bit
 * {"MX25L12835F",0xc22018, 0x0, 64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE1 | SPI_FLASH_NORMAL_WRITE},
 *
 *  Read: 4 bits, Write: 4 bit
 * {"GD25Q127C",  0xc84018, 0x0, 64 * 1024,   256, RD_FULL, SPI_FLASH_BUSWIDTH_QUAD_TYPE2 | SPI_FLASH_BUSWIDTH_QUAD_TYPE1_CMD31},
*/

const struct spi_flash_params ext_spi_flash_params_table[] = {
};

