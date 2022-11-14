/**
    Provide a head info to the loader parsing.

    @file       nvt_headinfo.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <asm/sections.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/modelext/bin_info.h>

#define __string(_x) #_x
#define __xstring(_x) __string(_x)

extern char uboot_bin_size[];

extern HEADINFO gHeadInfo __attribute__ ((section (".data.bininfo")));
HEADINFO gHeadInfo =
{
	(UINT32)&__image_copy_start, //<- fw CODE entry (4)
	{0}, ///<- reserved (4*19)
	"ub51068 ", //<- CHIP-NAME / TAG-NAME (8)
	"FFFFFFFF", //<- version (8)
	__xstring(_BUILD_DATE_), //<- releasedate (8)
	0xffffffff, //<- Bin File Length (4)
	0xffffffff, //<- Check Sum or CRC (4)
	0,///<- Length check for CRC (4)
	0,///<- where modelext data is. w by Ld / u-boot (4)
#ifdef _NVT_LINUX_SMP_ON_
	2,///<- Bin flag (4)
#else
	0,
#endif
	0,///<- Binary Tag for CRC (4)
};
