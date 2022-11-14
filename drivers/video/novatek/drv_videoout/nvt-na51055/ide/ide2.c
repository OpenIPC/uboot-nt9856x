/*
    Library for IDE2 regiseter control

    This is low level control library for ide display.

    @file       ide2.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#include "./include/ide_reg.h"
#include "./include/ide2_int.h"
//#include "Limits.h"

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
    @name ide Control
*/
//@{


static void generate_line_params(struct struct_point *p_src, struct struct_point *p_dst,
								 struct struct_point *p_test_point,
								 INT32 *p_param_a, INT32 *p_param_b, INT32 *p_param_c, UINT32 *p_compare)
{
	*p_param_a = p_dst->y - p_src->y;
	*p_param_b = p_src->x - p_dst->x;
	*p_param_c = p_src->x * p_dst->y - p_dst->x * p_src->y;

	// Force param C >= 0
	if (*p_param_c < 0) {
		*p_param_a *= -1;
		*p_param_b *= -1;
		*p_param_c *= -1;
	}

	DBG_IND("comparative 0x%x\r\n", (unsigned int)g_ide_line_comparative);
	switch (g_ide_line_comparative) {
	case IDE_LINE_COMPARATIVE_GTLT:
		if (((*p_param_a)*p_test_point->x + (*p_param_b)*p_test_point->y) > (*p_param_c)) {
			*p_compare = 2;
		} else {
			*p_compare = 3;
		}
		break;
	case IDE_LINE_COMPARATIVE_GTLE:
		if (((*p_param_a)*p_test_point->x + (*p_param_b)*p_test_point->y) > (*p_param_c)) {
			*p_compare = 2;
		} else {
			*p_compare = 1;
		}
		break;
	case IDE_LINE_COMPARATIVE_GELT:
		if (((*p_param_a)*p_test_point->x + (*p_param_b)*p_test_point->y) >= (*p_param_c)) {
			*p_compare = 0;
		} else {
			*p_compare = 3;
		}
		break;
	case IDE_LINE_COMPARATIVE_GELE:
		if (((*p_param_a)*p_test_point->x + (*p_param_b)*p_test_point->y) >= (*p_param_c)) {
			*p_compare = 0;
		} else {
			*p_compare = 1;
		}
		break;
	}
	DBG_IND("Result 0x%x\r\n", (unsigned int)*p_compare);

}

/*
static UINT32 invert_vcov_comparative(UINT32 uiCompare)
{
	switch (uiCompare)
	{
	case 0:         // >=
	    return 1;
	case 1:         // <=
	    return 0;
	case 2:         // >
	    return 3;
	case 3:         // <
	default:
	    return 2;
	}
}
*/

// caculate center point between point queue
static void get_center_point(struct struct_point *p_point, struct struct_point *p_center)
{
	INT32 i_count;
	struct struct_point *p_queue = p_point;

	if (p_center == NULL) {
		return;
	}
	if (p_point == NULL) {
		return;
	}

	i_count = 0;
	p_center->x = 0;
	p_center->y = 0;
	while (1) {
		i_count++;

		p_center->x += p_queue->x;
		p_center->y += p_queue->y;

		if (p_queue->p_next == p_point) {
			break;
		}

		p_queue = p_queue->p_next;
	}

	p_center->x = (p_center->x + (i_count / 2)) / i_count;
	p_center->y = (p_center->y + (i_count / 2)) / i_count;

	return;
}
/*
static INT32 idec_get_region_width(STRUCT_POINT *pPoints, INT32 *pMinX)
{
	INT32 min_x = LONG_MAX;
	INT32 max_x = LONG_MIN;
	STRUCT_POINT *pStart;

	pStart = pPoints;

	while (1)
	{
	    if (pPoints->x > max_x) max_x = pPoints->x;
	    if (pPoints->x < min_x) min_x = pPoints->x;

	    if (pPoints->p_next == pStart)
	    {
			break;
	    }
	    pPoints = pPoints->p_next;
	}

	*pMinX = min_x;

	return max_x - min_x + 1;
}
*/
/**
	Set IDE RGBD swap

	Swap dummy at the front or the end.

	@param[in] id	IDE ID
	@param[in] b_en
		- @b TRUE:	Dither ON.
		- @b FALSE: Dither OFF.

	@return void
*/
void idec_set_rgbd_swap(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.rgbd_swap = 1;
	} else {
		reg.bit.rgbd_swap = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide dithering

    Enable/Disable the dithering function and control the dithering freerun mode.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:  Dither ON.
		- @b FALSE: Dither OFF.
    @param[in] b_freerun
		- @b TRUE:  Dither pattern freerun.
		- @b FALSE: Dither pattern reset at vsync edge.

    @return void
*/
void idec_set_dithering(IDE_ID id, BOOL b_en, BOOL b_freerun)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.dither_en = 1;
	} else {
		reg.bit.dither_en = 0;
	}

	if (b_freerun) {
		reg.bit.dither_freerun = 1;
	} else {
		reg.bit.dither_freerun = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);

}

/**
    Set ide display device.

    select ide display output device format.

    @param[in] id   ide ID
    @param uidevice the display device for display
			- @b DISPLAY_DEVICE_CASIO2G: CASIO2G
			- @b DISPLAY_DEVICE_AU:AU
			- @b DISPLAY_DEVICE_TOPPOLY:TOPPOLY
			- @b DISPLAY_DEVICE_CCIR656:CCIR656
			- @b DISPLAY_DEVICE_CCIR601:CCIR601
			- @b DISPLAY_DEVICE_TV:TV ENCODER
			- @b DISPLAY_DEVICE_HDMI_24BIT:HDMI 24bit
			- @b DISPLAY_DEVICE_HDMI_16BIT:HDMI 16bit
			- @b DISPALY_DEVICE_PARALLEL:Parallel
			- @b DISPLAY_DEVICE_CCIR656_16BIT:CCIR656 16bit
			- @b DISPLAY_DEVICE_CCIR601_16BIT:CCIR601 16bit
			- @b DISPLAY_DEVICE_MI:Memory Interface
			- @b DISPLAY_DEVICE_MIPIDSI:MIPI DSI
			- @b DISPLAY_DEVICE_RGB_16BIT:RGB 16bit

    @return void
*/
void idec_set_device(IDE_ID id, IDE_DEVICE_TYPE uidevice)
{
	T_IDE_CTRL reg;

	//coverity[unsigned_compare]
	ide_chk_range(uidevice, DISPLAY_DEVICE_CASIO2G, DISPLAY_DEVICE_OUTPUT_DRAM);

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.disp_dev = uidevice;

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}


