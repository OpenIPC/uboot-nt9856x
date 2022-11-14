/**
    NVT evb board file
    To handle na51000 HW init.
    @file       na51000_hw_init.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/


#include <common.h>
#include <asm/mach-types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
#include <asm/arch/raw_scanMP.h>
#include <asm/arch/hardware.h>
#endif
#include <linux/libfdt.h>

#define WDT_REG_ADDR(ofs)       (IOADDR_WDT_REG_BASE+(ofs))
#define WDT_GETREG(ofs)         INW(WDT_REG_ADDR(ofs))
#define WDT_SETREG(ofs,value)   OUTW(WDT_REG_ADDR(ofs), (value))

#define CG_REG_ADDR(ofs)       (IOADDR_CG_REG_BASE+(ofs))
#define CG_GETREG(ofs)         INW(CG_REG_ADDR(ofs))
#define CG_SETREG(ofs,value)   OUTW(CG_REG_ADDR(ofs), (value))

#define PAD_REG_ADDR(ofs)       (IOADDR_PAD_REG_BASE+(ofs))
#define PAD_GETREG(ofs)         INW(PAD_REG_ADDR(ofs))
#define PAD_SETREG(ofs,value)   OUTW(PAD_REG_ADDR(ofs), (value))

#define TOP_REG_ADDR(ofs)       (IOADDR_TOP_REG_BASE+(ofs))
#define TOP_GETREG(ofs)         INW(TOP_REG_ADDR(ofs))
#define TOP_SETREG(ofs,value)   OUTW(TOP_REG_ADDR(ofs), (value))

#define CG_PLL_ENABLE_OFS 0x0
#define CG_ENABLE_OFS 0x74
#define CG_RESET_OFS 0x84
#define CG_PLL4_DIV0_OFS 0x1318
#define CG_PLL4_DIV1_OFS 0x131C
#define CG_PLL4_DIV2_OFS 0x1320
#define WDT_POS (1 << 17)

#define WDT_CTRL_OFS 0x0

#if defined(CONFIG_SD_CARD1_POWER_PIN) || defined(CONFIG_SD_CARD2_POWER_PIN) || defined(ETH_PHY_HW_RESET)
static void gpio_set_output(u32 pin)
{
	u32 reg_data;
	u32 ofs = (pin >> 5) << 2;

	pin &= (32 - 1);

	reg_data = INW(IOADDR_GPIO_REG_BASE + 0x20 + ofs);
	reg_data |= (1 << pin);    //output
	OUTW(IOADDR_GPIO_REG_BASE + 0x20 + ofs, reg_data);
}

static void gpio_set_pin(u32 pin)
{
	u32 tmp;
	u32 ofs = (pin >> 5) << 2;

	pin &= (32 - 1);
	tmp = (1 << pin);

	OUTW(IOADDR_GPIO_REG_BASE + 0x40 + ofs, tmp);
}

static void gpio_clear_pin(u32 pin)
{
	u32 tmp;
	u32 ofs = (pin >> 5) << 2;

	pin &= (32 - 1);
	tmp = (1 << pin);

	OUTW(IOADDR_GPIO_REG_BASE + 0x60 + ofs, tmp);
}
#endif

void nvt_ivot_reset_cpu(void)
{
	u32 reg_value;

	reg_value = CG_GETREG(CG_ENABLE_OFS);
	CG_SETREG(CG_ENABLE_OFS, reg_value | WDT_POS);

	reg_value = CG_GETREG(CG_RESET_OFS);
	CG_SETREG(CG_RESET_OFS, reg_value | WDT_POS);


	WDT_SETREG(WDT_CTRL_OFS, 0x5A960112);

	udelay(80);

	WDT_SETREG(WDT_CTRL_OFS, 0x5A960113);
}



static void ethernet_init(void)
{
#if 0
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset, len;
	u32 *cell = NULL;
	char path[20] = {0};
	u32 sensor_cfg = 0x0, eth_cfg = 0x0, reg_val = 0x0;

	sprintf(path,"/top@%x/sensor",IOADDR_TOP_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		return ;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n",__func__, __LINE__);
		return ;
	}

	sensor_cfg = __be32_to_cpu(cell[0]);
	debug("%s(%d) sensor_cfg = 0x%x\n", __func__, __LINE__, sensor_cfg);

	sprintf(path,"/eth@%x",IOADDR_ETH_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
		return ;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "sp-clk", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n", __func__, __LINE__);
		return ;
	}

	eth_cfg = __be32_to_cpu(cell[0]);
	debug("%s(%d) eth_cfg = 0x%x\n", __func__, __LINE__, eth_cfg);

	if ((sensor_cfg & 0x20000) || (eth_cfg == 1)) {
		/* Disable PLL4 */
		reg_val = CG_GETREG(CG_PLL_ENABLE_OFS);
		reg_val &= ~(0x1 << 4);
		CG_SETREG(CG_PLL_ENABLE_OFS, reg_val);

		CG_SETREG(CG_PLL4_DIV0_OFS, 0x0);
		CG_SETREG(CG_PLL4_DIV1_OFS, 0x0);
		CG_SETREG(CG_PLL4_DIV2_OFS, 0x32);

		/* spclk_sel: pll4 */
		reg_val = CG_GETREG(0x24);
		reg_val &= ~0x4300;
		reg_val |= (0x1 << 8);
		CG_SETREG(0x24, reg_val);

		reg_val = CG_GETREG(0x3C);
		reg_val &= ~0xFF000000;
		reg_val |= (0xB << 24);
		CG_SETREG(0x3C, reg_val);

		/* Enable PLL4 */
		reg_val = CG_GETREG(CG_PLL_ENABLE_OFS);
		reg_val |= (0x1 << 4);
		CG_SETREG(CG_PLL_ENABLE_OFS, reg_val);

		/* Enable SP_CLK */
		reg_val = CG_GETREG(0x70);
		reg_val |= (0x1 << 12);
		CG_SETREG(0x70, reg_val);

		/* Pinmux SP_CLK1 */
		reg_val = TOP_GETREG(0xC);
		reg_val &= ~(0x3 << 18);
		reg_val |= (0x1 << 18);
		TOP_SETREG(0xC, reg_val);

		/* Pinmux P_GPIO17 */
		reg_val = TOP_GETREG(0xA8);
		reg_val &= ~(0x1 << 17);
		TOP_SETREG(0xA8, reg_val);
	}
