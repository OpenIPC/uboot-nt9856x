/*
 *  nvt_emmc_utils.h
 *
 *  Author:	Alvin lin
 *  Created:	June 21, 2013
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_EMMC_UTILS_H__
#define __NVT_EMMC_UTILS_H__

struct bootloader_message {
    char command[32];
    char status[32];
    char recovery[1024];
};

struct boot_address {
	unsigned char *kernel_addr;
	unsigned char *ramdisk_addr;
};

#if defined(CONFIG_ARM64)
int nvt_set_fdt_ker_mem_range(unsigned char* fdt_buf);
#endif

int nvt_mmc_io_test(char* test_part_name, unsigned int run_count);
#endif

