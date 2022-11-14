/*
    Library for ide OSD1/2 regiseter control

    This is low level control library for ide OSD1/2.

    @file       ide_osd.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
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
@name ide OSD Control
*/
//@{

/**
    Enable/Disable OSD1.

    @param[in] id   ide ID
    @param[in] b_en enable/disable for the specific window
		- @b TRUE:Enable
		- @b FALSE:Disable.

    @return void
*/
void idec_set_o1_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.o1_en = 1;
	} else {
		reg.bit.o1_en = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

#if 0
/*
    Enable/Disable OSD2.

    @param[in] id   ide ID
    @param[in] b_en enable/disable for the specific window
		- @b TRUE:Enable
		- @b FALSE:Disable.

    @return void
*/
void idec_set_o2_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.o2_en = 1;
	} else {
		reg.bit.o2_en = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}
#endif

/**
    OSD1 window status.

    @param[in] id   ide ID
    @return
		- @b TRUE: enable
		- @b FALSE: disable.
*/
BOOL idec_get_o1_en(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.o1_en;
}

#if 0
/*
    OSD2 window status.

    @param[in] id   ide ID
    @return
		- @b TRUE: enable
		- @b FALSE: disable.
*/
BOOL idec_get_o2_en(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.o2_en;
}
#endif

//@}

/**
@name ide OSD Buffer
*/
//@{

/**
    Set ide OSD1 starting address.

    @param[in] id   ide ID
    @param[in] ui_addr The starting address of OSD1.

    @return void
*/
void idec_set_o1_buf_addr(IDE_ID id, UINT32 ui_addr)
{
	T_IDE_BUF_ADDR reg;

	reg.reg = idec_get_reg(id, IDE_O1_ST_OFS);
#if defined __UITRON || defined __ECOS
	reg.bit.addr = ide_platform_va2pa(ui_addr);
#else
	reg.bit.addr = ui_addr;
#endif
	idec_set_reg(id, IDE_O1_ST_OFS, reg.reg);
}

/**
    Set ide OSD1 alpha plane starting address.

    @param[in] id   ide ID
    @param[in] ui_addr The starting address of OSD1 alpha plane.

    @return void
*/
void idec_set_o1_buf_alpha_addr(IDE_ID id, UINT32 ui_addr)
{
	T_IDE_BUF_ADDR reg;

	reg.reg = idec_get_reg(id, IDE_O1_AST_OFS);
#if defined __UITRON || defined __ECOS
	reg.bit.addr = ide_platform_va2pa(ui_addr);
#else
	reg.bit.addr = ui_addr;
#endif

    idec_set_reg(id, IDE_O1_AST_OFS, reg.reg);
}

/**
    Set ide OSD1  Buffer dimension.

    @param[in] id   ide ID
    @param[in] ui_bw  buffer width.
    @param[in] ui_bh  buffer height.
    @param[in] ui_lineoffset buffer lineoffset.

    @return void
*/
void idec_set_o1_buf_dim(IDE_ID id, UINT32 ui_bw, UINT32 ui_bh, UINT32 ui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_OBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_O1_BUF_DIM_OFS);
	reg_dim.bit.width = ui_bw;
	reg_dim.bit.height = ui_bh;
	idec_set_reg(id, IDE_O1_BUF_DIM_OFS, reg_dim.reg);

	reg_attr.reg = idec_get_reg(id, IDE_O1_BUF_ATTR_OFS);
	reg_attr.bit.lofs = ui_lineoffset;
	idec_set_reg(id, IDE_O1_BUF_ATTR_OFS, reg_attr.reg);
}

/*
    Set ide OSD1 line buffer Enable/Disable

    Set ide OSD1 line buffer Enable/Disable

    @param[in] id   ide ID
    @param[in] bYlinebuf  Y/Palette Channel line buffer config
		- @b TRUE:   Enable
		- @b FALSE:   Disable
    @param[in] bCLinebuf  C/Alpha Channel line buffer config, no use in
		- @b TRUE:     Enable
		- @b FALSE:   Disable

    @return void
*/
/*void idec_setO1LineBufferEn(IDE_ID id,  BOOL bYlinebuf, BOOL bCLinebuf)
{
    T_IDE_OBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_O1_BUF_ATTR_OFS);
    reg_attr.bit.lbuf_en = bYlinebuf;
    //reg_attr.bit.c_lbuf_en = bCLinebuf;
    idec_set_reg(id, IDE_O1_BUF_ATTR_OFS, reg_attr.reg);
}*/

/*
     Get ide OSD1 line buffer Enable/Disable

     Get ide OSD1 line buffer Enable/Disable

    @param[in] id   ide ID
     @param[out] bYlinebuf  Y/Palette Channel line buffer config
		- @b TRUE:   Enable
		- @b FALSE:   Disable
    @param[out] bCLinebuf  C/Alpha Channel line buffer config, no use in
		- @b TRUE:     Enable
		- @b FALSE: Disable
     @return void

*/
/*BOOL idec_getO1LineBufferEn(IDE_ID id,  BOOL *bYlinebuf, BOOL *bCLinebuf)
{
    T_IDE_OBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_O1_BUF_ATTR_OFS);
    *bYlinebuf = reg_attr.bit.lbuf_en;
    *bCLinebuf = FALSE;
}*/


/**
    Set ide OSD1 buffer read order.

    @param[in] id   ide ID
    @param[in] b_l2r: Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[in] b_t2b: Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_set_o1_read_order(IDE_ID id, IDE_HOR_READ b_l2r, IDE_VER_READ b_t2b)
{
	T_IDE_OBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_O1_BUF_ATTR_OFS);

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

	idec_set_reg(id, IDE_O1_BUF_ATTR_OFS, reg.reg);
}

#if 0
/*
    Set ide OSD1 starting address.

    @param[in] id   ide ID
    @param[in] ui_addr The starting address of OSD1.

    @return void
*/
void idec_set_o2_buf_addr(IDE_ID id, UINT32 ui_addr)
{
	T_IDE_BUF_ADDR reg;

	reg.reg = idec_get_reg(id, IDE_O2_ST_OFS);
	reg.bit.addr = ui_addr;
	idec_set_reg(id, IDE_O2_ST_OFS, reg.reg);
}

