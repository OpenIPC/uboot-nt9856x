/*
    Library for ide Video1/2 regiseter control

    This is low level control library for ide Video1/2.

    @file       ide2_video.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2010.  All rights reserved.
*/
#include "./include/ide_reg.h"
#include "./include/ide2_int.h"
#include "../include/ide_protected.h"

#ifndef CHKPNT
#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printf("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif

/**
    @addtogroup mIDrvDisp_IDE
*/
//@{

/**
    @name ide Video Control
*/
//@{

/**
    Enable/Disable Video1.

    Enable/Disable Video1.

    @param[in] id   ide ID
    @param[in] b_en     enable/disable for the specific window
      - @b TRUE:    Enable
      - @b FALSE:   Disable.

    @return void
*/
void idec_set_v1_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.v1_en = 1;
	} else {
		reg.bit.v1_en = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Enable/Disable Video2.

    Enable/Disable Video2.

    @param[in] id   ide ID
    @param[in] b_en enable/disable for the specific window.
      - @b TRUE:    Enable
      - @b FALSE:   Disable.

    @return void
*/
void idec_set_v2_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.v2_en = 1;
	} else {
		reg.bit.v2_en = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Video1 window status.

    Video1 window status.

    @param[in] id   ide ID
    @return
      - @b TRUE:    Enable
      - @b FALSE:   Disable.
*/
BOOL idec_get_v1_en(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.v1_en;
}

/**
    Video2 window status.

    Video2 window status.

    @param[in] id   ide ID
    @return
      - @b TRUE:    Enable
      - @b FALSE:   Disable.
*/
BOOL idec_get_v2_en(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.v2_en;
}

//@}

/**
    @name ide Video Buffer
*/
//@{

/**
    Set ide Video1 Buffer 0 starting address.

    Set ide Video1 Buffer 0 starting address.

    @param[in] id   ide ID
    @param[in] ui_y_addr Y starting address.
    @param[in] ui_cb_addr CB starting address.
    @param[in] ui_cr_addr CR starting address.

    @return void
*/
void idec_set_v1_buf0_addr(IDE_ID id, UINT32 ui_y_addr, UINT32 ui_cb_addr, UINT32 ui_cr_addr)
{
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = 0;
	reg.bit.addr = ide_platform_va2pa(ui_y_addr);
	idec_set_reg(id, IDE_V1_DB0_Y_OFS, reg.reg);

	if (id == IDE_ID_1) {
		reg2.reg = idec_get_reg(id, IDE_V1_DB0_UV_OFS);
	} else {
		reg2.reg = idec_get_reg(id, IDE2_V1_DB0_UV_OFS);
	}
	reg2.bit.addr = ide_platform_va2pa(ui_cb_addr);
	idec_set_reg(id, IDE_V1_DB0_UV_OFS, reg2.reg);

	/*reg.reg = 0;
	reg.bit.addr = ui_cr_addr;
	idec_set_reg(id, IDE_V1_DB0_CR_OFS, reg.reg);*/
}

/**
    Set ide Video1 Buffer 1 starting address.

    Set ide Video1 Buffer 1 starting address.

    @param[in] id   ide ID
    @param[in] ui_y_addr  Y starting address.
    @param[in] ui_cb_addr CB starting address.
    @param[in] ui_cr_addr CR starting address.

    @return void
*/
void idec_set_v1_buf1_addr(IDE_ID id, UINT32 ui_y_addr, UINT32 ui_cb_addr, UINT32 ui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = 0;
	reg.bit.addr = ui_y_addr;
	idec_set_reg(id, IDE_V1_DB1_Y_OFS, reg.reg);

	reg2.reg = idec_get_reg(id, IDE_V1_DB1_UV_OFS);
	reg2.bit.addr = ui_cb_addr;
	idec_set_reg(id, IDE_V1_DB1_UV_OFS, reg2.reg);

	/*reg.reg = 0;
	reg.bit.addr = ui_cr_addr;
	idec_set_reg(id, IDE_V1_DB1_CR_OFS, reg.reg);*/
#endif
}

/**
    Set ide Video1 Buffer 2 starting address.

    Set ide Video1 Buffer 2 starting address.

    @param[in] id   ide ID
    @param[in] ui_y_addr  Y starting address.
    @param[in] ui_cb_addr CB starting address.
    @param[in] ui_cr_addr CR starting address.

    @return void
*/
void idec_set_v1_buf2_addr(IDE_ID id, UINT32 ui_y_addr, UINT32 ui_cb_addr, UINT32 ui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = 0;
	reg.bit.addr = ui_y_addr;
	idec_set_reg(id, IDE_V1_DB2_Y_OFS, reg.reg);

	reg2.reg = idec_get_reg(id, IDE_V1_DB2_UV_OFS);
	reg2.bit.addr = ui_cb_addr;
	idec_set_reg(id, IDE_V1_DB2_UV_OFS, reg2.reg);

	/*reg.reg = 0;
	reg.bit.addr = ui_cr_addr;
	idec_set_reg(id, IDE_V1_DB2_CR_OFS, reg.reg);*/
#endif
}

/**
    Set ide Video1 Buffer 0 odd start.

    Set ide Video1 Buffer 0 odd start.

    @param[in] id   ide ID
    @param[in] b_odd    FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_set_v1_buf0_odd(IDE_ID id, BOOL b_odd)
{
	UINT32 reg;

	if (id == IDE_ID_1) {
		reg = idec_get_reg(id, IDE_V1_DB0_UV_ODD_OFS);
	} else {
		reg = idec_get_reg(id, IDE2_V1_DB0_UV_ODD_OFS);
	}
	if (b_odd) {
		reg = 0x80000000;
	} else {
		reg = 0;
	}
	idec_set_reg(id, IDE_V1_DB0_UV_ODD_OFS, reg);
}

/*
    Set ide Video1 Buffer 1 odd start.

    Set ide Video1 Buffer 1 odd start.

    @param[in] id   ide ID
    @param[in] b_odd    FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_set_v1_buf1_odd(IDE_ID id, BOOL b_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V1_DB1_UV_OFS);
	if (b_odd) {
		reg.bit.vodd = 1;
	} else {
		reg.bit.vodd = 0;
	}
	idec_set_reg(id, IDE_V1_DB1_UV_OFS, reg.reg);
#endif
}

/*
    Set ide Video1 Buffer 2 odd start.

    Set ide Video1 Buffer 2 odd start.

    @param[in] id   ide ID
    @param[in] b_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_set_v1_buf2_odd(IDE_ID id, BOOL b_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V1_DB2_UV_OFS);
	if (b_odd) {
		reg.bit.vodd = 1;
	} else {
		reg.bit.vodd = 0;
	}
	idec_set_reg(id, IDE_V1_DB2_UV_OFS, reg.reg);
#endif
}

/*
    Set ide Video1 source input is interlace/progressive

    Set ide Video1 source input is interlace/progressive

    @param[in] id   ide ID
    @param[in] bInInterlace  source input is interlace/progressive
      - @b TRUE:   source input is interlace
      - @b FALSE:   source input is progressive

*/
/*void idec_setV1InInterlace(IDE_ID id, BOOL bInInterlace)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
    reg_attr.bit.in_interlace = bInInterlace;
    idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg_attr.reg);
}*/

/*
    Get ide Video1 source input is interlace/progressive

    Get ide Video1 source input is interlace/progressive

    @param[in] id   ide ID
    @return  uiInInterlace  source input is interlace/progressive
      - @b TRUE:   source input is interlace
      - @b FALSE:   source input is progressive

*/
/*BOOL idec_getV1InInterlace(IDE_ID id)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
    return reg_attr.bit.in_interlace;
}*/

/*
    Set ide Video1 line buffer Enable/Disable

    Set ide Video1 line buffer Enable/Disable

    @param[in] id   ide ID
    @param[in] bYlinebuf  Y Channel line buffer config
      - @b TRUE:   Enable
      - @b FALSE:   Disable
   @param[in] bCLinebuf  C Channel line buffer config
      - @b TRUE:     Enable
      - @b FALSE:   Disable


*/
/*void ide_csetV1LineBufferEn(IDE_ID id, BOOL bYlinebuf, BOOL bCLinebuf)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
    reg_attr.bit.y_lbuf_en = bYlinebuf;
    reg_attr.bit.c_lbuf_en = bCLinebuf;
    idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg_attr.reg);
}*/

/*
     Get ide Video1 line buffer Enable/Disable

     Get ide Video1 line buffer Enable/Disable

    @param[in] id   ide ID
     @param[out] bYlinebuf  Y Channel line buffer config
       - @b TRUE:   Enable
       - @b FALSE:   Disable
    @param[out] bCLinebuf  C Channel line buffer config
       - @b TRUE:     Enable
       - @b FALSE: Disable


*/
/*BOOL idec_getV1LineBufferEn(IDE_ID id, BOOL *bYlinebuf, BOOL *bCLinebuf)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
    *bYlinebuf = reg_attr.bit.y_lbuf_en;
    *bCLinebuf = reg_attr.bit.c_lbuf_en;
}*/





/**
    Set ide Video1 Buffer dimension.

    Set ide Video1 Buffer dimension.

    @param[in] id   ide ID
    @param[in] ui_bw         buffer width.
    @param[in] ui_bh         buffer height.
    @param[in] ui_lineoffset buffer lineoffset.

    @return void
*/
void idec_set_v1_buf_dim(IDE_ID id, UINT32 ui_bw, UINT32 ui_bh, UINT32 ui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_VBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_V1_BUF_DIM_OFS);
	reg_dim.bit.width = ui_bw;
	reg_dim.bit.height = ui_bh;
	idec_set_reg(id, IDE_V1_BUF_DIM_OFS, reg_dim.reg);

	reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
	reg_attr.bit.lofs = ui_lineoffset;
	idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg_attr.reg);
}