/**
    Set ide output to dram's format

    Set ide output to dram's format during dram out path

    @param[in]  id          ide ID
    @param[in]  is_yuv422   yuv422 or 420
		- @b FALSE  : 420
		- @b TRUE   : 422

    @return void
*/
void idec_set_dram_out_format(IDE_ID id, BOOL is_yuv422)
{
	T_IDE_CTRL reg;

	if (id == IDE_ID_2) {
		return;
	}
	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.out_dram_fmt = is_yuv422;

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}
/**
    Set ide pixel order.

    select LCD output pixel order.
    This is valid only for Serial RGB display device. (Casio2G / AU)

    @param[in] id   ide ID
    @param[in] b_pdir
		- @b IDE_PDIR_RGB: RGB (R..G..B.. R..G..B..)
		- @b IDE_PDIR_RBG: RBG (R..B..G.. R..B..G..)

    @return void
*/
void idec_set_pdir(IDE_ID id, IDE_PDIR b_pdir)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_pdir) {
		reg.bit.p_dir = 1;
	} else {
		reg.bit.p_dir = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide odd line begin color.

    This is valid only for Serial RGB display device. (Casio2G / AU)

    @param[in] id   ide ID
    @param[in] ui_odd
		- @b IDE_LCD_R:    Begin from R
		- @b IDE_LCD_G:    Begin from G
		- @b IDE_LCD_B:    Begin from B

    @return void
*/
void idec_set_odd(IDE_ID id, IDE_PORDER ui_odd)
{
	T_IDE_CTRL reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_odd, IDE_LCD_R, IDE_LCD_B);

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.lcd_odd = ui_odd;

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide even line begin color.

    This is valid only for Serial RGB display device. (Casio2G / AU)

    @param[in] id   ide ID
    @param[in] ui_even
		- @b IDE_LCD_R:    Begin from R
		- @b IDE_LCD_G:    Begin from G
		- @b IDE_LCD_B:    Begin from B

    @return void
*/
void idec_set_even(IDE_ID id, IDE_PORDER ui_even)
{
	T_IDE_CTRL reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_even, IDE_LCD_R, IDE_LCD_B);

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.lcd_even = ui_even;

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide output hsync active polarity.

    Set ide output hsync active polarity.

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Active LOW.
		- @b FALSE: Actice HIGH.

    @return void
*/
void idec_set_hs_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.hs_inv = 1;
	} else {
		reg.bit.hs_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide output vsync active polarity

    Set ide output vsync active polarity

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Active LOW.
		- @b FALSE: Actice HIGH.

    @return void
*/
void idec_set_vs_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.vs_inv = 1;
	} else {
		reg.bit.vs_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide output horizontal valid signal polarity.

    Set ide output horizontal valid signal polarity.

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Active LOW.
		- @b FALSE: Actice HIGH.

    @return void
*/
void idec_set_hvld_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.hvld_inv = 1;
	} else {
		reg.bit.hvld_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide output vertical valid signal polarity.

    Set ide output vertical valid signal polarity.

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Active LOW.
		- @b FALSE: Actice HIGH.

    @return void
*/
void idec_set_vvld_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.vvld_inv = 1;
	} else {
		reg.bit.vvld_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide clk signal polarity.

    Set ide clk signal polarity.

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Invert. (Active LOW)
		- @b FALSE: Not invert. (Active HIGH)

    @return void
*/
void idec_set_clk_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.clk_inv = 1;
	} else {
		reg.bit.clk_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide field signal polarity.

    Set ide field signal polarity.

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Invert. (Active LOW)
		- @b FALSE: Not invert. (Active HIGH)

    @return void
*/
void idec_set_fld_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.fld_inv = 1;
	} else {
		reg.bit.fld_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide data enable signal polarity.

    Set ide data enable signal polarity.

    @param[in] id   ide ID
    @param[in] b_inv
		- @b TRUE:  Invert. (Active LOW)
		- @b FALSE: Not invert. (Active HIGH)

    @return void
*/
void idec_set_de_inv(IDE_ID id, BOOL b_inv)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_inv) {
		reg.bit.de_inv = 1;
	} else {
		reg.bit.de_inv = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide RGBDummy mode select.

    Set ide RGBDummy mode select.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:  select RGB Dummy.
		- @b FALSE: Not select RGB Dummy.

    @return void
*/
void idec_set_rgbd(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.rgbd = 1;
	} else {
		reg.bit.rgbd = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    ide Through mode select.

    ide Through mode select.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:  Enable through mode.
		- @b FALSE: Disable through mode.

    @return void
*/
void idec_set_through(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.through = 1;
	} else {
		reg.bit.through = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide HDMI DDR mode select.

    Enable/Disable ide Output bus DDR mode.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:     Enable HDMI DDR mode
		- @b FALSE:    Disable HDMI DDR mode.

    @return void
*/
void idec_set_hdmi_ddr(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.hdmi_ddr = 1;
	} else {
		reg.bit.hdmi_ddr = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Enable/Disable ide.

    ide module global Enable/Disable.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:  Enable.
		- @b FALSE: Disable.

    @return void
*/
void idec_set_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.ide_en = 1;
	} else {
		reg.bit.ide_en = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set Load to ide.

    Most of the registers are actually loaded into ide controller at VSync time.  By triggering
    load, the modified VSync latched registers are loaded at VSync time instead of
    loaded instantly.

    @param[in] id   ide ID

    @return void
*/
void idec_set_load(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.load = 1;

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set Clock 1/2

    ide Clock 1/2 Enable/Disable.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:  Enable.
		- @b FALSE: Disable.

*/
void idec_set_clk1_2(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (b_en) {
		reg.bit.clk_1_2 = 1;
	} else {
		reg.bit.clk_1_2 = 0;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}


/**
    Set number of display device.

    Set number of display device.

    @param[in] id   ide ID
    @param[in] b_no
		- TRUE:  LCD + TV.
		- FALSE: LCD or TV only.

    @return void
    @note It is not used in NT96650
*/
void idec_set_dev_no(IDE_ID id, BOOL b_no)
{
	/*    T_IDE_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	    if(b_no)
			reg.bit.devno = 1;
	    else
			reg.bit.devno = 0;

	    idec_set_reg(id, IDE_CTRL_OFS, reg.reg);*/
}

/**
    Enable/Disable horizontal low pass filter.

    Enable/Disable horizontal low pass filter.

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:  Enable.
		- @b FALSE: Disable.

    @return void
    @note It is not used in NT96650
*/
void idec_set_hlpf_en(IDE_ID id, BOOL b_en)
{
	/*    T_IDE_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	    if(b_en)
			reg.bit.hlpf_en = 1;
	    else
			reg.bit.hlpf_en = 0;

	    idec_set_reg(id, IDE_CTRL_OFS, reg.reg);*/
}

/**
    Get Enable/disable horizontal low pass filter.

    Get Enable/disable horizontal low pass filter.

    @param[in] id   ide ID

    @return
		- @b FALSE:   Disabled
		- @b TRUE:    Enabled
*/
BOOL idec_get_hlpf_en(IDE_ID id)
{
	/*    T_IDE_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	    return reg.bit.hlpf_en;*/
	return FALSE;
}

/**
    Get ide status.

    Get ide Enable/Disable status.

    @param[in] id   ide ID
    @return
		- @b TRUE: enable
		- @b FALSE: disable
*/
BOOL idec_get_en(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.ide_en;
}

/**
    Get Load of ide.

    Return current status of load bit.  If no VSync occurs after the time the load bit is set,
    the load bit will not be cleared.

    @param[in] id   ide ID
    @return
		- @b FALSE: load
		- @b TRUE: not load.
*/
BOOL idec_get_load(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.load;
}

/**
    Get ide display device.

    Get ide display device.

    @param[in] id   ide ID
    @return display device id
*/
IDE_DEVICE_TYPE idec_get_device(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	/*if(reg.bit.devno == 1)
	    reg.bit.disp_dev = DISPLAY_DEVICE_TV;*/

	return reg.bit.disp_dev;
}

/**
    Get ide through mode select.

    Get ide through mode select.

    @param[in] id   ide ID
    @return
		- @b FALSE:through mode disabled
		- @b TRUE:through mode enabled
*/
BOOL idec_get_through(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.through;
}

/**
    Get ide RGBDummy mode select.

    Get ide RGBDummy mode select.

    @param[in] id   ide ID
    @return
		- @b FALSE:not RGBDummy mode
		- @b TRUE:RGBDummy mode
*/
BOOL idec_get_rgbd(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	return reg.bit.rgbd;
}


/**
    Get ide window Enable/Disable

    Get ide window Enable/Disable

    @param[in] id   ide ID
    @return Return combination of DISPLAY_*_EN (v1/v2/osd1/osd2 enable/disalbe)
*/
UINT32 idec_get_window_en(IDE_ID id)
{
	UINT32 ui_win_en = 0;
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	if (reg.bit.o1_en) {
		ui_win_en |= DISPLAY_OSD1_EN;
	}
#if IDE1_OSD2_EXIST
	if (reg.bit.o2_en) {
		ui_win_en |= DISPLAY_OSD2_EN;
	}
#endif
	if (reg.bit.v1_en) {
		ui_win_en |= DISPLAY_VIDEO1_EN;
	}
	if (reg.bit.v2_en) {
		ui_win_en |= DISPLAY_VIDEO2_EN;
	}

	return ui_win_en;
}

/**
    Set ide all window Disable

    Set ide all window Disable

    @param[in] id   ide ID
    @return void
*/
void idec_set_all_window_dis(IDE_ID id)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.o1_en = 0;
#if IDE1_OSD2_EXIST
	reg.bit.o2_en = 0;
#endif
	reg.bit.v1_en = 0;
	reg.bit.v2_en = 0;

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/**
    Set ide window enable

    Set ide window enable

    @param[in] id   ide ID
    @param[in] ui_wins   Combination of DISPLAY_xx_EN

    @return void
*/
void idec_set_all_window_en(IDE_ID id, UINT32 ui_wins)
{
	T_IDE_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_CTRL_OFS);

	reg.bit.o1_en = 0;
#if IDE1_OSD2_EXIST
	reg.bit.o2_en = 0;
#endif
	reg.bit.v1_en = 0;
	reg.bit.v2_en = 0;

	if (ui_wins & DISPLAY_OSD1_EN) {
		reg.bit.o1_en = 1;
	}
#if IDE1_OSD2_EXIST
	if (ui_wins & DISPLAY_OSD2_EN) {
		reg.bit.o2_en = 1;
	}
#endif
	if (ui_wins & DISPLAY_VIDEO1_EN) {
		reg.bit.v1_en = 1;
	}
	if (ui_wins & DISPLAY_VIDEO2_EN) {
		reg.bit.v2_en = 1;
	}

	idec_set_reg(id, IDE_CTRL_OFS, reg.reg);
}

/*
    Set ide dimension convert.

    This function will be blocked until dimension convert is completed.

    @param[in] id   ide ID

    @return None.
*/
/*
void idec_setDimCvert(IDE_ID id)
{
    T_IDE_DIM_CVERT reg;

    reg.reg = idec_get_reg(id, IDE_DIM_CVERT_OFS);

    reg.bit.cvert_en = 1;
    idec_set_reg(id, IDE_DIM_CVERT_OFS, reg.reg);

    if(ide_get_en())
    {
		while(1)
		{
			reg.reg = idec_get_reg(id, IDE_DIM_CVERT_OFS);
			if(reg.bit.cvert_en == 0)
			break;
		}
    }
}
*/

/*
    Set ide convert factor.

    @param[in] id   ide ID
    @param uiHFT  output horizontal dimension convert factor.(output/original).
    @param uiVFT  output vertical dimension convert factor.(output/original).

    @return None.
*/
/*
void idec_setCvertFactor(IDE_ID id, UINT32 uiHFT, UINT32 uiVFT)
{
    T_IDE_DIM_CVERT reg;

    reg.reg = idec_get_reg(id, IDE_DIM_CVERT_OFS);
    reg.bit.cvert_hft = uiHFT;
    reg.bit.cvert_vft = uiVFT;

    idec_set_reg(id, IDE_DIM_CVERT_OFS, reg.reg);
}
*/

/**
    Set OSD palette0 read/write access

    @param[in] id   ide ID
    @param[in] b_sel
		- @b FALSE: Enable ide access palette 0
		- @b TRUE:  Enable CPU access palette 0

    @return void
*/
void idec_set_pal0_rw(IDE_ID id, BOOL b_sel)
{
	UINT32 i;
	T_IDE_OSD_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_OSD_CTRL_OFS);

	if (b_sel) {
		reg.bit.pal0_rw = 1;
	} else {
		reg.bit.pal0_rw = 0;
	}

	idec_set_reg(id, IDE_OSD_CTRL_OFS, reg.reg);

	//SRAM access should delay at least 80ns: Add dummy read.
	for (i = 0; i < 10; i++) {
		reg.reg = idec_get_reg(id, IDE_OSD_CTRL_OFS);
	}
}

/**
    Set OSD palette1 read/write access

    @param[in] id   ide ID
    @param[in] b_sel
		- @b FALSE: Enable ide access palette 1
		- @b TRUE:  Enable CPU access palette 1

    @return void
*/
//void idec_set_pal1_rw(IDE_ID id, BOOL b_sel)
//{
#if 0
UINT32 i;
T_IDE_OSD_CTRL reg;

reg.reg = idec_get_reg(id, IDE_OSD_CTRL_OFS);

if (b_sel) {
	reg.bit.pal1_rw = 1;
} else {
	reg.bit.pal1_rw = 0;
}

idec_set_reg(id, IDE_OSD_CTRL_OFS, reg.reg);

//SRAM access should delay at least 80ns: Add dummy read.
for (i = 0; i < 5; i++) {
	reg.reg = idec_get_reg(id, IDE_OSD_CTRL_OFS);
}
#endif
//}

/**
    Get OSD palette0 read/write access

    Get OSD palette0 read/write access

    @param[in] id   ide ID
    @return
		- @b FALSE:   ide access is enabled
		- @b TRUE:    CPU access is enabled
*/
BOOL idec_get_pal0_rw(IDE_ID id)
{
	T_IDE_OSD_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_OSD_CTRL_OFS);

	return reg.bit.pal0_rw;
}

#if 0
/**
    Get OSD palette1 read/write access

    Get OSD palette1 read/write access

    @param[in] id   ide ID
    @return
      - @b FALSE:   ide access is enabled
      - @b TRUE:    CPU access is enabled
*/
BOOL idec_get_pal_rw(IDE_ID id)
{
	T_IDE_OSD_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_OSD_CTRL_OFS);

	return reg.bit.pal1_rw;
}
#endif

/**
    Set the color and blending operation of the specific entry of OSD palette.

    Set the color and blending operation of the specific entry of OSD palette.

    @param[in] id   ide ID
    @param[in] ui_entry      Osd palette entry(0~511)
    @param[in] ui_color_y     color Y of specific osd entry.
    @param[in] ui_color_cb    color CB of specific osd entry.
    @param[in] ui_color_cr    color CR of specific osd entry.
    @param[in] ui_alpha      value of specific osd entry.

    @return void
*/
void idec_set_pal_entry(IDE_ID id, UINT32 ui_entry, UINT8 ui_color_y, UINT8 ui_color_cb, UINT8 ui_color_cr, UINT8 ui_alpha)
{
	UINT32 ofs;
	T_IDE_PAL reg;

	ofs = IDE_PAL0_OFS + (ui_entry << 2);

	reg.reg = idec_get_reg(id, ofs);

	reg.bit.y = ui_color_y;
	reg.bit.cb = ui_color_cb;
	reg.bit.cr = ui_color_cr;
	reg.bit.alpha = ui_alpha;
	//#NT#Palette or shadow palette access right use same bit
#if 0
	if (ui_entry < IDE_PAL_NUM) {
		idec_set_pal0_rw(id, 1);
	} else
		//idec_set_pal1_rw(id, 1);
	{
		DBG_WRN("Wrong palette ui_entry[%03d]\r\n", ui_entry);
	}
#endif

	idec_set_pal0_rw(id, 1);

	idec_set_reg(id, ofs, reg.reg);

	idec_set_pal0_rw(id, 0);
	//#NT#Palette or shadow palette access right use same bit
#if 0
	if (ui_entry < IDE_PAL_NUM) {
		idec_set_pal0_rw(id, 0);
	} else
//      idec_set_pal1_rw(id, 0);
	{
		DBG_WRN("Wrong palette ui_entry[%03d]\r\n", ui_entry);
	}
#endif
}

/**
    Get the color and blending operation of the specific entry of OSD palette.

    Get the color and blending operation of the specific entry of OSD palette.

    @param[in] id   ide ID
    @param[in]  ui_entry     Osd palette entry(0~511)
    @param[out] ui_color_y    color Y of specific osd entry.
    @param[out] ui_color_cb   color CB of specific osd entry.
    @param[out] ui_color_cr   color CR of specific osd entry.
    @param[out] ui_alpha     value of specific osd entry.

    @return void
*/
void idec_get_pal_entry(IDE_ID id, UINT32 ui_entry, UINT8 *ui_color_y, UINT8 *ui_color_cb, UINT8 *ui_color_cr, UINT8 *ui_alpha)
{
	UINT32 ofs;
	T_IDE_PAL reg;

	ofs = IDE_PAL0_OFS + (ui_entry << 2);

	//#NT#Palette or shadow palette access right use same bit
#if 0
	if (ui_entry < IDE_PAL_NUM) {
		idec_set_pal0_rw(id, 1);
	} else
//      idec_set_pal1_rw(id, 1);
	{
		DBG_WRN("Wrong palette entry[%03d]\r\n", ui_entry);
	}
#endif
	idec_set_pal0_rw(id, 1);

	reg.reg = idec_get_reg(id, ofs);

	idec_set_pal0_rw(id, 0);

	//#NT#Palette or shadow palette access right use same bit
#if 0
	if (ui_entry < IDE_PAL_NUM) {
		idec_set_pal0_rw(id, 0);
	} else
//      idec_set_pal1_rw(id, 0);
	{
		DBG_WRN("Wrong palette entry[%03d]\r\n", ui_entry);
	}
#endif

	*ui_color_y = reg.bit.y;
	*ui_color_cb = reg.bit.cb;
	*ui_color_cr = reg.bit.cr;
	*ui_alpha = reg.bit.alpha;
}

/**
    Set the color and blending operation of the specific entry of OSD palette.

    Set the color and blending operation of the specific entry of OSD palette.
    This is simplified version of ide_set_pal_entry() without turn on/off the palette access.
    User must be call ide_setPalW() to enable/disable the palette access before and after this api.
    This API is chip version design dependent, user should care the parameter ui_palette's format.

    @param[in] id   ide ID
    @param[in] ui_entry      Osd palette entry(0~511)
    @param[in] ui_palette    The palette content.
		 - ui_palette[7~0]:      Cr value of the palette.
		 - ui_palette[15~8]:     Cb value of the palette.
		 - ui_palette[23~16]:    Y value of the palette.
		 - ui_palette[31~24]:    Alpha value of the palette.

    @return void
*/
void idec_set_pal(IDE_ID id, UINT32 ui_entry, UINT32 ui_palette)
{
	UINT32 ofs;

	ofs = IDE_PAL0_OFS + (ui_entry << 2);

	idec_set_reg(id, ofs, ui_palette);
}


/**
    Get the color and blending operation of the specific entry of OSD palette.

    Get the color and blending operation of the specific entry of OSD palette.
    This is simplified version of ide_get_pal_entry() without turn on/off the palette access.
    User must be call ide_setPalR() to enable/disable the palette access before and after this api.
    This API is chip version design dependent, user should care the parameter ui_palette's format.

    @param[in] id   ide ID
    @param[in] ui_entry      OSD palette entry(0~511)
    @param[out] ui_palette    The returned palette content.
		 - ui_palette[7~0]:      Cr value of the palette.
		 - ui_palette[15~8]:     Cb value of the palette.
		 - ui_palette[23~16]:    Y value of the palette.
		 - ui_palette[31~24]:    Alpha value of the palette.

    @return void
*/
void idec_get_pal(IDE_ID id, UINT32 ui_entry, UINT32 *ui_palette)
{
	UINT32 ofs;

	ofs = IDE_PAL0_OFS + (ui_entry << 2);

	*ui_palette = idec_get_reg(id, ofs);
}
/**
    Set ide background color.

    Set ide background color.

    @param[in] id   ide ID
    @param[in] ui_color_y     Y color.
    @param[in] ui_color_cb    CB color.
    @param[in] ui_color_cr    CR color.

    @return void
*/
void idec_set_background(IDE_ID id, UINT8 ui_color_y, UINT8 ui_color_cb, UINT8 ui_color_cr)
{
	T_IDE_BG reg;

	reg.reg = idec_get_reg(id, IDE_BG_OFS);

	reg.bit.y = ui_color_y;
	reg.bit.cb = ui_color_cb;
	reg.bit.cr = ui_color_cr;

	idec_set_reg(id, IDE_BG_OFS, reg.reg);
}

/**
    Get ide background color.

    Get ide background color.

    @param[in] id   ide ID
    @param[in] ui_color_y     Y color.
    @param[in] ui_color_cb    CB color.
    @param[in] ui_color_cr    CR color.

    @return void
*/
void idec_get_background(IDE_ID id, UINT8 *ui_color_y, UINT8 *ui_color_cb, UINT8 *ui_color_cr)
{
	T_IDE_BG reg;

	reg.reg = idec_get_reg(id, IDE_BG_OFS);

	*ui_color_y = reg.bit.y;
	*ui_color_cb = reg.bit.cb;
	*ui_color_cr = reg.bit.cr;
}



/**
    Set Subpixel interpolation enable/disable

    Set Subpixel interpolation enable/disable for Odd or Even lines

    @param[in] id   ide ID
    @param[in] b_odd_line
		- @b TRUE:  select Odd lines.
		- @b FALSE: select Even lines.
    @param[in] b_r
		- @b TRUE:  Enable R component subpixel interpolation.
		- @b FALSE: Disable R component subpixel interpolation.
    @param[in] b_g
		- @b TRUE:  Enable G component subpixel interpolation.
		- @b FALSE: Disable G component subpixel interpolation.
    @param[in] b_b
		- @b TRUE:  Enable B component subpixel interpolation.
		- @b FALSE: Disable B component subpixel interpolation.

    @return void
*/
void idec_set_subpixel(IDE_ID id, BOOL b_odd_line, BOOL b_r, BOOL b_g, BOOL b_b)
{
	T_IDE_SUBPIXEL  reg;


	reg.reg = idec_get_reg(id, IDE_SUBPIXEL_OFS);
	if (b_odd_line) {
		reg.bit.odd_r = b_r;
		reg.bit.odd_g = b_g;
		reg.bit.odd_b = b_b;
	} else {
		reg.bit.even_r = b_r;
		reg.bit.even_g = b_g;
		reg.bit.even_b = b_b;
	}
	idec_set_reg(id, IDE_SUBPIXEL_OFS, reg.reg);
}

/**
    Get Subpixel interpolation enable/disable

    Get Subpixel interpolation enable/disable for Odd or Even lines

    @param[in] id   ide ID
    @param[in] b_odd_line
		- @b TRUE:  select Odd lines.
		- @b FALSE: select Even lines.
    @param[out] b_r
		- @b TRUE:  Enable R component subpixel interpolation.
		- @b FALSE: Disable R component subpixel interpolation.
    @param[out] b_g
		- @b TRUE:  Enable G component subpixel interpolation.
		- @b FALSE: Disable G component subpixel interpolation.
    @param[out] b_b
		- @b TRUE:  Enable B component subpixel interpolation.
		- @b FALSE: Disable B component subpixel interpolation.

    @return void
*/
void idec_get_subpixel(IDE_ID id, BOOL b_odd_line, BOOL *b_r, BOOL *b_g, BOOL *b_b)
{
	T_IDE_SUBPIXEL  reg;


	reg.reg = idec_get_reg(id, IDE_SUBPIXEL_OFS);
	if (b_odd_line) {
		*b_r = reg.bit.odd_r;
		*b_g = reg.bit.odd_g;
		*b_b = reg.bit.odd_b;
	} else {
		*b_r = reg.bit.even_r;
		*b_g = reg.bit.even_g;
		*b_b = reg.bit.even_b;
	}
}

/*
    Set ide sif sync.

    @param[in] id   ide ID
    @param uiSIFst SIF valid signal start line count.
    @param uiSIFed SIF valid signal end line count.

    @return None.
*/
/*
void idec_setSifStartEnd(IDE_ID id, UINT32 uiSIFst, UINT32 uiSIFed)
{
    T_IDE_SIF reg;

    reg.reg = idec_get_reg(id, IDE_SIF_OFS);

    reg.bit.sif_enst = uiSIFst;
    reg.bit.sif_ened = uiSIFed;

    idec_set_reg(id, IDE_SIF_OFS, reg.reg);
}
*/

//@}

/**
@name ide Timing
*/
//@{

/**
    Set ide horizontal timing control.

    Set ide horizontal timing control.

    @param[in] id   ide ID
    @param[in] ui_hsynct     Horizontal sync time.
    @param[in] ui_htotal     Horizontal total time.
    @param[in] ui_hvalidst   Horizontal valid start time.
    @param[in] ui_hvalided   Horizontal valid end time.

    @return void
*/
void idec_set_hor_timing(IDE_ID id, UINT32 ui_hsynct, UINT32 ui_htotal, UINT32 ui_hvalidst, UINT32 ui_hvalided)
{
	T_IDE_TIMING1 reg1;
	T_IDE_TIMING2 reg2;

	reg1.reg = idec_get_reg(id, IDE_TIMING1_OFS);
	reg1.bit.hsynct = ui_hsynct;
	reg1.bit.htotal = ui_htotal;
	idec_set_reg(id, IDE_TIMING1_OFS, reg1.reg);

	reg2.reg = idec_get_reg(id, IDE_TIMING2_OFS);
	reg2.bit.hvldst = ui_hvalidst;
	reg2.bit.hvlded = ui_hvalided;
	idec_set_reg(id, IDE_TIMING2_OFS, reg2.reg);
}

/**
    Set ide vertical timing control.

    Set ide vertical timing control.

    @param[in] id   ide ID
    @param[in] ui_vsynct         vertical sync time.
    @param[in] ui_vtotal         vertical total time.
    @param[in] ui_odd_vvalidst    ODD field vertical valid start time.
    @param[in] ui_odd_vvalided    ODD field vertical valid end time.
    @param[in] ui_even_vvalidst   EVEN field vertical valid start time.
    @param[in] ui_even_vvalided   EVNE field vertical valid end time.

    @return void
*/
void idec_set_ver_timing(IDE_ID id, UINT32 ui_vsynct, UINT32 ui_vtotal, UINT32 ui_odd_vvalidst, UINT32 ui_odd_vvalided, UINT32 ui_even_vvalidst, UINT32 ui_even_vvalided)
{
	T_IDE_TIMING3 reg3;
	T_IDE_TIMING4 reg4;
	T_IDE_TIMING5 reg5;

	reg3.reg = idec_get_reg(id, IDE_TIMING3_OFS);
	reg3.bit.vsynct = ui_vsynct;
	reg3.bit.vtotal = ui_vtotal;
	idec_set_reg(id, IDE_TIMING3_OFS, reg3.reg);

	reg4.reg = idec_get_reg(id, IDE_TIMING4_OFS);
	reg4.bit.oddvvldst = ui_odd_vvalidst;
	reg4.bit.oddvvlded = ui_odd_vvalided;
	idec_set_reg(id, IDE_TIMING4_OFS, reg4.reg);

	reg5.reg = idec_get_reg(id, IDE_TIMING5_OFS);
	reg5.bit.evenvvldst = ui_even_vvalidst;
	reg5.bit.evenvvlded = ui_even_vvalided;
	idec_set_reg(id, IDE_TIMING5_OFS, reg5.reg);
}


/**
    Get ide horizontal timing control.

    Get the ide output interface horizontal timing control values.

    @param[in] id   ide ID
    @param[out] pui_hsynct    Horizontal sync time.
    @param[out] pui_htotal    Horizontal total time.
    @param[out] pui_hvalidst  Horizontal valid start time.
    @param[out] pui_hvalided  Horizontal valid end time.

    @return void
*/
void idec_get_hor_timing(IDE_ID id, UINT32 *pui_hsynct, UINT32 *pui_htotal, UINT32 *pui_hvalidst, UINT32 *pui_hvalided)
{
	T_IDE_TIMING1 reg1;
	T_IDE_TIMING2 reg2;

	reg1.reg = idec_get_reg(id, IDE_TIMING1_OFS);
	*pui_hsynct = reg1.bit.hsynct;
	*pui_htotal = reg1.bit.htotal;

	reg2.reg = idec_get_reg(id, IDE_TIMING2_OFS);
	*pui_hvalidst = reg2.bit.hvldst;
	*pui_hvalided = reg2.bit.hvlded;

}

/**
    Get ide vertical timing control.

    Get the ide output interface vertical timing control values.

    @param[in] id   ide ID
    @param[out] pui_vsynct        Vertical sync time.
    @param[out] pui_vtotal        Vertical total time.
    @param[out] pui_odd_vvalidst   ODD field vertical valid start time.
    @param[out] pui_odd_vvalided   ODD field vertical valid end time.
    @param[out] pui_even_vvalidst  EVEN field vertical valid start time.
    @param[out] pui_even_vvalided  EVNE field vertical valid end time.

    @return void
*/
void idec_get_ver_timing(IDE_ID id, UINT32 *pui_vsynct, UINT32 *pui_vtotal, UINT32 *pui_odd_vvalidst, UINT32 *pui_odd_vvalided, UINT32 *pui_even_vvalidst, UINT32 *pui_even_vvalided)
{
	T_IDE_TIMING3 reg3;
	T_IDE_TIMING4 reg4;
	T_IDE_TIMING5 reg5;

	reg3.reg = idec_get_reg(id, IDE_TIMING3_OFS);
	*pui_vsynct = reg3.bit.vsynct;
	*pui_vtotal = reg3.bit.vtotal;

	reg4.reg = idec_get_reg(id, IDE_TIMING4_OFS);
	*pui_odd_vvalidst = reg4.bit.oddvvldst;
	*pui_odd_vvalided = reg4.bit.oddvvlded;

	reg5.reg = idec_get_reg(id, IDE_TIMING5_OFS);
	*pui_even_vvalidst = reg5.bit.evenvvldst;
	*pui_even_vvalided = reg5.bit.evenvvlded;
}

/**
    Set ide Interlace load mode

    Set ide Interlace load mode

    @param[in] id   ide ID
    @param[in] b2field
		- @b TRUE: one field load
		- @b FALSE: two field load

    @return void
*/
void idec_set_inter_load_mode(IDE_ID id, BOOL b2field)
{
	T_IDE_TIMING4 reg4;

	reg4.reg = idec_get_reg(id, IDE_TIMING4_OFS);

	reg4.bit.inter_load_mode = b2field;

	idec_set_reg(id, IDE_TIMING4_OFS, reg4.reg);
}

/**
    Set ide hsync, vsync delay time

    Set ide hsync, vsync delay time

    @param[in] id   ide ID
    @param[in] ui_hs_delay    Hsync delay time in ide clocks 0 ~ 1
    @param[in] ui_vs_delay    Vsync delay time in ide clocks 0 ~ 1

    @return void
*/
void idec_set_sync_delay(IDE_ID id, UINT8 ui_hs_delay, UINT8 ui_vs_delay)
{
	T_IDE_TIMING1 reg1;
	T_IDE_TIMING3 reg3;

	reg1.reg = idec_get_reg(id, IDE_TIMING1_OFS);
	reg1.bit.hsdelay = ui_hs_delay;
	idec_set_reg(id, IDE_TIMING1_OFS, reg1.reg);

	reg3.reg = idec_get_reg(id, IDE_TIMING3_OFS);
	reg3.bit.vsdelay = ui_vs_delay;
	idec_set_reg(id, IDE_TIMING3_OFS, reg3.reg);
}

/**
    Set ide display is progressive or interlace mode.

    Set ide display is progressive or interlace mode.

    @param[in] id   ide ID
    @param[in] b_inter
		- @b FALSE: Display progressive mode.
		- @b TRUE:  Display interlaced mode.

    @return void
*/
void idec_set_interlace(IDE_ID id, BOOL b_inter)
{
	T_IDE_TIMING5 reg;

	reg.reg = idec_get_reg(id, IDE_TIMING5_OFS);
	if (b_inter) {
		reg.bit.interlace = 1;
	} else {
		reg.bit.interlace = 0;
	}
	idec_set_reg(id, IDE_TIMING5_OFS, reg.reg);
}

/**
    Set ide start even/odd field.

    Set ide start even/odd field.

    @param[in] id   ide ID
    @param[in] b_oddst
		- @b FALSE: start with even field.
		- @b TRUE:  start with odd field.

    @return void
*/
void idec_set_start_field(IDE_ID id, BOOL b_oddst)
{
	/*    T_IDE_TIMING5 reg;

	    reg.reg = idec_get_reg(id, IDE_TIMING5_OFS);
	    if(b_oddst)
			reg.bit.odd_st = 1;
	    else
			reg.bit.odd_st = 0;
	    idec_set_reg(id, IDE_TIMING5_OFS, reg.reg);*/
}

/**
    Get ide display is interlace or not.

    Get ide display is interlace or not.

    @param[in] id   ide ID
    @return
		- @b FALSE:progressive
		- @b TRUE: interlaced.
*/
BOOL idec_get_interlace(IDE_ID id)
{
	T_IDE_TIMING5 reg;

	reg.reg = idec_get_reg(id, IDE_TIMING5_OFS);

	return reg.bit.interlace;
}

/**
    Get ide start even/odd field.

    Get ide start even/odd field.

    @param[in] id   ide ID
    @return
		- @b FALSE:start with even field.
		- @b TRUE: start with odd field.
*/
BOOL idec_get_start_field(IDE_ID id)
{
	/*    T_IDE_TIMING5 reg;

	    reg.reg = idec_get_reg(id, IDE_TIMING5_OFS);

	    return reg.bit.odd_st;*/
	return FALSE;
}

/**
    Get ide current filed.

    Get ide current filed.

    @param[in] id   ide ID
    @return
		- @b FALSE: odd field
		- @b TRUE: even field
*/
BOOL idec_get_cur_field(IDE_ID id)
{
	T_IDE_TIMING5 reg;

	reg.reg = idec_get_reg(id, IDE_TIMING5_OFS);

	return reg.bit.cdfld;
}

// Digital Video Data Code Timing
/**
    Set ide digital video data code timing for CCIR656

    Set ide digital video data code timing for CCIR656

    @param[in] id   ide ID
    @param[in] ui_codd_blk_st  odd field vertical blank start time
    @param[in] ui_codd_blk_ed  odd field vertical blank end time
    @param[in] ui_ceven_blk_st even field vertical blank start time
    @param[in] ui_ceven_blk_ed even field vertical blank end time
    @param[in] ui_cfid_st     F-digital field identification start time
    @param[in] ui_cfid_ed     F-digital field identification end time

    @return void
*/
void idec_set_digital_timing(IDE_ID id, UINT32 ui_codd_blk_st, UINT32 ui_codd_blk_ed, UINT32 ui_ceven_blk_st, UINT32 ui_ceven_blk_ed, UINT32 ui_cfid_st, UINT32 ui_cfid_ed)
{
	T_IDE_DIGI_TIMING1 reg1;
	T_IDE_DIGI_TIMING2 reg2;
	T_IDE_DIGI_TIMING3 reg3;

	reg1.reg = idec_get_reg(id, IDE_DIGI_TIMING1_OFS);
	reg2.reg = idec_get_reg(id, IDE_DIGI_TIMING2_OFS);
	reg3.reg = idec_get_reg(id, IDE_DIGI_TIMING3_OFS);

	reg1.bit.coddblkst = ui_codd_blk_st;
	reg1.bit.coddblked = ui_codd_blk_ed;
	reg2.bit.cevenblkst = ui_ceven_blk_st;
	reg2.bit.cevenblked = ui_ceven_blk_ed;
	reg3.bit.cfidst = ui_cfid_st;
	reg3.bit.cfided = ui_cfid_ed;

	idec_set_reg(id, IDE_DIGI_TIMING1_OFS, reg1.reg);
	idec_set_reg(id, IDE_DIGI_TIMING2_OFS, reg2.reg);
	idec_set_reg(id, IDE_DIGI_TIMING3_OFS, reg3.reg);
}

//@}

/**
@name ide CSB, Gamma Control
*/
//@{

/**
    Set ide gamma enable/disable

    Set ide gamma enable/disable

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE: Disable gamma
		- @b TRUE:  Enable gamma

    @return void
*/
void idec_set_gamma_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.gamma_en = 1;
	} else {
		reg.bit.gamma_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);
}

