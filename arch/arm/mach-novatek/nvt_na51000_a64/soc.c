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
#include <asm/io.h>
//#include <asm/arch/soc.h>
#include <asm/arch/clock.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
//#include <asm/arch/gpio.h>
#include <asm/arch/na51000_regs.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <linux/sizes.h>
#include <linux/compiler.h>
#include <ns16550.h>
#include <dm.h>

#ifdef CONFIG_NVT_IVOT_DDR_SLOW_DOWN_SUPPORT
extern int nvt_ddr_slow_down(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static struct ns16550_platdata ns16550_com1_pdata = {
	.base = CONFIG_SYS_NS16550_COM1,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_CLK,
};

static struct ns16550_platdata ns16550_com2_pdata = {
	.base = CONFIG_SYS_NS16550_COM2,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_HSCLK,
};

static struct ns16550_platdata ns16550_com3_pdata = {
	.base = CONFIG_SYS_NS16550_COM3,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_HSCLK,
};

static struct ns16550_platdata ns16550_com4_pdata = {
	.base = CONFIG_SYS_NS16550_COM4,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_HSCLK,
};

static struct ns16550_platdata ns16550_com5_pdata = {
	.base = CONFIG_SYS_NS16550_COM5,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_HSCLK,
};

static struct ns16550_platdata ns16550_com6_pdata = {
	.base = CONFIG_SYS_NS16550_COM6,
	.reg_shift = 2,
	.clock = CONFIG_SYS_NS16550_HSCLK,
};

U_BOOT_DEVICE(ns16550_com1) = {
	"ns16550_serial", &ns16550_com1_pdata
#ifdef CONFIG_SYS_NS16550_COM2
	"ns16550_serial", &ns16550_com2_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM3
	"ns16550_serial", &ns16550_com3_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM4
	"ns16550_serial", &ns16550_com4_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM5
	"ns16550_serial", &ns16550_com5_pdata
#endif
#ifdef CONFIG_SYS_NS16550_COM6
	"ns16550_serial", &ns16550_com6_pdata
#endif
};
#endif


static void serial_preinit(void)
{
#if (!CONFIG_UART2_PINMUX_NONE || !CONFIG_UART3_PINMUX_NONE || \
	!CONFIG_UART4_PINMUX_NONE || !CONFIG_UART5_PINMUX_NONE || \
	!CONFIG_UART6_PINMUX_NONE)
	u32 value;
#endif

#ifdef CONFIG_SYS_NS16550_COM2
	#ifdef CONFIG_UART2_PINMUX_CHANNEL_1
		HAL_READ_UINT32(0xF00100A8, value);
		HAL_WRITE_UINT32(0xF00100A8, value & ~0x60000);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0xC;
		HAL_WRITE_UINT32(0xF0010024, value | 0x4);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x800);
	#elif defined(CONFIG_UART2_PINMUX_CHANNEL_2)
		HAL_READ_UINT32(0xF00100A0, value);
		HAL_WRITE_UINT32(0xF00100A0, value & ~0x18000);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0xC;
		HAL_WRITE_UINT32(0xF0010024, value | 0x8);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x800);
	#elif defined(CONFIG_UART2_PINMUX_CHANNEL_3)
		if (nvt_get_chip_id() == CHIP_NA51055) {
			HAL_READ_UINT32(0xF00100A8, value);
			HAL_WRITE_UINT32(0xF00100A8, value & ~0x3);

			HAL_READ_UINT32(0xF0010024, value);
			value &= ~0xC;
			HAL_WRITE_UINT32(0xF0010024, value | 0xC);

			HAL_READ_UINT32(0xF0020074, value);
			HAL_WRITE_UINT32(0xF0020074, value | 0x800);
		}
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM3
	#ifdef CONFIG_UART3_PINMUX_CHANNEL_1
		HAL_READ_UINT32(0xF00100A8, value);
		HAL_WRITE_UINT32(0xF00100A8, value & ~0x6000);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x70;
		HAL_WRITE_UINT32(0xF0010024, value | 0x10);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x400000);
	#elif defined(CONFIG_UART3_PINMUX_CHANNEL_2)
		HAL_READ_UINT32(0xF00100A0, value);
		HAL_WRITE_UINT32(0xF00100A0, value & ~0x600000);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x70;
		HAL_WRITE_UINT32(0xF0010024, value | 0x20);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x400000);
	#elif defined(CONFIG_UART3_PINMUX_CHANNEL_3)
		if (nvt_get_chip_id() == CHIP_NA51055) {
			HAL_READ_UINT32(0xF00100A8, value);
			HAL_WRITE_UINT32(0xF00100A8, value & ~0x30);

			HAL_READ_UINT32(0xF0010024, value);
			value &= ~0x70;
			HAL_WRITE_UINT32(0xF0010024, value | 0x30);

			HAL_READ_UINT32(0xF0020074, value);
			HAL_WRITE_UINT32(0xF0020074, value | 0x400000);
		}
	#elif defined(CONFIG_UART3_PINMUX_CHANNEL_4)
		HAL_READ_UINT32(0xF00100B0, value);
		HAL_WRITE_UINT32(0xF00100B0, value & ~0x50);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x70;
		HAL_WRITE_UINT32(0xF0010024, value | 0x40);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x400000);
	#elif defined(CONFIG_UART3_PINMUX_CHANNEL_5)
		HAL_READ_UINT32(0xF00100A0, value);
		HAL_WRITE_UINT32(0xF00100A0, value & ~0xC);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x70;
		HAL_WRITE_UINT32(0xF0010024, value | 0x50);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x400000);
	#endif