/**
    Set ide Video1 buffer read order.

    Set ide Video1 buffer read order.

    @param[in] id   ide ID
    @param[in] b_l2r: Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[in] b_t2b: Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_set_v1_read_order(IDE_ID id, IDE_HOR_READ b_l2r, IDE_VER_READ b_t2b)
{
	T_IDE_VBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);

	if (b_l2r) {
		reg.bit.l2r = 1;
	} else {
		reg.bit.l2r = 0;
	}

	if (b_t2b) {
		reg.bit.t2b = 1;
	} else {
		reg.bit.t2b = 0;
	}

	idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg.reg);
}

/*
    Set ide Video1 buffer operation.

    Set ide Video1 buffer operation.

    @param[in] id   ide ID
    @param[in] ui_bjmode: Buffer mode
		- IDE_VIDEO_BJMODE_CANT_CROSS_WRITE: Automatically change buffer according to IPE/IME signal
		- IDE_VIDEO_BJMODE_RW_INDEPENDENT  : Automatically change buffer according to ide's VSync signal
		- IDE_VIDEO_BJMODE_BUFFER_REPEAT   : Repeat the same buffer until user change buffer
    @param[in] ui_optbuf: Buffer in operation
		- IDE_VIDEO_BUFFER_OPT_0: Buffer 0 in operation
		- IDE_VIDEO_BUFFER_OPT_1: Buffer 1 in operation
		- IDE_VIDEO_BUFFER_OPT_2: Buffer 2 in operation
    @param[in] ui_buf_num: Operation buffer number
		- IDE_VIDEO_BUFFER_NUM_1: 1 buffer
		- IDE_VIDEO_BUFFER_NUM_2: 2 buffers
		- IDE_VIDEO_BUFFER_NUM_3: 3 buffers

    @return void
*/
void idec_set_v1_buf_op(IDE_ID id, IDE_BJMODE ui_bjmode, IDE_OP_BUF ui_optbuf, IDE_BUF_NUM ui_buf_num)
{
	/*    T_IDE_VBUF_ATTR reg;

	    ide_chk_range(ui_bjmode, IDE_VIDEO_BJMODE_CANT_CROSS_WRITE, IDE_VIDEO_BJMODE_BUFFER_REPEAT);
	    ide_chk_range(ui_optbuf, IDE_VIDEO_BUFFER_OPT_0, IDE_VIDEO_BUFFER_OPT_3);
	    ide_chk_range(ui_buf_num, IDE_VIDEO_BUFFER_NUM_1, IDE_VIDEO_BUFFER_NUM_4);

	    reg.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);

	    reg.bit.bjmode = ui_bjmode;
	    reg.bit.op_buf = ui_optbuf;
	    reg.bit.buf_num = ui_buf_num;
	    reg.bit.op_buf_en = 1;

	    idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg.reg);*/
}

/*
    Set ide Video1 buffer change in single buffer.

    Set ide Video1 buffer change in single buffer.

    @param[in] id   ide ID
    @param[in] ui_optbuf: Buffer in operation
		- IDE_VIDEO_BUFFER_OPT_0: Buffer 0 in operation
		- IDE_VIDEO_BUFFER_OPT_1: Buffer 1 in operation
		- IDE_VIDEO_BUFFER_OPT_2: Buffer 2 in operation

    @return void
*/
void idec_ch_v1_buf(IDE_ID id, IDE_OP_BUF ui_optbuf)
{
	/*    T_IDE_VBUF_ATTR reg;

	    reg.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);

	    reg.bit.op_buf = ui_optbuf;
	    reg.bit.op_buf_en = 1;

	    idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg.reg);

	    //ide_set_load();*/
}

/*
    Set ide Video1 sync source

    Set ide Video1 sync source

    @param[in] id   ide ID
    @param[in] uisel video sync source
		- @b IDE_SYNC_IME_PATH1
		- @b IDE_SYNC_IME_PATH2
		- @b IDE_SYNC_IME_PATH3
*/
void idec_set_v1_src(IDE_ID id, IDE_SYNC_SRC uisel)
{
	/*    T_IDE_VBUF_ATTR reg;

	    reg.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);

	    reg.bit.src = uisel;

	    idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg.reg);*/
}

/**
    Set ide Video2 Buffer 0 starting address.

    Set ide Video2 Buffer 0 starting address.

    @param[in] id   ide ID
    @param[in] ui_y_addr Y starting address.
    @param[in] ui_cb_addr CB starting address.
    @param[in] ui_cr_addr CR starting address.

    @return void
*/
void idec_set_v2_buf0_addr(IDE_ID id, UINT32 ui_y_addr, UINT32 ui_cb_addr, UINT32 ui_cr_addr)
{
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = 0;
	reg.bit.addr = ide_platform_va2pa(ui_y_addr);
	idec_set_reg(id, IDE_V2_DB0_Y_OFS, reg.reg);

	reg2.reg = idec_get_reg(id, IDE_V2_DB0_UV_OFS);
	reg2.bit.addr = ide_platform_va2pa(ui_cb_addr);
	idec_set_reg(id, IDE_V2_DB0_UV_OFS, reg2.reg);

	/*reg.reg = 0;
	reg.bit.addr = ui_cr_addr;
	idec_set_reg(id, IDE_V2_DB0_CR_OFS, reg.reg);*/
}

/*
    Set ide Video2 Buffer 1 starting address.

    Set ide Video2 Buffer 1 starting address.

    @param[in] id   ide ID
    @param[in] ui_y_addr Y starting address.
    @param[in] ui_cb_addr CB starting address.
    @param[in] ui_cr_addr CR starting address.

    @return void
*/
void idec_set_v2_buf1_addr(IDE_ID id, UINT32 ui_y_addr, UINT32 ui_cb_addr, UINT32 ui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = 0;
	reg.bit.addr = ui_y_addr;
	idec_set_reg(id, IDE_V2_DB1_Y_OFS, reg.reg);

	reg2.reg = idec_get_reg(id, IDE_V2_DB1_UV_OFS);
	reg2.bit.addr = ui_cb_addr;
	idec_set_reg(id, IDE_V2_DB1_UV_OFS, reg2.reg);

	/*reg.reg = 0;
	reg.bit.addr = ui_cr_addr;
	idec_set_reg(id, IDE_V2_DB1_CR_OFS, reg.reg);*/
#endif
}

/*
    Set ide Video2 Buffer 2 starting address.

    Set ide Video2 Buffer 2 starting address.

    @param[in] id   ide ID
    @param[in] ui_y_addr Y starting address.
    @param[in] ui_cb_addr CB starting address.
    @param[in] ui_cr_addr CR starting address.

    @return void
*/
void idec_set_v2_buf2_addr(IDE_ID id, UINT32 ui_y_addr, UINT32 ui_cb_addr, UINT32 ui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = 0;
	reg.bit.addr = ui_y_addr;
	idec_set_reg(id, IDE_V2_DB2_Y_OFS, reg.reg);

	reg2.reg = idec_get_reg(id, IDE_V2_DB2_UV_OFS);
	reg2.bit.addr = ui_cb_addr;
	idec_set_reg(id, IDE_V2_DB2_UV_OFS, reg2.reg);

	/*reg.reg = 0;
	reg.bit.addr = ui_cr_addr;
	idec_set_reg(id, IDE_V2_DB2_CR_OFS, reg.reg);*/
#endif
}

/**
    Set ide Video2 Buffer 0 odd start.

    Set ide Video2 Buffer 0 odd start.

    @param[in] id   ide ID
    @param[in] b_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_set_v2_buf0_odd(IDE_ID id, BOOL b_odd)
{
	UINT32 reg;

	reg = idec_get_reg(id, IDE_V2_DB0_UV_ODD_OFS);
	if (b_odd) {
		reg = 0x80000000;
	} else {
		reg = 0;
	}
	idec_set_reg(id, IDE_V2_DB0_UV_ODD_OFS, reg);
}

/*
    Set ide Video2 Buffer 1 odd start.

    Set ide Video2 Buffer 1 odd start.

    @param[in] id   ide ID
    @param[in] b_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_set_v2_buf1_odd(IDE_ID id, BOOL b_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V2_DB1_UV_OFS);
	if (b_odd) {
		reg.bit.vodd = 1;
	} else {
		reg.bit.vodd = 0;
	}
	idec_set_reg(id, IDE_V2_DB1_UV_OFS, reg.reg);
#endif
}

/*
    Set ide Video2 Buffer 2 odd start.

    Set ide Video2 Buffer 2 odd start.

    @param[in] id   ide ID
    @param[in] b_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_set_v2_buf2_odd(IDE_ID id, BOOL b_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V2_DB2_UV_OFS);
	if (b_odd) {
		reg.bit.vodd = 1;
	} else {
		reg.bit.vodd = 0;
	}
	idec_set_reg(id, IDE_V2_DB2_UV_OFS, reg.reg);
#endif
}

/*
    Set ide Video2 source input is interlace/progressive

    Set ide Video2 source input is interlace/progressive

    @param[in] id   ide ID
    @param[in] bInInterlace  source input is interlace/progressive
		- @b TRUE:   source input is interlace
		- @b FALSE:   source input is progressive

*/
/*void idec_setV2InInterlace(IDE_ID id, BOOL bInInterlace)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);
    reg_attr.bit.in_interlace = bInInterlace;
    idec_set_reg(id, IDE_V2_BUF_ATTR_OFS, reg_attr.reg);
}*/

/*
    Get ide Video2 source input is interlace/progressive

    Get ide Video2 source input is interlace/progressive

    @param[in] id   ide ID
    @return  uiInInterlace  source input is interlace/progressive
		- @b TRUE:   source input is interlace
		- @b FALSE:   source input is progressive

*/
/*BOOL idec_getV2InInterlace(IDE_ID id)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);
    return reg_attr.bit.in_interlace;
}*/

/*
    Set ide Video2 line buffer Enable/Disable

    Set ide Video2 line buffer Enable/Disable

    @param[in] id   ide ID
    @param[in] bYlinebuf  Y Channel line buffer config
		- @b TRUE:   Enable
		- @b FALSE:   Disable
   @param[in] bCLinebuf  C Channel line buffer config
		- @b TRUE:     Enable
		- @b FALSE:   Disable


*/
/*void idec_setV2LineBufferEn(IDE_ID id, BOOL bYlinebuf, BOOL bCLinebuf)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
    reg_attr.bit.y_lbuf_en = bYlinebuf;
    reg_attr.bit.c_lbuf_en = bCLinebuf;
    idec_set_reg(id, IDE_V1_BUF_ATTR_OFS, reg_attr.reg);
}*/