/*
    Fill Gamma table to gamma registers.

    Fill Gamma table to gamma registers.

    @param[in] id   ide ID
    @param[in] uipgamma     The pointer of the Gamma table with 17 elements.

    @return void
    @note It is not used in 96650
*/
void idec_fill_gamma(IDE_ID id, UINT8 *uipgamma)
{
	/*    UINT32 i;
	    T_IDE_GAMMA reg;

	    for(i=0; i<=IDE_GAMMA_LEVEL; i+=4)
	    {
			reg.reg = 0;
			reg.bit.gamma0 = uipgamma[i];
			if(i < IDE_GAMMA_LEVEL)
			{
				reg.bit.gamma1 = uipgamma[i+1];
				reg.bit.gamma2 = uipgamma[i+2];
				reg.bit.gamma3 = uipgamma[i+3];
			}

			idec_set_reg(id, IDE_GAMMA0_OFS + i, reg.reg);
	    }*/
}

/**
    Fill RGB Gamma table to gamma registers.

    Fill RGB Gamma table to gamma registers.

    @param[in] id   ide ID
    @param[in] uiprgamma     The pointer of the R Gamma table with 33 elements.
    @param[in] uipggamma     The pointer of the G Gamma table with 33 elements.
    @param[in] uipbgamma     The pointer of the B Gamma table with 33 elements.

    @return void
*/
void idec_fill_rgb_gamma(IDE_ID id, UINT8 *uiprgamma, UINT8 *uipggamma, UINT8 *uipbgamma)
{
	UINT32 i;
	T_IDE_GAMMA reg;

	for (i = 0; i <= IDE_GAMMA_LEVEL; i += 4) {

		if (uiprgamma != NULL) {
			reg.reg = 0;
			reg.bit.gamma0 = uiprgamma[i];
			if (i < IDE_GAMMA_LEVEL) {
				reg.bit.gamma1 = uiprgamma[i + 1];
				reg.bit.gamma2 = uiprgamma[i + 2];
				reg.bit.gamma3 = uiprgamma[i + 3];
			}
			idec_set_reg(id, IDE_RGAMMA0_OFS + i, reg.reg);
		}
		if (uipggamma != NULL) {
			reg.reg = 0;
			reg.bit.gamma0 = uipggamma[i];
			if (i < IDE_GAMMA_LEVEL) {
				reg.bit.gamma1 = uipggamma[i + 1];
				reg.bit.gamma2 = uipggamma[i + 2];
				reg.bit.gamma3 = uipggamma[i + 3];
			}
			idec_set_reg(id, IDE_GGAMMA0_OFS + i, reg.reg);
		}
		if (uipbgamma != NULL) {
			reg.reg = 0;
			reg.bit.gamma0 = uipbgamma[i];
			if (i < IDE_GAMMA_LEVEL) {
				reg.bit.gamma1 = uipbgamma[i + 1];
				reg.bit.gamma2 = uipbgamma[i + 2];
				reg.bit.gamma3 = uipbgamma[i + 3];
			}
			idec_set_reg(id, IDE_BGAMMA0_OFS + i, reg.reg);
		}

	}
}

/**
    Get ide gamma enable/disable

    Get ide gamma enable/disable

    @param[in] id   ide ID
    @return
		- @b FALSE:disable gamma
		- @b TRUE:enable gamma
*/
BOOL idec_get_gamma_en(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	return reg.bit.gamma_en;
}



