/*
 *
 *  Author:	Wayne Lin
 *  Created:	11 30, 2018
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __ARCH_NA51090EVB_H__
#define __ARCH_NA51090EVB_H__

#ifdef CONFIG_ARMV8_MULTIENTRY
/* Loader will use the register value as a pc address then slave core
   will jump to the address when register value is not equal to zero */
#define LOADER_CPU_RELEASE_ADDR 0xf016010c
#endif

#ifdef CONFIG_TARGET_NA51090_A64
/* Bit 0 of REG_ADDR_REMAP indicate the register address remapping
   from 0xFxxxxxxx to 0x2Fxxxxxxx */
#define REG_ADDR_REMAP 0xFFE41014
#endif

void ethernet_init(void);
void sdio_power_cycle(void);

#endif /* __ARCH_NA51090EVB_H__ */