/*
     Get ide Video2 line buffer Enable/Disable

     Get ide Video2 line buffer Enable/Disable

    @param[in] id   ide ID
     @param[out] bYlinebuf  Y Channel line buffer config
		- @b TRUE:   Enable
		- @b FALSE:   Disable
    @param[out] bCLinebuf  C Channel line buffer config
		- @b TRUE:     Enable
		- @b FALSE: Disable


*/
/*BOOL idec_getV2LineBufferEn(IDE_ID id, OOL *bYlinebuf, BOOL *bCLinebuf)
{
    T_IDE_VBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
    *bYlinebuf = reg_attr.bit.y_lbuf_en;
    *bCLinebuf = reg_attr.bit.c_lbuf_en;
}*/



/**
    Set ide Video2 Buffer dimension.

    Set ide Video2 Buffer dimension.

    @param[in] id   ide ID
    @param[in] ui_bw  buffer width.
    @param[in] ui_bh  buffer height.
    @param[in] ui_lineoffset buffer lineoffset.

    @return void
*/
void idec_set_v2_buf_dim(IDE_ID id, UINT32 ui_bw, UINT32 ui_bh, UINT32 ui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_VBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_V2_BUF_DIM_OFS);
	reg_dim.bit.width = ui_bw;
	reg_dim.bit.height = ui_bh;
	idec_set_reg(id, IDE_V2_BUF_DIM_OFS, reg_dim.reg);

	reg_attr.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);
	reg_attr.bit.lofs = ui_lineoffset;
	idec_set_reg(id, IDE_V2_BUF_ATTR_OFS, reg_attr.reg);
}

/**
    Set ide Video2 buffer read order.

    Set ide Video2 buffer read order.

    @param[in] id   ide ID
    @param[in] b_l2r: Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[in] b_t2b: Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_set_v2_read_order(IDE_ID id, IDE_HOR_READ b_l2r, IDE_VER_READ b_t2b)
{
	T_IDE_VBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);

	if (b_l2r) {
		reg.bit.l2r = 1;
	} else {
		reg.bit.l2r = 0;
	}

	if (b_t2b) {
		reg.bit.t2b = 1;
	} else {
		reg.bit.t2b = 0;
	}

	idec_set_reg(id, IDE_V2_BUF_ATTR_OFS, reg.reg);
}

/*
    Set ide Video2 buffer operation.

    Set ide Video2 buffer operation.

    @param[in] id   ide ID
    @param[in] ui_bjmode: Buffer mode
		- IDE_VIDEO_BJMODE_CANT_CROSS_WRITE: Automatically change buffer according to IPE/IME signal
		- IDE_VIDEO_BJMODE_RW_INDEPENDENT  : Automatically change buffer according to ide's VSync signal
		- IDE_VIDEO_BJMODE_BUFFER_REPEAT   : Repeat the same buffer until user change buffer
    @param[in] ui_optbuf: Buffer in operation
		- IDE_VIDEO_BUFFER_OPT_0: Buffer 0 in operation
		- IDE_VIDEO_BUFFER_OPT_1: Buffer 1 in operation
		- IDE_VIDEO_BUFFER_OPT_2: Buffer 2 in operation
    @param[in] ui_buf_num: Operation buffer number
		- IDE_VIDEO_BUFFER_NUM_1: 1 buffer
		- IDE_VIDEO_BUFFER_NUM_2: 2 buffers
		- IDE_VIDEO_BUFFER_NUM_3: 3 buffers

    @return void
*/
void idec_set_v2_buf_op(IDE_ID id, IDE_BJMODE ui_bjmode, IDE_OP_BUF ui_optbuf, IDE_BUF_NUM ui_buf_num)
{
	/*    T_IDE_VBUF_ATTR reg;

	    ide_chk_range(ui_bjmode, IDE_VIDEO_BJMODE_CANT_CROSS_WRITE, IDE_VIDEO_BJMODE_BUFFER_REPEAT);
	    ide_chk_range(ui_optbuf, IDE_VIDEO_BUFFER_OPT_0, IDE_VIDEO_BUFFER_OPT_2);
	    ide_chk_range(ui_buf_num, IDE_VIDEO_BUFFER_NUM_1, IDE_VIDEO_BUFFER_NUM_4);

	    reg.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);

	    reg.bit.bjmode = ui_bjmode;
	    reg.bit.op_buf = ui_optbuf;
	    reg.bit.buf_num = ui_buf_num;
	    reg.bit.op_buf_en = 1;

	    idec_set_reg(id, IDE_V2_BUF_ATTR_OFS, reg.reg);*/
}

/*
    Set ide Video2 buffer change in single buffer.

    Set ide Video2 buffer change in single buffer.

    @param[in] id   ide ID
    @param[in] ui_optbuf: Buffer in operation
		- IDE_VIDEO_BUFFER_OPT_0: Buffer 0 in operation
		- IDE_VIDEO_BUFFER_OPT_1: Buffer 1 in operation
		- IDE_VIDEO_BUFFER_OPT_2: Buffer 2 in operation

    @return void
*/
void idec_ch_v2_buf(IDE_ID id, IDE_OP_BUF ui_optbuf)
{
	/*    T_IDE_VBUF_ATTR reg;

	    reg.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);

	    reg.bit.op_buf = ui_optbuf;
	    reg.bit.op_buf_en = 1;

	    idec_set_reg(id, IDE_V2_BUF_ATTR_OFS, reg.reg);

	    //ide_set_load();*/
}

/*
    Set ide Video2 sync source

    Set ide Video2 sync source

    @param[in] id   ide ID
    @param[in] uisel video sync source
		- @b IDE_SYNC_IME_PATH1
		- @b IDE_SYNC_IME_PATH2
		- @b IDE_SYNC_IME_PATH3
*/
void idec_set_v2_src(IDE_ID id, IDE_SYNC_SRC uisel)
{
	/*    T_IDE_VBUF_ATTR reg;

	    reg.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);

	    reg.bit.src = uisel;

	    idec_set_reg(id, IDE_V2_BUF_ATTR_OFS, reg.reg);*/
}


/**
    Get ide Video1 Buffer0 Address for Y/Cb/Cr

    Get ide Video1 Buffer0 Address for Y/Cb/Cr

    @param[in] id   ide ID
    @param[out] pui_y_addr  The starting address of Y
    @param[out] pui_cb_addr The starting address of Cb
    @param[out] pui_cr_addr The starting address of Cr

    @return void
*/
void idec_get_v1_buf0_addr(IDE_ID id, UINT32 *pui_y_addr, UINT32 *pui_cb_addr, UINT32 *pui_cr_addr)
{
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;
	UINT32 ui_v1_db0_yofs = 0x0;
	UINT32 ui_v1_db0_uvofs = 0x0;

	if (IDE_ID_1 == id) {
		ui_v1_db0_yofs = IDE_V1_DB0_Y_OFS;
		ui_v1_db0_uvofs = IDE_V1_DB0_UV_OFS;
	} else {
		ui_v1_db0_yofs = IDE2_V1_DB0_Y_OFS;
		ui_v1_db0_uvofs = IDE2_V1_DB0_UV_OFS;
	}
	reg.reg = idec_get_reg(id, ui_v1_db0_yofs);
	*pui_y_addr = dma_getNonCacheAddr(reg.bit.addr);



	reg2.reg = idec_get_reg(id, ui_v1_db0_uvofs);
	*pui_cb_addr = dma_getNonCacheAddr(reg2.bit.addr);

	/*reg.reg = idec_get_reg(id, IDE_V1_DB0_CR_OFS);
	*pui_cr_addr = reg.bit.addr;*/
}

/*
    Get ide Video1 Buffer1 Address for Y/Cb/Cr

    Get ide Video1 Buffer1 Address for Y/Cb/Cr

    @param[in] id   ide ID
    @param[out] pui_y_addr  The starting address of Y
    @param[out] pui_cb_addr The starting address of Cb
    @param[out] pui_cr_addr The starting address of Cr

    @return void
*/
void idec_get_v1_buf1_addr(IDE_ID id, UINT32 *pui_y_addr, UINT32 *pui_cb_addr, UINT32 *pui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = idec_get_reg(id, IDE_V1_DB1_Y_OFS);
	*pui_y_addr = reg.bit.addr;

	reg2.reg = idec_get_reg(id, IDE_V1_DB1_UV_OFS);
	*pui_cb_addr = reg2.bit.addr;

	/*reg.reg = idec_get_reg(id, IDE_V1_DB1_CR_OFS);
	*pui_cr_addr = reg.bit.addr;*/
#endif
}

/*
    Get ide Video1 Buffer2 Address for Y/Cb/Cr

    Get ide Video1 Buffer2 Address for Y/Cb/Cr

    @param[in] id   ide ID
    @param[out] pui_y_addr  The starting address of Y
    @param[out] pui_cb_addr The starting address of Cb
    @param[out] pui_cr_addr The starting address of Cr

    @return void
*/
void idec_get_v1_buf2_addr(IDE_ID id, UINT32 *pui_y_addr, UINT32 *pui_cb_addr, UINT32 *pui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = idec_get_reg(id, IDE_V1_DB2_Y_OFS);
	*pui_y_addr = reg.bit.addr;

	reg2.reg = idec_get_reg(id, IDE_V1_DB2_UV_OFS);
	*pui_cb_addr = reg2.bit.addr;

	/*reg.reg = idec_get_reg(id, IDE_V1_DB2_CR_OFS);
	*pui_cr_addr = reg.bit.addr;*/
#endif
}

/**
    Get ide Video1 Buffer0 Odd Start

    Get ide Video1 Buffer0 Odd Start

    @param[in] id   ide ID
    @param[out] pb_odd FLASE:others TRUE:420 starts from odd line

    @return void
*/
void idec_get_v1_buf0_odd(IDE_ID id, BOOL *pb_odd)
{
	UINT32 reg;

	if (id == IDE_ID_1) {
		reg = idec_get_reg(id, IDE_V1_DB0_UV_ODD_OFS);
	} else {
		reg = idec_get_reg(id, IDE2_V1_DB0_UV_ODD_OFS);
	}
	*pb_odd = ((reg & 0x80000000) >> 31);
}