/**
    Get ide OSD1  Buffer dimension.

    @param[in] id   ide ID
    @param[in] ui_bw  buffer width.
    @param[in] ui_bh  buffer height.
    @param[in] ui_lineoffset buffer lineoffset.

    @return void
*/
void idec_set_o2_buf_dim(IDE_ID id, UINT32 ui_bw, UINT32 ui_bh, UINT32 ui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_OBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_O2_BUF_DIM_OFS);
	reg_dim.bit.width = ui_bw;
	reg_dim.bit.height = ui_bh;
	idec_set_reg(id, IDE_O2_BUF_DIM_OFS, reg_dim.reg);

	reg_attr.reg = idec_get_reg(id, IDE_O2_BUF_ATTR_OFS);
	reg_attr.bit.lofs = ui_lineoffset;
	idec_set_reg(id, IDE_O2_BUF_ATTR_OFS, reg_attr.reg);
}

/*
    Set ide OSD1 buffer read order.

    @param[in] id   ide ID
    @param[in] b_l2r: Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[in] b_t2b: Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_set_o2_read_order(IDE_ID id, IDE_HOR_READ b_l2r, IDE_VER_READ b_t2b)
{
	T_IDE_OBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_O2_BUF_ATTR_OFS);

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

	idec_set_reg(id, IDE_O2_BUF_ATTR_OFS, reg.reg);
}
#endif

/**
    Get ide OSD1 starting address.

    @param[in] id   ide ID
    @param[out] ui_addr The starting address of OSD1.

    @return void
*/
void idec_get_o1_buf_addr(IDE_ID id, UINT32 *ui_addr)
{
	T_IDE_BUF_ADDR reg;

	reg.reg = idec_get_reg(id, IDE_O1_ST_OFS);
	*ui_addr = dma_getNonCacheAddr(reg.bit.addr);
}

/**
    Get ide OSD1 alpha plane starting address.

    @param[in] id   ide ID
    @param[out] ui_addr The starting address of OSD1 alpha plane.

    @return void
*/
void idec_get_o1_alpha_buf_addr(IDE_ID id, UINT32 *ui_addr)
{
	T_IDE_BUF_ADDR reg;

	reg.reg = idec_get_reg(id, IDE_O1_AST_OFS);
	*ui_addr = dma_getNonCacheAddr(reg.bit.addr);
}

/**
    Get ide OSD2  Buffer dimension.

    @param[in] id   ide ID
    @param[out] pui_bw  buffer width.
    @param[out] pui_bh  buffer height.
    @param[out] pui_lineoffset buffer lineoffset.

    @return void
*/
void idec_get_o1_buf_dim(IDE_ID id, UINT32 *pui_bw, UINT32 *pui_bh, UINT32 *pui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_VBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_O1_BUF_DIM_OFS);
	*pui_bw = reg_dim.bit.width;
	*pui_bh = reg_dim.bit.height;

	reg_attr.reg = idec_get_reg(id, IDE_O1_BUF_ATTR_OFS);
	*pui_lineoffset = reg_attr.bit.lofs;
}


/*
    Set ide OSD2 line buffer Enable/Disable

    Set ide OSD2 line buffer Enable/Disable

    @param[in] id   ide ID
    @param[in] bYlinebuf  Y/Palette Channel line buffer config
		- @b TRUE:   Enable
		- @b FALSE:  Disable
    @param[in] bCLinebuf  C/Alpha Channel line buffer config, no use in
		- @b TRUE:   Enable
		- @b FALSE:  Disable
    @rerurn void
*/
/*void idec_setO2LineBufferEn(IDE_ID id, BOOL bYlinebuf, BOOL bCLinebuf)
{
    T_IDE_OBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_O2_BUF_ATTR_OFS);
    reg_attr.bit.lbuf_en = bYlinebuf;
    //reg_attr.bit.c_lbuf_en = bCLinebuf;
    idec_set_reg(id, IDE_O2_BUF_ATTR_OFS, reg_attr.reg);
}*/

/*
     Get ide OSD2 line buffer Enable/Disable

     Get ide OSD2 line buffer Enable/Disable

     @param[in] id   ide ID
     @param[out] bYlinebuf  Y/Palette Channel line buffer config
		- @b TRUE:   Enable
		- @b FALSE:  Disable
     @param[out] bCLinebuf  C/Alpha Channel line buffer config, no use in
		- @b TRUE:   Enable
		- @b FALSE:  Disable
     @rerurn void
*/
/*BOOL idec_getO2LineBufferEn(IDE_ID id, BOOL *bYlinebuf, BOOL *bCLinebuf)
{
    T_IDE_OBUF_ATTR reg_attr;

    reg_attr.reg = idec_get_reg(id, IDE_O2_BUF_ATTR_OFS);
    *bYlinebuf = reg_attr.bit.lbuf_en;
    *bCLinebuf = FALSE;
}*/


/**
    Get ide OSD1 buffer read order.

    @param[in] id   ide ID
    @param[out] pb_l2r: Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[out] pb_t2b: Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_get_o1_read_order(IDE_ID id, IDE_HOR_READ *pb_l2r, IDE_VER_READ *pb_t2b)
{
	T_IDE_VBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_O1_BUF_ATTR_OFS);
	*pb_l2r = reg.bit.l2r;
	*pb_t2b = reg.bit.t2b;
}

#if 0
/**
    Get ide OSD2 starting address.

    @param[in] id   ide ID
    @param[in] ui_addr The starting address of OSD1.

    @return void
*/
void idec_get_o2_buf_addr(IDE_ID id, UINT32 *ui_addr)
{
	T_IDE_BUF_ADDR reg;

	reg.reg = idec_get_reg(id, IDE_O2_ST_OFS);
	*ui_addr = reg.bit.addr;
}

/**
    Get ide OSD2  Buffer dimension.

    @param[in] id   ide ID
    @param[out] pui_bw  buffer width.
    @param[out] pui_bh  buffer height.
    @param[out] pui_lineoffset buffer lineoffset.

    @return void
*/
void idec_get_o2_buf_dim(IDE_ID id, UINT32 *pui_bw, UINT32 *pui_bh, UINT32 *pui_lineoffset)
{
	T_IDE_BUF_DIM reg_dim;
	T_IDE_VBUF_ATTR reg_attr;

	reg_dim.reg = idec_get_reg(id, IDE_O2_BUF_DIM_OFS);
	*pui_bw = reg_dim.bit.width;
	*pui_bh = reg_dim.bit.height;

	reg_attr.reg = idec_get_reg(id, IDE_O2_BUF_ATTR_OFS);
	*pui_lineoffset = reg_attr.bit.lofs;
}