#endif

#ifdef ETH_PHY_HW_RESET
	gpio_set_output(NVT_PHY_RST_PIN);
	gpio_clear_pin(NVT_PHY_RST_PIN);
	mdelay(20);
	gpio_set_pin(NVT_PHY_RST_PIN);
	mdelay(50);
#endif
}

static void usbphy_init(void)
{
	if (TOP_GETREG(0xF0) == 0x50210000) {
		//printf("********************%s(%d) Add USB528 INIT********************************************\n",__func__, __LINE__);
		OUTW(0xFF600400, (INW(0xFF600400))|(0x1 << 21));
		OUTW(0xFF6001C8, (INW(0xFF6001C8))|(0x1 << 31));
	} else {
		//printf("********************%s(%d) Add USB520 INIT********************************************\n",__func__, __LINE__);
		OUTW(0xF0600310, (INW(0xF0600310))|(0x1 << 1));
		OUTW(0xF06001C8, (INW(0xF06001C8))|(0x1 << 5));
	}
}

static void sdio_power_cycle(void)
{
#if (CONFIG_SD_CARD1_POWER_PIN || CONFIG_SD_CARD2_POWER_PIN)
	u32 reg_val;
#endif

#ifdef CONFIG_SD_CARD1_POWER_PIN
	gpio_set_output(CONFIG_SD_CARD1_POWER_PIN);
	if (CONFIG_SD_CARD1_ON_STATE)
		gpio_clear_pin(CONFIG_SD_CARD1_POWER_PIN);
	else
		gpio_set_pin(CONFIG_SD_CARD1_POWER_PIN);

	reg_val = TOP_GETREG(0xA0);
	reg_val |= 0x1F800;
	TOP_SETREG(0xA0, reg_val);

	reg_val = PAD_GETREG(0x0);
	reg_val &= ~0xFFC00000;
	reg_val |= 0x55400000;
	PAD_SETREG(0x0, reg_val);

	reg_val = PAD_GETREG(0x4);
	reg_val &= ~0x3;
	reg_val |= 0x1;
	PAD_SETREG(0x4, reg_val);

	gpio_set_output(C_GPIO(11));
	gpio_set_output(C_GPIO(12));
	gpio_set_output(C_GPIO(13));
	gpio_set_output(C_GPIO(14));
	gpio_set_output(C_GPIO(15));
	gpio_set_output(C_GPIO(16));
	gpio_clear_pin(C_GPIO(11));
	gpio_clear_pin(C_GPIO(12));
	gpio_clear_pin(C_GPIO(13));
	gpio_clear_pin(C_GPIO(14));
	gpio_clear_pin(C_GPIO(15));
	gpio_clear_pin(C_GPIO(16));
#endif

#ifdef CONFIG_SD_CARD2_POWER_PIN
	gpio_set_output(CONFIG_SD_CARD2_POWER_PIN);
	if (CONFIG_SD_CARD2_ON_STATE)
		gpio_clear_pin(CONFIG_SD_CARD2_POWER_PIN);
	else
		gpio_set_pin(CONFIG_SD_CARD2_POWER_PIN);

	reg_val = TOP_GETREG(0xA0);
	reg_val |= 0x7E0000;
	TOP_SETREG(0xA0, reg_val);

	reg_val = PAD_GETREG(0x4);
	reg_val &= ~0x3FFC;
	reg_val |= 0x1554;
	PAD_SETREG(0x4, reg_val);

	gpio_set_output(C_GPIO(17));
	gpio_set_output(C_GPIO(18));
	gpio_set_output(C_GPIO(19));
	gpio_set_output(C_GPIO(20));
	gpio_set_output(C_GPIO(21));
	gpio_set_output(C_GPIO(22));
	gpio_clear_pin(C_GPIO(17));
	gpio_clear_pin(C_GPIO(18));
	gpio_clear_pin(C_GPIO(19));
	gpio_clear_pin(C_GPIO(20));
	gpio_clear_pin(C_GPIO(21));
	gpio_clear_pin(C_GPIO(22));
#endif
}