/*
    Get ide Video1 Buffer1 Odd Start

    Get ide Video1 Buffer1 Odd Start

    @param[in] id   ide ID
    @param[out] pb_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_get_v1_buf1_odd(IDE_ID id, BOOL *pb_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V1_DB1_UV_OFS);
	*pb_odd = reg.bit.vodd;
#endif
}

/*
    Get ide Video1 Buffer2 Odd Start

    Get ide Video1 Buffer2 Odd Start

    @param[in] id   ide ID
    @param[out] pb_odd FLASE:others TRUE:420 starts from odd line

    @return void
*/
void idec_get_v1_buf2_odd(IDE_ID id, BOOL *pb_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V1_DB2_UV_OFS);
	*pb_odd = reg.bit.vodd;
#endif
}

/**
    Get ide Video1 Buffer dimension.

    Get ide Video1 Buffer dimension.

    @param[in] id   ide ID
    @param[out] pui_bw           buffer width.
    @param[out] pui_bh           buffer height.
    @param[out] pui_lineoffset   buffer lineoffset.

    @return void
*/
void idec_get_v1_buf_dim(IDE_ID id, UINT32 *pui_bw, UINT32 *pui_bh, UINT32 *pui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_VBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_V1_BUF_DIM_OFS);
	*pui_bw = reg_dim.bit.width;
	*pui_bh = reg_dim.bit.height;

	reg_attr.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
	*pui_lineoffset = reg_attr.bit.lofs;
}

/**
    Get ide Video1 buffer operation

    Get ide Video1 buffer operation

    @param[in] id   ide ID
    @param[out] pui_bjmode Buffer mode
    @param[out] pui_optbuf Buffer in operation
    @param[out] pui_bufnum Operation buffer number

    @return void
*/
void idec_get_v1_buf_op(IDE_ID id, IDE_BJMODE *pui_bjmode, IDE_OP_BUF *pui_optbuf, IDE_BUF_NUM *pui_bufnum)
{
	/*    T_IDE_VBUF_ATTR reg;

	    reg.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
	    *pui_bjmode = reg.bit.bjmode;
	    *pui_optbuf = reg.bit.op_buf;
	    *pui_bufnum = reg.bit.buf_num;*/
	*pui_bjmode = IDE_VIDEO_BJMODE_BUFFER_REPEAT;
	*pui_optbuf = IDE_VIDEO_BUFFER_OPT_0;
	*pui_bufnum = IDE_VIDEO_BUFFER_NUM_1;
}

/**
    Get ide Video1 buffer read order.

    Get ide Video1 buffer read order.

    @param[in] id   ide ID
    @param[out] pb_l2r Read order
    @param[out] pb_t2b Read order

    @return void
*/
void idec_get_v1_read_order(IDE_ID id, IDE_HOR_READ *pb_l2r, IDE_VER_READ *pb_t2b)
{
	T_IDE_VBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_V1_BUF_ATTR_OFS);
	*pb_l2r = reg.bit.l2r;
	*pb_t2b = reg.bit.t2b;
}

/**
    Get ide Video2 Buffer0 Address for Y/Cb/Cr

    Get ide Video2 Buffer0 Address for Y/Cb/Cr

    @param[in] id   ide ID
    @param[out] pui_y_addr  The starting address of Y
    @param[out] pui_cb_addr The starting address of Cb
    @param[out] pui_cr_addr The starting address of Cr

    @return void
*/
void idec_get_v2_buf0_addr(IDE_ID id, UINT32 *pui_y_addr, UINT32 *pui_cb_addr, UINT32 *pui_cr_addr)
{
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = idec_get_reg(id, IDE_V2_DB0_Y_OFS);
	*pui_y_addr = dma_getNonCacheAddr(reg.bit.addr);

	reg2.reg = idec_get_reg(id, IDE_V2_DB0_UV_OFS);
	*pui_cb_addr = dma_getNonCacheAddr(reg2.bit.addr);

	/*reg.reg = idec_get_reg(id, IDE_V2_DB0_CR_OFS);
	*pui_cr_addr = reg.bit.addr;*/
}

/*
    Get ide Video2 Buffer1 Address for Y/Cb/Cr

    Get ide Video2 Buffer1 Address for Y/Cb/Cr

    @param[in] id   ide ID
    @param[out] pui_y_addr  The starting address of Y
    @param[out] pui_cb_addr The starting address of Cb
    @param[out] pui_cr_addr The starting address of Cr

    @return void
*/
void idec_get_v2_buf1_addr(IDE_ID id, UINT32 *pui_y_addr, UINT32 *pui_cb_addr, UINT32 *pui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = idec_get_reg(id, IDE_V2_DB1_Y_OFS);
	*pui_y_addr = reg.bit.addr;

	reg2.reg = idec_get_reg(id, IDE_V2_DB1_UV_OFS);
	*pui_cb_addr = reg2.bit.addr;

	/*reg.reg = idec_get_reg(id, IDE_V2_DB1_CR_OFS);
	*pui_cr_addr = reg.bit.addr;*/
#endif
}

/*
    Get ide Video2 Buffer2 Address for Y/Cb/Cr

    Get ide Video2 Buffer2 Address for Y/Cb/Cr

    @param[in] id   ide ID
    @param[out] pui_y_addr  The starting address of Y
    @param[out] pui_cb_addr The starting address of Cb
    @param[out] pui_cr_addr The starting address of Cr

    @return void
*/
void idec_get_v2_buf2_addr(IDE_ID id, UINT32 *pui_y_addr, UINT32 *pui_cb_addr, UINT32 *pui_cr_addr)
{
#if 0
	T_IDE_BUF_ADDR reg;
	T_IDE_BUF_ADDR2 reg2;

	reg.reg = idec_get_reg(id, IDE_V2_DB2_Y_OFS);
	*pui_y_addr = reg.bit.addr;

	reg2.reg = idec_get_reg(id, IDE_V2_DB2_UV_OFS);
	*pui_cb_addr = reg2.bit.addr;

	/*reg.reg = idec_get_reg(id, IDE_V2_DB2_CR_OFS);
	*pui_cr_addr = reg.bit.addr;*/
#endif
}

/**
    Get ide Video2 Buffer0 Odd Start

    Get ide Video2 Buffer0 Odd Start

    @param[in] id   ide ID
    @param[out] pb_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_get_v2_buf0_odd(IDE_ID id, BOOL *pb_odd)
{
	UINT32 reg;

	reg = idec_get_reg(id, IDE_V2_DB0_UV_ODD_OFS);
	*pb_odd = ((reg & 0x80000000) >> 31);
}

/*
    Get ide Video2 Buffer1 Odd Start

    Get ide Video2 Buffer1 Odd Start

    @param[in] id   ide ID
    @param[out] pb_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_get_v2_buf1_odd(IDE_ID id, BOOL *pb_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V2_DB1_UV_OFS);
	*pb_odd = reg.bit.vodd;
#endif
}

/*
    Get ide Video2 Buffer2 Odd Start

    Get ide Video2 Buffer2 Odd Start

    @param[in] id   ide ID
    @param[out] pb_odd FALSE:others TRUE:420 starts from odd line

    @return void
*/
void idec_get_v2_buf2_odd(IDE_ID id, BOOL *pb_odd)
{
#if 0
	T_IDE_BUF_ADDR2 reg;

	reg.reg = idec_get_reg(id, IDE_V2_DB2_UV_OFS);
	*pb_odd = reg.bit.vodd;
#endif
}

/**
    Get ide Video2 Buffer dimension.

    Get ide Video2 Buffer dimension.

    @param[in] id   ide ID
    @param[out] pui_bw  buffer width.
    @param[out] pui_bh  buffer height.
    @param[out] pui_lineoffset buffer lineoffset.

    @return void
*/
void idec_get_v2_buf_dim(IDE_ID id, UINT32 *pui_bw, UINT32 *pui_bh, UINT32 *pui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_VBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_V2_BUF_DIM_OFS);
	*pui_bw = reg_dim.bit.width;
	*pui_bh = reg_dim.bit.height;

	reg_attr.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);
	*pui_lineoffset = reg_attr.bit.lofs;
}

/**
    Get ide Video2 buffer operation

    Get ide Video2 buffer operation

    @param[in] id   ide ID
    @param[out] pui_bjmode Buffer mode
    @param[out] pui_optbuf Buffer in operation
    @param[out] pui_bufnum Operation buffer number

    @return void
*/
void idec_get_v2_buf_op(IDE_ID id, IDE_BJMODE *pui_bjmode, IDE_OP_BUF *pui_optbuf, IDE_BUF_NUM *pui_bufnum)
{
	/*    T_IDE_VBUF_ATTR reg;

	    reg.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);
	    *pui_bjmode = reg.bit.bjmode;
	    *pui_optbuf = reg.bit.op_buf;
	    *pui_bufnum = reg.bit.buf_num;*/
	*pui_bjmode = IDE_VIDEO_BJMODE_BUFFER_REPEAT;
	*pui_optbuf = IDE_VIDEO_BUFFER_OPT_0;
	*pui_bufnum = IDE_VIDEO_BUFFER_NUM_1;
}

/**
    Get ide Video2 buffer read order.

    Get ide Video2 buffer read order.

    @param[in] id   ide ID
    @param[out] pb_l2r Read order
    @param[out] pb_t2b Read order

    @return void
*/
void idec_get_v2_read_order(IDE_ID id, IDE_HOR_READ *pb_l2r, IDE_VER_READ *pb_t2b)
{
	T_IDE_VBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_V2_BUF_ATTR_OFS);
	*pb_l2r = reg.bit.l2r;
	*pb_t2b = reg.bit.t2b;
}

//@}

/**
@name ide Video Scale Control
*/
//@{

/**
    Set ide Video1 Scaling Control.

    Set ide Video1 Scaling Control.

    @param[in] id   ide ID
    @param[in] b_hscaleup  FALSE:horizontal scaling down, TRUE:horizontal scaling up.
    @param[in] b_vscaleup  FALSE:vertical scaling down, TRUE:vertical scaling up.

    @return void
*/
void idec_set_v1_scale_ctrl(IDE_ID id, BOOL b_hscaleup, BOOL b_vscaleup)
{
	T_IDE_SCALE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_SCALE_CTRL_OFS);

	if (b_hscaleup) {
		reg.bit.v1_hud = 1;
	} else {
		reg.bit.v1_hud = 0;
	}

	if (b_vscaleup) {
		reg.bit.v1_vud = 1;
	} else {
		reg.bit.v1_vud = 0;
	}

	idec_set_reg(id, IDE_SCALE_CTRL_OFS, reg.reg);
}