/**
    Get ide OSD2 buffer read order.

    @param[in] id   ide ID
    @param[out] pb_l2r Read order
		- IDE_BUFFER_READ_L2R: read from left to right
		- IDE_BUFFER_READ_R2L: read from right to left
    @param[out] pb_t2b Read order
		- IDE_BUFFER_READ_T2B: read from top to bottom
		- IDE_BUFFER_READ_B2T: read from bottom to top

    @return void
*/
void idec_get_o2_read_order(IDE_ID id, IDE_HOR_READ *pb_l2r, IDE_VER_READ *pb_t2b)
{
	T_IDE_VBUF_ATTR reg;

	reg.reg = idec_get_reg(id, IDE_O2_BUF_ATTR_OFS);
	*pb_l2r = reg.bit.l2r;
	*pb_t2b = reg.bit.t2b;
}
#endif
//@}

/**
@name ide OSD Scale Control
*/
//@{

/**
    Set ide OSD1 Scaling Control.

    @param[in] id   ide ID
    @param[in] b_hscaleup
		- @b FALSE:horizontal scaling down
		- @b TRUE:horizontal scaling up.
    @param[in] b_vscaleup
		- @b FALSE:vertical scaling down
		- @b TRUE:vertical scaling up.

    @return void
*/
void idec_set_o1_scale_ctrl(IDE_ID id, BOOL b_hscaleup, BOOL b_vscaleup)
{
	T_IDE_SCALE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_SCALE_CTRL_OFS);

	if (b_hscaleup) {
		reg.bit.o1_hud = 1;
	} else {
		reg.bit.o1_hud = 0;
	}

	if (b_vscaleup) {
		reg.bit.o1_vud = 1;
	} else {
		reg.bit.o1_vud = 0;
	}

	idec_set_reg(id, IDE_SCALE_CTRL_OFS, reg.reg);
}

/**
    Set ide OSD1 scaling factor

    @param[in] id   ide ID
    @param[in] ui_hsf horizontal scale factor.
    @param[in] b_sub horizontal sub-sample option
		- @b FALSE:no sub-sample
		- @b TRUE:sub-sample
    @param[in] ui_vsf vertical scale factor.
    @param[in] b_vsub vertical sub-sample option

    @return void
*/
void idec_set_o1_scale_factor(IDE_ID id, UINT32 ui_hsf, BOOL b_sub, UINT32 ui_vsf, BOOL b_vsub)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR0_OFS);

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

	idec_set_reg(id, IDE_O1_WIN_ATTR0_OFS, reg.reg);
}

/**
    Set ide OSD1 vsf init value

    @param[in] id   ide ID
    @param[in] ui_init0 vsf init value 0
    @param[in] ui_init1 vsf init value 1

    @return void
*/
void idec_set_o1_vsf_init(IDE_ID id, UINT32 ui_init0, UINT32 ui_init1)
{
	T_IDE_VSF_INIT reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR3_OFS);

	reg.bit.vsf_init0 = ui_init0;
	reg.bit.vsf_init1 = ui_init1;

	idec_set_reg(id, IDE_O1_WIN_ATTR3_OFS, reg.reg);
}
#if 0
/**
    Set ide OSD2 Scaling Control.

    @param[in] id   ide ID
    @param[in] b_hscaleup
		- @b FALSE:horizontal scaling down
		- @b TRUE:horizontal scaling up.
    @param[in] b_vscaleup
		- @b FALSE:vertical scaling down
		- @b TRUE:vertical scaling up.

    @return void
*/
void idec_set_o2_scale_ctrl(IDE_ID id, BOOL b_hscaleup, BOOL b_vscaleup)
{
	T_IDE_SCALE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_SCALE_CTRL_OFS);

	if (b_hscaleup) {
		reg.bit.o2_hud = 1;
	} else {
		reg.bit.o2_hud = 0;
	}

	if (b_vscaleup) {
		reg.bit.o2_vud = 1;
	} else {
		reg.bit.o2_vud = 0;
	}

	idec_set_reg(id, IDE_SCALE_CTRL_OFS, reg.reg);
}

/**
    Set ide OSD2 scaling factor

    @param[in] id   ide ID
    @param[in] ui_hsf horizontal scale factor.
    @param[in] b_sub horizontal sub-sample option
		- @b FALSE:no sub-sample
		- @b TRUE:sub-sample
    @param[in] ui_vsf vertical scale factor.
    @param[in] b_vsub vertical sub-sample option

    @return void
*/
void idec_set_o2_scale_factor(IDE_ID id, UINT32 ui_hsf, BOOL b_sub, UINT32 ui_vsf, BOOL b_vsub)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR0_OFS);
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

	idec_set_reg(id, IDE_O2_WIN_ATTR0_OFS, reg.reg);
}

/**
    Set ide OSD2 vsf init value

    @param[in] id   ide ID
    @param[in] ui_init0 vsf init value 0
    @param[in] ui_init1 vsf init value 1

    @return void
*/
void idec_set_o2_vsf_init(IDE_ID id, UINT32 ui_init0, UINT32 ui_init1)
{
	T_IDE_VSF_INIT reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR3_OFS);

	reg.bit.vsf_init0 = ui_init0;
	reg.bit.vsf_init1 = ui_init1;

	idec_set_reg(id, IDE_O2_WIN_ATTR3_OFS, reg.reg);
}
#endif

/**
    Get ide OSD1 scaling factor

    @param[in] id   ide ID
    @param[out] ui_hsf horizontal scale factor.
    @param[out] b_sub horizontal sub-sample option
		- @b FALSE:no sub-sample
		- @b TRUE:sub-sample
    @param[out] ui_vsf vertical scale factor.
    @param[out] b_vsub vertical sub-sample option

    @return void
*/
void idec_get_o1_scale_factor(IDE_ID id, UINT32 *ui_hsf, BOOL *b_sub, UINT32 *ui_vsf, BOOL *b_vsub)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR0_OFS);

	*ui_hsf = reg.bit.hsf;
	*ui_vsf = reg.bit.vsf;
	*b_sub = reg.bit.sub;
	*b_vsub = reg.bit.vsub;
}