/*
    Get Gamma table from gamma registers.

    Get Gamma table from gamma registers.

    @param[in] id   ide ID
    @param[out] uipgamma The pointer of the Gamma table with 33 elements.

    @return void.
    @note It is not used in 96650
*/
void idec_get_gamma(IDE_ID id, UINT8 *uipgamma)
{
	/*    UINT32 i;
	    T_IDE_GAMMA reg;

	    for(i=0; i<=IDE_GAMMA_LEVEL; i+=4)
	    {
			reg.reg = idec_get_reg(id, IDE_GAMMA0_OFS + i);

			uipgamma[i] = reg.bit.gamma0;

			if(i < IDE_GAMMA_LEVEL)
			{
				uipgamma[i+1] = reg.bit.gamma1;
				uipgamma[i+2] = reg.bit.gamma2;
				uipgamma[i+3] = reg.bit.gamma3;
			}
	    }*/
}

/**
    Get Gamma table from gamma registers.

    Get Gamma table from gamma registers.

    @param[in] id   ide ID
    @param[out] uiprgamma     The pointer of the R Gamma table with 33 elements.
    @param[out] uipggamma     The pointer of the G Gamma table with 33 elements.
    @param[out] uipbgamma     The pointer of the B Gamma table with 33 elements.

    @return void.
*/
void idec_get_rgb_gamma(IDE_ID id, UINT8 *uiprgamma, UINT8 *uipggamma, UINT8 *uipbgamma)
{
	UINT32 i;
	T_IDE_GAMMA reg;

	for (i = 0; i <= IDE_GAMMA_LEVEL; i += 4) {
		if (uiprgamma != NULL) {
			if (id == IDE_ID_1) {
				reg.reg = idec_get_reg(id, IDE_RGAMMA0_OFS + i);
			} else {
				reg.reg = idec_get_reg(id, IDE2_RGAMMA0_OFS + i);
			}

			uiprgamma[i] = reg.bit.gamma0;

			if (i < IDE_GAMMA_LEVEL) {
				uiprgamma[i + 1] = reg.bit.gamma1;
				uiprgamma[i + 2] = reg.bit.gamma2;
				uiprgamma[i + 3] = reg.bit.gamma3;
			}
		}
		if (uipggamma != NULL) {
			if (id == IDE_ID_1) {
				reg.reg = idec_get_reg(id, IDE_GGAMMA0_OFS + i);
			} else {
				reg.reg = idec_get_reg(id, IDE2_GGAMMA0_OFS + i);
			}

			uipggamma[i] = reg.bit.gamma0;

			if (i < IDE_GAMMA_LEVEL) {
				uipggamma[i + 1] = reg.bit.gamma1;
				uipggamma[i + 2] = reg.bit.gamma2;
				uipggamma[i + 3] = reg.bit.gamma3;
			}
		}
		if (uipbgamma != NULL) {
			if (id == IDE_ID_1) {
				reg.reg = idec_get_reg(id, IDE_BGAMMA0_OFS + i);
			} else {
				reg.reg = idec_get_reg(id, IDE2_BGAMMA0_OFS + i);
			}

			uipbgamma[i] = reg.bit.gamma0;

			if (i < IDE_GAMMA_LEVEL) {
				uipbgamma[i + 1] = reg.bit.gamma1;
				uipbgamma[i + 2] = reg.bit.gamma2;
				uipbgamma[i + 3] = reg.bit.gamma3;
			}
		}

	}

}



/*
    Set ide contrast.

    Set ide contrast.

    @param[in] id   ide ID
    @param[in] ui_ctrst  Contrast value (0 ~ 0.984375) * 64.

    @return void
    @note It is not used in 96650
*/
void idec_set_ctrst(IDE_ID id, UINT32 ui_ctrst)
{
	/*    T_IDE_PANEL_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	    reg.bit.ctrst = ui_ctrst;
	    idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);*/
}

/*
    Get ide contrast.

    Get ide contrast.

    @param[in] id   ide ID
    @return contrast value (0 ~ 0.984375) * 64.
    @note It is not used in 96650
*/
UINT32 idec_get_ctrst(IDE_ID id)
{
	/*    T_IDE_PANEL_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	    return reg.bit.ctrst;*/
	return 0;
}

/*
    Set ide brightness

    Set ide brightness

    @param[in] id   ide ID
    @param[in] ui_brt    Brightness value (-64 ~ 63) in 2's complement.

    @return void
    @note It is not used in 96650
*/
void idec_set_brt(IDE_ID id, UINT32 ui_brt)
{
	/*    T_IDE_PANEL_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	    reg.bit.brt = ui_brt;
	    idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);*/
}

/*
    Get ide brightness

    Get ide brightness

    @param[in] id   ide ID
    @return Brightness value (-64 ~ 63) in 2's complement.
    @note It is not used in 96650
*/
INT8 idec_get_brt(IDE_ID id)
{
	/*    INT8 ret;

	    T_IDE_PANEL_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	    ret = (INT8)(reg.bit.brt | ((reg.bit.brt&0x40)<<1));
	    return ret;*/
	return 0;
}

/*
    Set ide saturation.

    Set ide saturation.

    @param[in] id   ide ID
    @param[in] ui_cmults     The saturation value (0 ~ 7.984375) * 64.

    @return void
    @note It is not used in 96650
*/
void idec_set_cmults(IDE_ID id, UINT32 ui_cmults)
{
	/*    T_IDE_PANEL_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	    reg.bit.cmults = ui_cmults;
	    idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);*/
}

/*
    Get ide saturation.

    Get ide saturation.

    @param[in] id   ide ID
    @return the saturation value (0 ~ 7.984375) * 64.
    @note It is not used in 96650
*/
UINT32 idec_get_cmults(IDE_ID id)
{
	/*    T_IDE_PANEL_CTRL reg;

	    reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	    return reg.bit.cmults;*/
	return 0;
}

/**
    Set ide YCC Clamp

    Set ide YCC Clamp

    @param[in] id   ide ID
    @param[in] clamp
		- @b IDE_YCCCLAMP_NOCLAMP
		- @b IDE_YCCCLAMP_1_254
		- @b IDE_YCCCLAMP_16_235

    @return void
*/
void idec_set_clamp(IDE_ID id,  IDE_YCCCLAMP clamp)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	reg.bit.ycc_clm = clamp;

	idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);
}

/**
    Get ide CB,CR exchange.

    Get ide CB,CR exchange.

    @param[in] id   ide ID

    @return
		- @b IDE_YCCCLAMP_NOCLAMP
		- @b IDE_YCCCLAMP_1_254
		- @b IDE_YCCCLAMP_16_235

*/
IDE_YCCCLAMP idec_get_clamp(IDE_ID id)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	return reg.bit.ycc_clm;
}


/**
    Set ide CB,CR exchange.

    Set ide CB,CR exchange.

    @param[in] id   ide ID
    @param[in] b_cex
		- @b FALSE: CB first
		- @b TRUE:  CR first.

    @return void
*/
void idec_set_cex(IDE_ID id, BOOL b_cex)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	if (b_cex) {
		reg.bit.cex = 1;
	} else {
		reg.bit.cex = 0;
	}
	idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);
}

/**
    Get ide CB,CR exchange.

    Get ide CB,CR exchange.

    @param[in] id   ide ID

    @return
		- @b FALSE: CB first
		- @b TRUE:  CR first.

*/
BOOL idec_get_cex(IDE_ID id)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	return reg.bit.cex;
}


/**
    Set ide Y,C exchange.

    Set ide Y,C exchange.

    @param[in] id   ide ID
    @param[in] b_ycex
		- @b FALSE:   C first
		- @b TRUE:    Y first.

    @return void
*/
void idec_set_ycex(IDE_ID id, BOOL b_ycex)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);
	if (b_ycex) {
		reg.bit.yc_ex = 1;
	} else {
		reg.bit.yc_ex = 0;
	}
	idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);
}


/**
    Get ide Y,C exchange.

    Get ide Y,C exchange.

    @param[in] id   ide ID
    @return
		- @b FALSE:C first
		- @b TRUE:Y first
*/
BOOL idec_get_ycex(IDE_ID id)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	return reg.bit.yc_ex;
}

/*
    Set ide contrast/saturation/brightness enable/disable

    Set ide contrast/saturation/brightness enable/disable

    @param[in] id   ide ID
    @param[in] b_en
		- FALSE: Disable
		- TRUE:  Enable

    @return void
    @note It is not used in 96650
*/
void idec_set_csb_en(IDE_ID id, BOOL b_en)
{
	/*    T_IDE_CTRL2 reg;

	    reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	    if(b_en)
			reg.bit.csb_en = 1;
	    else
			reg.bit.csb_en = 0;

	    idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);*/
}

/*
    Get ide contrast/saturation/brightness enable/disable

    Get ide contrast/saturation/brightness enable/disable

    @param[in] id   ide ID
    @return FALSE: CSB disabled. TRUE: CSB enabled.
    @note It is not used in 96650
*/
BOOL idec_get_csb_en(IDE_ID id)
{
	/*    T_IDE_CTRL2 reg;

	    reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	    return  reg.bit.csb_en;*/
	return FALSE;
}

//@}

/**
@name ide Output Component
*/
//@{

/**
    Set ide R/G/B component dither valid bits

    Set ide R/G/B component dither valid bits

    @param[in] id   ide ID
    @param[in] b_rsel IDE_DITHER_4BITS:4bits IDE_DITHER_5BITS:5bits IDE_DITHER_6BITS:6bits IDE_DITHER_7BITS:7bits
    @param[in] b_gsel IDE_DITHER_4BITS:4bits IDE_DITHER_5BITS:5bits IDE_DITHER_6BITS:6bits IDE_DITHER_7BITS:7bits
    @param[in] b_bsel IDE_DITHER_4BITS:4bits IDE_DITHER_5BITS:5bits IDE_DITHER_6BITS:6bits IDE_DITHER_7BITS:7bits

    @return void
*/
void idec_set_dither_vbits(IDE_ID id, IDE_DITHER_VBITS b_rsel, IDE_DITHER_VBITS b_gsel, IDE_DITHER_VBITS b_bsel)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	reg.bit.r_vbits = b_rsel;
	reg.bit.g_vbits = b_gsel;
	reg.bit.b_vbits = b_bsel;

	idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);
}

/**
    Set ide output component selection

    Set ide output component selection

    @param[in] id   ide ID
    @param[in] ui_comp0  IDE_COMPONENT_R:R/Y IDE_COMPONENT_G:G/Cb IDE_COMPONENT_B:B/Cr
    @param[in] ui_comp1  IDE_COMPONENT_R:R/Y IDE_COMPONENT_G:G/Cb IDE_COMPONENT_B:B/Cr
    @param[in] ui_comp2  IDE_COMPONENT_R:R/Y IDE_COMPONENT_G:G/Cb IDE_COMPONENT_B:B/Cr
    @param[in] b_bit_swp TRUE:bit swap  FALSE:no bit swap
    @param[in] b_len    TRUE:output length 6bits  FALSE:output length 8bits

    @return void
*/
void idec_set_out_comp(IDE_ID id, IDE_OUT_COMP ui_comp0, IDE_OUT_COMP ui_comp1, IDE_OUT_COMP ui_comp2, BOOL b_bit_swp, BOOL b_len)
{
	T_IDE_PANEL_CTRL reg;


	//coverity[unsigned_compare]
	ide_chk_range(ui_comp0, IDE_COMPONENT_R, IDE_COMPONENT_B);

	//coverity[unsigned_compare]
	ide_chk_range(ui_comp1, IDE_COMPONENT_R, IDE_COMPONENT_B);

	//coverity[unsigned_compare]
	ide_chk_range(ui_comp2, IDE_COMPONENT_R, IDE_COMPONENT_B);

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	reg.bit.out_comp0_sel = ui_comp0;
	reg.bit.out_comp1_sel = ui_comp1;
	reg.bit.out_comp2_sel = ui_comp2;

	if (b_bit_swp) {
		reg.bit.out_comp_bswp = 1;
	} else {
		reg.bit.out_comp_bswp = 0;
	}

	if (b_len) {
		reg.bit.out_comp_len = 1;
	} else {
		reg.bit.out_comp_len = 0;
	}

	idec_set_reg(id, IDE_PANEL_CTRL_OFS, reg.reg);
}

/**
    Get ide output component selection

    Get ide output component selection

    @param[in] id   ide ID
    @param[out] ui_comp0     IDE_COMPONENT_R:R/Y IDE_COMPONENT_G:G/Cb IDE_COMPONENT_B:B/Cr
    @param[out] ui_comp1     IDE_COMPONENT_R:R/Y IDE_COMPONENT_G:G/Cb IDE_COMPONENT_B:B/Cr
    @param[out] ui_comp2     IDE_COMPONENT_R:R/Y IDE_COMPONENT_G:G/Cb IDE_COMPONENT_B:B/Cr
    @param[out] b_bit_swp    TRUE:bit swap  FALSE:no bit swap
    @param[out] b_len       TRUE:output length 6bits  FALSE:output length 8bits

    @return void
*/
void idec_get_out_comp(IDE_ID id, IDE_OUT_COMP *ui_comp0, IDE_OUT_COMP *ui_comp1, IDE_OUT_COMP *ui_comp2, BOOL *b_bit_swp, BOOL *b_len)
{
	T_IDE_PANEL_CTRL reg;

	reg.reg = idec_get_reg(id, IDE_PANEL_CTRL_OFS);

	*ui_comp0  = reg.bit.out_comp0_sel;
	*ui_comp1  = reg.bit.out_comp1_sel;
	*ui_comp2  = reg.bit.out_comp2_sel;
	*b_bit_swp = reg.bit.out_comp_bswp;
	*b_len    = reg.bit.out_comp_len;
}

//@}

/**
@name ide Interrupt
*/
//@{

/**
    Get ide interrupt status.

    Get ide interrupt status.

    @param[in] id   ide ID
    @return combination of ide interrupt source flags.
*/
UINT32 idec_get_interrupt_status(IDE_ID id)
{
	T_IDE_INT reg;

	reg.reg = idec_get_reg(id, IDE_INT_OFS);

	return reg.reg & IDE_INTSTS_MSK;
}

/**
    Clear ide interrupt status.

    Clear ide interrupt status.

    @param[in] id   ide ID
    @param[in] ui_int_status  The combination of ide interrupt source flags.

    @return void
*/
void idec_clear_interrupt_status(IDE_ID id, UINT32 ui_int_status)
{
	T_IDE_INT reg;

	reg.reg = idec_get_reg(id, IDE_INT_OFS);

	reg.reg &= ~IDE_INTSTS_MSK;

	reg.reg |= ui_int_status & IDE_INTSTS_MSK;

	idec_set_reg(id, IDE_INT_OFS, reg.reg);
}

/**
    Set ide interrupt enable flags

    All interrupt enable flag should be specified in ui_int_en.

    @param[in] id   ide ID
    @param[in] ui_int_en  The combination of ide interrupt source.

    @return void
*/
void idec_set_interrupt_en(IDE_ID id, UINT32 ui_int_en)
{
	T_IDE_INT reg;

	reg.reg = idec_get_reg(id, IDE_INT_OFS);

	reg.reg &= IDE_INTEN_MSK;

	//set irqen and don't clear irqsts
	reg.reg |= (ui_int_en & IDE_INTEN_MSK);
	idec_set_reg(id, IDE_INT_OFS, reg.reg);
}

/**
    Get ide interrupt enable flags

    All interrupt enable flag should be specified in ui_int_en.

    @param[in] id   ide ID

    @return ui_int_en  The combination of ide interrupt source.
*/
UINT32 idec_get_interrupt_en(IDE_ID id)
{
	T_IDE_INT reg;

	reg.reg = idec_get_reg(id, IDE_INT_OFS);

	return (reg.reg & 0x0000FFFF);
}