/**
    Set ide Video1 scaling factor

    Set ide Video1 scaling factor

    @param[in] id   ide ID
    @param[in] ui_hsf horizontal scale factor.
    @param[in] b_sub horizontal sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.
    @param[in] ui_vsf vertical scale factor.
    @param[in] b_vsub vertical sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.

    @return void
*/
void idec_set_v1_scale_factor(IDE_ID id, UINT32 ui_hsf, BOOL b_sub, UINT32 ui_vsf, BOOL b_vsub)
{
	T_IDE_VIDEO_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR0_OFS);

	reg.bit.hsf = ui_hsf;
	reg.bit.vsf = ui_vsf;

	if (b_sub) {
		reg.bit.sub = 1;
	} else {
		reg.bit.sub = 0;
	}

	if (b_vsub) {
		reg.bit.vsub = 1;
	} else {
		reg.bit.vsub = 0;
	}


	idec_set_reg(id, IDE_V1_WIN_ATTR0_OFS, reg.reg);
}

/**
    Set ide Video1 vsf init value

    Set ide Video1 vsf init value

    @param[in] id   ide ID
    @param[in] ui_init0 vsf init value 0
    @param[in] ui_init1 vsf init value 1

    @return void
*/
void idec_set_v1_vsf_init(IDE_ID id, UINT32 ui_init0, UINT32 ui_init1)
{
	T_IDE_VSF_INIT reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR3_OFS);

	reg.bit.vsf_init0 = ui_init0;
	reg.bit.vsf_init1 = ui_init1;

	idec_set_reg(id, IDE_V1_WIN_ATTR3_OFS, reg.reg);
}

/**
    Set ide Video2 Scaling Control.

    Set ide Video2 Scaling Control.

    @param[in] id   ide ID
    @param[in] b_hscaleup  FALSE:horizontal scaling down, TRUE:horizontal scaling up.
    @param[in] b_vscaleup  FALSE:vertical scaling down, TRUE:vertical scaling up.

    @return void
*/
void idec_set_v2_scale_ctrl(IDE_ID id, BOOL b_hscaleup, BOOL b_vscaleup)
{
	T_IDE_SCALE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_SCALE_CTRL_OFS);

	if (b_hscaleup) {
		reg.bit.v2_hud = 1;
	} else {
		reg.bit.v2_hud = 0;
	}

	if (b_vscaleup) {
		reg.bit.v2_vud = 1;
	} else {
		reg.bit.v2_vud = 0;
	}

	idec_set_reg(id, IDE_SCALE_CTRL_OFS, reg.reg);
}

/**
    Set ide Video2 scaling factor

    Set ide Video2 scaling factor

    @param[in] id   ide ID
    @param[in] ui_hsf horizontal scale factor.
    @param[in] b_sub horizontal sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.
    @param[in] ui_vsf vertical scale factor.
    @param[in] b_vsub vertical sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.

    @return void
*/
void idec_set_v2_scale_factor(IDE_ID id, UINT32 ui_hsf, BOOL b_sub, UINT32 ui_vsf, BOOL b_vsub)
{
	T_IDE_VIDEO_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR0_OFS);

	reg.bit.hsf = ui_hsf;
	reg.bit.vsf = ui_vsf;

	if (b_sub) {
		reg.bit.sub = 1;
	} else {
		reg.bit.sub = 0;
	}

	if (b_vsub) {
		reg.bit.vsub = 1;
	} else {
		reg.bit.vsub = 0;
	}


	idec_set_reg(id, IDE_V2_WIN_ATTR0_OFS, reg.reg);
}

/**
    Set ide Video2 vsf init value

    Set ide Video2 vsf init value

    @param[in] id   ide ID
    @param[in] ui_init0 vsf init value 0
    @param[in] ui_init1 vsf init value 1

    @return void
*/
void idec_set_v2_vsf_init(IDE_ID id, UINT32 ui_init0, UINT32 ui_init1)
{
	T_IDE_VSF_INIT reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR3_OFS);

	reg.bit.vsf_init0 = ui_init0;
	reg.bit.vsf_init1 = ui_init1;

	idec_set_reg(id, IDE_V2_WIN_ATTR3_OFS, reg.reg);
}

/**
    Get ide Video1 scaling factor

    Get ide Video1 scaling factor

    @param[in] id   ide ID
    @param[out] ui_hsf horizontal scale factor.
    @param[out] b_sub horizontal sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.
    @param[out] ui_vsf vertical scale factor.
    @param[out] b_vsub vertical sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.

    @return void
*/
void idec_get_v1_scale_factor(IDE_ID id, UINT32 *ui_hsf, BOOL *b_sub, UINT32 *ui_vsf, BOOL *b_vsub)
{
	T_IDE_VIDEO_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR0_OFS);

	*ui_hsf = reg.bit.hsf;
	*ui_vsf = reg.bit.vsf;
	*b_sub = reg.bit.sub;
	*b_vsub = reg.bit.vsub;
}

/**
    Get ide Video2 scaling factor

    Get ide Video2 scaling factor

    @param[in] id   ide ID
    @param[out] ui_hsf horizontal scale factor.
    @param[out] b_sub horizontal sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.
    @param[out] ui_vsf vertical scale factor.
    @param[out] b_vsub vertical sub-sample option,FALSE:no sub-sample, TRUE:sub-sample.

    @return void
*/
void idec_get_v2_scale_factor(IDE_ID id, UINT32 *ui_hsf, BOOL *b_sub, UINT32 *ui_vsf, BOOL *b_vsub)
{
	T_IDE_VIDEO_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR0_OFS);

	*ui_hsf = reg.bit.hsf;
	*ui_vsf = reg.bit.vsf;
	*b_sub = reg.bit.sub;
	*b_vsub = reg.bit.vsub;
}

//@}

/**
@name ide Video Window
*/
//@{

/**
    Set ide Video1 horizontal scaling method

    Set ide Video1 horizontal scaling method

    @param[in] id   ide ID
    @param[in] hsm  horizontal scaling method,0:duplicate/drop, 1:bilinear.

    @return void
*/
void idec_set_v1_hsm(IDE_ID id, IDE_SCALE_METHOD hsm)
{
	T_IDE_VIDEO_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR0_OFS);

	if (hsm) {
		reg.bit.hsm = 1;
	} else {
		reg.bit.hsm = 0;
	}

	idec_set_reg(id, IDE_V1_WIN_ATTR0_OFS, reg.reg);
}

/*
    Set ide Video1 vertical scaling method

    Set ide Video1 vertical scaling method

    @param[in] id   ide ID
    @param[in] vsm vertical scaling method,0:duplicate/drop, 1:bilinear.

    @return void
*/
/*void idec_setV1Vsm(IDE_ID id, IDE_SCALE_METHOD vsm)
{
    T_IDE_VIDEO_WIN_ATTR0 reg;

    reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR0_OFS);

    if(vsm)
		reg.bit.vsm = 1;
    else
		reg.bit.vsm = 0;

    idec_set_reg(id, IDE_V1_WIN_ATTR0_OFS, reg.reg);
}*/


/**
    Set ide Video1 window dimension.

    Set ide Video1 window dimension.

    @param[in] id   ide ID
    @param[in] ui_win_w window width.
    @param[in] ui_win_h window height.

    @return void
*/
void idec_set_v1_win_dim(IDE_ID id, UINT32 ui_win_w, UINT32 ui_win_h)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR1_OFS);

	reg.bit.width = ui_win_w;
	reg.bit.height = ui_win_h;

	idec_set_reg(id, IDE_V1_WIN_ATTR1_OFS, reg.reg);
}

/**
    Set ide Video1 window position

    Set ide Video1 window position

    @param[in] id   ide ID
    @param[in] ui_x The start position at X-axis.
    @param[in] ui_y The start position at Y-axis.

    @return void
*/
void idec_set_v1_win_pos(IDE_ID id, UINT32 ui_x, UINT32 ui_y)
{
	T_IDE_VIDEO_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR2_OFS);

	reg.bit.x = ui_x;
	reg.bit.y = ui_y;

	idec_set_reg(id, IDE_V1_WIN_ATTR2_OFS, reg.reg);
}

/**
    Set ide Video1 window format.

    Set ide Video1 window format.

    @param[in] id   ide ID
    @param[in] ui_fmt: window data format
		- COLOR_YCBCR444: YCbCr 444
		- COLOR_YCBCR422: YCbCr 422
		- COLOR_YCBCR420: YCbCr 420
		- COLOR_YCC422P : YCbCr 422P
		- COLOR_YCC420P : YCbCr 420P

    @return void
*/
void idec_set_v1_fmt(IDE_ID id, IDE_COLOR_FORMAT ui_fmt)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	if (ui_fmt != COLOR_YCBCR444 &&
		ui_fmt != COLOR_YCBCR422 &&
		ui_fmt != COLOR_YCBCR420 &&
		ui_fmt != COLOR_YCC422P &&
		ui_fmt != COLOR_YCC420P) {
		DBG_ERR("Unsupported Video1 format %d\r\n", (int)ui_fmt);
		return;
	}

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR1_OFS);

	reg.bit.fmt = ui_fmt;

	idec_set_reg(id, IDE_V1_WIN_ATTR1_OFS, reg.reg);
}

/**
    Set ide Video2 horizontal scaling method

    Set ide Video2 horizontal scaling method

    @param[in] id   ide ID
    @param[in] hsm  horizontal scaling method,0:duplicate/drop, 1:bilinear.

    @return void
*/
void idec_set_v2_hsm(IDE_ID id, IDE_SCALE_METHOD hsm)
{
	T_IDE_VIDEO_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR0_OFS);

	if (hsm) {
		reg.bit.hsm = 1;
	} else {
		reg.bit.hsm = 0;
	}

	idec_set_reg(id, IDE_V2_WIN_ATTR0_OFS, reg.reg);
}

/*
    Set ide Video2 vertical scaling method

    Set ide Video2 vertical  scaling method

    @param[in] id   ide ID
    @param[in] vsm vertical  scaling method,0:duplicate/drop, 1:bilinear.

    @return void
*/
/*void idec_setV2Vsm(IDE_ID id, IDE_SCALE_METHOD vsm)
{
    T_IDE_VIDEO_WIN_ATTR0 reg;

    reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR0_OFS);

    if(vsm)
		reg.bit.vsm = 1;
    else
		reg.bit.vsm = 0;

    idec_set_reg(id, IDE_V2_WIN_ATTR0_OFS, reg.reg);
}*/