#if 0
/**
    Get ide OSD2 scaling factor

    @param[in] id   ide ID
    @param[out] ui_hsf horizontal scale factor.
    @param[out] b_sub horizontal sub-sample option
		- @b FALSE:no sub-sample
		- @b TRUE:sub-sample.
    @param[out] ui_vsf vertical scale factor.
    @param[out] b_vsub vertical sub-sample option

    @return void
*/
void idec_get_o2_scale_factor(IDE_ID id, UINT32 *ui_hsf, BOOL *b_sub, UINT32 *ui_vsf, BOOL *b_vsub)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR0_OFS);

	*ui_hsf = reg.bit.hsf;
	*ui_vsf = reg.bit.vsf;
	*b_sub = reg.bit.sub;
	*b_vsub = reg.bit.vsub;
}
#endif

/**
    Set ide OSD1 horizontal scaling method

    @param[in] id   ide ID
    @param[in] hsm horizontal scaling method
		- IDE_SCALEMETHOD_DROP:duplicate/drop
		- IDE_SCALEMETHOD_BILINEAR:bilinear

    @return void
*/
void idec_set_o1_hsm(IDE_ID id, IDE_SCALE_METHOD hsm)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR0_OFS);

	if (hsm) {
		reg.bit.hsm = 1;
	} else {
		reg.bit.hsm = 0;
	}

	idec_set_reg(id, IDE_O1_WIN_ATTR0_OFS, reg.reg);
}

/**
    Set ide OSD1 Vertical scaling method

    @param[in] id   ide ID
    @param[in] vsm vertical scaling method
		- IDE_SCALEMETHOD_DROP:duplicate/drop
		- IDE_SCALEMETHOD_BILINEAR:bilinear

    @return void
*/
void idec_set_o1_vsm(IDE_ID id, IDE_SCALE_METHOD vsm)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR0_OFS);

	if (vsm) {
		reg.bit.vsm = 1;
	} else {
		reg.bit.vsm = 0;
	}

	idec_set_reg(id, IDE_O1_WIN_ATTR0_OFS, reg.reg);
}


//@}

/**
@name ide OSD Window
*/
//@{

/**
    Set ide OSD1 window dimension.

    @param[in] id   ide ID
    @param[in] ui_win_w window width.
    @param[in] ui_win_h window height.

    @return void
*/
void idec_set_o1_win_dim(IDE_ID id, UINT32 ui_win_w, UINT32 ui_win_h)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR1_OFS);

	reg.bit.width = ui_win_w;
	reg.bit.height = ui_win_h;

	idec_set_reg(id, IDE_O1_WIN_ATTR1_OFS, reg.reg);
}

/**
    Set ide OSD1 window position

    @param[in] id   ide ID
    @param[in] ui_x The start position at X-axis.
    @param[in] ui_y The start position at Y-axis.

    @return void
*/
void idec_set_o1_win_pos(IDE_ID id, UINT32 ui_x, UINT32 ui_y)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR2_OFS);

	reg.bit.x = ui_x;
	reg.bit.y = ui_y;

	idec_set_reg(id, IDE_O1_WIN_ATTR2_OFS, reg.reg);
}

/*
    Set ide OSD1 Palette select

    @param[in] id   ide ID
    @param[in] ui_psel IDE_PALETTE_LOW256 or IDE_PALETTE_HIGH256.

    @return void
    @note It is not used in 96650
*/
/*
void idec_set_o1_palette_sel(IDE_ID id, IDE_PALETTE_SEL ui_psel)
{
    T_IDE_OSD_WIN_ATTR2 reg;

    reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR2_OFS);
    reg.bit.psel = ui_psel;
    idec_set_reg(id, IDE_O1_WIN_ATTR2_OFS, reg.reg);
}
*/

/**
    Set ide OSD1 Palette High Address

    @param[in] id   ide ID
    @param[in] ui_hi_addr High bit of addr for palette

    @return void
*/
void idec_set_o1_palette_high_addr(IDE_ID id, UINT8 ui_hi_addr)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR2_OFS);
	reg.bit.oh = ui_hi_addr;
	idec_set_reg(id, IDE_O1_WIN_ATTR2_OFS, reg.reg);
}

/**
    Set ide OSD1 window format.

    @param[in] id   ide ID
    @param[in] ui_fmt window data format
		- COLOR_1_BIT: 1-bit color
		- COLOR_2_BIT: 2-bit color
		- COLOR_4_BIT: 4-bit color
		- COLOR_8_BIT: 8-bit color
		- COLOR_ARGB4565: ARGB 4565
		- COLOR_ARGB8565: ARGB 8565

    @return void
*/
void idec_set_o1_fmt(IDE_ID id, IDE_COLOR_FORMAT ui_fmt)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	if (ui_fmt != COLOR_1_BIT &&
		ui_fmt != COLOR_2_BIT &&
		ui_fmt != COLOR_4_BIT &&
		ui_fmt != COLOR_8_BIT &&
		ui_fmt != COLOR_ARGB4565 &&
		ui_fmt != COLOR_ARGB8565 &&
		ui_fmt != COLOR_ARGB8888 &&
		ui_fmt != COLOR_ARGB4444 &&
		ui_fmt != COLOR_ARGB1555) {
		DBG_ERR("Unsupported OSD1 format %d\r\n", (int)ui_fmt);
		return;
	}

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR1_OFS);

	reg.bit.fmt = ui_fmt;

	idec_set_reg(id, IDE_O1_WIN_ATTR1_OFS, reg.reg);
}


#if 0
/**
    Set ide OSD2 horizontal scaling method

    @param[in] id   ide ID
    @param[in] hsm horizontal scaling method
		- IDE_SCALEMETHOD_DROP:duplicate/drop
		- IDE_SCALEMETHOD_BILINEAR:bilinear

    @return void
*/
void idec_set_o2_hsm(IDE_ID id, IDE_SCALE_METHOD hsm)
{
	T_IDE_OSD_WIN_ATTR0 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR0_OFS);

	if (hsm) {
		reg.bit.hsm = 1;
	} else {
		reg.bit.hsm = 0;
	}

	idec_set_reg(id, IDE_O2_WIN_ATTR0_OFS, reg.reg);
}

/**
    Set ide OSD2 window dimension.

    @param[in] id   ide ID
    @param[in] ui_win_w window width.
    @param[in] ui_win_h window height.

    @return void
*/
void idec_set_o2_win_dim(IDE_ID id, UINT32 ui_win_w, UINT32 ui_win_h)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR1_OFS);

	reg.bit.width = ui_win_w;
	reg.bit.height = ui_win_h;

	idec_set_reg(id, IDE_O2_WIN_ATTR1_OFS, reg.reg);
}

