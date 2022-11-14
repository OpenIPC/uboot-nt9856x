/*
    Display object for driving display engine

    @file       display.c
    @ingroup    mISYSDisp
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#include "./include/display_obj_platform.h"

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

#if 0
#else
//NVTVER_VERSION_ENTRY(disp, 1, 00, 000, /* branch version */ 00);
#endif

volatile UINT32          gui_disp_obj_opened[DISP_MAX] = {0};



/*
    DISPAY OBJECT Definition
*/
static DISP_OBJ    gp_disp_obj[DISP_MAX] = {
	{DISP_1, disp_get_disp1_cap, disp_open_disp1, disp_close_disp1, disp_is_disp1_opened, disp_set_disp1_control, disp_set_disp1_layer_ctrl, disp_set_disp1_device_ctrl, disp_set_disp1_load, disp_wait_disp1_frame_end, disp_wait_disp1_yuv_output_done, NULL},
};

/*
    Display Capability
*/
const DISP_CAP  disp_cap[DISP_MAX] = {
	// DISP-1 CAP
	{
		//Display
		(DISPABI_SUBPIXEL | DISPABI_CC | DISPABI_RGBGAMMA | DISPABI_DITHER | DISPABI_ICST) | \
		(DISPABI_TVOUT | DISPABI_MIOUT | DISPABI_HDMIOUT | DISPABI_RGBSEROUT8 | DISPABI_YUVSEROUT8 | DISPABI_RGBPARALLEL) | \
		(DISPABI_CCIR601OUT8 | DISPABI_CCIR656OUT8 | DISPABI_CCIR601OUT16 | DISPABI_CCIR656OUT16 | DISPABI_RGBDELTA16) | \
		(DISPABI_MIPIDSI),
		//V1
		(DISPLYR_EXIST | DISPLYR_YUV422PACK | DISPLYR_YUV420PACK),
		//V2
		(DISPLYR_EXIST | DISPLYR_YUV422PACK | DISPLYR_YUV420PACK),
		//O1
		(DISPLYR_EXIST | DISPLYR_PALE1 | DISPLYR_PALE2 | DISPLYR_PALE4 | DISPLYR_PALE8 | \
		DISPLYR_ARGB8565 | DISPLYR_ARGB4565),
		//O2
		(DISPLYR_EXIST | DISPLYR_PALE1 | DISPLYR_PALE2 | DISPLYR_PALE4 | DISPLYR_PALE8)
	},

};

/*
    The Display device object for each display object
*/
volatile PDISPDEV_OBJ  gp_disp_dev[DISP_MAX][DISPDEV_ID_MAX - 1] = {
	/* Display Object 1 */
	{
		NULL,               // Embedded TV-NTSC/PAL
		NULL,               // Embedded HDMI
		NULL,               // panel
		NULL                // Embedded MIPIDSI
	},
};



/** \addtogroup  mISYSDisp*/
//@{



/**
    Get Specified Display control object

    This object can be used to control display device such as TV/panel/HDMI.
    And also the display engine controls.

    @param[in] disp_id   Specified display control object ID.

    @return The pointer of the display control object.
*/
PDISP_OBJ disp_get_display_object(DISP_ID disp_id)
{
	if (disp_id >= DISP_MAX) {
		DBG_ERR("display object %d not exist!!", (int)disp_id);
		return NULL;
	}
	return &(gp_disp_obj[disp_id]);
}
#if !(defined __UITRON || defined __ECOS || defined __FREERTOS)
EXPORT_SYMBOL(disp_get_display_object);
#endif
#if 0
/**
    Get display device object version information

    Get display device object version information. Return pointer of 9 characters (include
    null terminated character). Example: 2.00.002

    @return Pointer of 8 characters of version information + null terminated character.
*/
CHAR *disp_get_ver_info(void)
{
	return g_disp_obj_ver_info;
}

/**
    Get display device object build date and time

    Get display device object build date and time. Return pointer of 22 characters (include
    null terminated character). Example: Mar 19 2009, 18:29:28

    @return Pointer of 21 characters of build date and time + null terminated character.
*/
CHAR *disp_get_build_date(void)
{
	return g_disp_obj_build_date;
}
#endif


//@}

