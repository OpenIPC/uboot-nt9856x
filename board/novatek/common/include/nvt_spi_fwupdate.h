/*
 *  board/nvt/evb670btk/nvt_spi_fwupdate.h
 *
 *  Author:	Alvin lin
 *  Created:	Dec 16, 2011
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_SPI_FIRMWARE_UPDATE_H__
#define __NVT_SPI_FIRMWARE_UPDATE_H__

#include "nvt_spi_partition.h"

int nvt_spi_burn_bin(SPI_PARTITION partition,u8* buf,u32 len);

#endif