/**
    Set ide OSD2 window position

    @param[in] id   ide ID
    @param[in] ui_x The start position at X-axis.
    @param[in] ui_y The start position at Y-axis.

    @return void
*/
void idec_set_o2_win_pos(IDE_ID id, UINT32 ui_x, UINT32 ui_y)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR2_OFS);

	reg.bit.x = ui_x;
	reg.bit.y = ui_y;

	idec_set_reg(id, IDE_O2_WIN_ATTR2_OFS, reg.reg);
}

/*
    Set ide OSD2 Palette select

    @param[in] id   ide ID
    @param[in] ui_psel IDE_PALETTE_LOW256 or IDE_PALETTE_HIGH256.

    @return void
    @note It is not used in 96650
*/
void idec_set_o2_palette_sel(IDE_ID id, IDE_PALETTE_SEL ui_psel)
{
	/*    T_IDE_OSD_WIN_ATTR2 reg;

	    reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR2_OFS);
	    reg.bit.psel = ui_psel;
	    idec_set_reg(id, IDE_O2_WIN_ATTR2_OFS, reg.reg);*/
}

/**
    Set ide OSD2 Palette High Address

    @param[in] id   ide ID
    @param[in] ui_hi_addr High bit of addr for palette

    @return void
*/
void idec_set_o2_palette_high_addr(IDE_ID id, UINT8 ui_hi_addr)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR2_OFS);
	reg.bit.oh = ui_hi_addr;
	idec_set_reg(id, IDE_O2_WIN_ATTR2_OFS, reg.reg);
}

/**
    Set ide OSD2 window format.

    @param[in] id   ide ID
    @param[in] ui_fmt: window data format
		- COLOR_1_BIT: 1-bit color
		- COLOR_2_BIT: 2-bit color
		- COLOR_4_BIT: 4-bit color
		- COLOR_8_BIT: 8-bit color

    @return void
*/
void idec_set_o2_fmt(IDE_ID id, IDE_COLOR_FORMAT ui_fmt)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	if (ui_fmt != COLOR_1_BIT &&
		ui_fmt != COLOR_2_BIT &&
		ui_fmt != COLOR_4_BIT &&
		ui_fmt != COLOR_8_BIT) {
		DBG_ERR("Unsupported OSD2 format %d\r\n", ui_fmt);
		return;
	}

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR1_OFS);

	reg.bit.fmt = ui_fmt;

	idec_set_reg(id, IDE_O2_WIN_ATTR1_OFS, reg.reg);
}
#endif

/**
    Get ide OSD1 window dimension.

    @param[in] id   ide ID
    @param[out] ui_win_w Window Width
    @param[out] ui_win_h Window Height

    @return void
*/
void idec_get_o1_win_dim(IDE_ID id, UINT32 *ui_win_w, UINT32 *ui_win_h)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR1_OFS);
	*ui_win_w = reg.bit.width;
	*ui_win_h = reg.bit.height;
}

/**
    Set ide OSD1 window position

    @param[in] id   ide ID
    @param[out] ui_x The start position at X-axis.
    @param[out] ui_y The start position at Y-axis.

    @return void
*/
void idec_get_o1_win_pos(IDE_ID id, UINT32 *ui_x, UINT32 *ui_y)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR2_OFS);

	*ui_x = reg.bit.x;
	*ui_y = reg.bit.y;
}

/**
    Get ide OSD1 Palette select

    @param[in] id   ide ID
    @param[out] ui_psel IDE_PALETTE_LOW256 or IDE_PALETTE_HIGH256.

    @return void
*/
void idec_get_o1_palette_sel(IDE_ID id, IDE_PALETTE_SEL *ui_psel)
{
	/*    T_IDE_OSD_WIN_ATTR2 reg;

	    reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR2_OFS);
	    *ui_psel = reg.bit.psel;*/
	*ui_psel = IDE_PALETTE_LOW256;
}

/**
    Get ide OSD1 Palette High Address

    @param[in] id   ide ID
    @param[out] ui_hi_addr High bit of addr for palette

    @return void
*/
void idec_get_o1_palette_high_addr(IDE_ID id, UINT8 *ui_hi_addr)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR2_OFS);
	*ui_hi_addr = reg.bit.oh;
}

/**
    Get ide OSD1 window format.

    @param[in] id   ide ID
    @param[out] ui_fmt: window data format

    @return void
*/
void idec_get_o1_fmt(IDE_ID id, IDE_COLOR_FORMAT *ui_fmt)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_O1_WIN_ATTR1_OFS);

	*ui_fmt = reg.bit.fmt;
}
#if 0
/**
    Get ide OSD2 window dimension.

    @param[in] id   ide ID
    @param[out] ui_win_w Window Width
    @param[out] ui_win_h Window Height

    @return void
*/
void idec_get_o2_win_dim(IDE_ID id, UINT32 *ui_win_w, UINT32 *ui_win_h)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR1_OFS);
	*ui_win_w = reg.bit.width;
	*ui_win_h = reg.bit.height;
}

/**
    Set ide OSD2 window position

    @param[in] id   ide ID
    @param[out] ui_x The start position at X-axis.
    @param[out] ui_y The start position at Y-axis.

    @return void
*/
void idec_get_o2_win_pos(IDE_ID id, UINT32 *ui_x, UINT32 *ui_y)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR2_OFS);

	*ui_x = reg.bit.x;
	*ui_y = reg.bit.y;
}

/**
    Get ide OSD2 Palette select

    @param[in] id   ide ID
    @param[out] ui_psel IDE_PALETTE_LOW256 or IDE_PALETTE_HIGH256.

    @return void
*/
void idec_get_o2_palette_sel(IDE_ID id, IDE_PALETTE_SEL *ui_psel)
{
	/*    T_IDE_OSD_WIN_ATTR2 reg;

	    reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR2_OFS);
	    *ui_psel = reg.bit.psel;*/
	*ui_psel = IDE_PALETTE_HIGH256;
}

/**
    Get ide OSD2 Palette High Address

    @param[in] id   ide ID
    @param[out] ui_hi_addr High bit of addr for palette

    @return void
*/
void idec_get_o2_palette_high_addr(IDE_ID id, UINT8 *ui_hi_addr)
{
	T_IDE_OSD_WIN_ATTR2 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR2_OFS);
	*ui_hi_addr = reg.bit.oh;
}

