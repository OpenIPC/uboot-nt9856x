/**
	Provide basic CPU info

    @file       soc.c
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
//#include <asm/arch/soc.h>
#include <asm/arch/clock.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
//#include <asm/arch/gpio.h>

#include <linux/sizes.h>
#include <linux/compiler.h>
#include <ns16550.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static struct ns16550_platdata ns16550_com1_pdata = {
	.base = CONFIG_SYS_NS16550_COM1,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_CLK,
};

U_BOOT_DEVICE(ns16550_com1) = {
	"ns16550_serial", &ns16550_com1_pdata
};
#endif

void enable_caches(void)
{
	dcache_enable();
	icache_enable();
}


void __weak reset_cpu(unsigned long ignored)
{
}


#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	return 0;
}
#endif

/*
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 */
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

/*
 * Print board information
 */
int checkboard(void)
{
	return 0;
}

/*
 * Print CPU information
 */
#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	printf("CPU:   Novatek %s @ %lu MHz\n", _CHIP_NAME_, get_cpu_clk()/1000);

	return 0;
}
#endif

void s_init(void)
{
	//u32 val;

	/* Do we want to combine ARM_UART_B and STBC_UART_1 on EVB board?
	 * NO! So we need set fc040250[bit0] to "0"
	 */
	// val = *(volatile unsigned int *)0xfc040250;
	// if ((val & 0x01) == 0x01) {
		// val &= ~0x01;
		// writel(0x0, 0xfc040250);
	// }

	gd->cpu_clk = get_cpu_clk() * 1000;
}

#if !defined(CONFIG_SYS_L2CACHE_OFF) && defined(CONFIG_SYS_PL310_BASE)
#define L2_MEM_BASE			(CONFIG_SYS_PL310_BASE)
#define L2_REG1_BASE			(L2_MEM_BASE + 0x100)   /* Control */
#define L2_REG1_AUX_CTRL		(L2_MEM_BASE + 0x104)   /* Aux ctrl */
#define L2_REG1_PREFETCH_CTRL		(L2_MEM_BASE + 0xF60)   /* Aux ctrl */
#define L2_REG1_CONTROL			(*((volatile unsigned long *)(L2_REG1_BASE + 0x00)))
#define K_L2_REG1_CONTROL_EN_ON		1
#define K_L2_REG1_CONTROL_EN_OFF	0
#define S_L2_REG1_CONTROL_EN		(0)
void v7_outer_cache_enable(void)
{
#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
	nvt_ivot_l2_cache(0);
	v7_outer_cache_inval_all();
	nvt_ivot_l2_cache(1);
	v7_outer_cache_inval_all();
#else
	L2_REG1_CONTROL = (K_L2_REG1_CONTROL_EN_OFF << S_L2_REG1_CONTROL_EN);
	v7_outer_cache_inval_all();
	L2_REG1_CONTROL = (K_L2_REG1_CONTROL_EN_ON << S_L2_REG1_CONTROL_EN);
	v7_outer_cache_inval_all();
#endif
}

void v7_outer_cache_disable(void)
{
#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
	nvt_ivot_l2_cache(0);
	v7_outer_cache_inval_all();
#else
	L2_REG1_CONTROL = (K_L2_REG1_CONTROL_EN_OFF << S_L2_REG1_CONTROL_EN);
	v7_outer_cache_inval_all();
#endif
}
#endif

#define CLEAR_MMU()	\
	__asm__ __volatile__("mcr p15, 0, r0, c8, c7, 0\n\t");

#define DISABLE_MMU()	\
   ({ \
	 unsigned long val = 0; \
	__asm__ __volatile__( \
		"mrc p15, 0, %0, c1, c0, 0\n\t" \
		"bic %0, %0, #0x1\n\t" \
		"mcr p15, 0, %0, c1, c0, 0\n\t" \
		: "=r" (val) \
		: "r" (val) \
		); \
	})

#define CLEAR_MMU()	\
	__asm__ __volatile__("mcr p15, 0, r0, c8, c7, 0\n\t");


void cpu_cache_initialization(void)
{
	unsigned int actlr;
	unsigned long aux_val = 0;
	unsigned long prefetch_ctrl_val = 0;

#ifdef CONFIG_SYS_L2_PL310
	#ifdef CONFIG_NVT_IVOT_OPTEE_SUPPORT
	nvt_ivot_actlr_smp_cfg(1);
	#else
	asm volatile ("mrc p15, 0, %0, c1, c0, 1" : "=r" (actlr));
	// ACTLR.SMP = 1
	actlr &= ~0x40;
	asm volatile ("mcr p15, 0, %0, c1, c0, 1" : : "r" (actlr));
	printf("ACTLR: 0x%08x\n", actlr);
	actlr |= 0x40;
	asm volatile ("mcr p15, 0, %0, c1, c0, 1" : : "r" (actlr));
	printf("ACTLR: 0x%08x\n", actlr);
	#endif
#endif

	printf("Disable MMU\n");
	DISABLE_MMU();
	printf("Clear MMU\n");
	CLEAR_MMU();

#if !defined(CONFIG_SYS_L2CACHE_OFF) && defined(CONFIG_SYS_PL310_BASE)
	aux_val = readl(L2_REG1_AUX_CTRL);
	prefetch_ctrl_val = readl(L2_REG1_PREFETCH_CTRL);
	printf("Uboot L2 cache aux val: 0x%08lx\n", aux_val);
	printf("Uboot L2 cache prefetch ctrl val: 0x%08lx\n", prefetch_ctrl_val);
	printf("Uboot L2 cache ctrl val: 0x%08lx\n", L2_REG1_CONTROL);
#endif
	printf("Done\n");
}
