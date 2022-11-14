/**
    Clock info

    @file       clock.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/IOAddress.h>

#define CG_CPU_CKCTRL_REG_OFFSET 0x100
#define CPU_CLK_SEL_CPU_APLL  0x80000000
#define CPU_CLOCK_RATE_RATIO0_OFS 0x107E0
#define CPU_CLOCK_RATE_RATIO1_OFS 0x107E4
#define CPU_CLOCK_RATE_RATIO2_OFS 0x107E8

inline void apll_set_data(u8 offset, u8 value)
{
#if 0
	writel(value, (APLL_BASE_ADDR + (offset * 4)));
#endif
}

inline u32 apll_get_data(u8 offset)
{
	return 0;
}

#if 0
inline void apll_enable(pll_page_t page)
{
	if (PLL_PAGE_0 == page) {
		writel(APLL_PAGE_0_EN, (APLL_PAGE_EN_ADDR));
	}
	else if (PLL_PAGE_B == page) {
		writel(APLL_PAGE_B_EN, (APLL_PAGE_EN_ADDR));
	}
	else {
		/* ignore */
	}
}
#endif

inline void mpll_set_data(u8 offset, u8 value)
{
#if 0
	writel(value, (MPLL_BASE_ADDR + (offset * 4)));
#endif
}

inline u32 mpll_get_data(u8 offset)
{
	return 0;
#if 0
	return readl((MPLL_BASE_ADDR + (offset * 4)));
#endif
}

#if 0
inline void mpll_enable(pll_page_t page)
{
	if (PLL_PAGE_0 == page) {
		writel(MPLL_PAGE_0_EN, (MPLL_PAGE_EN_ADDR));
	}
	else if (PLL_PAGE_B == page) {
		writel(MPLL_PAGE_B_EN, (MPLL_PAGE_EN_ADDR));
	}
	else {
		/* ignore */
	}
}
#endif

void set_sys_mpll(unsigned long off, unsigned long val)
{
#if 0
	val <<= 17; val  /= 12;

	mpll_enable(PLL_PAGE_B);

	mpll_set_data((off + 0), ((val >> 0) & 0xff));
	mpll_set_data((off + 1), ((val >> 8) & 0xff));
	mpll_set_data((off + 2), ((val >> 16) & 0xff));
#endif
}

unsigned long get_sys_mpll(unsigned long off)
{
#if 0
	unsigned long val;

	mpll_enable(PLL_PAGE_B);

	val  = mpll_get_data(off);
	val |= (mpll_get_data((off + 1)) << 8);
	val |= (mpll_get_data((off + 2)) << 16);

	val *= 12;val += ((1UL << 17) - 1); val >>= 17;

	return val;
#endif
	return 0;
}

void set_cpu_clk(unsigned long freq)
{
	/* no implement */
}

unsigned long get_cpu_clk(void)
{
#ifdef CONFIG_NVT_FPGA_EMULATION_CA9
	return 24000000;
#else
	u32 cpu_clk_sel, cpu_clk = 0, pll16_clk;
	unsigned int cpu_freq_ratio0, cpu_freq_ratio1, cpu_freq_ratio2;

	cpu_clk_sel = readl(IOADDR_CG_REG_BASE + CG_CPU_CKCTRL_REG_OFFSET) & 0x80000000;

	switch (cpu_clk_sel) {
	case CPU_CLK_SEL_CPU_APLL: {
		cpu_freq_ratio0 = readl(IOADDR_CG_REG_BASE + \
					CPU_CLOCK_RATE_RATIO0_OFS);
		cpu_freq_ratio1 = readl(IOADDR_CG_REG_BASE + \
					CPU_CLOCK_RATE_RATIO1_OFS);
		cpu_freq_ratio2 = readl(IOADDR_CG_REG_BASE + \
					CPU_CLOCK_RATE_RATIO2_OFS);
		pll16_clk = (cpu_freq_ratio0 & 0xFF) | ((cpu_freq_ratio1 && 0xFF) << 8) | \
				((cpu_freq_ratio2 & 0xFF) << 16);

		pll16_clk = (12 * pll16_clk / 131072) * 1000;
		cpu_clk = pll16_clk * 8;

		break;
	}
	default:

		cpu_clk = 80000000;

		break;
	}

	return cpu_clk;
#endif
}