/**
    Get ide OSD2 window format.

    @param[in] id   ide ID
    @param[out] ui_fmt window data format

    @return void
*/
void idec_get_o2_fmt(IDE_ID id, IDE_COLOR_FORMAT *ui_fmt)
{
	T_IDE_OSD_WIN_ATTR1 reg;

	reg.reg = idec_get_reg(id, IDE_O2_WIN_ATTR1_OFS);

	*ui_fmt = reg.bit.fmt;
}
#endif
//@}

/**
@name ide OSD color Key and Alpha Blending
*/
//@{

/*
    Set ide OSD color Key Enable

    @param[in] id   ide ID
    @param[in] b_en FALSE:disable color key, TRUE:enable color key

    @return void
*/
void idec_set_osd_colorkey_en(IDE_ID id, BOOL b_en)
{
	/*    T_IDE_OSD_OPT reg;

	    reg.reg = idec_get_reg(id, IDE_OSD_OPT_OFS);

	    if(b_en)
			reg.bit.ck_en = 1;
	    else
			reg.bit.ck_en = 0;

	    idec_set_reg(id, IDE_OSD_OPT_OFS, reg.reg);*/
}

/*
    Set ide OSD color Key

    @param[in] id   ide ID
    @param[in] ui_ck The index of osd color key

    @return void
*/
void idec_set_osd_colorkey(IDE_ID id, UINT8 ui_ck)
{
	/*    T_IDE_OSD_OPT reg;

	    reg.reg = idec_get_reg(id, IDE_OSD_OPT_OFS);

	    reg.bit.ck = ui_ck;

	    idec_set_reg(id, IDE_OSD_OPT_OFS, reg.reg);*/
}

/*
    Get ide OSD color Key Enable

    @param[in] id   ide ID
    @param[out] b_en color key is enabled or disabled

    @return void
*/
void idec_get_osd_colorkey_en(IDE_ID id, BOOL *b_en)
{
	/*    T_IDE_OSD_OPT reg;

	    reg.reg = idec_get_reg(id, IDE_OSD_OPT_OFS);
	    *b_en = reg.bit.ck_en;*/
}

/*
    Get ide OSD color Key

    @param[in] id   ide ID
    @param[out] ui_ck The index of osd color key

    @return void
*/
void idec_get_osd_colorkey(IDE_ID id, UINT8 *ui_ck)
{
	/*    T_IDE_OSD_OPT reg;

	    reg.reg = idec_get_reg(id, IDE_OSD_OPT_OFS);
	    *ui_ck = reg.bit.ck;*/
}
/**
    Set ide OSD1 color Key Enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable color key
		- @b TRUE:enable color key

    @return void
*/
void idec_set_osd1_colorkey_en(IDE_ID id, BOOL b_en)
{
	T_IDE_OSD_COLORKEY_CTL reg;

	reg.reg = idec_get_reg(id, IDE_O1_COLORKEY_CTL_OFS);
	reg.bit.ck_en = b_en;
	idec_set_reg(id, IDE_O1_COLORKEY_CTL_OFS, reg.reg);
}
/**
    Get ide OSD1 color Key Enable

    @param[in] id   ide ID
    @param[out] b_en
		- @b FALSE:disable color key
		- @b TRUE:enable color key

    @return void
*/
void idec_get_osd1_colorkey_en(IDE_ID id, BOOL *b_en)
{
	T_IDE_OSD_COLORKEY_CTL reg;

	reg.reg = idec_get_reg(id, IDE_O1_COLORKEY_CTL_OFS);
	*b_en = reg.bit.ck_en;

}
/**
    Set ide OSD1 color Key operation

    @param[in] id   ide ID
    @param[in] ck_op
		- IDE_OSD_COLORKEY_YEQUKEY: = color key
		- IDE_OSD_COLORKEY_YSMALLKEY: < color key
		- IDE_OSD_COLORKEY_YBIGKEY > color key
    @return void
*/
void idec_set_osd1_colorkey_op(IDE_ID id, IDE_OSD_COLORKEY_OP ck_op)
{
	T_IDE_OSD_COLORKEY_CTL reg;

	reg.reg = idec_get_reg(id, IDE_O1_COLORKEY_CTL_OFS);
	reg.bit.ck_op = ck_op;
	idec_set_reg(id, IDE_O1_COLORKEY_CTL_OFS, reg.reg);


}
/**
    Get ide OSD1 color Key operation

    @param[in] id   ide ID
    @param[out] ck_op
		- IDE_OSD_COLORKEY_EQUAL: = color key
		- IDE_OSD_COLORKEY_EQUAL_A: = color key + alpha key
    @return void
*/
void idec_get_osd1_colorkey_op(IDE_ID id, IDE_OSD_COLORKEY_OP *ck_op)
{
	T_IDE_OSD_COLORKEY_CTL reg;

	reg.reg = idec_get_reg(id, IDE_O1_COLORKEY_CTL_OFS);
	*ck_op = reg.bit.ck_op;

}
/**
    Set ide OSD1 color Key value

    @param[in] id   ide ID
    @param[in] ui_key_r   R value
    @param[in] ui_key_g   G value
    @param[in] ui_key_b   B value

    @return void
*/
void idec_set_osd1_colorkey(IDE_ID id, UINT8 ui_key_r, UINT8 ui_key_g, UINT8 ui_key_b, UINT8 alpha)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O1_COLORKEY_OFS);
	reg.bit.ck_y = ui_key_r;
	reg.bit.ck_cb = ui_key_g;
	reg.bit.ck_cr = ui_key_b;
	reg.bit.ck_alpha = alpha;

	idec_set_reg(id, IDE_O1_COLORKEY_OFS, reg.reg);
}
/**
    Get ide OSD1 color Key value

    @param[in] id   ide ID
    @param[out] ui_key_y   Y value
    @param[out] ui_key_cb  Cb value
    @param[out] ui_key_cr  Cr value

    @return void
*/
void idec_get_osd1_colorkey(IDE_ID id, UINT8 *ui_key_r, UINT8 *ui_key_g, UINT8 *ui_key_b, UINT8 *alpha)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O1_COLORKEY_OFS);
	*ui_key_r = reg.bit.ck_y;
	*ui_key_g = reg.bit.ck_cb;
	*ui_key_b = reg.bit.ck_cr;
	*alpha = reg.bit.ck_alpha;
}

