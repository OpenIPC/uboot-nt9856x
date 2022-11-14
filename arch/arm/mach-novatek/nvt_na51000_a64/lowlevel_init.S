/**
	To provide the low level init.

    @file       lowlevel_init.S
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>



ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

	mrs     x15, DAIF
	ldr     x15, =0x3C0
	msr     DAIF, x15

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, x1, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All slaves will enter EL1
	 */

	/* switch to EL2 */
	adr	x4, lowlevel_in_el2_v1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2_v1:

	/* switch to EL1 */
	adr	x4, lowlevel_in_el1_v1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1
lowlevel_in_el1_v1:
	mov	lr, x29
	ret

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
	/* switch to EL2 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2:

	/* switch to EL1 */
	adr	x4, lowlevel_in_el1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1
lowlevel_in_el1:

	mov	lr, x29			/* Restore LR */
	ret

ENDPROC(lowlevel_init)

