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
#include <asm/ptrace.h>

/*
 * save_boot_params
 *
 * Stack pointer is not yet initialized at this moment.
 * Don't save anything to stack even if compiled with -O0
 */

ENTRY(save_boot_params)
	b	save_boot_params_ret		@ back to my caller
ENDPROC(save_boot_params)

ENTRY(lowlevel_init)
	cps	#MON_MODE
	MRC	p15, 0, r1, c1,c1,0
	isb
	orr	r0, r1, #1
	MCR	p15,0,r0,c1,c1,0
	isb
	mov	r0, #0
	mcrr	p15, 4, r0, r0, c14             /* CNTVOFF = 0 */
	isb
	mcr	p15, 0, r1, c1, c1, 0           /* Set Secure bit */
	isb
	cps	#SVC_MODE

	/*
	 * Setup a temporary stack
	 */
	ldr	sp, =CONFIG_SYS_INIT_SP_ADDR
	bic	sp, sp, #7 /* 8-byte alignment for ABI compliance */
#ifdef CONFIG_SPL_BUILD
	ldr	r9, =gdata
#else
	sub	sp, sp, #GD_SIZE
	bic	sp, sp, #7
	mov	r9, sp
#endif
	/*
	 * Save the old lr(passed in ip) and the current lr to stack
	 */
	push	{ip, lr}

	/*
	 * go setup pll, mux, memory
	 */
	bl	s_init
	pop	{ip, pc}
ENDPROC(lowlevel_init)


ENTRY(local_load)
    STMFD   r13!, {r4-r8, r14}
    MOV	  r4, r0

    MOVW  r2, #0xF066

    @ ==========================================================================
    @ Check if address < 4 (only allow read address 0~3)
    @ ==========================================================================
    MOV   r0, #-33				        @r0 = E_PAR

	MOV   r8, #8	                	@param != this No. break
#   MOV   r8, #31		                @param > 31. break(all address can be read)

    MOV   r5, #0x1F
    AND   r5, r4, r5
    CMP   r8, r5

    BGT   END_PROG_LOAD

    @ ==========================================================================
    @ each key set with 4 fields
    @ ==========================================================================
    MOV   r0, r2, lsl#16

    MOV   r1, #0x01<<0
    STR   r1, [r0, #0x20]       @ R1 = EFUSE_SETUP_EN
1:
    LDR   r1, [r0, #0x30]
    UBFX  r1, r1, #0, #1		@ R1 = EFUSE_SETUP_EN
    CMP   r1, #0x01
    BNE   1b

    @ ======================================================================== @
    @ Start to read address n                                                  @
    @ ======================================================================== @
    MOV   r1, #0x0				@ eFuse index from = 16 (chip ID efuse data)
    ADD   r1, r1, r4	        @ r1 = r1 + (r4) now r1 is address to read

    MOV   r5, #0x0
    STR   r5, [r0, #0x00]

    STR   r1, [r0, #0x04]
    LDR   r2, [r0, #0x20]
    ORR   r2, r2, #0x01<<4		@ OPERATION_EN
    STR   r2, [r0, #0x20]
2:
    LDR   r2, [r0, #0x20]
    UBFX  r2, r2, #4, #1        @ R2 = OPERATION_EN, polling until bit[4] = 0
    CMP   r2, #0x01
    BEQ   2b


3:
    LDR   r2, [r0, #0x30]
    UBFX  r2, r2, #1, #1		@ R2 = OPERATION_EN
    CMP   r2, #0x01
    BNE   3b

    LDR   r2, [r0, #0x30]
    STR	  r2, [r0, #0x30]


    MOV   r2, #0
    STR   r2, [r0, #0x20]

    LDR   r0, [r0, #0x8]

END_PROG_LOAD:
    LDMFD   r13!, {r4-r8, pc}   //If r7 !=1 than return
    nop
ENDPROC(local_load)


ENTRY(local_store)
    STMFD   r13!, {r4-r8, r14}		@ Only allow program efuse once if column address >= 16
    MOV	  r4, r0
    MOVW  r2, #0xF066

    MOV   r0, #-33					@r0 = E_PAR

    MOV   r8, #8                    @addr

    MOV   r5, #0x1F
    AND   r5, r4, r5
    CMP   r8, r5
	BGT   END_PROG_STORE
    @ ==========================================================================
    @ now r0 = 0xF0660000
    @ ==========================================================================
    MOV   r0, r2, lsl#16

    MOV   r1, #0x01<<0
    STR   r1, [r0, #0x20]
1:
    LDR   r1, [r0, #0x30]
    UBFX  r1, r1, #0, #1		    @ R1 = EFUSE_SETUP_EN
    CMP   r1, #0x01
	BNE   1b

    @ ======================================================================== @
    @ Start to program address n                                               @
    @ ======================================================================== @
    MOV   r1, #0x0				    @ eFuse index from = 16 (chip ID efuse data)
	ADD   r1, r1, r4	            @ r1 = r1 + (r4) now r1 is address to read

    MOV   r5, #0x4
    STR   r5, [r0, #0x00]           @ config as program mode


    STR   r1, [r0, #0x04]
    LDR   r2, [r0, #0x20]
    ORR   r2, r2, #0x01<<4		    @ OPERATION_EN
    STR   r2, [r0, #0x20]
2:
    LDR   r2, [r0, #0x20]
    UBFX  r2, r2, #4, #1            @ R2 = OPERATION_EN polling bit[4] until 0
    CMP   r2, #0x01
    BEQ   2b

    LDR   r2, [r0, #0x30]
    STR	  r2, [r0, #0x30]


    MOV   r2, #0
    STR   r2, [r0, #0x20]
    STR   r2, [r0, #0x04]

    MOV   r0, #0

END_PROG_STORE:
    LDMFD   r13!, {r4-r8, pc}   	//If r7 !=1 than return
	nop
ENDPROC(local_store)