#if 0
/**
    Set ide OSD2 color Key Enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable color key
		- @b TRUE:enable color key

    @return void
*/
void idec_set_osd2_color_key_en(IDE_ID id, BOOL b_en)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O2_COLORKEY_OFS);
	reg.bit.ck_en = b_en;
	idec_set_reg(id, IDE_O2_COLORKEY_OFS, reg.reg);
}
/**
    Get ide OSD2 color Key Enable

    @param[in] id   ide ID
    @param[out] b_en
		- @b FALSE:disable color key
		- @b TRUE:enable color key

    @return void
*/
void idec_get_osd2_color_key_en(IDE_ID id, BOOL *b_en)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O2_COLORKEY_OFS);
	*b_en = reg.bit.ck_en;

}
/**
    Set ide OSD2 color Key operation

    @param[in] id   ide ID
    @param[in] ck_op
		- IDE_OSD_COLORKEY_YEQUKEY: = color key
		- IDE_OSD_COLORKEY_YSMALLKEY: < color key
		- IDE_OSD_COLORKEY_YBIGKEY > color key
    @return void
*/
void idec_set_osd2_color_key_op(IDE_ID id, IDE_OSD_COLORKEY_OP ck_op)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O2_COLORKEY_OFS);
	reg.bit.ck_op = ck_op;
	idec_set_reg(id, IDE_O2_COLORKEY_OFS, reg.reg);


}
/**
    Get ide OSD2 color Key operation

    @param[in] id   ide ID
    @param[out] ck_op
		- IDE_OSD_COLORKEY_YEQUKEY: = color key
		- IDE_OSD_COLORKEY_YSMALLKEY: < color key
		- IDE_OSD_COLORKEY_YBIGKEY > color key
    @return void
*/
void idec_get_osd2_color_key_op(IDE_ID id, IDE_OSD_COLORKEY_OP *ck_op)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O2_COLORKEY_OFS);
	*ck_op = reg.bit.ck_op;

}
/**
    Set ide OSD2 color Key value

    @param[in] id   ide ID
    @param[in] ui_key_y   Y value
    @param[in] ui_key_cb  Cb value
    @param[in] ui_key_cr  Cr value

    @return void
*/
void idec_set_osd2_color_key(IDE_ID id, UINT8 ui_key_y, UINT8 ui_key_cb, UINT8 ui_key_cr)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O2_COLORKEY_OFS);
	reg.bit.ck_y = ui_key_y;
	reg.bit.ck_cb = ui_key_cb;
	reg.bit.ck_cr = ui_key_cr;

	idec_set_reg(id, IDE_O2_COLORKEY_OFS, reg.reg);
}
/**
    Get ide OSD2 color Key value

    @param[in] id   ide ID
    @param[out] ui_key_y   Y value
    @param[out] ui_key_cb  Cb value
    @param[out] ui_key_cr  Cr value

    @return void
*/
void idec_get_osd2_color_key(IDE_ID id, UINT8 *ui_key_y, UINT8 *ui_key_cb, UINT8 *ui_key_cr)
{
	T_IDE_OSD_COLORKEY reg;

	reg.reg = idec_get_reg(id, IDE_O2_COLORKEY_OFS);
	*ui_key_y = reg.bit.ck_y;
	*ui_key_cb = reg.bit.ck_cb;
	*ui_key_cr = reg.bit.ck_cr;
}
#endif

/**
    Set ide OSD1 LowPass Enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable
		- @b TRUE:enable

    @return void
*/
void idec_set_o1_lowpass_en(IDE_ID id, BOOL b_en)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O1_LOWPASS_OFS);
	reg.bit.lp_en = b_en;
	idec_set_reg(id, IDE_O1_LOWPASS_OFS, reg.reg);
}
/**
    Get ide OSD1 LowPass Enable

    @param[in] id   ide ID
    @return
		- @b FALSE:disable
		- @b TRUE:enable
*/
BOOL idec_get_o1_lowpass_en(IDE_ID id)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O1_LOWPASS_OFS);
	return reg.bit.lp_en;
}
/**
    Set ide OSD1 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_set_o1_lowpass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O1_LOWPASS_OFS);
	reg.bit.lp_tap0 = ui_pcoef[0];
	reg.bit.lp_tap1 = ui_pcoef[1];
	reg.bit.lp_tap2 = ui_pcoef[2];
	idec_set_reg(id, IDE_O1_LOWPASS_OFS, reg.reg);
}
/**
    Get ide OSD1 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_get_o1_lowpass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O1_LOWPASS_OFS);
	ui_pcoef[0] = reg.bit.lp_tap0;
	ui_pcoef[1] = reg.bit.lp_tap1;
	ui_pcoef[2] = reg.bit.lp_tap2;
}
#if 0
/**
    Set ide OSD2 LowPass Enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable
		- @b TRUE:enable

    @return void
*/
void idec_set_o2_low_pass_en(IDE_ID id, BOOL b_en)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O2_LOWPASS_OFS);
	reg.bit.lp_en = b_en;
	idec_set_reg(id, IDE_O2_LOWPASS_OFS, reg.reg);
}
/**
    Get ide OSD2 LowPass Enable

    @param[in] id   ide ID
    @return
		- @b FALSE:disable
		- @b TRUE:enable
*/
BOOL idec_get_o2_low_pass_en(IDE_ID id)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O2_LOWPASS_OFS);
	return reg.bit.lp_en;
}
/**
    Set ide OSD2 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_set_o2_low_pass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O2_LOWPASS_OFS);
	reg.bit.lp_tap0 = ui_pcoef[0];
	reg.bit.lp_tap1 = ui_pcoef[1];
	reg.bit.lp_tap2 = ui_pcoef[2];
	idec_set_reg(id, IDE_O2_LOWPASS_OFS, reg.reg);
}
/**
    Get ide OSD1 LowPass Coef

    @param[in] id   ide ID
    @param[in] ui_pcoef  the pointer of coef. (3 coef)

    @return void
*/
void idec_get_o2_low_pass_coef(IDE_ID id, UINT8 *ui_pcoef)
{
	T_IDE_LOWPASS_OPT reg;

	reg.reg = idec_get_reg(id, IDE_O2_LOWPASS_OFS);
	ui_pcoef[0] = reg.bit.lp_tap0;
	ui_pcoef[1] = reg.bit.lp_tap1;
	ui_pcoef[2] = reg.bit.lp_tap2;
}
#endif

