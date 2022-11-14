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

	mov	lr, x29
	ret

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
	/* switch to EL2 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2:


	mov	lr, x29			/* Restore LR */
	ret

ENDPROC(lowlevel_init)

#if 1
ENTRY(local_load)
    dmb     sy

    mov     x1, #0xf0a90000
    movk    x1, #0x2, lsl #32
    str     wzr, [x1]
    dmb     sy

    add     x1, x1, #0x20
    mov     w2, #0x1                        // #1
    str     w2, [x1]

    mov     x2, #0x30                       // #48
    movk    x2, #0xf0a9, lsl #16
    movk    x2, #0x2, lsl #32
    mov     x3, x2

POLL_SET_EN:
    ldr     w4, [x2]                        //0x2F0A90030
    dmb     sy
    tbz     w4, #0, POLL_SET_EN             //if 0x30 == 0, polling continue

    and     w0, w0, #0x1f                   //input & 0x1F (row addr)
    dmb     sy
    mov     x2, #0x4                        // #4
    movk    x2, #0xf0a9, lsl #16
    movk    x2, #0x2, lsl #32
    str     w0, [x2]                        //0x2F0A90004 = w0(param0)

    ldr     w2, [x1]                        //w2 = 0x2F0A90020
    dmb     sy
    orr     w0, w2, #0x10
    dmb     sy
    str     w0, [x1]                        //0x2F0A90020 = 0x11 -> op en

POLL_OP:
    tbnz    w0, #4, POLL_OP_DONE

    mov     x0, #0x8
    movk    x0, #0xf0a9, lsl #16
    movk    x0, #0x2, lsl #32
    ldr     w0, [x0]                        //x0=0x2FA90008
    dmb     sy
    dmb     sy

    str     wzr, [x1]                       //0x2F0A90020 = 0
POLL_STATUS:
    ldr     w1, [x3]                        //w1 = 0x2F0A9030
    dmb     sy
    tbnz    w1, #0, POLL_STATUS
    mov     w0, w0
    ret

POLL_OP_DONE:
	ldr     w0, [x1]
	dmb     sy
	b       POLL_OP
ENDPROC(local_load)

ENTRY(local_store)
    dmb     sy
    mov     x1, #0xf0a90000
    mov     w2, #0x4
    movk    x1, #0x2, lsl #32
    str     w2, [x1]                        //0x2F0A90000 = 4
    dmb     sy                              //set up as program mode

    add     x1, x1, #0x20                   //x1=0x2F0A90020
    mov     w2, #0x1                        // #1
    str     w2, [x1]                        //0x2F0A90020 = 1

    mov     x2, #0x30                       // #48
    movk    x2, #0xf0a9, lsl #16
    movk    x2, #0x2, lsl #32
    mov     x3, x2

POLL_SET_EN_P:
    ldr     w4, [x2]                        //0x2F0A90030
    dmb     sy
    tbz     w4, #0, POLL_SET_EN_P             //if 0x30 == 0, polling continue

    dmb     sy
    mov     x2, #0x4                        // #4
    movk    x2, #0xf0a9, lsl #16
    movk    x2, #0x2, lsl #32
    str     w0, [x2]                        //0x2F0A90004 = w0(param0)

    ldr     w2, [x1]                        //w2 = 0x2F0A90020
    dmb     sy
    orr     w0, w2, #0x10
    dmb     sy
    str     w0, [x1]                        //0x2F0A90020 = 0x11 -> op en

POLL_OP_P:
    tbnz    w0, #4, POLL_OP_DONE_P
    dmb     sy

    str     wzr, [x1]                       //0x2F0A90020 = 0
POLL_STATUS_P:
    ldr     w0, [x3]                        //w1 = 0x2F0A9030
    dmb     sy
    tbnz    w0, #0, POLL_STATUS_P
    mov     x0, #0x0                        // #0
    ret

POLL_OP_DONE_P:
	ldr     w0, [x1]
	dmb     sy
	mov     w0, w0
	b       POLL_OP_P
ENDPROC(local_store)

#endif