static void axi_bridge_timeout_en(void)
{
	OUTW(0xF0012018, 0x7c);
	OUTW(0xF0013018, 0x7c);
}

int nvt_ivot_hw_init(void)
{
	ethernet_init();

	sdio_power_cycle();

	axi_bridge_timeout_en();

	usbphy_init();

	return 0;
}

int nvt_ivot_hw_init_early(void)
{
	return 0;
}

#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
typedef void (*LDR_GENERIC_CB)(void);
#define JUMP_ADDR 0xf07C0000

void nvt_ddr_scan(void)
{
	void (*image_entry)(void) = NULL;
	printf("memcpy->[0x%08x]", (int)JUMP_ADDR);

	if (nvt_get_chip_id() == CHIP_NA51084) {
		memcpy((void *)JUMP_ADDR, ddr_scan_528, sizeof(ddr_scan_528));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan_528));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan_528), ARCH_DMA_MINALIGN));
	} else {
		memcpy((void *)JUMP_ADDR, ddr_scan, sizeof(ddr_scan));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan), ARCH_DMA_MINALIGN));
	}
	printf("done\r\n");
	printf("Jump into sram\n");
	image_entry = (LDR_GENERIC_CB)(*((unsigned long*)JUMP_ADDR));
	image_entry();
}
#endif