/**
    Set ide Video2 window dimension.

    Set ide Video2 window dimension.

    @param[in] id   ide ID
    @param[in] ui_win_w window width.
    @param[in] ui_win_h window height.

    @return void
*/
void idec_set_v2_win_dim(IDE_ID id, UINT32 ui_win_w, UINT32 ui_win_h)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR1_OFS);

	reg.bit.width = ui_win_w;
	reg.bit.height = ui_win_h;

	idec_set_reg(id, IDE_V2_WIN_ATTR1_OFS, reg.reg);
}

/**
    Set ide Video2 window position

    Set ide Video2 window position

    @param[in] id   ide ID
    @param[in] ui_x The start position at X-axis.
    @param[in] ui_y The start position at Y-axis.

    @return void
*/
void idec_set_v2_win_pos(IDE_ID id, UINT32 ui_x, UINT32 ui_y)
{
	T_IDE_VIDEO_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR2_OFS);

	reg.bit.x = ui_x;
	reg.bit.y = ui_y;

	idec_set_reg(id, IDE_V2_WIN_ATTR2_OFS, reg.reg);
}

/**
    Set ide Video2 window format.

    Set ide Video2 window format.

    @param[in] id   ide ID
    @param[in] ui_fmt: window data format
		- COLOR_YCBCR444: YCbCr 444
		- COLOR_YCBCR422: YCbCr 422
		- COLOR_YCBCR420: YCbCr 420
		- COLOR_ARGB4565: ARGB 4565
		- COLOR_ARGB8565: ARGB 8565
		- COLOR_YCC422P : YCbCr 422P
		- COLOR_YCC420P : YCbCr 420P

    @return void
*/
void idec_set_v2_fmt(IDE_ID id, IDE_COLOR_FORMAT ui_fmt)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	if (ui_fmt != COLOR_YCBCR444 &&
		ui_fmt != COLOR_YCBCR422 &&
		ui_fmt != COLOR_YCBCR420 &&
		ui_fmt != COLOR_YCC422P &&
		ui_fmt != COLOR_YCC420P &&
		ui_fmt != COLOR_ARGB4565 &&
		ui_fmt != COLOR_ARGB8565) {
		DBG_ERR("Unsupported Video1 format %d\r\n", (int)ui_fmt);
		return;
	}

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR1_OFS);

	reg.bit.fmt = ui_fmt;

	idec_set_reg(id, IDE_V2_WIN_ATTR1_OFS, reg.reg);
}

/**
    Get ide Video1 window dimension.

    Get ide Video1 window dimension.

    @param[in] id   ide ID
    @param[out] ui_win_w Window Width
    @param[out] ui_win_h Window Height

    @return void
*/
void idec_get_v1_win_dim(IDE_ID id, UINT32 *ui_win_w, UINT32 *ui_win_h)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR1_OFS);
	*ui_win_w = reg.bit.width;
	*ui_win_h = reg.bit.height;
}

/**
    Set ide Video1 window position

    Set ide Video1 window position

    @param[in] id   ide ID
    @param[out] ui_x     The start position at X-axis.
    @param[out] ui_y     The start position at Y-axis.

    @return void
*/
void idec_get_v1_win_pos(IDE_ID id, UINT32 *ui_x, UINT32 *ui_y)
{
	T_IDE_VIDEO_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR2_OFS);

	*ui_x = reg.bit.x;
	*ui_y = reg.bit.y;
}

/**
    Get ide Video1 window format.

    Get ide Video1 window format.

    @param[in] id   ide ID
    @param[out]  ui_fmt  window data format

    @return void
*/
void idec_get_v1_fmt(IDE_ID id, IDE_COLOR_FORMAT *ui_fmt)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_V1_WIN_ATTR1_OFS);

	*ui_fmt = reg.bit.fmt;
}

/**
    Get ide Video2 window dimension.

    Get ide Video2 window dimension.

    @param[in] id   ide ID
    @param[in] ui_win_w Window Width
    @param[in] ui_win_h Window Height

    @return void
*/
void idec_get_v2_win_dim(IDE_ID id, UINT32 *ui_win_w, UINT32 *ui_win_h)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR1_OFS);
	*ui_win_w = reg.bit.width;
	*ui_win_h = reg.bit.height;
}

/**
    Set ide Video2 window position

    Set ide Video2 window position

    @param[in] id   ide ID
    @param[in] ui_x The start position at X-axis.
    @param[in] ui_y The start position at Y-axis.

    @return void
*/
void idec_get_v2_win_pos(IDE_ID id, UINT32 *ui_x, UINT32 *ui_y)
{
	T_IDE_VIDEO_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR2_OFS);

	*ui_x = reg.bit.x;
	*ui_y = reg.bit.y;
}

/**
    Get ide Video2 window format.

    Get ide Video2 window format.

    @param[in] id   ide ID
    @param[out] ui_fmt: window data format

    @return void
*/
void idec_get_v2_fmt(IDE_ID id, IDE_COLOR_FORMAT *ui_fmt)
{
	T_IDE_VIDEO_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_V2_WIN_ATTR1_OFS);

	*ui_fmt = reg.bit.fmt;
}

//@}

/**
@name ide Video color Key and Alpha Blending
*/
//@{

/**
    Set ide Video color key

    Set ide Video color key

    @param[in] id   ide ID
    @param[in] ui_ck_y color key Y.
    @param[in] ui_ck_cb color key CB.
    @param[in] ui_ck_cr color key CR.

    @return void
*/
void idec_set_video_colorkey(IDE_ID id, UINT8 ui_ck_y, UINT8 ui_ck_cb, UINT8 ui_ck_cr)
{
	T_IDE_VIDEO_OPT reg;

	reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);
	reg.bit.ck_y = ui_ck_y;
	reg.bit.ck_cb = ui_ck_cb;
	reg.bit.ck_cr = ui_ck_cr;

	idec_set_reg(id, IDE_VIDEO_OPT_OFS, reg.reg);
}

/*
    Set ide video1 and video2 blending operation.

    @param[in] id   ide ID
    @param[in] uiOp the blending operation for video1 and video2.
		- IDE_VIDEO_BLEND_VIDEOCK:videock
		- IDE_VIDEO_BLEND_VIDEOCK7_8:videock*7/8 + video1(2)/8
		- IDE_VIDEO_BLEND_VIDEOCK3_4:videock*3/4 + video1(2)/4
		- IDE_VIDEO_BLEND_VIDEOCK1_2:videock/2   + video1(2)/2
		- IDE_VIDEO_BLEND_VIDEOCK1_4:videock/4   + video1(2)*3/4
		- IDE_VIDEO_BLEND_VIDEOCK1_8:videock/8   + video1(2)*7/8
		- IDE_VIDEO_BLEND_VIDEO1OR2:video1(2)

    @return void
*/
/*
void idec_setVideoBlendOp(IDE_ID id, IDE_VIDEO_BLEND_OP uiOp)
{
    T_IDE_VIDEO_OPT reg;

    ide_chk_range(uiOp, IDE_VIDEO_BLEND_VIDEOCK, IDE_VIDEO_BLEND_VIDEO1OR2);

    reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);
    reg.bit.vopt = uiOp;
    idec_set_reg(id, IDE_VIDEO_OPT_OFS, reg.reg);
}
*/

/**
    Set ide video1 and video2 color key selection.

    Set ide video1 and video2 color key selection.

    @param[in] id   ide ID
    @param[in] b_sel IDE_VIDEO_COLORKEY_COMPAREVIDEO2 or IDE_VIDEO_COLORKEY_COMPAREVIDEO1

    @return void
*/
void idec_set_video_colorkey_sel(IDE_ID id, IDE_VIDEO_COLORKEY_SEL b_sel)
{
	T_IDE_VIDEO_OPT reg;

	reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);

	if (b_sel) {
		reg.bit.ck_sel = 1;
	} else {
		reg.bit.ck_sel = 0;
	}

	idec_set_reg(id, IDE_VIDEO_OPT_OFS, reg.reg);
}

/**
    Set ide video1 and video2 color key operation.

    Set ide video1 and video2 color key operation.

    @param[in] id   ide ID
    @param[in] ui_ck_op the colorkey operation for video.
		- if (VIDEOCK_SEL=0)
		- IDE_VIDEO_COLORKEY_VIDEO1OR2: videock = video1;
		- IDE_VIDEO_COLORKEY_YSMALLKEY: videock = (video2_Y < VDO_YKEY) ? video1 : video2;
		- IDE_VIDEO_COLORKEY_YEQUKEY: videock = (video2_Y == VDO_KEY && video2_CB == VDO_CBKEY && video2_CR == VDO_CRKEY) ? video1 : video2;
		- IDE_VIDEO_COLORKEY_YBIGKEY: videock = (video2_Y > VDO_YKEY) ? video1 : video2;
		- else if (VIDEOCK_SEL=0)
		- IDE_VIDEO_COLORKEY_VIDEO1OR2: videock = video1;
		- IDE_VIDEO_COLORKEY_YSMALLKEY: videock = (video1_Y < VDO_YKEY) ? video2 : video1;
		- IDE_VIDEO_COLORKEY_YEQUKEY: videock = (video1_Y == VDO_KEY && video1_CB == VDO_CBKEY && video1_CR == VDO_CRKEY) ? video2 : video1;
		- IDE_VIDEO_COLORKEY_YBIGKEY: videock = (video1_Y > VDO_YKEY) ? video2 : video1;

    @return void
*/
void idec_set_video_colorkey_op(IDE_ID id, IDE_VIDEO_COLORKEY_OP ui_ck_op)
{
	T_IDE_VIDEO_OPT reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_ck_op, IDE_VIDEO_COLORKEY_VIDEO1OR2, IDE_VIDEO_COLORKEY_YBIGKEY);

	reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);

	reg.bit.vopt2 = ui_ck_op;

	idec_set_reg(id, IDE_VIDEO_OPT_OFS, reg.reg);
}

