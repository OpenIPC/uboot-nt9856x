/**
    NVT common header file
    Define DFU functions
    @file       nvt_DFU.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __ARCH_COMMON_NVT_DFU_H__
#define __ARCH_COMMON_NVT_DFU_H__

#include <asm/io.h>
void set_dfu_alt_info(char *interface, char *devstr);
#endif /* __ARCH_COMMON_NVT_DFU_H__ */
