/**
	CA9 global timer driver

    @file       timer.c
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
#include <asm/arch-armv7/globaltimer.h>
#include <asm/arch/clock.h>
#include <asm/arch/na51055_regs.h>
#include <div64.h>

DECLARE_GLOBAL_DATA_PTR;

static struct globaltimer *global_timer = \
			(struct globaltimer *)GLOBAL_TIMER_BASE_ADDR;

#define CLK2MHZ(clk)	(clk / 1000 / 1000)

u32 get_timer_clk(void)
{
	return (get_cpu_clk() / 1000) / 8;
}

static u64 get_cpu_global_timer(void)
{
	u32 low, high;
	u64 timer;

	u32 old = readl(&global_timer->cnt_h);
	while (1) {
		low = readl(&global_timer->cnt_l);
		high = readl(&global_timer->cnt_h);
		if (old == high)
			break;
		else
			old = high;
	}

	timer = high;
	return (u64)((timer << 32) | low);
}

int timer_init(void)
{
	u32 val = 0;

	val = readl(&global_timer->ctl);
	if (!(val & 0x01)) {
		writel(0x01, &global_timer->ctl);
		printf("ARM CA9 global timer init successfully\n");
	} else {
		printf("ARM CA9 global timer had already been initiated\n");
	}

	return 0;
}

void __udelay(unsigned long usec)
{
	u64 wait_tick = 0;
	unsigned long long now_tick, start = get_cpu_global_timer();

	wait_tick = get_timer_clk() * usec;
	do {
		now_tick = get_cpu_global_timer();
	} while ((now_tick - start) < wait_tick);
}

ulong get_timer(ulong base)
{
	u64 now = get_cpu_global_timer(), timer_freq = get_timer_clk();
	ulong time_msec;

	do_div(now, timer_freq * 1000);
	time_msec = (ulong)now;

	return time_msec - base;
}

unsigned long long get_ticks(void)
{
	return get_cpu_global_timer();
}

ulong get_tbclk(void)
{
	return (ulong)get_timer_clk();
}