#endif

#ifdef CONFIG_SYS_NS16550_COM4
	if (nvt_get_chip_id() == CHIP_NA51084) {
	#ifdef CONFIG_UART4_PINMUX_CHANNEL_1
		HAL_READ_UINT32(0xF00100A8, value);
		HAL_WRITE_UINT32(0xF00100A8, value & ~0x3);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x3000;
		HAL_WRITE_UINT32(0xF0010024, value | 0x1);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x800000);
	#elif defined(CONFIG_UART4_PINMUX_CHANNEL_2)
		HAL_READ_UINT32(0xF00100A0, value);
		HAL_WRITE_UINT32(0xF00100A0, value & ~0x30);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x3000;
		HAL_WRITE_UINT32(0xF0010024, value | 0x2);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x800000);
	#endif
	}
#endif

#ifdef CONFIG_SYS_NS16550_COM5
	if (nvt_get_chip_id() == CHIP_NA51084) {
	#ifdef CONFIG_UART5_PINMUX_CHANNEL_1
		HAL_READ_UINT32(0xF00100A8, value);
		HAL_WRITE_UINT32(0xF00100A8, value & ~0x30);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0xC000;
		HAL_WRITE_UINT32(0xF0010024, value | 0x4000);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x1000000);
	#elif defined(CONFIG_UART5_PINMUX_CHANNEL_2)
		HAL_READ_UINT32(0xF00100D0, value);
		HAL_WRITE_UINT32(0xF00100D0, value & ~0x18000);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0xC000;
		HAL_WRITE_UINT32(0xF0010024, value | 0x8000);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x1000000);
	#endif
	}
#endif

#ifdef CONFIG_SYS_NS16550_COM6
	if (nvt_get_chip_id() == CHIP_NA51084) {
	#ifdef CONFIG_UART6_PINMUX_CHANNEL_1
		HAL_READ_UINT32(0xF00100A8, value);
		HAL_WRITE_UINT32(0xF00100A8, value & ~0x300);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x30000;
		HAL_WRITE_UINT32(0xF0010024, value | 0x10000);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x2000000);
	#elif defined(CONFIG_UART6_PINMUX_CHANNEL_2)
		HAL_READ_UINT32(0xF00100D0, value);
		HAL_WRITE_UINT32(0xF00100D0, value & ~0x180000);

		HAL_READ_UINT32(0xF0010024, value);
		value &= ~0x30000;
		HAL_WRITE_UINT32(0xF0010024, value | 0x20000);

		HAL_READ_UINT32(0xF0020074, value);
		HAL_WRITE_UINT32(0xF0020074, value | 0x2000000);
	#endif
	}
#endif
}

void enable_caches(void)
{
	dcache_enable();
	icache_enable();
}


void __weak reset_cpu(unsigned long ignored)
{
}


#ifdef CONFIG_ARCH_MISC_INIT
#ifdef CONFIG_NOVATEK_PAD
extern void nvt_pad_init(void);
#endif
int arch_misc_init(void)
{
#ifdef CONFIG_NOVATEK_PAD
	nvt_pad_init();
#endif
	return 0;
}
#endif

/*
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 */
int dram_init(void)
{
#ifdef CONFIG_NVT_IVOT_DDR_SLOW_DOWN_SUPPORT
	// Default not enabled in nvt platform.
	// If specific project needs slow down DDR freq at boot,
	// plz uncomment below line.
//	nvt_ddr_slow_down();
#endif
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

	serial_preinit();

	gd->cpu_clk = get_cpu_clk() * 1000;
}

void cpu_cache_initialization(void)
{
	return;
}

u32 nvt_get_chip_id(void)
{
	unsigned int chip_id;

	chip_id = (readl(IOADDR_TOP_REG_BASE + TOP_VERSION_REG_OFS) >> 16) & 0xFFFF;

	return chip_id;
}