/**
    Clear ide interrupt enable flags

    Clear ide interrupt enable flags

    @param[in] id   ide ID
    @param[in] ui_int the combination of ide interrupt source.

    @return void
*/
void idec_clr_interrupt_en(IDE_ID id, UINT32 ui_int)
{
	T_IDE_INT reg;

	reg.reg = idec_get_reg(id, IDE_INT_OFS);

	reg.reg &= IDE_INTEN_MSK;

	reg.reg &= ~(ui_int & IDE_INTEN_MSK);

	idec_set_reg(id, IDE_INT_OFS, reg.reg);
}

//@}

/**
@name ide Inverse color Space Transform
*/
//@{

/**
    Set ide FCST coefficients

    Set ide FCST coefficients

    @param[in] id   ide ID
    @param[in] ui_pcoef  fcst coefficients with 4 elements

    @return void
*/
void idec_set_fcst_coef(IDE_ID id, UINT32 *ui_pcoef)
{
	T_IDE_CST0_COEF reg = {0x0};

	reg.bit.coef0 = ui_pcoef[0] & 0xFF;
	reg.bit.coef1 = ui_pcoef[1] & 0xFF;
	reg.bit.coef2 = ui_pcoef[2] & 0xFF;
	reg.bit.coef3 = ui_pcoef[3] & 0xFF;

	idec_set_reg(id, IDE_CST0_0_OFS, reg.reg);
}

/**
    Get ide FCST coefficients

    Get ide FCST coefficients

    @param[in] id   ide ID
    @param[in] ui_pcoef  fcst coefficients with 4 elements

    @return void
*/
void idec_get_fcst_coef(IDE_ID id, UINT8 *cst0_coef0, UINT8 *cst0_coef1, UINT8 *cst0_coef2, UINT8 *cst0_coef3)
{
	T_IDE_CST0_COEF reg = {0x0};

	reg.reg = idec_get_reg(id, IDE_CST0_0_OFS);
	*cst0_coef0 = reg.bit.coef0;
	*cst0_coef1 = reg.bit.coef1;
	*cst0_coef2 = reg.bit.coef2;
	*cst0_coef3 = reg.bit.coef3;
}

/**
    Set ide ICST0 type

    Set ide ICST0 type

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable cst
		- @b TRUE:enable cst

    @return void
*/
void idec_set_icst0(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.icst0_en = 1;
	} else {
		reg.bit.icst0_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);
}

/**
    Get ide ICST0 Enable

    Get ide ICST0 Enable

    @param[in] id   ide ID
    @return
		- @b FALSE: ICST disabled
		- @b TRUE: ICST enabled
*/
BOOL idec_get_icst0(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.icst0_en;
}


/**
    Set ide ICST1 type

    Set ide ICST1 type

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable cst
		- @b TRUE:enable cst

    @return void
*/
void idec_set_icst(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.icst1_en = 1;
	} else {
		reg.bit.icst1_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);
}

/**
    Get ide ICST Enable

    Get ide ICST Enable

    @param[in] id   ide ID
    @return
		- @b FALSE: ICST disabled
		- @b TRUE: ICST enabled
*/
BOOL idec_get_icst(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.icst1_en;
}

/**
    Set ide ICST pre-offset

    Set ide ICST pre-offset

    @param[in] id   ide ID
    @param[in]  ui_y     pre-offset y
    @param[in]  ui_cb    pre-offset Cb
    @param[in]  ui_cr    pre-offset Cr

    @return void
*/
void idec_set_icst0_pre_offset(IDE_ID id, UINT32 ui_y, UINT32 ui_cb, UINT32 ui_cr)
{
	T_IDE_ICST0_PREOFS reg;

	reg.reg = idec_get_reg(id, IDE_ICST0_PREOFS_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_ICST0_PREOFS_OFS, reg.reg);
}

/**
    Get ide ICST pre-offset

    Get ide ICST pre-offset

    @param[in] id   ide ID
    @param[out] ui_y     pre-offset y
    @param[out] ui_cb    pre-offset Cb
    @param[out] ui_cr    pre-offset Cr

    @return void
*/
void idec_get_icst0_pre_offset(IDE_ID id, INT32 *ui_y, INT32 *ui_cb, INT32 *ui_cr)
{
	T_IDE_ICST0_PREOFS reg;

	reg.reg = idec_get_reg(id, IDE_ICST0_PREOFS_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;

	if (*ui_y & 0x100) {
		*ui_y |= 0xFFFFFF00;
	}
	if (*ui_cb & 0x100) {
		*ui_cb |= 0xFFFFFF00;
	}
	if (*ui_cr & 0x100) {
		*ui_cr |= 0xFFFFFF00;
	}
}


/**
    Set ide ICST0 coefficients

    Set ide ICST0 coefficients

    @param[in] id   ide ID
    @param[in] ui_pcoef  icst coefficients with 9 elements

    @return void
*/
void idec_set_icst0_coef(IDE_ID id, UINT32 *ui_pcoef)
{
	UINT32 i;
	T_IDE_ICST0_COEF reg;

	for (i = 0; i < IDE_ICST0_COEF_NUM; i += 2) {
		reg.reg = 0;
		reg.bit.coef0 = ui_pcoef[i];
		if ((i + 1) != IDE_ICST0_COEF_NUM) {
			reg.bit.coef1 = ui_pcoef[i + 1];
		}

		idec_set_reg(id, IDE_ICST0_0_OFS + i * 2, reg.reg);
	}
}

/**
    Get ide ICST coefficients

    Get ide ICST coefficients

    @param[in] id   ide ID
    @param[out] ui_pcoef     icst coefficients with 9 elements

    @return void
*/
void idec_get_icst0_coef(IDE_ID id, INT32 *ui_pcoef)
{
	UINT32 i;
	T_IDE_ICST0_COEF reg;

	for (i = 0; i < IDE_ICST0_COEF_NUM; i += 2) {
		reg.reg = idec_get_reg(id, IDE_ICST0_0_OFS + i * 2);

		ui_pcoef[i] = reg.bit.coef0;
		if (ui_pcoef[i] & 0x800) {
			ui_pcoef[i] |= 0xFFFFF800;
		}

		if ((i + 1) != IDE_ICST0_COEF_NUM) {
			ui_pcoef[i + 1] = reg.bit.coef1;
			if (ui_pcoef[i + 1] & 0x800) {
				ui_pcoef[i + 1] |= 0xFFFFF800;
			}
		}
	}
}


/**
    Set ide ICST output offset

    Set ide ICST output offset

    @param[in] id   ide ID
    @param[in] ui_y  output offset y
    @param[in] ui_cb output offset Cb
    @param[in] ui_cr output offset Cr

    @return void
*/
void idec_set_out_offset(IDE_ID id, UINT32 ui_y, UINT32 ui_cb, UINT32 ui_cr)
{
	T_IDE_ICST0_POSTOFS reg;

	reg.reg = idec_get_reg(id, IDE_ICST0_OUTOFS_OFS);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_ICST0_OUTOFS_OFS, reg.reg);
}

/**
    Get ide ICST output offset

    Get ide ICST output offset

    @param[in] id   ide ID
    @param[out] ui_y output offset y
    @param[out] ui_cb output offset Cb
    @param[out] ui_cr output offset Cr

    @return void
*/
void idec_get_out_offset(IDE_ID id, INT32 *ui_y, INT32 *ui_cb, INT32 *ui_cr)
{
	T_IDE_ICST0_POSTOFS reg;

	reg.reg = idec_get_reg(id, IDE_ICST0_OUTOFS_OFS);

	*ui_y  = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;

	if (*ui_y & 0x100) {
		*ui_y |= 0xFFFFFF00;
	}
	if (*ui_cb & 0x100) {
		*ui_cb |= 0xFFFFFF00;
	}
	if (*ui_cr & 0x100) {
		*ui_cr |= 0xFFFFFF00;
	}
}



/**
    Set ide output limits

    Set ide output limits

    @param[in] id   ide ID
    @param[in] ui_y_low   lower limit y
    @param[in] ui_y_up    upper limit y
    @param[in] ui_cb_low  lower limit cb
    @param[in] ui_cb_up   upper limit cb
    @param[in] ui_cr_low  lower limit cr
    @param[in] ui_cr_up   upper limit cr

    @return void
*/
void idec_set_out_limit(IDE_ID id, UINT8 ui_y_low, UINT8 ui_y_up, UINT8 ui_cb_low, UINT8 ui_cb_up, UINT8 ui_cr_low, UINT8 ui_cr_up)
{
	T_IDE_ICST0_OUTLMT1 reg1;
	T_IDE_ICST0_OUTLMT2 reg2;

	reg1.reg = idec_get_reg(id, IDE_ICST0_OUTLMT1_OFS);
	reg2.reg = idec_get_reg(id, IDE_ICST0_OUTLMT2_OFS);

	reg1.bit.y_lower_lmt = ui_y_low;
	reg1.bit.y_upper_lmt = ui_y_up;
	reg1.bit.cb_lower_lmt = ui_cb_low;
	reg1.bit.cb_upper_lmt = ui_cb_up;
	reg2.bit.cr_lower_lmt = ui_cr_low;
	reg2.bit.cr_upper_lmt = ui_cr_up;

	idec_set_reg(id, IDE_ICST0_OUTLMT1_OFS, reg1.reg);
	idec_set_reg(id, IDE_ICST0_OUTLMT2_OFS, reg2.reg);
}

/**
    Set ide ICST1 coefficients

    Set ide ICST1 coefficients

    @param[in] id   ide ID
    @param[in] ui_pcoef  icst coefficients with 4 elements

    @return void
*/
void idec_set_icst_coef(IDE_ID id, UINT32 *ui_pcoef)
{
	UINT32 i;
	T_IDE_ICST1_COEF reg;

	//for(i=0; i<IDE_ICST1_COEF_NUM; i+=2)
	i = 0;
	{
		reg.reg = 0;
		reg.bit.coef0 = ui_pcoef[i];
		reg.bit.coef1 = ui_pcoef[i + 1];
		reg.bit.coef2 = ui_pcoef[i + 2];
		reg.bit.coef3 = ui_pcoef[i + 3];
		idec_set_reg(id, IDE_ICST1_COFS, reg.reg);
	}
}

/**
    Get ide ICST coefficients

    Get ide ICST coefficients

    @param[in] id   ide ID
    @param[out] ui_pcoef     icst coefficients with 4 elements

    @return void
*/
void idec_get_icst_coef(IDE_ID id, INT32 *ui_pcoef)
{
	UINT32 i;
	T_IDE_ICST1_COEF reg;

	//for(i=0; i<IDE_ICST1_COEF_NUM; i+=2)
	i = 0;
	{
		reg.reg = idec_get_reg(id, IDE_ICST1_COFS);

		ui_pcoef[i] = reg.bit.coef0;
		ui_pcoef[i + 1] = reg.bit.coef1;
		ui_pcoef[i + 2] = reg.bit.coef2;
		ui_pcoef[i + 3] = reg.bit.coef3;
	}
}

/**
    Set ide CST1  type

    Set ide CST1 type

    @param[in] id   ide ID
    @param[in] b_en
		- @b FALSE:disable cst
		- @b TRUE:enable cst

    @return void
*/
void idec_set_cst1(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.cst1_en = 1;
	} else {
		reg.bit.cst1_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);
}

/**
    Get ide ICST Enable

    Get ide ICST Enable

    @param[in] id   ide ID
    @return
		- @b FALSE: CST1 disabled
		- @b TRUE: CST1 enabled
*/
BOOL idec_get_cst1(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.cst1_en;
}


//@}