//@}

//@}

/*
    Set OSD1 burst length

    Set OSD1 burst length

    @param[in] id   ide ID
    @param[in] chdma  channel dma of palette or alpha burst length
    @param[in] chrgb  channel rgb burst length

    @return
		- @b FALSE:something error
		- @b TRUE:ok
*/
BOOL idec_set_o1_burst_len(IDE_ID id, IDE_DMA_BURST_LEN chdma, IDE_DMA_BURST_LEN chrgb)
{
	T_IDE_DMA_LEN reg;

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);

#if 0
	if (id == IDE_ID_1) {
		if ((chdma > IDE_DMA_BURST_LEN_32) || (chrgb > IDE_DMA_BURST_LEN_64)) {
			DBG_WRN("ide: O1 DMA burst larger than 32/64\r\n");
			return FALSE;
		}
	} else {
#endif
		if ((chdma > IDE_DMA_BURST_LEN_64) || (chrgb > IDE_DMA_BURST_LEN_64)) {
			DBG_WRN("ide: O1 DMA burst larger than 64/64\r\n");
			return FALSE;
		}
#if 0
	}
#endif
	reg.bit.o1_a_len = chdma;
	reg.bit.o1_rgb_len = chrgb;
	idec_set_reg(id, IDE_DMA_LEN_OFS, reg.reg);

	return TRUE;
}

/*
    Get OSD1 burst length

    Get OSD1 burst length

    @param[in] id   ide ID
    @param[out] chy  channel dma of palette or alpha burst length
    @param[out] chc  channel rgb burst length

    @return
		- @b TRUE:ok
*/
BOOL idec_get_o1_burst_len(IDE_ID id, IDE_DMA_BURST_LEN *chdma, IDE_DMA_BURST_LEN *chrgb)
{
	T_IDE_DMA_LEN reg;

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);

	*chdma = reg.bit.o1_a_len;
	*chrgb = reg.bit.o1_rgb_len;

	return TRUE;
}


/**
    Set ide FD & Line layer swap

    Set ide OSD1 & OSD2 layer swap

    @param[in] id       ide ID
    @param[in] b_swap    Swap FD line or not(FALSE:FD->Line(up to low)/TRUE:Line->FD(up to low))


    @return void
*/
void idec_set_fd_line_layer_swap(IDE_ID id, BOOL b_swap)
{
	T_IDE_BLEND2    ui_ide_blend2;

	ui_ide_blend2.reg = idec_get_reg(id, IDE_BLEND2_OFS);


	ui_ide_blend2.bit.fd_line_lyr_swap = b_swap;

	idec_set_reg(id, IDE_BLEND2_OFS, ui_ide_blend2.reg);
}

/**
    Get ide FD & Line layer swap

    Get ide OSD1 & OSD2 layer swap

    @param[in] id       ide ID

     @return
		- @b TRUE: swap (Line->FD:up to low)
		- @b FALSE: not swap(FD->Line:up to low)
*/
BOOL idec_get_fd_line_layer_swap(IDE_ID id)
{
	T_IDE_BLEND2    ui_ide_blend2;

	ui_ide_blend2.reg = idec_get_reg(id, IDE_BLEND2_OFS);

	return (BOOL)ui_ide_blend2.bit.fd_line_lyr_swap;
}


#if 0
/*
    Set OSD2 burst length

    Set OSD2 burst length

    @param[in] id   ide ID
    @param[in] chdma  channel dma of palette or alpha burst length
    @param[in] chrgb  not used, reserved.

    @return
		- @b FALSE:something error
		- @b TRUE:ok
*/
BOOL idec_set_o2_burst_len(IDE_ID id, IDE_DMA_BURST_LEN chdma, IDE_DMA_BURST_LEN chrgb)
{
	T_IDE_DMA_LEN reg;

	if (id > IDE_ID_1) {
		return FALSE;
	}

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);
	if ((chdma > IDE_DMA_BURST_LEN_64)) {
		DBG_WRN("ide: O2 DMA burst larger than 64\r\n");
		return FALSE;
	}
	reg.bit.o2_dma_len = chdma;
	idec_set_reg(id, IDE_DMA_LEN_OFS, reg.reg);

	return TRUE;
}

/*
    Get OSD2 burst length

    Get OSD2 burst length

    @param[in] id   ide ID
    @param[out] chdma  channel dma of palette or alpha burst length
    @param[out] chrgb  not used, reserved.

    @return
		- @b TRUE:ok
*/
BOOL idec_get_o2_burst_len(IDE_ID id, IDE_DMA_BURST_LEN *chdma, IDE_DMA_BURST_LEN *chrgb)
{
	T_IDE_DMA_LEN reg;

	reg.reg = idec_get_reg(id, IDE_DMA_LEN_OFS);

	*chdma = reg.bit.o1_a_len;
	*chrgb = 0;

	return TRUE;
}

/**
    Set ide OSD1 & OSD2 layer swap

    Set ide OSD1 & OSD2 layer swap

    @param[in] id       ide ID
    @param[in] b_swap    OSD layer swap or not

    @return void
*/
void idec_set_osd_layer_swap(IDE_ID id, BOOL b_swap)
{
	T_IDE_BLEND2    ui_ide_blend2;

	ui_ide_blend2.reg = idec_get_reg(id, IDE_BLEND2_OFS);

	if (id > IDE_ID_1) {
		return;
	}

	ui_ide_blend2.bit.osd_1_2_swap = b_swap;

	idec_set_reg(id, IDE_BLEND2_OFS, ui_ide_blend2.reg);
}

/**
    Get ide OSD1 & OSD2 layer swap

    Set ide OSD1 & OSD2 layer swap enable or not

    @param[in] id       ide ID

    @return
		- @b TRUE: swap
		- @b FALSE: not swap
*/
BOOL idec_get_osd_layer_swap(IDE_ID id)
{
	T_IDE_BLEND2    ui_ide_blend2;

	if (id > IDE_ID_1) {
		return FALSE;
	}
	ui_ide_blend2.reg = idec_get_reg(id, IDE_BLEND2_OFS);

	return ui_ide_blend2.bit.osd_1_2_swap == 1 ? TRUE : FALSE;
}
#endif