/**
    Get ide Video color key

    Get ide Video color key

    @param[in] id   ide ID
    @param[out] ui_ck_y color key Y.
    @param[out] ui_ck_cb color key CB.
    @param[out] ui_ck_cr color key CR.

    @return void
*/
void idec_get_video_colorkey(IDE_ID id, UINT8 *ui_ck_y, UINT8 *ui_ck_cb, UINT8 *ui_ck_cr)
{
	T_IDE_VIDEO_OPT reg;

	reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);

	*ui_ck_y = reg.bit.ck_y;
	*ui_ck_cb = reg.bit.ck_cb;
	*ui_ck_cr = reg.bit.ck_cr;
}

/*
    Get ide video1 and video2 blending operation.

    @param[in] id   ide ID
    @param[out] uiOp the blending operation for video1 and video2.
		- IDE_VIDEO_BLEND_VIDEOCK:videock
		- IDE_VIDEO_BLEND_VIDEOCK7_8:videock*7/8 + video1(2)/8
		- IDE_VIDEO_BLEND_VIDEOCK3_4:videock*3/4 + video1(2)/4
		- IDE_VIDEO_BLEND_VIDEOCK1_2:videock/2   + video1(2)/2
		- IDE_VIDEO_BLEND_VIDEOCK1_4:videock/4   + video1(2)*3/4
		- IDE_VIDEO_BLEND_VIDEOCK1_8:videock/8   + video1(2)*7/8
		- IDE_VIDEO_BLEND_VIDEO1OR2:video1(2)

    @return void
*/
/*
void idec_getVideoBlendOp(IDE_ID id, IDE_VIDEO_BLEND_OP *uiOp)
{
    T_IDE_VIDEO_OPT reg;

    reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);

    *uiOp = reg.bit.vopt;
}
*/

/**
    Get ide video1 and video2 color key selection.

    Get ide video1 and video2 color key selection.

    @param[in] id   ide ID
    @param[out] b_sel IDE_VIDEO_COLORKEY_COMPAREVIDEO2 or IDE_VIDEO_COLORKEY_COMPAREVIDEO1

    @return void
*/
void idec_get_video_colorkey_sel(IDE_ID id, IDE_VIDEO_COLORKEY_SEL *b_sel)
{
	T_IDE_VIDEO_OPT reg;

	reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);

	*b_sel = reg.bit.ck_sel;
}

/**
    Get ide video1 and video2 color key operation.

    Get ide video1 and video2 color key operation.

    @param[in] id   ide ID
    @param[out] ui_ck_op the colorkey operation for video.
		- if (VIDEOCK_SEL=0)
		- IDE_VIDEO_COLORKEY_VIDEO1OR2: videock = video1;
		- IDE_VIDEO_COLORKEY_YSMALLKEY: videock = (video2_Y < VDO_YKEY) ? video1 : video2;
		- IDE_VIDEO_COLORKEY_YEQUKEY: videock = (video2_Y == VDO_KEY && video2_CB == VDO_CBKEY && video2_CR == VDO_CRKEY) ? video1 : video2;
		- IDE_VIDEO_COLORKEY_YBIGKEY: videock = (video2_Y > VDO_YKEY) ? video1 : video2;
		- else if (VIDEOCK_SEL=0)
		- IDE_VIDEO_COLORKEY_VIDEO1OR2: videock = video1;
		- IDE_VIDEO_COLORKEY_YSMALLKEY: videock = (video1_Y < VDO_YKEY) ? video2 : video1;
		- IDE_VIDEO_COLORKEY_YEQUKEY: videock = (video1_Y == VDO_KEY && video1_CB == VDO_CBKEY && video1_CR == VDO_CRKEY) ? video2 : video1;
		- IDE_VIDEO_COLORKEY_YBIGKEY: videock = (video1_Y > VDO_YKEY) ? video2 : video1;

    @return void
*/
void idec_get_video_colorkey_op(IDE_ID id, IDE_VIDEO_COLORKEY_OP *ui_ck_op)
{
	T_IDE_VIDEO_OPT reg;

	reg.reg = idec_get_reg(id, IDE_VIDEO_OPT_OFS);

	*ui_ck_op = reg.bit.vopt2;
}
/**
    Set ide video1 LowPass Enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable
		- @b TRUE:enable

    @return void
*/
void idec_set_v1_lowpass_en(IDE_ID id, BOOL b_en)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V1_LOWPASS_OFS);
	reg.bit.lp_en = b_en;
	idec_set_reg(id, IDE_V1_LOWPASS_OFS, reg.reg);
}
/**
    Get ide video1 LowPass Enable

    @param[in] id   ide ID
    @return
		- @b FALSE:disable
		- @b TRUE:enable
*/
BOOL idec_get_v1_lowpass_en(IDE_ID id)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V1_LOWPASS_OFS);
	return reg.bit.lp_en;
}
/**
    Set ide video1 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_set_v1_lowpass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V1_LOWPASS_OFS);
	reg.bit.lp_tap0 = ui_pcoef[0];
	reg.bit.lp_tap1 = ui_pcoef[1];
	reg.bit.lp_tap2 = ui_pcoef[2];
	idec_set_reg(id, IDE_V1_LOWPASS_OFS, reg.reg);
}
/**
    Get ide video1 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_get_v1_lowpass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V1_LOWPASS_OFS);
	ui_pcoef[0] = reg.bit.lp_tap0;
	ui_pcoef[1] = reg.bit.lp_tap1;
	ui_pcoef[2] = reg.bit.lp_tap2;
}



//@}

/**
    @name ide Video Auto Blinking
*/
//@{

/**
    Set ide Video1 Auto Blinking Setting

    Set ide Video1 Auto Blinking Setting

    @param[in] id   ide ID
    @param[in] b_ovr FALSE: disable, TRUE: enable auto blinking in over-exposure area
    @param[in] b_und FALSE: disable, TRUE: enable auto blinking in under-exposure area
    @param[in] b_sel FALSE: compare to threshold Y only, TRUE: compare to Y/Cb/Cr threshold

    @return void
*/
void idec_set_v1_blink(IDE_ID id, BOOL b_ovr, BOOL b_und, BOOL b_sel)
{
	T_IDE_BLINK reg;

	reg.reg = idec_get_reg(id, IDE_V1_BLINK_OFS);

	if (b_ovr) {
		reg.bit.ovrexp_en = 1;
	} else {
		reg.bit.ovrexp_en = 0;
	}

	if (b_und) {
		reg.bit.undexp_en = 1;
	} else {
		reg.bit.undexp_en = 0;
	}

	if (b_sel) {
		reg.bit.blink_sel = 1;
	} else {
		reg.bit.blink_sel = 0;
	}

	idec_set_reg(id, IDE_V1_BLINK_OFS, reg.reg);
}

/**
    Set ide Video1 Auto Blinking Setting

    Set ide Video1 Auto Blinking Setting

    @param[in] id   ide ID
    @param[in] ui_cnt auto blinking frame count

    @return void
*/
void idec_set_v1_count(IDE_ID id, UINT8 ui_cnt)
{
	T_IDE_BLINK reg;

	reg.reg = idec_get_reg(id, IDE_V1_BLINK_OFS);

	reg.bit.cnt_set = 1;
	reg.bit.blink_cnt = ui_cnt;

	idec_set_reg(id, IDE_V1_BLINK_OFS, reg.reg);
}

/**
    Get ide Video1 Auto Blinking Setting

    Get ide Video1 Auto Blinking Setting

    @param[in] id   ide ID
    @return Auto blinking frame count
*/
UINT8 idec_get_v1_count(IDE_ID id)
{
	T_IDE_BLINK reg;

	reg.reg = idec_get_reg(id, IDE_V1_BLINK_OFS);

	return reg.bit.blink_cnt;
}

/**
    Set ide Video1 Over-exposure Threshold

    Set ide Video1 Over-exposure Threshold

    @param[in] id   ide ID
    @param[in] ui_y  threshold Y
    @param[in] ui_cb threshold Cb
    @param[in] ui_cr threshold Cr

    @return void
*/
void idec_set_v1_ovrexp_threshold(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_OVREXP_TH_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V1_OVREXP_TH_OFS, reg.reg);
}

/**
    Set ide Video1 Over-exposure color

    Set ide Video1 Over-exposure color

    @param[in] id   ide ID
    @param[in] ui_y  blinking color Y
    @param[in] ui_cb blinking color Cb
    @param[in] ui_cr blinking color Cr

    @return void
*/
void idec_set_v1_ovrexp_color(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_OVREXP_COLOR_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V1_OVREXP_COLOR_OFS, reg.reg);
}

/**
    Get ide Video1 Over-exposure color

    Get ide Video1 Over-exposure color

    @param[in] id   ide ID
    @param[out] ui_y  blinking color Y
    @param[out] ui_cb blinking color Cb
    @param[out] ui_cr blinking color Cr

    @return void
*/
void idec_get_v1_ovrexp_color(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_OVREXP_COLOR_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Set ide Video1 Under-exposure Threshold

    Set ide Video1 Under-exposure Threshold

    @param[in] id   ide ID
    @param[in] ui_y  threshold Y
    @param[in] ui_cb threshold Cb
    @param[in] ui_cr threshold Cr

    @return void
*/
void idec_set_v1_undexp_threshold(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_UNDEXP_TH_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V1_UNDEXP_TH_OFS, reg.reg);
}

/**
    Set ide Video1 Under-exposure Threshold

    Set ide Video1 Under-exposure Threshold

    @param[in] id   ide ID
    @param[in] ui_y  blinking color Y
    @param[in] ui_cb blinking color Cb
    @param[in] ui_cr blinking color Cr

    @return void
*/
void idec_set_v1_undexp_color(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_UNDEXP_COLOR_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V1_UNDEXP_COLOR_OFS, reg.reg);
}