/**
    @name ide color Control
*/
//@{
/**
    Set ide color Control enable

    Set ide color Control enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:enable
		- @b FALSE:disable

    @return void
*/
void idec_set_color_ctrl_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.color_ctrl_en = 1;
	} else {
		reg.bit.color_ctrl_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);

}
/**
    Get ide color Control enable

    Get ide color Control enable

    @param[in] id   ide ID

    @return void
		- @b TRUE:enable
		- @b FALSE:disable
*/
BOOL idec_get_color_ctrl_en(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.color_ctrl_en;
}
/**
    Set ide color Component Adj. enable

    Set ide color Component Adj. enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:enable
		- @b FALSE:disable

    @return void
*/
void idec_set_color_comp_adj_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.color_comp_adj_en = 1;
	} else {
		reg.bit.color_comp_adj_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);

}
/**
    Get ide color Component Adj. enable

    Get ide color Component Adj. enable

    @param[in] id   ide ID

    @return void
		- @b TRUE:enable
		- @b FALSE:disable
*/
BOOL idec_get_color_comp_adj_en(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.color_comp_adj_en;
}
/**
    Set ide color Control Hue Adj. enable

    Set ide color Control Hue Adj. enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:enable
		- @b FALSE:disable

    @return void
*/
void idec_set_color_ctrl_hue_adj_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.color_comphue_adj_en = 1;
	} else {
		reg.bit.color_comphue_adj_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);

}
/**
    Get ide color Control Hue Adj. enable

    Get ide color Control Hue Adj. enable

    @param[in] id   ide ID

    @return void
		- @b TRUE:enable
		- @b FALSE:disable
*/
BOOL idec_get_color_ctrl_hue_adj_en(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.color_comphue_adj_en;
}
/**
    Set ide color Component Y Contrast enable

    Set ide color Component Y Contrast enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:enable
		- @b FALSE:disable

    @return void
*/
void idec_set_color_comp_ycon_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.color_comp_ycon_en = 1;
	} else {
		reg.bit.color_comp_ycon_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);

}
/**
    Get ide color Component Y Contrast enable

    Get ide color Component Y Contrast enable

    @param[in] id   ide ID

    @return void
		- @b TRUE:enable
		- @b FALSE:disable
*/
BOOL idec_get_color_comp_ycon_en(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.color_comp_ycon_en;
}
/**
    Set ide color Component C Contrast enable

    Set ide color Component C Contrast enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE:enable
		- @b FALSE:disable

    @return void
*/
void idec_set_color_comp_ccon_en(IDE_ID id, BOOL b_en)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);

	if (b_en) {
		reg.bit.color_comp_ccon_en = 1;
	} else {
		reg.bit.color_comp_ccon_en = 0;
	}

	idec_set_reg(id, IDE_CTRL2_OFS, reg.reg);

}
/**
    Get ide color Component C Contrast enable

    Get ide color Component C Contrast enable

    @param[in] id   ide ID

    @return void
		- @b TRUE:enable
		- @b FALSE:disable
*/
BOOL idec_get_color_comp_ccon_en(IDE_ID id)
{
	T_IDE_CTRL2 reg;

	reg.reg = idec_get_reg(id, IDE_CTRL2_OFS);
	return reg.bit.color_comp_ccon_en;
}
/**
    Set ide color Control Hue table

    Set ide color Control Hue table

    @param[in] id   ide ID
    @param[in] hue24tbl the pointer of Hue table.

    @return void
*/
void idec_set_color_ctrl_hue(IDE_ID id, UINT8 *hue24tbl)
{
	UINT32 i;
	T_IDE_CC_HUEM reg;

	for (i = 0; i < IDE_CC_HUE_NUM; i += 4) {
		reg.reg = 0;
		reg.bit.cc_huem0 = hue24tbl[i];
		reg.bit.cc_huem1 = hue24tbl[i + 1];
		reg.bit.cc_huem2 = hue24tbl[i + 2];
		reg.bit.cc_huem3 = hue24tbl[i + 3];

		idec_set_reg(id, IDE_CC_HUE0_OFS + i, reg.reg);
	}

}
/**
    Get ide color Control Hue table

    Get ide color Control Hue table

    @param[in] id   ide ID
    @param[out] hue24tbl the pointer of Hue table.

    @return void
*/
void idec_get_color_ctrl_hue(IDE_ID id, UINT8 *hue24tbl)
{
	UINT32 i;
	T_IDE_CC_HUEM reg;

	for (i = 0; i < IDE_CC_HUE_NUM; i += 4) {
		if (IDE_ID_1 == id) {
			reg.reg = idec_get_reg(id, IDE_CC_HUE0_OFS + i);
		} else {
			reg.reg = idec_get_reg(id, IDE2_CC_HUE0_OFS + i);
		}

		hue24tbl[i] = reg.bit.cc_huem0;
		hue24tbl[i + 1] = reg.bit.cc_huem1;
		hue24tbl[i + 2] = reg.bit.cc_huem2;
		hue24tbl[i + 3] = reg.bit.cc_huem3;
	}

}
/**
    Set ide color Control Intensity table

    Set ide color Control Intensity table

    @param[in] id   ide ID
    @param[in] intensity24tbl the pointer of Int table.

    @return void
*/
void idec_set_color_ctrl_int(IDE_ID id, INT8 *intensity24tbl)
{
	UINT32 i;
	T_IDE_CC_INTM reg;

	for (i = 0; i < IDE_CC_INT_NUM; i += 4) {
		reg.reg = 0;
		reg.bit.cc_intm0 = intensity24tbl[i];
		reg.bit.cc_intm1 = intensity24tbl[i + 1];
		reg.bit.cc_intm2 = intensity24tbl[i + 2];
		reg.bit.cc_intm3 = intensity24tbl[i + 3];

		idec_set_reg(id, IDE_CC_INT0_OFS + i, reg.reg);
	}

}
/**
    Get ide color Control Intensity table

    Get ide color Control Intensity table

    @param[in] id   ide ID
    @param[out] intensity24tbl the pointer of Int table.

    @return void
*/
void idec_get_color_ctrl_int(IDE_ID id, INT8 *intensity24tbl)
{
	UINT32 i;
	T_IDE_CC_INTM reg;

	for (i = 0; i < IDE_CC_INT_NUM; i += 4) {
		if (IDE_ID_1 == id) {
			reg.reg = idec_get_reg(id, IDE_CC_INT0_OFS + i);
		} else {
			reg.reg = idec_get_reg(id, IDE2_CC_INT0_OFS + i);
		}

		intensity24tbl[i] = reg.bit.cc_intm0;
		intensity24tbl[i + 1] = reg.bit.cc_intm1;
		intensity24tbl[i + 2] = reg.bit.cc_intm2;
		intensity24tbl[i + 3] = reg.bit.cc_intm3;
	}

}
/**
    Set ide color Control Saturation table

    Set ide color Control Saturation table

    @param[in] id   ide ID
    @param[in] sat24tbl the pointer of Sat table.

    @return void
*/
void idec_set_color_ctrl_sat(IDE_ID id, INT8 *sat24tbl)
{
	UINT32 i;
	T_IDE_CC_SATM reg;

	for (i = 0; i < IDE_CC_SAT_NUM; i += 4) {
		reg.reg = 0;
		reg.bit.cc_satm0 = sat24tbl[i];
		reg.bit.cc_satm1 = sat24tbl[i + 1];
		reg.bit.cc_satm2 = sat24tbl[i + 2];
		reg.bit.cc_satm3 = sat24tbl[i + 3];

		idec_set_reg(id, IDE_CC_SAT0_OFS + i, reg.reg);
	}

}
/**
    Get ide color Control Saturation table

    Get ide color Control Saturation table

    @param[in] id   ide ID
    @param[out] sat24tbl the pointer of Sat table.

    @return void
*/
void idec_get_color_ctrl_sat(IDE_ID id, INT8 *sat24tbl)
{
	UINT32 i;
	T_IDE_CC_SATM reg;

	for (i = 0; i < IDE_CC_SAT_NUM; i += 4) {
		if (IDE_ID_1 == id) {
			reg.reg = idec_get_reg(id, IDE_CC_SAT0_OFS + i);
		} else {
			reg.reg = idec_get_reg(id, IDE2_CC_SAT0_OFS + i);
		}

		sat24tbl[i] = reg.bit.cc_satm0;
		sat24tbl[i + 1] = reg.bit.cc_satm1;
		sat24tbl[i + 2] = reg.bit.cc_satm1;
		sat24tbl[i + 3] = reg.bit.cc_satm3;
	}

}
/**
    Set ide color Control DDS table

    Set ide color Control DDS table

    @param[in] id   ide ID
    @param[in] dds8tbl the pointer of DDS table.

    @return void
*/
void idec_set_color_ctrl_dds(IDE_ID id, UINT8 *dds8tbl)
{
	UINT32 i;
	T_IDE_CC_DDSM reg;

	for (i = 0; i < IDE_CC_DDS_NUM; i += 4) {
		reg.reg = 0;
		reg.bit.cc_ddsm0 = dds8tbl[i];
		reg.bit.cc_ddsm1 = dds8tbl[i + 1];
		reg.bit.cc_ddsm2 = dds8tbl[i + 2];
		reg.bit.cc_ddsm3 = dds8tbl[i + 3];

		idec_set_reg(id, IDE_CC_DDS0_OFS + i, reg.reg);
	}

}
/**
    Get ide color Control DDS table

    Get ide color Control DDS table

    @param[in] id   ide ID
    @param[out] dds8tbl the pointer of DDS table.

    @return void
*/
void idec_get_color_ctrl_dds(IDE_ID id, UINT8 *dds8tbl)
{
	UINT32 i;
	T_IDE_CC_DDSM reg;

	for (i = 0; i < IDE_CC_DDS_NUM; i += 4) {
		if (IDE_ID_1 == id) {
			reg.reg = idec_get_reg(id, IDE_CC_DDS0_OFS + i);
		} else {
			reg.reg = idec_get_reg(id, IDE2_CC_DDS0_OFS + i);
		}

		dds8tbl[i] = reg.bit.cc_ddsm0;
		dds8tbl[i + 1] = reg.bit.cc_ddsm1;
		dds8tbl[i + 2] = reg.bit.cc_ddsm2;
		dds8tbl[i + 3] = reg.bit.cc_ddsm3;
	}

}
/**
    Set ide color Control Int/Sat offset

    Set ide color Control Int/Sat offset

    @param[in] id   ide ID
    @param[in] int_ofs the int offset
    @param[in] sat_ofs the sat offset

    @return void
*/
void idec_set_color_ctrl_int_sat_ofs(IDE_ID id, INT8 int_ofs, INT8 sat_ofs)
{
	T_IDE_CC_INTOFS reg;

	reg.reg = 0;
	reg.bit.cc_int_ofs = int_ofs;
	reg.bit.cc_sat_ofs = sat_ofs;

	idec_set_reg(id, IDE_CC_INTOFS_OFS, reg.reg);
}
/**
    Get ide color Control Int/Sat offset

    Get ide color Control Int/Sat offset

    @param[in] id   ide ID
    @param[out] int_ofs the int offset
    @param[out] sat_ofs the sat offset

    @return void
*/
void idec_get_color_ctrl_int_sat_ofs(IDE_ID id, INT8 *int_ofs, INT8 *sat_ofs)
{
	T_IDE_CC_INTOFS reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CC_INTOFS_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CC_INTOFS_OFS);
	}

	*int_ofs = reg.bit.cc_int_ofs;
	*sat_ofs = reg.bit.cc_sat_ofs;

#if 0
	if (*int_ofs & 0x80)
		//coverity[extra_high_bits]
		//sign extension
	{
		*int_ofs |= 0xFF80;
	}
	if (*sat_ofs & 0x100)
		//coverity[extra_high_bits]
		//sign extension
	{
		*sat_ofs |= 0xFF80;
	}
#endif
}

//CCA
/**
    Set ide color Comp. Y contrast

    Set ide color Comp. Y contrast

    @param[in] id   ide ID
    @param[in] cony the y contrast

    @return void
*/
void idec_set_color_comp_ycon(IDE_ID id, UINT8 cony)
{
	T_IDE_CCA0 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA0_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA0_OFS);
	}

	reg.bit.cca_y_con = cony;

	idec_set_reg(id, IDE_CCA0_OFS, reg.reg);

}
/**
    Get ide color Comp. Y contrast

    Get ide color Comp. Y contrast

    @param[in] id   ide ID
    @param[out] cony the y contrast

    @return void
*/
void idec_get_color_comp_ycon(IDE_ID id, UINT8 *cony)
{
	T_IDE_CCA0 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA0_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA0_OFS);
	}

	*cony = reg.bit.cca_y_con;

}
/**
    Set ide color Comp. C contrast

    Set ide color Comp. C contrast

    @param[in] id   ide ID
    @param[in] conc the c contrast

    @return void
*/
void idec_set_color_comp_ccon(IDE_ID id, UINT8 conc)
{
	T_IDE_CCA0 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA0_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA0_OFS);
	}

	reg.bit.cca_c_con = conc;

	idec_set_reg(id, IDE_CCA0_OFS, reg.reg);

}
/**
    Get ide color Comp. C contrast

    Get ide color Comp. C contrast

    @param[in] id   ide ID
    @param[out] conc the c contrast

    @return void
*/
void idec_get_color_comp_ccon(IDE_ID id, UINT8 *conc)
{
	T_IDE_CCA0 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA0_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA0_OFS);
	}

	*conc = reg.bit.cca_c_con;

}
/**
    Set ide color Comp. Y offset

    Set ide color Comp. Y offset

    @param[in] id   ide ID
    @param[in] yofs the y offset

    @return void
*/
void idec_set_color_comp_yofs(IDE_ID id, INT8 yofs)
{
	T_IDE_CCA1 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA1_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA1_OFS);
	}

	reg.bit.cca_y_ofs = yofs;

	idec_set_reg(id, IDE_CCA1_OFS, reg.reg);

}
/**
    Get ide color Comp. Y offset

    Get ide color Comp. Y offset

    @param[in] id   ide ID
    @param[out] yofs the y offset

    @return void
*/
void idec_get_color_comp_yofs(IDE_ID id, INT8 *yofs)
{
	T_IDE_CCA1 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA1_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA1_OFS);
	}

	*yofs = reg.bit.cca_y_ofs;
}
/**
    Set ide color Comp. C offset

    Set ide color Comp. C offset

    @param[in] id   ide ID
    @param[in] cbofs the cb offset
    @param[in] crofs the cr offset

    @return void
*/
void idec_set_color_comp_cofs(IDE_ID id, UINT8 cbofs, UINT8 crofs)
{
	T_IDE_CCA1 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA1_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA1_OFS);
	}

	reg.bit.cca_cb_ofs = cbofs;
	reg.bit.cca_cr_ofs = crofs;

	idec_set_reg(id, IDE_CCA1_OFS, reg.reg);

}
/**
    Get ide color Comp. C offset

    Get ide color Comp. C offset

    @param[in] id   ide ID
    @param[out] cbofs the cb offset
    @param[out] crofs the cr offset

    @return void
*/
void idec_get_color_comp_cofs(IDE_ID id, UINT8 *cbofs, UINT8 *crofs)
{
	T_IDE_CCA1 reg;

	if (IDE_ID_1 == id) {
		reg.reg = idec_get_reg(id, IDE_CCA1_OFS);
	} else {
		reg.reg = idec_get_reg(id, IDE2_CCA1_OFS);
	}
	*cbofs = reg.bit.cca_cb_ofs;
	*crofs = reg.bit.cca_cr_ofs;
}
//@}


/**
    @name ide Alpha Blending
*/
//@{
/**
    Set ide Alpha Blending configuration

    Set ide Alpha Blending configuration

    @param[in] id   ide ID
    @param[in] ui_layer          layer select.
    @param[in] ui_alhpa_type      Alpha Blending type
    @param[in] ui_global_alpha    Global alpha value

    @return void
*/
void idec_set_alpha_blending(IDE_ID id, IDE_BLEND_LAYER ui_layer, IDE_ALPHA_TYPE ui_alhpa_type, UINT8 ui_global_alpha)
{
	T_IDE_BLEND reg;
	T_IDE_BLEND2 reg2;

	////                                                [alpha1]
	//Default assume layer from lower -> upper layer = VDO1--->OSD1
	if (id == IDE_ID_2) {
		/*
		reg.reg = idec_get_reg(id, IDE_BLEND_OFS);
		reg2.reg = idec_get_reg(id, IDE_BLEND2_OFS);
		if (ui_layer ==  IDE_BLEND_O1) {
			reg.bit.alpha_sel4 = ui_alhpa_type;
			reg2.bit.global_alpha4 = ui_global_alpha;
			idec_set_reg(id, IDE_BLEND2_OFS, reg2.reg);
		} else if (ui_layer == IDE_BLEND_O1_GLBALPHA5) {
			reg.bit.alpha_sel4 = ui_alhpa_type;
			reg2.bit.global_alpha5 = ui_global_alpha;
			idec_set_reg(id, IDE_BLEND2_OFS, reg2.reg);
		} else {
			DBG_WRN("IDE2 not Support Blending of this layer!!\r\n");
			return;
		}
		idec_set_reg(id, IDE_BLEND_OFS, reg.reg);
		*/
		DBG_WRN("There is no IDE2\r\n");
	}
	//IDE1
	else {
		//                                                  [alpha1] [alpha2][alpha3][alpha4]
		//Default assume layer from lower -> upper layer = VDO1--->VDO2--->Line--->FD--->OSD1
		reg.reg = idec_get_reg(id, IDE_BLEND_OFS);


		if (ui_layer == IDE_BLEND_V1) {
			reg2.reg = idec_get_reg(id, IDE_BLEND2_OFS);
			reg.bit.alpha_sel4 = ui_alhpa_type;
			reg2.bit.global_alpha5 = ui_global_alpha;
			idec_set_reg(id, IDE_BLEND2_OFS, reg2.reg);
		} else if (ui_layer == IDE_BLEND_V2) {
			reg.bit.alpha_sel1 = ui_alhpa_type;
			reg.bit.global_alpha1 = ui_global_alpha;
		} else if (ui_layer == IDE_BLEND_FD) {
			reg.bit.alpha_sel2 = ui_alhpa_type;
			reg.bit.global_alpha2 = ui_global_alpha;
		} else if (ui_layer == IDE_BLEND_O1) {
			reg.bit.alpha_sel3 = ui_alhpa_type;
			reg.bit.global_alpha3 = ui_global_alpha;
		} else if (ui_layer == IDE_BLEND_O1_GLBALPHA5) {
			reg2.reg = idec_get_reg(id, IDE_BLEND2_OFS);
			reg.bit.alpha_sel3 = ui_alhpa_type;
			reg2.bit.global_alpha4 = ui_global_alpha;
			idec_set_reg(id, IDE_BLEND2_OFS, reg2.reg);
		}

		idec_set_reg(id, IDE_BLEND_OFS, reg.reg);

	}
}

