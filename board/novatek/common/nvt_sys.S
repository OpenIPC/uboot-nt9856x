/*
 *  board/novatek/evb658/nvt_sys.S
 *
 *  Author:	Alvin lin
 *  Created:	Oct 18, 2015
 *  Copyright:	Novatek Inc.
 *
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(set_kaddr)
	mov r8, r0
	mov pc, lr
ENDPROC(set_kaddr)

ENTRY(jump_to_secos)
	mov r4, r1
	mov r8, r0
	mov r0, #0
	mov r1, #0
	mov r3, #0
	mov pc, r4
ENDPROC(jump_to_secos)	
