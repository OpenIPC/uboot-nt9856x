/*
    Library for ide regiseter control

    This is low level control library for ide display.

    @file       ide2_int.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/

#include "./include/ide_reg.h"
#include "./include/ide2_int.h"

/**
    @addtogroup mIDrvDisp_IDE
*/
//@{
/*
   ide set register

   ide set register

   @param[in] id   ide ID
   @param[in] offset   The register offset
   @param[in] value    The value of specific register offset

   @return void
*/
void idec_set_reg(UINT32 id, UINT32 offset, REGVALUE value)
{
	if (id == IDE_ID_1) {
		IDE_SETREG(offset, value);
	} else {
		IDE2_SETREG(offset, value);
	}
}
/*
   ide get register

   ide get register

   @param[in] id   ide ID
   @param[in] offset   The register offset

   @return The value of specific register offset
*/
REGVALUE idec_get_reg(UINT32 id, UINT32 offset)
{
	if (id == IDE_ID_1) {
		return IDE_GETREG(offset);
	} else {
		return IDE2_GETREG(offset);
	}
}


//@}
