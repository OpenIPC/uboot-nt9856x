// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Novatek Inc. - All Rights Reserved
 */

#include <asm/io.h>
#include "common.h"
#include <errno.h>
#include <malloc.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <linux/libfdt.h>

#define PAD_GETREG(ofs)         readl((void __iomem *)(IOADDR_PAD_REG_BASE + ofs))
#define PAD_SETREG(ofs,value)   writel(value, (void __iomem *)(IOADDR_PAD_REG_BASE + ofs))

#define PAD_PIN_NOT_EXIST       (64) // For backward compatible
#define PAD_PUPD0_REG_OFS       0x00

static ER pad_set_pull_updown(u32 pin, u32 pulltype)
{
	UINT dw_offset, bit_offset;
	REGVALUE    reg_data;

	if(pin == PAD_PIN_NOT_EXIST) {
		pr_err("Not Existed Pad Pin\r\n");
		return E_NOEXS;
	}

	bit_offset = pin & 0x1F;
	dw_offset = (pin >> 5);

	reg_data = PAD_GETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2));
	reg_data &= ~(3 << bit_offset);
	PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
	reg_data |= (pulltype << bit_offset);
	PAD_SETREG(PAD_PUPD0_REG_OFS + (dw_offset << 2), reg_data);
	return E_OK;
}

static int fdt_node_check_pad_config(const void *fdt, int nodeoffset)
{
	const void *prop;
	int len;

	prop = fdt_getprop(fdt, nodeoffset, "pad_config", &len);

	if (prop)
		return len;
	else
		return 0;
}

static void fdt_node_offset_by_pad_config(const void *fdt, int startoffset)
{
	int offset;
	int ndepth = 0;

	for (offset = fdt_next_node(fdt, startoffset, &ndepth);
		(offset >= 0) && (ndepth > 0);
		offset = fdt_next_node(fdt, offset, &ndepth)) {
		if (ndepth == 1) {
			if (fdt_node_check_pad_config(fdt, offset) > 0) {
				u32 *cell = NULL;
				cell = (u32*)fdt_getprop((const void*)fdt, offset, "pad_config", NULL);
				if (pad_set_pull_updown(__be32_to_cpu(cell[2]), __be32_to_cpu(cell[3]))) {
					pr_err("failed to set pin 0x%x pull value 0x%x\n", __be32_to_cpu(cell[2]), __be32_to_cpu(cell[3]));
					break;
				}
			}
		}
	}
}


void nvt_pad_init(void)
{
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	char path[20] = {0};

	sprintf(path,"/top@%x",IOADDR_TOP_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	fdt_node_offset_by_pad_config((const void*)fdt_addr, nodeoffset);
}