/**
    Get ide Alpha Blending configuration

    Get ide Alpha Blending configuration

    @param[in]  id   ide ID
    @param[in]  ui_layer          layer select.
    @param[out] ui_alhpa_type      Alpha Blending type
    @param[out] ui_global_alpha    Global alpha value

    @return void
*/
void idec_get_alpha_blending(IDE_ID id, IDE_BLEND_LAYER ui_layer, IDE_ALPHA_TYPE *ui_alhpa_type, UINT8 *ui_global_alpha)
{
	T_IDE_BLEND reg;
	T_IDE_BLEND2 reg2;


	if (id == IDE_ID_2) {
		/*
		reg.reg = idec_get_reg(id, IDE_BLEND_OFS);
		reg2.reg = idec_get_reg(id, IDE_BLEND2_OFS);
		if (ui_layer ==  IDE_BLEND_O1) {
			*ui_alhpa_type = reg.bit.alpha_sel4;
			*ui_global_alpha = reg2.bit.global_alpha4;
		} else if (ui_layer == IDE_BLEND_O1_GLBALPHA5) {
			*ui_alhpa_type = reg.bit.alpha_sel4;
			*ui_global_alpha = reg2.bit.global_alpha5;
		} else {
			DBG_WRN("IDE2 not Support Blending of this layer!!\r\n");
			return;
		}
		*/
		DBG_WRN("There is no IDE2\r\n");
	}
	//IDE1
	else {
		//                                                  [alpha1] [alpha2][alpha3][alpha4]
		//Default assume layer from lower -> upper layer = VDO1--->VDO2--->Line--->FD--->OSD1
		reg.reg = idec_get_reg(id, IDE_BLEND_OFS);


		if (ui_layer == IDE_BLEND_V1) {
			reg2.reg = idec_get_reg(id, IDE_BLEND2_OFS);
			*ui_alhpa_type = reg.bit.alpha_sel4;
			*ui_global_alpha = reg2.bit.global_alpha5;
		} else if (ui_layer == IDE_BLEND_V2) {
			*ui_alhpa_type = reg.bit.alpha_sel1;
			*ui_global_alpha = reg.bit.global_alpha1;
		} else if (ui_layer == IDE_BLEND_FD) {
			*ui_alhpa_type = reg.bit.alpha_sel2;
			*ui_global_alpha = reg.bit.global_alpha2;
		} else if (ui_layer == IDE_BLEND_O1) {
			*ui_alhpa_type = reg.bit.alpha_sel3;
			*ui_global_alpha = reg.bit.global_alpha3;
		} else if (ui_layer == IDE_BLEND_O1_GLBALPHA5) {
			reg2.reg = idec_get_reg(id, IDE_BLEND2_OFS);
			*ui_alhpa_type = reg.bit.alpha_sel3;
			*ui_global_alpha = reg2.bit.global_alpha4;
		}
	}
}
//@}

#if 0
/*
    Set ide O1 Palette Shadow enable

    Set ide O1 Palette Shadow enable

    @param[in] id   ide ID
    @param[in] b_en
      - @b TRUE: enable
      - @b FALSE: disable

    @return void

*/
void idec_set_o1_pal_shw_en(IDE_ID id, BOOL b_en)
{
	T_IDE_PAL_SHADOW reg;

	reg.reg = idec_get_reg(id, IDE_PAL_SHADOW_OFS);

	reg.bit.o1_pal_shw_en = b_en;

	idec_set_reg(id, IDE_PAL_SHADOW_OFS, reg.reg);
}
#endif

/**
    Set ide LineBuffer CPU read/write enable

    Set ide LineBuffer CPU read/write enable

    @param[in] id   ide ID
    @param[in] b_en
		- @b TRUE: enable
		- @b FALSE: disable

    @return void

*/
void idec_set_lb_read_en(IDE_ID id, BOOL b_en)
{
	T_IDE_PAL_SHADOW reg;

	reg.reg = idec_get_reg(id, IDE_PAL_SHADOW_OFS);

	reg.bit.ide_lb_r_en = b_en;

	idec_set_reg(id, IDE_PAL_SHADOW_OFS, reg.reg);
}

#if 0
/**
    Set ide FD auto expend enable

    Set ide FD auto expend enable when Through mode/Dummy mode.

    @param[in] id   ide ID
    @param[in] b_en
      - @b TRUE: enable
      - @b FALSE: disable

    @return void

*/
void idec_set_fd_exp_en(IDE_ID id, BOOL b_en)
{
	T_IDE_PAL_SHADOW reg;

	reg.reg = idec_get_reg(id, IDE_PAL_SHADOW_OFS);

	reg.bit.ide_fd_expend_en = b_en;

	idec_set_reg(id, IDE_PAL_SHADOW_OFS, reg.reg);
}
#endif

/**
    Set ide FD enable

    Set ide FD enable

    @param[in] id   ide ID
    @param[in] ui_en   Combination of IDE_FD_X

    @return void
*/
void idec_set_fd_all_en(IDE_ID id, UINT32 ui_en)
{
	T_IDE_FD_EN reg;

	reg.reg = idec_get_reg(id, IDE_FD_EN_OFS);

	reg.reg |= ui_en;

	idec_set_reg(id, IDE_FD_EN_OFS, reg.reg);
}
/**
    Set ide FD disable

    Set ide FD disable

    @param[in] id   ide ID
    @param[in] ui_dis   Combination of IDE_FD_X

    @return void
*/
void idec_set_fd_all_dis(IDE_ID id, UINT32 ui_dis)
{
	T_IDE_FD_EN reg;

	reg.reg = idec_get_reg(id, IDE_FD_EN_OFS);

	reg.reg &= ~(ui_dis);

	idec_set_reg(id, IDE_FD_EN_OFS, reg.reg);
}
/**
    Get ide FD enable

    Get ide FD enable

    @param[in] id   ide ID
    @return Combination of IDE_FD_X
*/
UINT32 idec_get_fd_all_en(IDE_ID id)
{
	T_IDE_FD_EN reg;

	reg.reg = idec_get_reg(id, IDE_FD_EN_OFS);

	return reg.reg;
}
/**
    Set ide FD enable by single

    Set ide FD enable by single

    @param[in] id   ide ID
    @param[in] ui_num   specify single FD rect. enable

    @return void
*/
void idec_set_fd_en(IDE_ID id, IDE_FD_NUM ui_num)
{
	T_IDE_FD_EN reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if (ui_num > IDE_FD_15) {
		return;
	}

	reg.reg = idec_get_reg(id, IDE_FD_EN_OFS);

	reg.reg |= ui_num;//(1 << ui_num);

	idec_set_reg(id, IDE_FD_EN_OFS, reg.reg);
}
/**
    Set ide FD disable by single

    Set ide FD disable by single

    @param[in] id   ide ID
    @param[in] ui_num   specify single FD rect. disable

    @return void
*/
void idec_set_fd_dis(IDE_ID id, IDE_FD_NUM ui_num)
{
	T_IDE_FD_EN reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if (ui_num > IDE_FD_15) {
		return;
	}

	reg.reg = idec_get_reg(id, IDE_FD_EN_OFS);

	reg.reg &= ~(ui_num);//(1 << ui_num);

	idec_set_reg(id, IDE_FD_EN_OFS, reg.reg);
}
/*
    Check ide FD number

    Check ide FD number valid

    @param[in] id   ide ID
    @param[in] ui_num FD number to check.

    @return which group of FD number.

*/
static UINT32 idec_chkfd_num(IDE_ID id, IDE_FD_NUM ui_num)
{
	UINT32 ui;
	UINT32 uiret;

	uiret = 0x0;
	for (ui = 0; ui < 16; ui++) {
		if (ui_num & 0x01) {
			return uiret;
		}

		ui_num = ui_num >> 1;
		uiret++;
	}
	uiret = 0;
	return uiret;
}

/**
    Set ide FD window position

    Set ide FD window position

    @param[in] id   ide ID
    @param[in] ui_num
    @param[in] ui_x The start position at X-axis.
    @param[in] ui_y The start position at Y-axis.

    @return void
*/
void idec_set_fd_win_pos(IDE_ID id, IDE_FD_NUM ui_num, UINT32 ui_x, UINT32 ui_y)
{
	T_IDE_FD_ATTR0 reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;
	reg.reg = idec_get_reg(id, IDE_FD_ATTR0_OFS + i);

	reg.bit.x = ui_x;
	reg.bit.y = ui_y;

	idec_set_reg(id, IDE_FD_ATTR0_OFS + i, reg.reg);
}


/*
    Check ide Line number

    Check ide Line number valid

    @param[in] id   ide ID
    @param[in] ui_num Line number to check.

    @return which group of FD number.

*/
static UINT32 idec_chkline_num(IDE_ID id, IDE_LINE_NUM ui_num)
{
	UINT32 ui;
	UINT32 uiret;

	uiret = 0x0;
	for (ui = 0; ui < 16; ui++) {
		if (ui_num & 0x01) {
			return uiret;
		}

		ui_num = ui_num >> 1;
		uiret++;

	}
	uiret = 0;
	return uiret;
}


/**
    Set ide Line disable

    Set ide Line disable

    @param[in] id   ide ID
    @param[in] ui_dis   Combination of IDE_Line_X

    @return void
*/
void idec_set_line_all_dis(IDE_ID id, UINT32 ui_dis)
{
	T_IDE_LINE_EN reg;

	reg.reg = idec_get_reg(id, IDE_LINE_EN_OFS);

	reg.reg &= ~(ui_dis);

	idec_set_reg(id, IDE_LINE_EN_OFS, reg.reg);
}

/**
    Get ide Line enable

    Get ide Line enable

    @param[in] id   ide ID
    @return Combination of IDE_FD_X
*/
UINT32 idec_get_line_all_en(IDE_ID id)
{
	T_IDE_LINE_EN reg;

	reg.reg = idec_get_reg(id, IDE_LINE_EN_OFS);

	return reg.reg;
}
/**
    Set ide Line enable by single

    Set ide Line enable by single

    @param[in] id   ide ID
    @param[in] ui_num   specify single FD rect. enable

    @return void
*/
void idec_set_line_en(IDE_ID id, IDE_LINE_NUM ui_num)
{
	T_IDE_LINE_EN reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_LINE_0, IDE_LINE_15);

	if (ui_num > IDE_LINE_15) {
		return;
	}

	reg.reg = idec_get_reg(id, IDE_LINE_EN_OFS);

	reg.reg |= ui_num;//(1 << ui_num);

	idec_set_reg(id, IDE_LINE_EN_OFS, reg.reg);
}
/**
    Set ide Line disable by single

    Set ide Line disable by single

    @param[in] id   ide ID
    @param[in] ui_num   specify single Line rect. disable

    @return void
*/
void idec_set_line_dis(IDE_ID id, IDE_LINE_NUM ui_num)
{
	T_IDE_LINE_EN reg;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_LINE_0, IDE_LINE_15);

	if (ui_num > IDE_LINE_15) {
		return;
	}

	reg.reg = idec_get_reg(id, IDE_LINE_EN_OFS);

	reg.reg &= ~(ui_num);//(1 << ui_num);

	idec_set_reg(id, IDE_LINE_EN_OFS, reg.reg);
}

/**
    Set ide Line source alpha

    Set ide Line source alpha

    @param[in] id   ide ID
    @param[in] ui_num   specify single Line rect. disable

    @return void
*/
ER idec_set_line_source_alpha(IDE_ID id, IDE_LINE_NUM ui_num, UINT8 source_alpha)
{
	T_IDE_LINE_X_COLOR reg;
	UINT32  no_of_line;

	if (id > IDE_ID_1) {
		DBG_ERR("Line layer only in IDE1");
		return E_SYS;
	}
	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_LINE_0, IDE_LINE_15);

	if (ui_num > IDE_LINE_15) {
		return E_SYS;
	}

	no_of_line = __builtin_ctz(ui_num);

	DBG_IND("idec_set_line_source_alpha line[%d] offset = 0x%08x\r\n", (int)no_of_line, (unsigned int)(IDE_LINE0_COLOR_OFS + no_of_line * IDE_LINE_X_COLOR));

	reg.reg = idec_get_reg(id, IDE_LINE0_COLOR_OFS + no_of_line * IDE_LINE_X_COLOR);

	reg.bit.alpha = source_alpha;

	idec_set_reg(id, IDE_LINE0_COLOR_OFS + no_of_line * IDE_LINE_X_COLOR, reg.reg);

	return E_OK;
}

/**
    Get ide Line color.

    Get ide Line color.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[out] ui_a  source alpha.
    @param[out] ui_y  color of y channel.
    @param[out] ui_cb color of cb channel.
    @param[out] ui_cr color of cr channel.

    @return void
*/
void idec_get_line_color(IDE_ID id, IDE_LINE_NUM ui_num, UINT8 *ui_a, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_LINE_X_COLOR reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_LINE_0, IDE_LINE_15);

	if ((ui_num < IDE_LINE_0) || (ui_num > IDE_LINE_15)) {
		return;
	}
	i = idec_chkline_num(id, ui_num);

	reg.reg = idec_get_reg(id, IDE_LINE0_COLOR_OFS + i * IDE_LINE_SET_REGION_OFFSET);

	*ui_a = reg.bit.alpha;
	*ui_y = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

/**
    Get ide Line parameter.

    Get ide Line parameter.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[in] sub_line  0~4 four sub-line make a line.
    @param[out] sign_a  sign bit of a.
    @param[out] sign_b  sign bit of b.
    @param[out] a
    @param[out] b
    @param[out] slope
    @param[out] compare


    @return void
*/
void idec_get_line_para(IDE_ID id, IDE_LINE_NUM ui_num, UINT32 sub_line, BOOL *sign_a, BOOL *sign_b, UINT32 *a, UINT32 *b, UINT32 *slope, UINT32 *compare)
{
	T_IDE_LINE_ATTR line_attr;
	T_IDE_LINE_SLOPE line_slope;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_LINE_0, IDE_LINE_15);

	if ((ui_num < IDE_LINE_0) || (ui_num > IDE_LINE_15)) {
		return;
	}
	i = idec_chkline_num(id, ui_num);

	line_attr.reg = idec_get_reg(id, IDE_LINE0_0_ATTR_OFS + i * IDE_LINE_SET_REGION_OFFSET + sub_line * IDE_EACH_LINE_REGION_OFFSET);
	line_slope.reg = idec_get_reg(id, IDE_LINE0_0_SLOPE_ATTR_OFS + i * IDE_LINE_SET_REGION_OFFSET + sub_line * IDE_EACH_LINE_REGION_OFFSET);

	*sign_a = line_attr.bit.LINE_X_A_PARAM_SIGN_BIT;
	*sign_b = line_attr.bit.LINE_X_B_PARAM_SIGN_BIT;
	*a = line_attr.bit.LINE_X_A_PARAM;
	*b = line_attr.bit.LINE_X_B_PARAM;

	*slope = line_slope.bit.LINE_X_SLOPE_PARAM;
	*compare = line_slope.bit.LINE_X_COMPARE;
}



/**
    Set ide Line related parameter

    Set ide Line related parameter

    @note (x1, y1) to (x2, y2)
    @param[in] id           ide ID
    @param[in] ui_num        number of Line (nth line)
    @param[in] ui_x1         The start position of line
    @param[in] ui_y1         The start position of line
    @param[in] ui_x2         The end position of line
    @param[in] ui_y2         The end position of line
    @param[in] ui_boarder    minmum value is 2 (line need to compose by 4 point) => better set odd value


    @return
*/