/**
    Get ide Video1 Under-exposure color

    Get ide Video1 Under-exposure color

    @param[in] id   ide ID
    @param[out] ui_y  blinking color Y
    @param[out] ui_cb blinking color Cb
    @param[out] ui_cr blinking color Cr

    @return void
*/
void idec_get_v1_undexp_color(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_UNDEXP_COLOR_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Set ide Video2 Auto Blinking Setting

    Set ide Video2 Auto Blinking Setting

    @param[in] id   ide ID
    @param[in] b_ovr 0: disable 1: enable auto blinking in over-exposure area
    @param[in] b_und 0: disable 1: enable auto blinking in under-exposure area
    @param[in] b_sel 0: compare to threshold Y only 1: compare to Y/Cb/Cr threshold

    @return void
*/
void idec_set_v2_blink(IDE_ID id, BOOL b_ovr, BOOL b_und, BOOL b_sel)
{
	T_IDE_BLINK reg;

	reg.reg = idec_get_reg(id, IDE_V2_BLINK_OFS);

	if (b_ovr) {
		reg.bit.ovrexp_en = 1;
	} else {
		reg.bit.ovrexp_en = 0;
	}

	if (b_und) {
		reg.bit.undexp_en = 1;
	} else {
		reg.bit.undexp_en = 0;
	}

	if (b_sel) {
		reg.bit.blink_sel = 1;
	} else {
		reg.bit.blink_sel = 0;
	}

	idec_set_reg(id, IDE_V2_BLINK_OFS, reg.reg);
}

/**
    Set ide Video2 Auto Blinking Setting

    Set ide Video2 Auto Blinking Setting

    @param[in] id   ide ID
    @param[in] ui_cnt    Auto blinking frame count

    @return void
*/
void idec_set_v2_count(IDE_ID id, UINT8 ui_cnt)
{
	T_IDE_BLINK reg;

	reg.reg = idec_get_reg(id, IDE_V2_BLINK_OFS);

	reg.bit.cnt_set = 1;
	reg.bit.blink_cnt = ui_cnt;

	idec_set_reg(id, IDE_V2_BLINK_OFS, reg.reg);
}

/**
    Get ide Video2 Auto Blinking Setting

    Get ide Video2 Auto Blinking Setting

    @param[in] id   ide ID
    @return Auto blinking frame count
*/
UINT8 idec_get_v2_count(IDE_ID id)
{
	T_IDE_BLINK reg;

	reg.reg = idec_get_reg(id, IDE_V2_BLINK_OFS);

	return reg.bit.blink_cnt;
}

/**
    Set ide Video2 Over-exposure Threshold

    Set ide Video2 Over-exposure Threshold

    @param[in] id   ide ID
    @param[in] ui_y  threshold Y
    @param[in] ui_cb threshold Cb
    @param[in] ui_cr threshold Cr

    @return void
*/
void idec_set_v2_ovrexp_threshold(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_OVREXP_TH_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V2_OVREXP_TH_OFS, reg.reg);
}

/**
    Set ide Video2 Over-exposure color

    Set ide Video2 Over-exposure color

    @param[in] id   ide ID
    @param[in] ui_y  blinking color Y
    @param[in] ui_cb blinking color Cb
    @param[in] ui_cr blinking color Cr

    @return void
*/
void idec_set_v2_ovrexp_color(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_OVREXP_COLOR_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V2_OVREXP_COLOR_OFS, reg.reg);
}

/**
    Get ide Video2 Over-exposure color

    Get ide Video2 Over-exposure color

    @param[in] id   ide ID
    @param[out] ui_y  blinking color Y
    @param[out] ui_cb blinking color Cb
    @param[out] ui_cr blinking color Cr

    @return void
*/
void idec_get_v2_ovrexp_color(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_OVREXP_COLOR_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Set ide Video2 Under-exposure Threshold

    Set ide Video2 Under-exposure Threshold

    @param[in] id   ide ID
    @param[in] ui_y  threshold Y
    @param[in] ui_cb threshold Cb
    @param[in] ui_cr threshold Cr

    @return void
*/
void idec_set_v2_undexp_threshold(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_UNDEXP_TH_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V2_UNDEXP_TH_OFS, reg.reg);
}

/**
    Set ide Video2 Under-exposure color

    Set ide Video2 Under-exposure color

    @param[in] id   ide ID
    @param[in] ui_y  blinking color Y
    @param[in] ui_cb blinking color Cb
    @param[in] ui_cr blinking color Cr

    @return void
*/
void idec_set_v2_undexp_color(IDE_ID id, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_UNDEXP_COLOR_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_V2_UNDEXP_COLOR_OFS, reg.reg);
}

/**
    Get ide Video2 Under-exposure color

    Get ide Video2 Under-exposure color

    @param[in] id   ide ID
    @param[out] ui_y  blinking color Y
    @param[out] ui_cb blinking color Cb
    @param[out] ui_cr blinking color Cr

    @return void
*/
void idec_get_v2_undexp_color(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_UNDEXP_COLOR_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Get ide Video1 Over-exposure Threshold

    Get ide Video1 Over-exposure Threshold

    @param[in] id   ide ID
    @param[out] ui_y  threshold Y
    @param[out] ui_cb threshold Cb
    @param[out] ui_cr threshold Cr

    @return void
*/
void idec_get_v1_ovrexp_threshold(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_OVREXP_TH_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Get ide Video2 Over-exposure Threshold

    Get ide Video2 Over-exposure Threshold

    @param[in] id   ide ID
    @param[out] ui_y  threshold Y
    @param[out] ui_cb threshold Cb
    @param[out] ui_cr threshold Cr

    @return void
*/
void idec_get_v2_ovrexp_threshold(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_OVREXP_TH_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}


/**
    Get ide Video1 Under-exposure Threshold

    Get ide Video1 Under-exposure Threshold

    @param[in] id   ide ID
    @param[out] ui_y  threshold Y
    @param[out] ui_cb threshold Cb
    @param[out] ui_cr threshold Cr

    @return void
*/
void idec_get_v1_undexp_threshold(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V1_UNDEXP_TH_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Get ide Video2 Under-exposure Threshold

    Get ide Video2 Under-exposure Threshold

    @param[in] id   ide ID
    @param[out] ui_y  threshold Y
    @param[out] ui_cb threshold Cb
    @param[out] ui_cr threshold Cr

    @return void
*/
void idec_get_v2_undexp_threshold(IDE_ID id, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_EXP_COLOR reg;

	reg.reg = idec_get_reg(id, IDE_V2_UNDEXP_TH_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

//@}

/**
    Set ide video2 LowPass Enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable
		- @b TRUE:enable

    @return void
*/
void idec_set_v2_lowpass_en(IDE_ID id, BOOL b_en)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V2_LOWPASS_OFS);
	reg.bit.lp_en = b_en;
	idec_set_reg(id, IDE_V2_LOWPASS_OFS, reg.reg);
}
/**
    Get ide video1 LowPass Enable

    @param[in] id   ide ID
    @return
		- @b FALSE:disable
		- @b TRUE:enable
*/
BOOL idec_get_v2_lowpass_en(IDE_ID id)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V2_LOWPASS_OFS);
	return reg.bit.lp_en;
}
/**
    Set ide video2 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_set_v2_lowpass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V2_LOWPASS_OFS);
	reg.bit.lp_tap0 = ui_pcoef[0];
	reg.bit.lp_tap1 = ui_pcoef[1];
	reg.bit.lp_tap2 = ui_pcoef[2];
	idec_set_reg(id, IDE_V2_LOWPASS_OFS, reg.reg);
}
/**
    Get ide video2 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_get_v2_lowpass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_V2_LOWPASS_OFS);
	ui_pcoef[0] = reg.bit.lp_tap0;
	ui_pcoef[1] = reg.bit.lp_tap1;
	ui_pcoef[2] = reg.bit.lp_tap2;
}

//@}

// Non-public APIs

/*
    Set Video1 burst length

    Set Video1 burst length

    @param[in] id   ide ID
    @param[in] chy  channel y burst length
    @param[in] chc  channel c burst length

    @return
		- @b FALSE:something error
		- @b TRUE:ok
*/
BOOL idec_set_v1_burst_len(IDE_ID id, IDE_DMA_BURST_LEN chy, IDE_DMA_BURST_LEN chc)
{
	T_IDE_DMA_LEN reg;

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);
	if ((chy > IDE_DMA_BURST_LEN_64) || (chc > IDE_DMA_BURST_LEN_64)) {
		DBG_WRN("ide: V1 DMA burst larger than 64/64\r\n");
		return FALSE;
	}
	reg.bit.v1_y_len = chy;
	reg.bit.v1_c_len = chc;
	idec_set_reg(id, IDE_DMA_LEN_OFS, reg.reg);

	return TRUE;
}

/*
    Get Video1 burst length

    Get Video1 burst length

    @param[in] id   ide ID
    @param[out] chy  channel y burst length
    @param[out] chc  channel c burst length

    @return
		- @b TRUE:ok
*/
BOOL idec_get_v1_burst_len(IDE_ID id, IDE_DMA_BURST_LEN *chy, IDE_DMA_BURST_LEN *chc)
{
	T_IDE_DMA_LEN reg;

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);

	*chy = reg.bit.v1_y_len;
	*chc = reg.bit.v1_c_len;

	return TRUE;
}

/*
    Set Video2 burst length

    Set Video2 burst length

    @param[in] id   ide ID
    @param[in] chy  channel y burst length
    @param[in] chc  channel c burst length

    @return
		- @b FALSE:something error
		- @b TRUE:ok
*/
BOOL idec_set_v2_burst_len(IDE_ID id, IDE_DMA_BURST_LEN chy, IDE_DMA_BURST_LEN chc)
{
	T_IDE_DMA_LEN reg;

	if (id > IDE_ID_1) {
		return FALSE;
	}

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);
	if ((chy > IDE_DMA_BURST_LEN_64) || (chc > IDE_DMA_BURST_LEN_64)) {
		DBG_WRN("ide: V2 DMA burst larger than 64/64\r\n");
		return FALSE;
	}
	reg.bit.v2_y_len = chy;
	reg.bit.v2_c_len = chc;
	idec_set_reg(id, IDE_DMA_LEN_OFS, reg.reg);

	return TRUE;
}

/*
    Get Video2 burst length

    Get Video2 burst length

    @param[in] id   ide ID
    @param[out] chy  channel y burst length
    @param[out] chc  channel c burst length

    @return
		- @b TRUE:ok
*/
BOOL idec_get_v2_burst_len(IDE_ID id, IDE_DMA_BURST_LEN *chy, IDE_DMA_BURST_LEN *chc)
{
	T_IDE_DMA_LEN reg;

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);

	*chy = reg.bit.v2_y_len;
	*chc = reg.bit.v2_c_len;

	return TRUE;
}