ER idec_draw_line(IDE_ID id, IDE_LINE_NUM ui_num, UINT32 ui_x1, UINT32 ui_y1, UINT32 ui_x2, UINT32 ui_y2, UINT32 ui_boarder, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	UINT32  ui_line_en;
	UINT32  line_set;
	UINT32  line_num;
	UINT32  v_left, v_right;
	INT32   i_line0_a, i_line0_b, i_line0_c;    // line0: top_left <--> top_right
	UINT32  ui_line0_compare;
	INT32   i_line1_a, i_line1_b, i_line1_c;    // line1: top_right <--> buttom_right
	UINT32  ui_line1_compare;
	INT32   i_line2_a, i_line2_b, i_line2_c;    // line2: bottom_right <--> bottom_left
	UINT32  ui_line2_compare;
	INT32   i_line3_a, i_line3_b, i_line3_c;    // line3: bottom_left <--> top_left
	UINT32  ui_line3_compare;
	STRUCT_POINT        center_p = {0};
	STRUCT_POINT        points[4];
	STRUCT_POINT        *p_top_left, *p_top_right;
	STRUCT_POINT        *p_bottom_left, *p_bottom_right;

	INT32  x1, y1, x2, y2, x3, y3, x4, y4;


	T_IDE_LINE_ATTR     line0_reg1 = {0};
	T_IDE_LINE_ATTR     line1_reg1 = {0};
	T_IDE_LINE_ATTR     line2_reg1 = {0};
	T_IDE_LINE_ATTR     line3_reg1 = {0};
	T_IDE_LINE_SLOPE    line0_reg2 = {0};
	T_IDE_LINE_SLOPE    line1_reg2 = {0};
	T_IDE_LINE_SLOPE    line2_reg2 = {0};
	T_IDE_LINE_SLOPE    line3_reg2 = {0};

	//1st = 0xD60~0xD60+0x20
	//2nd =
	UINT32  ui_line_att_offset[4];
	UINT32  ui_line_slope_offset[4];
	UINT32  ui_y_cb_cr;

	if (ui_boarder < 2) {
		ui_boarder = 2;
		DBG_WRN("Minimum border is 2, force set as 2\r\n");
	}

	line_set = __builtin_ctz(ui_num);

	if (line_set >= IDE_LINE_EN_NUM) {
		DBG_ERR("Line set exceed 16 %d\r\n", (int)line_set);
		return E_SYS;
	}
	for (line_num = 0; line_num < 4; line_num++) {
		ui_line_att_offset[line_num] = IDE_LINE0_0_ATTR_OFS + IDE_LINE_SET_REGION_OFFSET * line_set + line_num * IDE_EACH_LINE_REGION_OFFSET;
		ui_line_slope_offset[line_num] = IDE_LINE0_0_SLOPE_ATTR_OFS + IDE_LINE_SET_REGION_OFFSET * line_set + line_num * IDE_EACH_LINE_REGION_OFFSET;;
		DBG_IND("Line set[%d]=>[%d] line =>  AttrOffset = 0x%02x\r\n", (int)line_set, (int)line_num, (unsigned int)ui_line_att_offset[line_num]);
		DBG_IND("Line set[%d]=>[%d] line => SlopeOffset = 0x%02x\r\n", (int)line_set, (int)line_num, (unsigned int)ui_line_att_offset[line_num]);
	}
	ui_y_cb_cr = IDE_LINE0_COLOR_OFS + IDE_LINE_SET_REGION_OFFSET * line_set;
	DBG_IND("Set[%d]colorofset = 0x%02x\r\n", (int)line_set, (unsigned int)ui_y_cb_cr);
	DBG_IND("Boarder=[%d]\r\n", (int)ui_boarder);

	if ((ui_boarder % 2) == 0) {
		//even boarder
		v_left = (ui_boarder >> 1) - 1;
		v_right = (ui_boarder >> 1);
	} else {
		// odd boarder
		v_left = v_right = (ui_boarder >> 1);
	}


	if (ui_y1 == ui_y2) {
		//Horizential
#if 0
		if (ui_y1 < v_left) {
			y1 = 0;
		} else {
			y1 = ui_y1 - v_left;
		}
		if (ui_y2 < v_left) {
			y3 = 0;
		} else {
			y3 = ui_y2 - v_left;
		}
#endif
		y1 = ui_y1 - v_left;
		y3 = ui_y2 - v_left;

		y2 = ui_y1 + v_right;
		y4 = ui_y2 + v_right;

		x1 = x2 = ui_x1;
		x3 = x4 = ui_x2;
	} else {
#if 0
		if (ui_x1 < v_left) {
			x1 = 0;
		} else {
			x1 = ui_x1 - v_left;
		}

		if (ui_x2 < v_left) {
			x3 = 0;

		} else {
			x3 = ui_x2 - v_left;
		}
#endif
		x1 = ui_x1 - v_left;
		x3 = ui_x2 - v_left;

		x2 = ui_x1 + v_right;
		x4 = ui_x2 + v_right;

		y1 = y2 = ui_y1;
		y3 = y4 = ui_y2;
	}





	// Calculate line equations
	p_top_left = &points[0];
	p_top_right = &points[1];
	p_bottom_right = &points[2];
	p_bottom_left = &points[3];

	p_top_left->x = x1;
	p_top_left->y = y1;
	p_top_right->x = x2;
	p_top_right->y = y2;
	p_bottom_right->x = x4;
	p_bottom_right->y = y4;
	p_bottom_left->x = x3;
	p_bottom_left->y = y3;

	DBG_IND("(x1, y1)=(%d, %d), (x2, y2)=(%d, %d)\r\n", (int)x1, (int)y1, (int)x2, (int)y2);
	DBG_IND("(x3, y3)=(%d, %d), (x4, y4)=(%d, %d)\r\n", (int)x3, (int)y3, (int)x4, (int)y4);


	DBG_IND("Top Left %d, %d\r\n", (int)p_top_left->x, (int)p_top_left->y);
	DBG_IND("Top Right %d, %d\r\n", (int)p_top_right->x, (int)p_top_right->y);
	DBG_IND("Bottom Right %d, %d\r\n", (int)p_bottom_right->x, (int)p_bottom_right->y);
	DBG_IND("Bottom Left %d, %d\r\n", (int)p_bottom_left->x, (int)p_bottom_left->y);

	p_top_left->p_cw = p_top_left->p_next = p_top_right;
	p_top_left->p_ccw = p_top_left->p_prev = p_bottom_left;
	p_top_right->p_cw = p_top_right->p_next = p_bottom_right;
	p_top_right->p_ccw = p_top_right->p_prev = p_top_left;
	p_bottom_right->p_cw = p_bottom_right->p_next = p_bottom_left;
	p_bottom_right->p_ccw = p_bottom_right->p_prev = p_top_right;
	p_bottom_left->p_cw = p_bottom_left->p_next = p_top_left;
	p_bottom_left->p_ccw = p_bottom_left->p_prev = p_bottom_right;

	get_center_point(p_top_left, &center_p);

	// line0 (p_top_left --> p_top_right)
	generate_line_params(p_top_left, p_top_right, &center_p,
						 &i_line0_a, &i_line0_b, &i_line0_c, &ui_line0_compare);

	// line1 (p_top_right --> p_bottom_right)
	generate_line_params(p_top_right, p_bottom_right, &center_p,
						 &i_line1_a, &i_line1_b, &i_line1_c, &ui_line1_compare);

	// line2 (p_bottom_right --> p_bottom_left)
	generate_line_params(p_bottom_right, p_bottom_left, &center_p,
						 &i_line2_a, &i_line2_b, &i_line2_c, &ui_line2_compare);

	// line3 (p_bottom_left --> p_top_left)
	generate_line_params(p_bottom_left, p_top_left, &center_p,
						 &i_line3_a, &i_line3_b, &i_line3_c, &ui_line3_compare);

	line0_reg1.bit.LINE_X_A_PARAM = abs(i_line0_a);
	line0_reg1.bit.LINE_X_A_PARAM_SIGN_BIT = (i_line0_a < 0) ? 1 : 0;
	line0_reg1.bit.LINE_X_B_PARAM = abs(i_line0_b);
	line0_reg1.bit.LINE_X_B_PARAM_SIGN_BIT = (i_line0_b < 0) ? 1 : 0;
	line0_reg2.bit.LINE_X_SLOPE_PARAM = abs(i_line0_c);
	line0_reg2.bit.LINE_X_COMPARE = ui_line0_compare;

	line1_reg1.bit.LINE_X_A_PARAM = abs(i_line1_a);
	line1_reg1.bit.LINE_X_A_PARAM_SIGN_BIT = (i_line1_a < 0) ? 1 : 0;
	line1_reg1.bit.LINE_X_B_PARAM = abs(i_line1_b);
	line1_reg1.bit.LINE_X_B_PARAM_SIGN_BIT = (i_line1_b < 0) ? 1 : 0;
	line1_reg2.bit.LINE_X_SLOPE_PARAM = abs(i_line1_c);
	line1_reg2.bit.LINE_X_COMPARE = ui_line1_compare;

	line2_reg1.bit.LINE_X_A_PARAM = abs(i_line2_a);
	line2_reg1.bit.LINE_X_A_PARAM_SIGN_BIT = (i_line2_a < 0) ? 1 : 0;
	line2_reg1.bit.LINE_X_B_PARAM = abs(i_line2_b);
	line2_reg1.bit.LINE_X_B_PARAM_SIGN_BIT = (i_line2_b < 0) ? 1 : 0;
	line2_reg2.bit.LINE_X_SLOPE_PARAM = abs(i_line2_c);
	line2_reg2.bit.LINE_X_COMPARE = ui_line2_compare;

	line3_reg1.bit.LINE_X_A_PARAM = abs(i_line3_a);
	line3_reg1.bit.LINE_X_A_PARAM_SIGN_BIT = (i_line3_a < 0) ? 1 : 0;
	line3_reg1.bit.LINE_X_B_PARAM = abs(i_line3_b);
	line3_reg1.bit.LINE_X_B_PARAM_SIGN_BIT = (i_line3_b < 0) ? 1 : 0;
	line3_reg2.bit.LINE_X_SLOPE_PARAM = abs(i_line3_c);
	line3_reg2.bit.LINE_X_COMPARE = ui_line3_compare;

	idec_set_reg(id, ui_line_att_offset[0], line0_reg1.reg);
	idec_set_reg(id, ui_line_slope_offset[0], line0_reg2.reg);

	idec_set_reg(id, ui_line_att_offset[1], line1_reg1.reg);
	idec_set_reg(id, ui_line_slope_offset[1], line1_reg2.reg);

	idec_set_reg(id, ui_line_att_offset[2], line2_reg1.reg);
	idec_set_reg(id, ui_line_slope_offset[2], line2_reg2.reg);

	idec_set_reg(id, ui_line_att_offset[3], line3_reg1.reg);
	idec_set_reg(id, ui_line_slope_offset[3], line3_reg2.reg);

	idec_set_reg(id, ui_y_cb_cr, ((ui_y << 8) | (ui_cb << 16) | (ui_cr << 24)));

	ui_line_en = idec_get_reg(id, IDE_LINE_EN_OFS);
	idec_set_reg(id, IDE_LINE_EN_OFS, ui_line_en | (ui_num));

	return E_OK;
}

/**
    Get ide FD window position

    Get ide FD window position

    @param[in] id   ide ID
    @param[in] ui_num
    @param[out] ui_x The start position at X-axis.
    @param[out] ui_y The start position at Y-axis.

    @return void
*/
void idec_get_fd_win_pos(IDE_ID id, IDE_FD_NUM ui_num, UINT32 *ui_x, UINT32 *ui_y)
{
	T_IDE_FD_ATTR0 reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;
	reg.reg = idec_get_reg(id, IDE_FD_ATTR0_OFS + i);

	*ui_x = reg.bit.x;
	*ui_y = reg.bit.y;
}

/**
    Set ide FD window dimension.

    Set ide FD window dimension.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[in] ui_win_w window width.
    @param[in] ui_win_h window height.

    @return void
*/
void idec_set_fd_win_dim(IDE_ID id, IDE_FD_NUM ui_num, UINT32 ui_win_w, UINT32 ui_win_h)
{
	T_IDE_FD_ATTR1 reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;
	reg.reg = idec_get_reg(id, IDE_FD_ATTR1_OFS + i);

	reg.bit.w = ui_win_w;
	reg.bit.h = ui_win_h;

	idec_set_reg(id, IDE_FD_ATTR1_OFS + i, reg.reg);
}

/**
    Get ide FD window dimension.

    Get ide FD window dimension.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[out] ui_win_w window width.
    @param[out] ui_win_h window height.

    @return void
*/
void idec_get_fd_win_dim(IDE_ID id, IDE_FD_NUM ui_num, UINT32 *ui_win_w, UINT32 *ui_win_h)
{
	T_IDE_FD_ATTR1 reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;
	reg.reg = idec_get_reg(id, IDE_FD_ATTR1_OFS + i);

	*ui_win_w = reg.bit.w;
	*ui_win_h = reg.bit.h;
}


/**
    Set ide FD Border.

    Set ide FD Border.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[in] ui_bor_h Border of horizontal.
    @param[in] ui_bor_v Border of vertical.

    @return void
*/
void idec_set_fd_win_bord(IDE_ID id, IDE_FD_NUM ui_num, UINT32 ui_bor_h, UINT32 ui_bor_v)
{
	T_IDE_FD_ATTR0 reg;
	T_IDE_FD_ATTR1 reg1;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;
	reg.reg = idec_get_reg(id, IDE_FD_ATTR0_OFS + i);

	reg.bit.border_h = ui_bor_h;

	idec_set_reg(id, IDE_FD_ATTR0_OFS + i, reg.reg);

	reg1.reg = idec_get_reg(id, IDE_FD_ATTR1_OFS + i);

	reg1.bit.border_v = ui_bor_v;

	idec_set_reg(id, IDE_FD_ATTR1_OFS + i, reg1.reg);
}

/**
    Get ide FD Border.

    Get ide FD Border.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[out] ui_bor_h Border of horizontal.
    @param[out] ui_bor_v Border of vertical.

    @return void
*/
void idec_get_fd_win_bord(IDE_ID id, IDE_FD_NUM ui_num, UINT32 *ui_bor_h, UINT32 *ui_bor_v)
{
	T_IDE_FD_ATTR0 reg;
	T_IDE_FD_ATTR1 reg1;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;
	reg.reg = idec_get_reg(id, IDE_FD_ATTR0_OFS + i);

	*ui_bor_h = reg.bit.border_h;

	reg1.reg = idec_get_reg(id, IDE_FD_ATTR1_OFS + i);

	*ui_bor_v = reg1.bit.border_v;

}

/**
    Set ide FD color.

    Set ide FD color.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[in] ui_y  color of y channel.
    @param[in] ui_cb color of cb channel.
    @param[in] ui_cr color of cr channel.

    @return void
*/
void idec_set_fd_color(IDE_ID id, IDE_FD_NUM ui_num, UINT8 ui_y, UINT8 ui_cb, UINT8 ui_cr)
{
	T_IDE_FD_COLOR reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;

	reg.reg = idec_get_reg(id, IDE_FD_COLOR_OFS + i);

	reg.bit.y = ui_y;
	reg.bit.cb = ui_cb;
	reg.bit.cr = ui_cr;

	idec_set_reg(id, IDE_FD_COLOR_OFS + i, reg.reg);
}

/**
    Get ide FD color.

    Get ide FD color.

    @param[in] id   ide ID
    @param[in] ui_num
    @param[out] ui_y  color of y channel.
    @param[out] ui_cb color of cb channel.
    @param[out] ui_cr color of cr channel.

    @return void
*/
void idec_get_fd_color(IDE_ID id, IDE_FD_NUM ui_num, UINT8 *ui_y, UINT8 *ui_cb, UINT8 *ui_cr)
{
	T_IDE_FD_COLOR reg;
	UINT32 i;

	//coverity[unsigned_compare]
	ide_chk_range(ui_num, IDE_FD_0, IDE_FD_15);

	if ((ui_num < IDE_FD_0) || (ui_num > IDE_FD_15)) {
		return;
	}

	i = (idec_chkfd_num(id, ui_num) * 3) * 4;

	reg.reg = idec_get_reg(id, IDE_FD_COLOR_OFS + i);

	*ui_y = reg.bit.y;
	*ui_cb = reg.bit.cb;
	*ui_cr = reg.bit.cr;
}

//@}
