/*
    Display object 1 for driving display engine

    @file       disp1.c
    @ingroup    mISYSDisp
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/
#include "./include/display_obj_platform.h"

ER dispdev1_ioctrl(DISPDEV_IOCTRL_OP disp_dev_ctrl, PDISPDEV_IOCTRL_PARAM p_disp_dev_param);
/*
#if _FPGA_EMULATION_
#define DEV_RGB_SERIAL_FORCE_DIV_ZERO   0
#endif
*/
extern UINT32           gui_disp_obj_opened[DISP_MAX];
extern const DISP_CAP   disp_cap[DISP_MAX];
extern PDISPDEV_OBJ     gp_disp_dev[DISP_MAX][DISPDEV_ID_MAX - 1];

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

static volatile DISP_INFO g_disp1_info = {
	/* Display DATA */
	{
		DISPDEV_ID_NULL,            // active_dev
		DISPCTRL_SRCCLK_DEFAULT,    // src_clk
		0x4ED,                      // ui_global_win_width
		0xF0,                       // ui_global_win_height
		{{0x140, 0xF0, 0x140 }, {0x140, 0xF0, 0x140 } },      // pui_vdo_buf_dim[DISPVDO_LYRNUM][3]
		{{0x140, 0xF0 }, {0x140, 0xF0 } },              // pui_vdo_win_dim[DISPVDO_LYRNUM][2]
		{{0x140, 0xF0, 0x140 }, {0x140, 0xF0, 0x140 } },      // pui_osd_buf_dim[DISPVDO_LYRNUM][3]
		{{0x140, 0xF0 }, {0x140, 0xF0 } },              // pui_osd_win_dim[DISPVDO_LYRNUM][2]
		{DISPACTBUF_0 },             // act_vdo_index[LYR]
		{DISPACTBUF_0 },             // act_osd_index[LYR]
		{{{0, 0, 0 }, {0, 0, 0 }, {0, 0, 0 } }, {{0, 0, 0 }, {0, 0, 0 }, {0, 0, 0 } } }, // p_vdo_buf_addr[LYR][DISPACTBUF_NUM][DISPVDOBUF_MAX]
		{{{0, 0 }, {0, 0 }, {0, 0 } }, {{0, 0 }, {0, 0 }, {0, 0 } } }, // p_osd_buf_addr[LYR][DISPACTBUF_NUM][DISPOSDBUF_MAX]
		{{0, 0 }, {0, 0 } },           // pui_vdo_buf_xy[DISPVDO_LYRNUM][2]
		{{0, 0 }, {0, 0 } },           // pui_osd_buf_xy[DISPOSD_LYRNUM][2]
	},

	/* Display DEVICE DATA */
	{
		320,                        // ui_buf_width   for project layer usage
		240,                        // ui_buf_height  for project layer usage
		320,                        // ui_win_width   for project layer usage
		240,                        // ui_win_height  for project layer usage
		DISPDEV_TVADJUST_DEFAULT,   // tv_adjust
		NULL,                       // panel_adjust
		HDMI_AUDIO32KHZ,            // HDMI audio format
		HDMI_640X480P60,            // HDMI video format

		DISPDEV_LCDCTRL_SIF,        // lcd_ctrl
#if defined(_BSP_NA51055_)
        SIF_CH2,                    // sif_ch
#else
        SIF_CH1,                    // sif_ch
#endif
		0xFFFFFFF,                  // ui_gpio_sif_sen
		0xFFFFFFF,                  // ui_gpio_sif_clk
		0xFFFFFFF,                  // ui_gpio_sif_data

		0xFFFFFFF,                  // Current Backlight Lvl
		0xFFFFFFF,                  // ui_gpio_sif_data
		DISPDEV_TYPE_RGB_SERIAL,    // display device
		FALSE,                      // TV user define parameter enable/disable
		{0, 0, 0, 0, 0, 0 },         // TV parameter (NTSC)
		{0, 0, 0, 0, 0, 0 },         // TV parameter (PAL)
		FALSE                       // TV full screen
	}
};

#if 1
static void disp_translate_buf_address(DISPLAYER layer)
{
	IDE_HOR_READ        l2r;
	IDE_VER_READ        t2b;
	IDE_COLOR_FORMAT    fmt;
	UINT32              i, fac_w = 0, fac_h = 0;
	UINT32              addr_y, addr_u, addr_v;
	UINT32              ui_x, ui_y;

	displyr1_debug(("disp_lyr: translateBufAddress (%d)\r\n", (int)layer));

	if (layer == DISPLAYER_VDO1) {
		ide_get_v1_read_order(&l2r, &t2b);
		ide_get_v1_fmt(&fmt);

		if (fmt == COLOR_YCBCR422) {
			fac_w = 1;
			fac_h = 0;
		} else if (fmt == COLOR_YCBCR420) {
			fac_w = 1;
			fac_h = 1;
		} else if (fmt == COLOR_YCC422P) {
			fac_w = 0;
			fac_h = 0;
		} else if (fmt == COLOR_YCC420P) {
			fac_w = 0;
			fac_h = 1;
		} else {
			fac_w = 0;
			fac_h = 0;
		}



		//for (i=DISPACTBUF_0; i<DISPACTBUF_NUM; i++)
		i = g_disp1_info.disp_data.act_vdo_index[DISP_VDO1];
		//{
		addr_y = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][i][DISPVDOBUF_Y];
		addr_u = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][i][DISPVDOBUF_CB];
		addr_v = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][i][DISPVDOBUF_CR];
		ui_x  = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO1][DISPBUF_X];
		ui_y  = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO1][DISPBUF_Y];

		if ((ui_x > g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W])) {
			ui_x = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
		}

		if ((ui_y > g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H])) {
			ui_y = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
		}

		addr_y = addr_y + (ui_x >> fac_w) + ((ui_y) * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L]));
		addr_u = addr_u + (ui_x >> fac_w) + ((ui_y >> fac_h) * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L]));
		addr_v = addr_v + (ui_x >> fac_w) + ((ui_y >> fac_h) * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L]));

		if (l2r) {
			addr_y += (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] - 1);
			addr_u += ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] >> fac_w) - 1);
			addr_v += ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] >> fac_w) - 1);
		}

		if (t2b) {
			addr_y +=  g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L]       * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] - 1);
			addr_u += (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] >> fac_w) * ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] >> fac_h) - 1);
			addr_v += (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] >> fac_w) * ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] >> fac_h) - 1);
		}

		/*switch(i)
		{
		case DISPACTBUF_0:
		    ide_set_v1_buf0_addr(addr_y, addr_u, addr_v);
		    break;

		case DISPACTBUF_1:
		    ide_set_v1_buf1_addr(addr_y, addr_u, addr_v);
		    break;

		case DISPACTBUF_2:
		    ide_set_v1_buf2_addr(addr_y, addr_u, addr_v);
		    break;

		default:
		    break;
		}*/
		ide_set_v1_buf0_addr(addr_y, addr_u, addr_v);

		//}

	} else if (layer == DISPLAYER_VDO2) {
		ide_get_v2_read_order(&l2r, &t2b);
		ide_get_v2_fmt(&fmt);

		if (fmt == COLOR_YCBCR422) {
			fac_w = 1;
			fac_h = 0;
		} else if (fmt == COLOR_YCBCR420) {
			fac_w = 1;
			fac_h = 1;
		} else if (fmt == COLOR_YCC422P) {
			fac_w = 0;
			fac_h = 0;
		} else if (fmt == COLOR_YCC420P) {
			fac_w = 0;
			fac_h = 1;
		} else {
			fac_w = 0;
			fac_h = 0;
		}


		i = g_disp1_info.disp_data.act_vdo_index[DISP_VDO2];
		//for (i=DISPACTBUF_0; i<DISPACTBUF_NUM; i++)
		//{
		addr_y = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][i][DISPVDOBUF_Y];
		addr_u = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][i][DISPVDOBUF_CB];
		addr_v = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][i][DISPVDOBUF_CR];
		ui_x  = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO2][DISPBUF_X];
		ui_y  = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO2][DISPBUF_Y];

		if ((ui_x > g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W])) {
			ui_x = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W];
		}

		if ((ui_y > g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H])) {
			ui_y = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H];
		}

		addr_y = addr_y + (ui_x >> fac_w) + ((ui_y) * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L]));
		addr_u = addr_u + (ui_x >> fac_w) + ((ui_y >> fac_h) * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L]));
		addr_v = addr_v + (ui_x >> fac_w) + ((ui_y >> fac_h) * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L]));

		if (l2r) {
			addr_y += (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] - 1);
			addr_u += ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] >> fac_w) - 1);
			addr_v += ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] >> fac_w) - 1);
		}

		if (t2b) {
			addr_y +=  g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L]       * (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] - 1);
			addr_u += (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] >> fac_w) * ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] >> fac_h) - 1);
			addr_v += (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] >> fac_w) * ((g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] >> fac_h) - 1);
		}

		/*switch(i)
		{
		case DISPACTBUF_0:
		    ide_set_v2_buf0_addr(addr_y, addr_u, addr_v);
		    break;

		case DISPACTBUF_1:
		    ide_set_v2_buf1_addr(addr_y, addr_u, addr_v);
		    break;

		case DISPACTBUF_2:
		    ide_set_v2_buf2_addr(addr_y, addr_u, addr_v);
		    break;

		default:
		    break;
		}*/
		ide_set_v2_buf0_addr(addr_y, addr_u, addr_v);

		//}

	} else if (layer == DISPLAYER_OSD1) {
		UINT32 index, width;
		UINT32 width_alpha = 0;

		index = g_disp1_info.disp_data.act_osd_index[DISP_OSD1];

		ide_get_o1_read_order(&l2r, &t2b);
		ide_get_o1_fmt(&fmt);
		switch (fmt) {
		case COLOR_1_BIT:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] + 7) >> 3;
			fac_w = 3;
			break;

		case COLOR_2_BIT:
			width = ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] << 1) + 7) >> 3;
			fac_w = 2;
			break;

		case COLOR_4_BIT:
			width = ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] << 2) + 7) >> 3;
			fac_w = 1;
			break;

		case COLOR_8_BIT:
		default:
			width = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
			fac_w = 0;
			break;

		case COLOR_ARGB4565:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W]) << 1;
			width_alpha = ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] << 2) + 7) >> 3;
			break;

		case COLOR_ARGB8565:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W]) << 1;
			width_alpha = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
			break;

		case COLOR_ARGB8888:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W]) << 2;
			width_alpha = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
			break;

		case COLOR_ARGB4444:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W]) << 1;
			width_alpha = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
			break;

		case COLOR_ARGB1555:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W]) << 1;
			width_alpha = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
			break;

		}

		addr_y = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][index][DISPOSDBUF_PALE];
		addr_u = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][index][DISPOSDBUF_ALPHA];
		ui_x  = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD1][DISPBUF_X];
		ui_y  = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD1][DISPBUF_Y];

		if ((ui_x > g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W])) {
			ui_x = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
		}

		if ((ui_y > g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H])) {
			ui_y = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
		}

		if ((fmt == COLOR_ARGB4565) || (fmt == COLOR_ARGB8565)) {
			if (fmt == COLOR_ARGB8565) {
				addr_u = addr_u + (ui_x) + ((ui_y) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 1));
			} else { //4565
				addr_u = addr_u + (ui_x >> 1) + ((ui_y) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 2));
			}
			addr_y = addr_y + (ui_x << 1) + ((ui_y) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L]));
		} else {
			addr_y = addr_y + (ui_x >> fac_w) + ((ui_y) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L]));
		}

		if (l2r) {
			addr_y += (width - 1);
			if ((fmt == COLOR_ARGB4565) || (fmt == COLOR_ARGB8565)) {
				addr_u += (width_alpha - 1);
			}
		}

		if (t2b) {
			if ((fmt == COLOR_ARGB4565) || (fmt == COLOR_ARGB8565)) {
				if ((fmt == COLOR_ARGB4565)) {
					addr_u += ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 2) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] - 1));
				} else {
					addr_u += ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 1) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] - 1));
				}
				addr_y += (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] - 1));
			} else {
				addr_y += (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] - 1));
			}
		}

		ide_set_o1_buf_addr(addr_y);
		if ((fmt == COLOR_ARGB4565) || (fmt == COLOR_ARGB8565)) {
			//DBG_IND("DISP : O1Addr[BUF_ALPHA] = 0x%08x dma_getPhyAddr(addr_u) = 0x%08x", addr_u, display_obj_platform_va2pa(addr_u));
			ide_set_o1_buf_alpha_addr(addr_u);
		}
	}
#if IDE1_OSD2_EXIST
	else if (layer == DISPLAYER_OSD2) {
		UINT32 index, width;

		index = g_disp1_info.disp_data.act_osd_index[DISP_OSD2];

		ide_get_o2_read_order(&l2r, &t2b);
		ide_get_o2_fmt(&fmt);
		switch (fmt) {
		case COLOR_1_BIT:
			width = (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W] + 7) >> 3;
			fac_w = 3;
			break;

		case COLOR_2_BIT:
			width = ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W] << 1) + 7) >> 3;
			fac_w = 2;
			break;

		case COLOR_4_BIT:
			width = ((g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W] << 2) + 7) >> 3;
			fac_w = 1;
			break;

		case COLOR_8_BIT:
		default:
			width = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W];
			fac_w = 0;
			break;

		}

		addr_y = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][index][DISPOSDBUF_PALE];
		ui_x  = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD2][DISPBUF_X];
		ui_y  = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD2][DISPBUF_Y];

		if ((ui_x > g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W])) {
			ui_x = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W];
		}

		if ((ui_y > g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H])) {
			ui_y = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H];
		}

		addr_y = addr_y + (ui_x >> fac_w) + ((ui_y) * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L]));

		if (l2r) {
			addr_y += (width - 1);
		}

		if (t2b) {
			addr_y += (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L] * (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H] - 1));
		}

		ide_set_o2_buf_addr(addr_y);
	}
#endif

}

#if DISP_PATCHBUF_RESIZE
static void disp_translate_buf_size(DISPLAYER layer)
{
	UINT32 ui_x, ui_y;
	UINT32  scale_x, scale_y;
	UINT64 temp_x, temp_y;
	INT32  delta_x, delta_y;
	IDE_COLOR_FORMAT ui_fmt;

	displyr1_debug(("disp_lyr: translateBufSize (%d)\r\n", (int)layer));
	if ((layer == DISPLAYER_VDO1)) {

		temp_x = ((UINT64)g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] << 15);
		display_obj_platform_do_div(&temp_x, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W]);
		temp_y = ((UINT64)g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] << 12);
		display_obj_platform_do_div(&temp_y, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H]);

		scale_x = (UINT32) temp_x;
		scale_y = (UINT32) temp_y;

		ide_get_v1_win_pos(&ui_x, &ui_y);


		delta_x = (INT32)ui_x + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W] - (INT32)g_disp1_info.disp_data.ui_global_win_width;
		delta_y = (INT32)ui_y + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H] - (INT32)g_disp1_info.disp_data.ui_global_win_height;
		if (delta_x > 0) {
			temp_x = scale_x * ((UINT64)delta_x);
			delta_x = (UINT32) (temp_x >> 15);
		} else {
			delta_x = 0;
		}

		if (delta_y > 0) {
			temp_y = scale_y * ((UINT64)delta_y);
			delta_y = (UINT32) (temp_y >> 12);
		} else {
			delta_y = 0;
		}

		ide_set_v1_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] - delta_x, g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] - delta_y, (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] >> 2));
	} else if ((layer == DISPLAYER_VDO2)) {
		temp_x = ((UINT64)g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] << 15);
		display_obj_platform_do_div(&temp_x, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W]);
		temp_y = ((UINT64)g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] << 12);
		display_obj_platform_do_div(&temp_y, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H]);

		scale_x = (UINT32) temp_x;
		scale_y = (UINT32) temp_y;

		ide_get_v2_win_pos(&ui_x, &ui_y);

		delta_x = (INT32)ui_x + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W] - (INT32)g_disp1_info.disp_data.ui_global_win_width;
		delta_y = (INT32)ui_y + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H] - (INT32)g_disp1_info.disp_data.ui_global_win_height;

		if (delta_x > 0) {
			temp_x = scale_x * ((UINT64)delta_x);
			delta_x = (UINT32) (temp_x >> 15);
		} else {
			delta_x = 0;
		}

		if (delta_y > 0) {
			temp_y = scale_y * ((UINT64)delta_y);
			delta_y = (UINT32) (temp_y >> 12);
		} else {
			delta_y = 0;
		}

		ide_set_v2_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] - delta_x, g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] - delta_y, (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] >> 2));

	} else if ((layer == DISPLAYER_OSD1)) {

		temp_x = ((UINT64)g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_OSD1][DISPDIM_W] << 15);
		display_obj_platform_do_div(&temp_x, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD1][DISPDIM_W]);
		temp_y = ((UINT64)g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_OSD1][DISPDIM_H] << 12);
		display_obj_platform_do_div(&temp_y, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD1][DISPDIM_H]);

		scale_x = (UINT32) temp_x;
		scale_y = (UINT32) temp_y;

		ide_get_o1_win_pos(&ui_x, &ui_y);

		delta_x = (INT32)ui_x + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD1][DISPDIM_W] - (INT32)g_disp1_info.disp_data.ui_global_win_width;
		delta_y = (INT32)ui_y + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD1][DISPDIM_H] - (INT32)g_disp1_info.disp_data.ui_global_win_height;

		if (delta_x > 0) {
			temp_x = scale_x * ((UINT64)delta_x);
			delta_x = (UINT32) (temp_x >> 15);
		} else {
			delta_x = 0;
		}

		if (delta_y > 0) {
			temp_y = scale_y * ((UINT64)delta_y);
			delta_y = (UINT32) (temp_y >> 12);
		} else {
			delta_y = 0;
		}

		idec_get_o1_fmt(IDE_ID_1, &ui_fmt);

		//if ((ui_fmt == COLOR_ARGB4565) || (ui_fmt == COLOR_ARGB8565))
		//{
		//    ide_set_o1_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] - delta_x, g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] - delta_y, (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L]>>2));
		//}
		//else
		//{
		ide_set_o1_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] - delta_x, g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] - delta_y, (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 2));
		//}
	}
#if IDE1_OSD2_EXIST
	else {
		temp_x = (UINT64) (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_OSD2][DISPDIM_W] << 15);
		display_obj_platform_do_div(&temp_x, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD2][DISPDIM_W]);
		temp_y = (UINT64) (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_OSD2][DISPDIM_H] << 12);
		display_obj_platform_do_div(&temp_y, g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD2][DISPDIM_H]);

		scale_x = (UINT32) temp_x;
		scale_y = (UINT32) temp_y;

		ide_get_o2_win_pos(&ui_x, &ui_y);

		delta_x = (INT32)ui_x + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD2][DISPDIM_W] - (INT32)g_disp1_info.disp_data.ui_global_win_width;
		delta_y = (INT32)ui_y + (INT32)g_disp1_info.disp_data.pui_vdo_win_dim[DISP_OSD2][DISPDIM_H] - (INT32)g_disp1_info.disp_data.ui_global_win_height;

		if (delta_x > 0) {
			temp_x = scale_x * ((UINT64)delta_x);
			delta_x = (UINT32) (temp_x >> 15);
		} else {
			delta_x = 0;
		}

		if (delta_y > 0) {
			temp_y = scale_y * ((UINT64)delta_y);
			delta_y = (UINT32) (temp_y >> 12);
		} else {
			delta_y = 0;
		}

		ide_set_o2_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W] - delta_x, g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H] - delta_y, (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L] >> 2));

	}
#endif
}
#endif


#endif

static UINT8 int_clamp(UINT8 prx, UINT8 lb, UINT8 ub)
{
	if (prx < lb) {
		return lb;
	} else if (prx > ub) {
		return ub;
	} else {
		return prx;
	}
}

/*
    Get display object capability

    @return Constant pointer to display capability.
*/
const PDISP_CAP   disp_get_disp1_cap(void)
{
	return (const PDISP_CAP) &disp_cap[DISP_1];
}

/*
    Open display object access

    @return
     - @b E_OK: display object open success.
*/
ER disp_open_disp1(void)
{
	if (!gui_disp_obj_opened[DISP_1]) {
		gui_disp_obj_opened[DISP_1]++;
		return ide_open();
	}

	gui_disp_obj_opened[DISP_1]++;
	return E_OK;
}

/*
    Close display object access

    @return
     - @b E_OK: display object close done.
*/
ER disp_close_disp1(void)
{
	if (gui_disp_obj_opened[DISP_1]) {
		gui_disp_obj_opened[DISP_1]--;
		if (gui_disp_obj_opened[DISP_1]) {
			return E_OK;
		}
	}


	if (g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL) {
		DBG_WRN("Dev1 has not closed!\r\n");
	}

	return E_OK;
}

/*
    Check if the display object access is opened

    @return
     - @b TRUE:  display object is opened.
     - @b FALSE: display object has not opened.
*/
BOOL disp_is_disp1_opened(void)
{
	return (gui_disp_obj_opened[DISP_1] > 0);
}

/*
    Load display configurations.

    @param[in] b_wait_done
     - @b TRUE:  Set load and Wait the configurations activating.
     - @b FALSE: Set load and then return immediately.

    @return void
*/
void disp_set_disp1_load(BOOL b_wait_done)
{
	if (!gui_disp_obj_opened[DISP_1]) {
		return;
	}

	ide_set_load();

	ide_wait_frame_end(b_wait_done);

}

/*
    Wait display frame end

    @return void
*/
void disp_wait_disp1_frame_end(BOOL is_block)
{
	if (!gui_disp_obj_opened[DISP_1]) {
		return;
	}

	ide_wait_frame_end(is_block);
}

/*
    Wait output dram done

    @return void
*/

void disp_wait_disp1_yuv_output_done(void)
{
	if (!gui_disp_obj_opened[DISP_1]) {
		return;
	}

	ide_wait_yuv_output_done();
}


/*
    The Control Interface between Display Object and the display Device
*/
ER dispdev1_ioctrl(DISPDEV_IOCTRL_OP disp_dev_ctrl, PDISPDEV_IOCTRL_PARAM p_disp_dev_param)
{
	if (!gui_disp_obj_opened[DISP_1]) {
		return E_NOEXS;
	}

	switch (disp_dev_ctrl) {

	///////////////////////
	/* SET control group */
	///////////////////////
	case DISPDEV_IOCTRL_SET_ENABLE: {
			dispctr1_debug(("DISPDEVCTR: SET_ENABLE (%d)\r\n", (int)p_disp_dev_param->SEL.SET_ENABLE.b_en));
			if (p_disp_dev_param->SEL.SET_ENABLE.b_en && (!ide_is_opened())) {
				ide_open();
			}

			ide_set_en(p_disp_dev_param->SEL.SET_ENABLE.b_en);
		}
		break;

	case DISPDEV_IOCTRL_SET_DEVICE: {
			//dispctr1_debug(("DISPDEVCTR: SET_DEVICE(%d)\r\n",p_disp_dev_param->SEL.SET_DEVICE.disp_dev_type));
			DBG_IND("DISPDEVCTR: SET_DEVICE(%d)\r\n", (int)p_disp_dev_param->SEL.SET_DEVICE.disp_dev_type);
			g_disp1_info.disp_dev_data.disp_dev_type = p_disp_dev_param->SEL.SET_DEVICE.disp_dev_type;
			DBG_IND("DISPDEVCTR: g_disp1_info.disp_dev_data.disp_dev_type=%d\r\n", (int)g_disp1_info.disp_dev_data.disp_dev_type);
			ide_set_rgbd(FALSE);
			ide_set_through(FALSE);
			switch (p_disp_dev_param->SEL.SET_DEVICE.disp_dev_type) {
			case  DISPDEV_TYPE_RGB_SERIAL:
				ide_set_device(DISPLAY_DEVICE_CASIO2G);
				break;
			case DISPDEV_TYPE_RGBD:
				ide_set_device(DISPLAY_DEVICE_CASIO2G);
				ide_set_rgbd(TRUE);
				break;
			case DISPDEV_TYPE_RGB_THROUGH:
				ide_set_device(DISPLAY_DEVICE_CASIO2G);
				ide_set_through(TRUE);
				break;
			case DISPDEV_TYPE_YUV:
				ide_set_device(DISPLAY_DEVICE_TOPPOLY);
				break;
			case DISPDEV_TYPE_CCIR601_8BIT:
				ide_set_device(DISPLAY_DEVICE_CCIR601);
				break;
			case DISPDEV_TYPE_CCIR656_8BIT:
				ide_set_device(DISPLAY_DEVICE_CCIR656);
				break;
			case DISPDEV_TYPE_CCIR601_16BIT:
				ide_set_device(DISPLAY_DEVICE_CCIR601_16BIT);
				break;
			case DISPDEV_TYPE_CCIR656_16BIT:
				ide_set_device(DISPLAY_DEVICE_CCIR656_16BIT);
				break;
			case DISPDEV_TYPE_RGB_PARALL:
				ide_set_device(DISPALY_DEVICE_PARALLEL);
				break;
			case DISPDEV_TYPE_EMBD_MIPIDSI:
				ide_set_device(DISPLAY_DEVICE_MIPIDSI);
				//dsi_set_config(DSI_CONFIG_ID_SRC, DSI_SRC_IDE);
				break;
			case DISPDEV_TYPE_RGBDELTA_16BIT:
				ide_set_device(DISPLAY_DEVICE_RGB_16BIT);
				break;
			case DISPDEV_TYPE_OUTPUT_DRAM:
				ide_set_device(DISPLAY_DEVICE_OUTPUT_DRAM);
				break;
			case DISPDEV_TYPE_EMBD_TV:
			case DISPDEV_TYPE_MI:
			case DISPDEV_TYPE_EMBD_HDMI:
			case DISPDEV_TYPE_INF_HDMI_16BIT:
			default:
				DBG_WRN("SET_DEVICE no support! (%d)\r\n", (int)p_disp_dev_param->SEL.SET_DEVICE.disp_dev_type);
				return E_NOSPT;

			}
		}
		break;

	case DISPDEV_IOCTRL_SET_SRGB_OUTORDER: {
			dispctr1_debug(("DISPDEVCTR: SET_SRGB_OUTORDER\r\n"));
			ide_set_pdir(p_disp_dev_param->SEL.SET_SRGB_OUTORDER.pix_order);
			ide_set_odd(p_disp_dev_param->SEL.SET_SRGB_OUTORDER.odd_start);
			ide_set_even(p_disp_dev_param->SEL.SET_SRGB_OUTORDER.even_start);
		}
		break;

	case DISPDEV_IOCTRL_SET_SYNC_INVERT: {
			dispctr1_debug(("DISPDEVCTR: SET_SYNC_INVERT\r\n"));
			ide_set_hs_inv(p_disp_dev_param->SEL.SET_SYNC_INVERT.b_hs_inv);
			ide_set_vs_inv(p_disp_dev_param->SEL.SET_SYNC_INVERT.b_vs_inv);
			ide_set_clk_inv(p_disp_dev_param->SEL.SET_SYNC_INVERT.b_clk_inv);
		}
		break;

	case DISPDEV_IOCTRL_SET_VLD_INVERT: {
			dispctr1_debug(("DISPDEVCTR: SET_VLD_INVERT\r\n"));
			ide_set_hvld_inv(p_disp_dev_param->SEL.SET_VLD_INVERT.b_hvld_inv);
			ide_set_vvld_inv(p_disp_dev_param->SEL.SET_VLD_INVERT.b_vvld_inv);
			ide_set_fld_inv(p_disp_dev_param->SEL.SET_VLD_INVERT.b_field_inv);
			ide_set_de_inv(p_disp_dev_param->SEL.SET_VLD_INVERT.b_de_inv);
		}
		break;

	case DISPDEV_IOCTRL_SET_SUBPIXEL: {
			dispctr1_debug(("DISPDEVCTR: SET_SUBPIXEL\r\n"));
			ide_set_subpixel(TRUE, p_disp_dev_param->SEL.SET_SUBPIXEL.b_odd_r, p_disp_dev_param->SEL.SET_SUBPIXEL.b_odd_g, p_disp_dev_param->SEL.SET_SUBPIXEL.b_odd_b);
			ide_set_subpixel(FALSE, p_disp_dev_param->SEL.SET_SUBPIXEL.b_even_r, p_disp_dev_param->SEL.SET_SUBPIXEL.b_even_g, p_disp_dev_param->SEL.SET_SUBPIXEL.b_even_b);
		}
		break;

	case DISPDEV_IOCTRL_SET_WINDOW_H_TIMING: {
			dispctr1_debug(("DISPDEVCTR: SET_WINDOW_H_TIMING\r\n"));
			ide_set_hor_timing(p_disp_dev_param->SEL.SET_WINDOW_H_TIMING.ui_hsync, p_disp_dev_param->SEL.SET_WINDOW_H_TIMING.ui_htotal, p_disp_dev_param->SEL.SET_WINDOW_H_TIMING.ui_hvld_start, p_disp_dev_param->SEL.SET_WINDOW_H_TIMING.ui_hvld_end);

			g_disp1_info.disp_data.ui_global_win_width = p_disp_dev_param->SEL.SET_WINDOW_H_TIMING.ui_hvld_end - p_disp_dev_param->SEL.SET_WINDOW_H_TIMING.ui_hvld_start + 1;

#if DISP_PATCHBUF_RESIZE
			// Remap all layers
			disp_translate_buf_size(DISPLAYER_VDO1);
			disp_translate_buf_size(DISPLAYER_VDO2);
			disp_translate_buf_size(DISPLAYER_OSD1);
#if IDE1_OSD2_EXIST
//      disp_translate_buf_size(DISPLAYER_OSD2);
#endif
#endif
		}
		break;

	case DISPDEV_IOCTRL_SET_WINDOW_V_TIMING: {
			dispctr1_debug(("DISPDEVCTR: SET_WINDOW_V_TIMING\r\n"));
			ide_set_ver_timing(p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vsync, p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vtotal, p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vvld_odd_start, p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vvld_odd_end, p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vvld_even_start, p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vvld_even_end);

			g_disp1_info.disp_data.ui_global_win_height = p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vvld_odd_end - p_disp_dev_param->SEL.SET_WINDOW_V_TIMING.ui_vvld_odd_start + 1;

#if DISP_PATCHBUF_RESIZE
			// Remap all layers
			disp_translate_buf_size(DISPLAYER_VDO1);
			disp_translate_buf_size(DISPLAYER_VDO2);
			disp_translate_buf_size(DISPLAYER_OSD1);
#if IDE1_OSD2_EXIST
//      disp_translate_buf_size(DISPLAYER_OSD2);
#endif
#endif
		}
		break;

	case DISPDEV_IOCTRL_SET_WINDOW_OUT_TYPE: {
			dispctr1_debug(("DISPDEVCTR: SET_WINDOW_OUT_TYPE\r\n"));
			ide_set_interlace(p_disp_dev_param->SEL.SET_WINDOW_OUT_TYPE.b_interlaced);
			ide_set_start_field(p_disp_dev_param->SEL.SET_WINDOW_OUT_TYPE.b_field_odd_st);
		}
		break;

	case DISPDEV_IOCTRL_SET_SYNCDELAY: {
			dispctr1_debug(("DISPDEVCTR: SET_SYNCDELAY\r\n"));
			ide_set_sync_delay(p_disp_dev_param->SEL.SET_SYNCDELAY.ui_hsync_dly, p_disp_dev_param->SEL.SET_SYNCDELAY.ui_vsync_dly);
		}
		break;

	case DISPDEV_IOCTRL_SET_CCIR656_SYNCCODE: {
			dispctr1_debug(("DISPDEVCTR: SET_CCIR656_SYNCCODE\r\n"));
			ide_set_digital_timing(p_disp_dev_param->SEL.SET_CCIR656_SYNCCODE.ui_odd_start, p_disp_dev_param->SEL.SET_CCIR656_SYNCCODE.ui_odd_end, p_disp_dev_param->SEL.SET_CCIR656_SYNCCODE.ui_even_start, p_disp_dev_param->SEL.SET_CCIR656_SYNCCODE.ui_even_end, p_disp_dev_param->SEL.SET_CCIR656_SYNCCODE.ui_field_start, p_disp_dev_param->SEL.SET_CCIR656_SYNCCODE.ui_field_end);
		}
		break;

	case DISPDEV_IOCTRL_SET_GAMMA_EN: {
			dispctr1_debug(("DISPDEVCTR: SET_GAMMA_EN\r\n"));
			ide_set_gamma_en(p_disp_dev_param->SEL.SET_GAMMA_EN.b_en);

			//if(p_disp_dev_param->SEL.SET_GAMMA_EN.bEnY)
			//    ide_fill_gamma(p_disp_dev_param->SEL.SET_GAMMA_EN.p_gamma_tab_y);
		}
		break;

	case DISPDEV_IOCTRL_SET_CSB_EN: {
			dispctr1_debug(("DISPDEVCTR: SET_CSB_EN\r\n"));
			/*if(p_disp_dev_param->SEL.SET_CSB_EN.b_en == FALSE)
			{
			    // 96220 has no CSB disable, Set the configurations to default values.
			    ide_set_csb_en(FALSE);
			    ide_set_ctrst(0x40);
			    ide_set_brt(0x0);
			    ide_set_cmults(0x40);
			}
			else
			{
			    ide_set_csb_en(TRUE);
			    ide_set_ctrst(p_disp_dev_param->SEL.SET_CSB_EN.ui_contrast);
			    ide_set_brt(p_disp_dev_param->SEL.SET_CSB_EN.ui_brightness);
			    ide_set_cmults(p_disp_dev_param->SEL.SET_CSB_EN.ui_saturation);
			}*/
			dispctr1_debug(("DISPDEVCTR No Support set csb en!\r\n"));
		}
		break;

	case DISPDEV_IOCTRL_SET_YC_EXCHG: {
			dispctr1_debug(("DISPDEVCTR: SET_YC_EXCHG\r\n"));
			ide_set_ycex(p_disp_dev_param->SEL.SET_YC_EXCHG.b_yc_exchg);
			ide_set_cex(p_disp_dev_param->SEL.SET_YC_EXCHG.b_cbcr_exchg);
		}
		break;

	case DISPDEV_IOCTRL_SET_CLAMP: {
			dispctr1_debug(("DISPDEVCTR: SET_CLAMP\r\n"));
			idec_set_clamp(IDE_ID_1, p_disp_dev_param->SEL.SET_CLAMP.ui_clamp);
		}
		break;

	case DISPDEV_IOCTRL_SET_DITHER_EN: {
			dispctr1_debug(("DISPDEVCTR: SET_DITHER_EN\r\n"));
			if (p_disp_dev_param->SEL.SET_DITHER_EN.b_en) {
				ide_set_dithering(TRUE, p_disp_dev_param->SEL.SET_DITHER_EN.b_free_run);
				ide_set_dither_vbits(p_disp_dev_param->SEL.SET_DITHER_EN.r_bits, p_disp_dev_param->SEL.SET_DITHER_EN.g_bits, p_disp_dev_param->SEL.SET_DITHER_EN.b_bits);
			} else {
				ide_set_dithering(FALSE, FALSE);
			}
		}
		break;

	case DISPDEV_IOCTRL_SET_OUT_COMPONENT: {
			dispctr1_debug(("DISPDEVCTR: SET_OUT_COMPONENT\r\n"));
			ide_set_out_comp(p_disp_dev_param->SEL.SET_OUT_COMPONENT.comp0, p_disp_dev_param->SEL.SET_OUT_COMPONENT.comp1, p_disp_dev_param->SEL.SET_OUT_COMPONENT.comp2, p_disp_dev_param->SEL.SET_OUT_COMPONENT.b_bit_swap, p_disp_dev_param->SEL.SET_OUT_COMPONENT.b_length);
		}
		break;

	case DISPDEV_IOCTRL_SET_ICST_EN: {
			dispctr1_debug(("DISPDEVCTR: SET_ICST_EN\r\n"));
			ide_config_icst(p_disp_dev_param->SEL.SET_ICST_EN.b_en, p_disp_dev_param->SEL.SET_ICST_EN.select);
		}
		break;

	case DISPDEV_IOCTRL_SET_OUT_LIMIT: {
			dispctr1_debug(("DISPDEVCTR: SET_OUT_LIMIT\r\n"));
			ide_set_out_limit(p_disp_dev_param->SEL.SET_OUT_LIMIT.ui_y_low, p_disp_dev_param->SEL.SET_OUT_LIMIT.ui_y_up, p_disp_dev_param->SEL.SET_OUT_LIMIT.ui_cb_low, p_disp_dev_param->SEL.SET_OUT_LIMIT.ui_cb_up, p_disp_dev_param->SEL.SET_OUT_LIMIT.ui_cr_low, p_disp_dev_param->SEL.SET_OUT_LIMIT.ui_cr_up);
		}
		break;

	case DISPDEV_IOCTRL_SET_CONST_OFS: {
			dispctr1_debug(("DISPDEVCTR: SET_CONST_OFS\r\n"));
			ide_set_constant_window_offset(p_disp_dev_param->SEL.SET_CONST_OFS.ui_win_const_ofs_x, p_disp_dev_param->SEL.SET_CONST_OFS.ui_win_const_ofs_y);
		}
		break;

	case DISPDEV_IOCTRL_SET_DISPSIZE: {
			dispdev1_debug(("DISPDEVCTR: Set DISPSIZE\r\n"));
			g_disp1_info.disp_dev_data.ui_buf_width  = p_disp_dev_param->SEL.SET_DISPSIZE.ui_buf_width;
			g_disp1_info.disp_dev_data.ui_buf_height = p_disp_dev_param->SEL.SET_DISPSIZE.ui_buf_height;
			g_disp1_info.disp_dev_data.ui_win_width  = p_disp_dev_param->SEL.SET_DISPSIZE.ui_win_width;
			g_disp1_info.disp_dev_data.ui_win_height = p_disp_dev_param->SEL.SET_DISPSIZE.ui_win_height;
		}
		break;

	case DISPDEV_IOCTRL_SET_CLK1_2: {
			dispctr1_debug(("DISPDEVCTR: SET_CLK1_2\r\n"));
			idec_set_clk1_2(IDE_ID_1, p_disp_dev_param->SEL.SET_CLK1_2.b_clk1_2);
		}
		break;

	case DISPDEV_IOCTRL_SET_RGBD_SWAP: {
			dispctr1_debug(("DISPDEVCTR: SET_RGBD_SWAP\r\n"));
			idec_set_rgbd_swap(IDE_ID_1, p_disp_dev_param->SEL.SET_RGBD_SWAP.swap);
		}
		break;

	case DISPDEV_IOCTRL_SET_CLK_FREQ: {

			DISPCTRL_SRCCLK dispclksrc;
			ER er_return;

			dispctr1_debug(("DISPDEVCTR: SET_CLK_FREQ\r\n"));

			dispclksrc = g_disp1_info.disp_data.src_clk;

			er_return = display_obj_platform_set_clk_src(IDE_ID_1, dispclksrc);

			if (er_return != E_OK)
				return er_return;

			if (p_disp_dev_param->SEL.SET_CLK_FREQ.b_ycc8bit) {
				DBG_IND("p_disp_dev_param->SEL.SET_CLK_FREQ.b_ycc8bit = %d ui_freq = %d\r\n", (int)p_disp_dev_param->SEL.SET_CLK_FREQ.b_ycc8bit, (int)p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq);
				ide_platform_set_freq(IDE_ID_1, p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq);
				DBG_IND("ui_freq = %d\r\n", (int)ide_platform_get_freq(IDE_ID_1));
				ide_platform_set_iffreq(IDE_ID_1, ide_platform_get_freq(IDE_ID_1) * 2);
			} else {
			/*
				UINT32 ui_src_clk_freq;
				UINT32 ui_clock_div_in;
				UINT32 ui_clock_div_out;
				float  in_freq, out_freq;
			*/
				ide_platform_set_freq(IDE_ID_1, p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq);
				DBG_IND("p_disp_dev_param->SEL.SET_CLK_FREQ.b_ycc8bit = %d p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq = %d disp_dev_type = %d\r\n", (int)p_disp_dev_param->SEL.SET_CLK_FREQ.b_ycc8bit, (int)p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq, (int)g_disp1_info.disp_dev_data.disp_dev_type);
				DBG_IND("ui_freq = %d\r\n", (int)ide_platform_get_freq(IDE_ID_1));
				switch (g_disp1_info.disp_dev_data.disp_dev_type) {
				case DISPDEV_TYPE_RGB_SERIAL:
				case DISPDEV_TYPE_RGB_PARALL:
				case DISPDEV_TYPE_CCIR601_16BIT:
				case DISPDEV_TYPE_CCIR656_16BIT:
				case DISPDEV_TYPE_RGBDELTA_16BIT:
					ide_platform_set_iffreq(IDE_ID_1, p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq);
					DBG_IND("if ui_freq = %d\r\n", (int)ide_platform_get_iffreq(IDE_ID_1));
					break;
				case DISPDEV_TYPE_RGBD:
					ide_platform_set_iffreq(IDE_ID_1, ide_platform_get_freq(IDE_ID_1) << 2);
					/*
					ui_src_clk_freq = pll_getClockRate(PLL_CLKSEL_IDE_CLKSRC);
					if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_480) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_1);
					} else if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_PLL2) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_2);
					} else if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_4);
					} else if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_PLL14) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_14);
					} else {
						DBG_ERR("unknow ide clock source [%d]\r\n", ui_src_clk_freq);
					}

					ui_clock_div_in = (pll_getClockRate(PLL_CLKSEL_IDE_CLKDIV) & 0xFF) + 1;
					ui_clock_div_out = ((pll_getClockRate(PLL_CLKSEL_IDE_OUTIF_CLKDIV) >> 16) & 0xFF) + 1;

					in_freq = (float)(((float)ui_src_clk_freq / (float)ui_clock_div_in) / 1000000);
					out_freq = (float)(((float)ui_src_clk_freq / (float)ui_clock_div_out) / 1000000);
					if (ui_clock_div_in % ui_clock_div_out) {
						DBG_DUMP("^RIDE source clock = %d MHz\r\n", ui_src_clk_freq / 1000000);
						DBG_DUMP("^RIn div[%d] vs Out div[%d]\r\n", ui_clock_div_in, ui_clock_div_out);
						DBG_ERR("In clk vs out clk not 1:4 (RGB Dummy mode) => %f MHz : %f MHz!!!\r\n", in_freq, out_freq);
						DBG_DUMP("^R => panel might be corrupt!!! Driver Table target [%.4f]MHz must modified.\r\n", (float)(p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq / 1000000));
					}
					*/
					break;
				case DISPDEV_TYPE_RGB_THROUGH:
					ide_platform_set_iffreq(IDE_ID_1, ide_platform_get_freq(IDE_ID_1) * 3);
					/*
					ui_src_clk_freq = pll_getClockRate(PLL_CLKSEL_IDE_CLKSRC);
					if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_480) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_1);
					} else if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_PLL2) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_2);
					} else if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_4);
					} else if (ui_src_clk_freq == PLL_CLKSEL_IDE_CLKSRC_PLL14) {
						ui_src_clk_freq = pll_getPLLFreq(PLL_ID_14);
					} else {
						DBG_ERR("unknow ide clock source [%d]\r\n", ui_src_clk_freq);
					}

					ui_clock_div_in = (pll_getClockRate(PLL_CLKSEL_IDE_CLKDIV) & 0xFF) + 1;
					ui_clock_div_out = ((pll_getClockRate(PLL_CLKSEL_IDE_OUTIF_CLKDIV) >> 16) & 0xFF) + 1;

					in_freq = (float)(((float)ui_src_clk_freq / (float)ui_clock_div_in) / 1000000);
					out_freq = (float)(((float)ui_src_clk_freq / (float)ui_clock_div_out) / 1000000);
					if (ui_clock_div_in % ui_clock_div_out) {
						DBG_DUMP("^RIDE source clock = %d MHz\r\n", ui_src_clk_freq / 1000000);
						DBG_DUMP("^RIn div[%d] vs Out div[%d]\r\n", ui_clock_div_in, ui_clock_div_out);
						DBG_ERR("In clk vs out clk not 1:3 (RGBThrougth mode) => %f MHz : %f MHz!!!\r\n", in_freq, out_freq);
						DBG_DUMP("^R => panel might be corrupt!!! Driver Table target [%.4f]MHz must modified.\r\n", (float)(p_disp_dev_param->SEL.SET_CLK_FREQ.ui_freq / 1000000));

					}
					*/
					break;

				default:
					break;
				}
			}

		}
		break;

	case DISPDEV_IOCTRL_SET_CLK_EN: {
			DISPCTRL_SRCCLK dispclksrc;
			ER er_return;

			dispctr1_debug(("DISPDEVCTR: SET_CLK_EN\r\n"));

			dispclksrc = g_disp1_info.disp_data.src_clk;

			er_return = display_obj_platform_pll_en(dispclksrc);
			if (er_return != E_OK)
				return er_return;

			if (p_disp_dev_param->SEL.SET_CLK_EN.b_clk_en == TRUE) {
				ide_platform_clk_en(IDE_ID_1);
			} else {
				ide_platform_clk_dis(IDE_ID_1);
			}
		}
		break;

	///////////////////////
	/* GET control group */
	///////////////////////
	case DISPDEV_IOCTRL_GET_ENABLE: {
			dispctr1_debug(("DISPDEVCTR: GET_ENABLE\r\n"));
			p_disp_dev_param->SEL.GET_ENABLE.b_en = ide_get_en();
		}
		break;

	case DISPDEV_IOCTRL_GET_DEVICE: {
			//IDE_DEVICE_TYPE ide_device;
			//BOOL            bRgbd;
			//BOOL            bThrough;
			//DISPDEV_TYPE    disp_device = DISPDEV_TYPE_RGB_SERIAL;

			dispctr1_debug(("DISPDEVCTR: GET_DEVICE\r\n"));
			p_disp_dev_param->SEL.GET_DEVICE.disp_dev_type = g_disp1_info.disp_dev_data.disp_dev_type;
/*
			ide_device = ide_get_device();
			bRgbd = ide_get_rgbd();
			bThrough = ide_get_through();

			switch (ide_device) {
			case DISPLAY_DEVICE_CASIO2G:
				if ((bThrough == FALSE) && (bRgbd == FALSE)) {
					disp_device = DISPDEV_TYPE_RGB_SERIAL;
				} else if (bThrough == TRUE) {
					disp_device = DISPDEV_TYPE_RGB_THROUGH;
				} else if (bRgbd == TRUE) {
					disp_device = DISPDEV_TYPE_RGBD;
				}
				break;
			case DISPLAY_DEVICE_TOPPOLY:
				disp_device = DISPDEV_TYPE_YUV;
				break;
			case DISPLAY_DEVICE_CCIR601:
				disp_device = DISPDEV_TYPE_CCIR601_8BIT;
				break;
			case DISPLAY_DEVICE_CCIR656:
				disp_device = DISPDEV_TYPE_CCIR656_8BIT;
				break;

			case DISPLAY_DEVICE_CCIR601_16BIT:
				disp_device = DISPDEV_TYPE_CCIR601_16BIT;
				break;

			case DISPLAY_DEVICE_CCIR656_16BIT:
				disp_device = DISPDEV_TYPE_CCIR656_16BIT;
				break;

			case DISPLAY_DEVICE_MI:
				disp_device = DISPDEV_TYPE_MI;
				break;

			case DISPLAY_DEVICE_TV:
				disp_device = DISPDEV_TYPE_EMBD_TV;
				break;

			case DISPLAY_DEVICE_HDMI_24BIT:
				disp_device = DISPDEV_TYPE_EMBD_HDMI;
				break;

			case DISPALY_DEVICE_PARALLEL:
				disp_device = DISPDEV_TYPE_RGB_PARALL;
				break;

			case DISPLAY_DEVICE_MIPIDSI:
				disp_device = DISPDEV_TYPE_EMBD_MIPIDSI;
				break;

			case DISPLAY_DEVICE_RGB_16BIT:
				disp_device = DISPDEV_TYPE_RGBDELTA_16BIT;
				break;

			default:
				debug_err(("GET_DEVICE no support! (%d)\r\n",));
				return E_NOSPT;

			}
			p_disp_dev_param->SEL.GET_DEVICE.disp_dev_type = disp_device;*/
		}
		break;


	case DISPDEV_IOCTRL_GET_SRCCLK: {
			dispctr1_debug(("DISPDEVCTR: GET_SRCCLK\r\n"));
			p_disp_dev_param->SEL.GET_SRCCLK.src_clk = g_disp1_info.disp_data.src_clk;
		}
		break;

	case DISPDEV_IOCTRL_GET_REG_IF: {
			dispdev1_debug(("DISPDEVCTR: GET_REG_IF\r\n"));

			p_disp_dev_param->SEL.GET_REG_IF.lcd_ctrl     = g_disp1_info.disp_dev_data.lcd_ctrl;
			p_disp_dev_param->SEL.GET_REG_IF.ui_sif_ch     = g_disp1_info.disp_dev_data.sif_ch;
			p_disp_dev_param->SEL.GET_REG_IF.ui_gpio_sen   = g_disp1_info.disp_dev_data.ui_gpio_sif_sen;
			p_disp_dev_param->SEL.GET_REG_IF.ui_gpio_clk   = g_disp1_info.disp_dev_data.ui_gpio_sif_clk;
			p_disp_dev_param->SEL.GET_REG_IF.ui_gpio_data  = g_disp1_info.disp_dev_data.ui_gpio_sif_data;
		}
		break;

	case DISPDEV_IOCTRL_GET_HDMIMODE: {
			dispdev1_debug(("DISPDEVCTR: GET_HDMIMODE\r\n"));
			p_disp_dev_param->SEL.GET_HDMIMODE.audio_id = g_disp1_info.disp_dev_data.hdmi_aud_fmt;
			p_disp_dev_param->SEL.GET_HDMIMODE.video_id = g_disp1_info.disp_dev_data.hdmi_vdo_fmt;
		}
		break;

	case DISPDEV_IOCTRL_GET_ACT_DEVICE: {
			dispdev1_debug(("DISPDEVCTR: GET_ACT_DEVICE\r\n"));
			p_disp_dev_param->SEL.GET_ACT_DEVICE.dev_id = g_disp1_info.disp_data.active_dev;
		}
		break;

	case DISPDEV_IOCTRL_GET_PANEL_ADJUST: {
			dispdev1_debug(("DISPDEVCTR: GET_PANEL_ADJUST\r\n"));
			p_disp_dev_param->SEL.GET_PANEL_ADJUST.pfp_adjust = g_disp1_info.disp_dev_data.panel_adjust;
		}
		break;

	case DISPDEV_IOCTRL_GET_TVADJUST: {
			dispdev1_debug(("DISPDEVCTR: GET_TVADJUST\r\n"));
			p_disp_dev_param->SEL.GET_TVADJUST.tv_adjust    = g_disp1_info.disp_dev_data.tv_adjust;
		}
		break;

	case DISPDEV_IOCTRL_GET_GAMMA_EN: {
			dispctr1_debug(("DISPDEVCTR: GET_GAMMA_EN\r\n"));
			p_disp_dev_param->SEL.GET_GAMMA_EN.b_en = ide_get_gamma_en();
		}
		break;

	case DISPDEV_IOCTRL_GET_CSB_EN: {
			dispctr1_debug(("DISPDEVCTR: GET_CSB_EN\r\n"));
			p_disp_dev_param->SEL.GET_CSB_EN.b_en = FALSE;
			p_disp_dev_param->SEL.GET_CSB_EN.ui_contrast = 0;
			p_disp_dev_param->SEL.GET_CSB_EN.ui_saturation = 0;
			p_disp_dev_param->SEL.GET_CSB_EN.ui_brightness = 0;
		}
		break;

	case DISPDEV_IOCTRL_GET_TVPAR: {
			dispdev1_debug(("DISPDEVCTR: GET_TVPAR\r\n"));

			p_disp_dev_param->SEL.GET_TVPAR.b_en_user         = g_disp1_info.disp_dev_data.b_tv_en_user;
			p_disp_dev_param->SEL.GET_TVPAR.ui_ntsc_bll      = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_bll;
			p_disp_dev_param->SEL.GET_TVPAR.ui_ntsc_brl      = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_brl;
			p_disp_dev_param->SEL.GET_TVPAR.ui_ntsc_setup    = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_setup;
			p_disp_dev_param->SEL.GET_TVPAR.ui_ntsc_y_scaling = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_y_scaling;
			p_disp_dev_param->SEL.GET_TVPAR.ui_ntsc_cb_scaling = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_cb_scaling;
			p_disp_dev_param->SEL.GET_TVPAR.ui_ntsc_cr_scaling = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_cr_scaling;
			p_disp_dev_param->SEL.GET_TVPAR.ui_pal_bll       = g_disp1_info.disp_dev_data.tv_par_pal.ui_bll;
			p_disp_dev_param->SEL.GET_TVPAR.ui_pal_brl       = g_disp1_info.disp_dev_data.tv_par_pal.ui_brl;
			p_disp_dev_param->SEL.GET_TVPAR.ui_pal_setup     = g_disp1_info.disp_dev_data.tv_par_pal.ui_setup;
			p_disp_dev_param->SEL.GET_TVPAR.ui_pal_y_scaling  = g_disp1_info.disp_dev_data.tv_par_pal.ui_y_scaling;
			p_disp_dev_param->SEL.GET_TVPAR.ui_pal_cb_scaling = g_disp1_info.disp_dev_data.tv_par_pal.ui_cb_scaling;
			p_disp_dev_param->SEL.GET_TVPAR.ui_pal_cr_scaling = g_disp1_info.disp_dev_data.tv_par_pal.ui_cr_scaling;
		}
		break;

	case DISPDEV_IOCTRL_GET_TVFULL: {
			dispdev1_debug(("DISPDEVCTR: GET_TVFULL\r\n"));
			p_disp_dev_param->SEL.GET_TVFULL.b_en_full        = g_disp1_info.disp_dev_data.tv_full;
		}
		break;


	///////////////////////
	/*       OTHERS      */
	///////////////////////
	case DISPDEV_IOCTRL_SET_LOAD: {
			ide_set_load();
		}
		break;

	case DISPDEV_IOCTRL_WAIT_FRAMEEND: {
			idec_set_callback(IDE_ID_1, NULL);
			ide_wait_frame_end(TRUE);
		}
		break;

	case DISPDEV_IOCTRL_WAIT_DMA_DONE: {
			ide_wait_yuv_output_done();
		}
		break;

	case DISPDEV_IOCTRL_SET_HLPF_EN:
	default: {
			DBG_WRN("DISPLAYDEV No Support!(%d)\r\n", disp_dev_ctrl);
			return E_NOSPT;
		}

	}

	return E_OK;
}


#if 1
/*
    Display Control

    This API is used as the display engine global control API.
    Such as source clock selection, Device interface, Output Timing Generator,
    ...etc.

    @param[in] disp_ctrl     Display Control command selection. Refer to display.h.
    @param[in] p_disp_param   Display Control parameters according to the control command.

    @return
     - @b E_NOEXS:  Display object has not opened.
     - @b E_NOSPT:  Control Command no support.
     - @b E_OK:     Control Command done.
*/
ER disp_set_disp1_control(DISPCTRL_OP disp_ctrl, PDISPCTRL_PARAM p_disp_param)
{
	if (!gui_disp_obj_opened[DISP_1]) {
		return E_NOEXS;
	}

	switch (disp_ctrl) {

	///////////////////////
	/* SET control group */
	///////////////////////
	case DISPCTRL_SET_ENABLE: {
			dispctr1_debug(("DISPCTR: SET_ENABLE (%d)\r\n", (int)p_disp_param->SEL.SET_ENABLE.b_en));
			if (p_disp_param->SEL.SET_ENABLE.b_en && (!ide_is_opened())) {
				ide_open();
			}

			ide_set_en(p_disp_param->SEL.SET_ENABLE.b_en);
		}
		break;

	case DISPCTRL_SET_ALL_LYR_EN: {
			dispctr1_debug(("DISPCTR: SET_ALL_LYR_EN(%d)\r\n", (int)p_disp_param->SEL.SET_ALL_LYR_EN.b_en));
			if (p_disp_param->SEL.SET_ALL_LYR_EN.b_en) {
				ide_set_all_window_en(p_disp_param->SEL.SET_ALL_LYR_EN.disp_lyr);
			} else {
				ide_set_all_window_dis();
			}
		}
		break;

	case DISPCTRL_SET_BACKGROUND: {
			dispctr1_debug(("DISPCTR: SET_BACKGROUND\r\n"));
			ide_set_background(p_disp_param->SEL.SET_BACKGROUND.ui_color_y, p_disp_param->SEL.SET_BACKGROUND.ui_color_cb, p_disp_param->SEL.SET_BACKGROUND.ui_color_cr);
		}
		break;

	case DISPCTRL_SET_GAMMA_EN: {
			dispctr1_debug(("DISPCTR: SET_GAMMA_EN\r\n"));
			ide_set_gamma_en(p_disp_param->SEL.SET_GAMMA_EN.b_en);

			//if(p_disp_param->SEL.SET_GAMMA_EN.bEnY)
			//    ide_fill_gamma(p_disp_param->SEL.SET_GAMMA_EN.p_gamma_tab_y);
		}
		break;

	case DISPCTRL_SET_ICST_EN: {
			dispctr1_debug(("DISPCTR: SET_ICST_EN\r\n"));
			ide_config_icst(p_disp_param->SEL.SET_ICST_EN.b_en, p_disp_param->SEL.SET_ICST_EN.select);
		}
		break;

	case DISPCTRL_SET_CONST_OFS: {
			dispctr1_debug(("DISPCTR: SET_CONST_OFS\r\n"));
			ide_set_constant_window_offset(p_disp_param->SEL.SET_CONST_OFS.ui_win_const_ofs_x, p_disp_param->SEL.SET_CONST_OFS.ui_win_const_ofs_y);
		}
		break;

	case DISPCTRL_SET_SRCCLK: {
			dispctr1_debug(("DISPCTR: SET_SRCCLK\r\n"));
			g_disp1_info.disp_data.src_clk = p_disp_param->SEL.SET_SRCCLK.src_clk;
		}
		break;

	case DISPCTRL_SET_GAMMA_Y: {
			dispctr1_debug(("DISPCTR: SET_GAMMA_Y\r\n"));

			DBG_WRN("DISPCTL No Support!(%d)\r\n", (int)disp_ctrl);

			//ide_fill_gamma(p_disp_param->SEL.SET_GAMMA_Y.p_gamma_tab_y);

		}
		break;

	case DISPCTRL_SET_GAMMA_RGB: {
			dispctr1_debug(("DISPCTR: SET_GAMMA_RGB\r\n"));

			idec_fill_rgb_gamma(IDE_ID_1, p_disp_param->SEL.SET_GAMMA_RGB.p_gamma_tab_r, p_disp_param->SEL.SET_GAMMA_RGB.p_gamma_tab_g, p_disp_param->SEL.SET_GAMMA_RGB.p_gamma_tab_b);

		}
		break;

	case DISPCTRL_SET_ICST0_EN: {
			dispctr1_debug(("DISPCTR: SET_ICST0_EN\r\n"));

			idec_set_icst0(IDE_ID_1, p_disp_param->SEL.SET_ICST0_EN.b_en);
		}
		break;

	case DISPCTRL_SET_ICST0_COEF: {
			dispctr1_debug(("DISPCTR: SET_ICST0_COEF\r\n"));

			idec_set_icst0_pre_offset(IDE_ID_1, (UINT32)p_disp_param->SEL.SET_ICST_COEF.i_pre_ofs_y, (UINT32)p_disp_param->SEL.SET_ICST_COEF.i_pre_ofs_cb, (UINT32)p_disp_param->SEL.SET_ICST_COEF.i_pre_ofs_cr);
			idec_set_icst0_coef(IDE_ID_1, (UINT32 *)p_disp_param->SEL.SET_ICST_COEF.pi_coef);
			idec_set_out_offset(IDE_ID_1, (UINT32)p_disp_param->SEL.SET_ICST_COEF.i_post_ofs_y, (UINT32)p_disp_param->SEL.SET_ICST_COEF.i_post_ofs_cb, (UINT32)p_disp_param->SEL.SET_ICST_COEF.i_post_ofs_cr);
		}
		break;

	case DISPCTRL_SET_CST_EN: {
			dispctr1_debug(("DISPCTR: SET_CST_EN\r\n"));

			idec_set_cst1(IDE_ID_1, p_disp_param->SEL.SET_CST_EN.b_en);
		}
		break;

	case DISPCTRL_SET_CC_EN: {
			dispctr1_debug(("DISPCTR: SET_CC_EN\r\n"));

			idec_set_color_ctrl_en(IDE_ID_1, p_disp_param->SEL.SET_CC_EN.b_en);
		}
		break;

	case DISPCTRL_SET_CCA_EN: {
			dispctr1_debug(("DISPCTR: SET_CCA_EN\r\n"));

			idec_set_color_comp_adj_en(IDE_ID_1, p_disp_param->SEL.SET_CCA_EN.b_en);
		}
		break;

	case DISPCTRL_SET_CCA_HUE_EN: {
			dispctr1_debug(("DISPCTR: SET_CCA_HUE_EN\r\n"));

			idec_set_color_ctrl_hue_adj_en(IDE_ID_1, p_disp_param->SEL.SET_CCA_HUE_EN.b_en);
		}
		break;

	case DISPCTRL_SET_CCA_YCON_EN: {
			dispctr1_debug(("DISPCTR: SET_CCA_YCON_EN\r\n"));

			idec_set_color_comp_ycon_en(IDE_ID_1, p_disp_param->SEL.SET_CCA_YCON_EN.b_en);
		}
		break;

	case DISPCTRL_SET_CCA_CCON_EN: {
			dispctr1_debug(("DISPCTR: SET_CCA_CCON_EN\r\n"));

			idec_set_color_comp_ccon_en(IDE_ID_1, p_disp_param->SEL.SET_CCA_CCON_EN.b_en);
		}
		break;

	case DISPCTRL_SET_CC_HUE: {
			dispctr1_debug(("DISPCTR: SET_CC_HUE\r\n"));

			idec_set_color_ctrl_hue(IDE_ID_1, p_disp_param->SEL.SET_CC_HUE.p_hue_tab);
		}
		break;

	case DISPCTRL_SET_CC_INT: {
			dispctr1_debug(("DISPCTR: SET_CC_INT\r\n"));

			idec_set_color_ctrl_int(IDE_ID_1, p_disp_param->SEL.SET_CC_INT.p_int_tab);
		}
		break;

	case DISPCTRL_SET_CC_SAT: {
			dispctr1_debug(("DISPCTR: SET_CC_SAT\r\n"));

			idec_set_color_ctrl_sat(IDE_ID_1, p_disp_param->SEL.SET_CC_SAT.p_sat_tab);
		}
		break;

	case DISPCTRL_SET_CC_DDS: {
			dispctr1_debug(("DISPCTR: SET_CC_DDS\r\n"));

			idec_set_color_ctrl_dds(IDE_ID_1, p_disp_param->SEL.SET_CC_DDS.p_dds_tab);
		}
		break;

	case DISPCTRL_SET_CC_INT_OFS: {
			INT8 iintofs, isatofs;

			dispctr1_debug(("DISPCTR: SET_CC_INT_OFS\r\n"));

			idec_get_color_ctrl_int_sat_ofs(IDE_ID_1, &iintofs, &isatofs);

			idec_set_color_ctrl_int_sat_ofs(IDE_ID_1, p_disp_param->SEL.SET_CC_INT_OFS.iintofs, isatofs);
		}
		break;

	case DISPCTRL_SET_CC_SAT_OFS: {
			INT8 iintofs, isatofs;

			dispctr1_debug(("DISPCTR: SET_CC_SAT_OFS\r\n"));

			idec_get_color_ctrl_int_sat_ofs(IDE_ID_1, &iintofs, &isatofs);

			idec_set_color_ctrl_int_sat_ofs(IDE_ID_1, iintofs, p_disp_param->SEL.SET_CC_SAT_OFS.isatofs);
		}
		break;

	case DISPCTRL_SET_CCA_YCON: {
			dispctr1_debug(("DISPCTR: SET_CCA_YCON\r\n"));

			idec_set_color_comp_ycon(IDE_ID_1, p_disp_param->SEL.SET_CCA_YCON.uiycon);
		}
		break;

	case DISPCTRL_SET_CCA_CCON: {
			dispctr1_debug(("DISPCTR: SET_CCA_CCON\r\n"));

			idec_set_color_comp_ccon(IDE_ID_1, p_disp_param->SEL.SET_CCA_CCON.uiccon);
		}
		break;

	case DISPCTRL_SET_CCA_YOFS: {
			dispctr1_debug(("DISPCTR: SET_CCA_YOFS\r\n"));

			idec_set_color_comp_yofs(IDE_ID_1, p_disp_param->SEL.SET_CCA_YOFS.iyofs);
		}
		break;

	case DISPCTRL_SET_CCA_COFS: {
			dispctr1_debug(("DISPCTR: SET_CCA_COFS\r\n"));

			idec_set_color_comp_cofs(IDE_ID_1, p_disp_param->SEL.SET_CCA_COFS.uicbofs, p_disp_param->SEL.SET_CCA_COFS.uicrofs);
		}
		break;


	///////////////////////
	/* GET control group */
	///////////////////////
	case DISPCTRL_GET_ENABLE: {
			dispctr1_debug(("DISPCTR: GET_ENABLE\r\n"));
			p_disp_param->SEL.GET_ENABLE.b_en = ide_get_en();
		}
		break;

	case DISPCTRL_GET_ALL_LYR_EN: {
			dispctr1_debug(("DISPCTR: GET_ALL_LYR_EN\r\n"));
			p_disp_param->SEL.GET_ALL_LYR_EN.disp_lyr = ide_get_window_en();
		}
		break;


	case DISPCTRL_GET_BACKGROUND: {
			UINT8 ui_y, ui_cb, ui_cr;

			dispctr1_debug(("DISPCTR: GET_BACKGROUND\r\n"));

			ide_get_background(&ui_y, &ui_cb, &ui_cr);
			p_disp_param->SEL.GET_BACKGROUND.ui_color_y = ui_y;
			p_disp_param->SEL.GET_BACKGROUND.ui_color_cb = ui_cb;
			p_disp_param->SEL.GET_BACKGROUND.ui_color_cr = ui_cr;
		}
		break;

	case DISPCTRL_GET_GAMMA_EN: {
			dispctr1_debug(("DISPCTR: GET_GAMMA_EN\r\n"));
			p_disp_param->SEL.GET_GAMMA_EN.b_en = ide_get_gamma_en();

			//if(p_disp_param->SEL.GET_GAMMA_EN.b_en)
			//    ide_fill_gamma(p_disp_param->SEL.SET_GAMMA_EN.p_gamma_tab_y);
		}
		break;

	case DISPCTRL_GET_ICST_EN: {
			BOOL b_en;
			CST_SEL SEL;

			dispctr1_debug(("DISPCTR: GET_ICST_EN\r\n"));

			ide_getconfig_icst(&b_en, &SEL);
			p_disp_param->SEL.GET_ICST_EN.b_en = b_en;
			p_disp_param->SEL.GET_ICST_EN.select = SEL;
		}
		break;

	case DISPCTRL_GET_CONST_OFS: {
			UINT32 ui_x, ui_y;

			dispctr1_debug(("DISPCTR: GET_CONST_OFS\r\n"));

			ide_get_constant_window_offset(&ui_x, &ui_y);
			p_disp_param->SEL.GET_CONST_OFS.ui_win_const_ofs_x = ui_x;
			p_disp_param->SEL.GET_CONST_OFS.ui_win_const_ofs_y = ui_y;
		}
		break;


	case DISPCTRL_GET_SRCCLK: {
			dispctr1_debug(("DISPCTR: GET_SRCCLK\r\n"));
			p_disp_param->SEL.GET_SRCCLK.src_clk = g_disp1_info.disp_data.src_clk;
		}
		break;

	case DISPCTRL_GET_GAMMA_Y: {
			dispctr1_debug(("DISPCTR: GET_GAMMA_Y\r\n"));

			DBG_WRN("DISPCTL No Support!(%d)\r\n", (int)disp_ctrl);
		}
		break;

	case DISPCTRL_GET_GAMMA_RGB: {
			dispctr1_debug(("DISPCTR: GET_GAMMA_RGB\r\n"));

			idec_get_rgb_gamma(IDE_ID_1, p_disp_param->SEL.SET_GAMMA_RGB.p_gamma_tab_r, p_disp_param->SEL.SET_GAMMA_RGB.p_gamma_tab_g, p_disp_param->SEL.SET_GAMMA_RGB.p_gamma_tab_b);

		}
		break;

	case DISPCTRL_GET_ICST0_EN: {
			dispctr1_debug(("DISPCTR: GET_ICST0_EN\r\n"));

			p_disp_param->SEL.GET_ICST0_EN.b_en = idec_get_icst0(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_ICST0_COEF: {
			INT32 ui_pre_y, ui_pre_cb, ui_pre_cr;
			INT32 ui_post_y, ui_post_cb, ui_post_cr;

			dispctr1_debug(("DISPCTR: GET_ICST0_COEF\r\n"));

			idec_get_icst0_pre_offset(IDE_ID_1, &ui_pre_y, &ui_pre_cb, &ui_pre_cr);
			idec_get_icst0_coef(IDE_ID_1, p_disp_param->SEL.SET_ICST_COEF.pi_coef);
			idec_get_out_offset(IDE_ID_1, &ui_post_y, &ui_post_cb, &ui_post_cr);

			p_disp_param->SEL.SET_ICST_COEF.i_pre_ofs_y  = (INT16)ui_pre_y;
			p_disp_param->SEL.SET_ICST_COEF.i_pre_ofs_cb = (INT16)ui_pre_cb;
			p_disp_param->SEL.SET_ICST_COEF.i_pre_ofs_cr = (INT16)ui_pre_cr;

			p_disp_param->SEL.SET_ICST_COEF.i_post_ofs_y  = (INT16)ui_post_y;
			p_disp_param->SEL.SET_ICST_COEF.i_post_ofs_cb = (INT16)ui_post_cb;
			p_disp_param->SEL.SET_ICST_COEF.i_post_ofs_cr = (INT16)ui_post_cr;
		}
		break;


	case DISPCTRL_GET_CST_EN: {
			dispctr1_debug(("DISPCTR: GET_CST_EN\r\n"));

			p_disp_param->SEL.GET_CST_EN.b_en = idec_get_cst1(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_CC_EN: {
			dispctr1_debug(("DISPCTR: GET_CC_EN\r\n"));

			p_disp_param->SEL.GET_CC_EN.b_en = idec_get_color_ctrl_en(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_CCA_EN: {
			dispctr1_debug(("DISPCTR: GET_CCA_EN\r\n"));

			p_disp_param->SEL.GET_CCA_EN.b_en = idec_get_color_comp_adj_en(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_CCA_HUE_EN: {
			dispctr1_debug(("DISPCTR: GET_CCA_HUE_EN\r\n"));

			p_disp_param->SEL.GET_CCA_HUE_EN.b_en = idec_get_color_ctrl_hue_adj_en(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_CCA_YCON_EN: {
			dispctr1_debug(("DISPCTR: GET_CCA_YCON_EN\r\n"));

			p_disp_param->SEL.GET_CCA_YCON_EN.b_en = idec_get_color_comp_ycon_en(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_CCA_CCON_EN: {
			dispctr1_debug(("DISPCTR: GET_CCA_CCON_EN\r\n"));

			p_disp_param->SEL.GET_CCA_CCON_EN.b_en = idec_get_color_comp_ccon_en(IDE_ID_1);
		}
		break;

	case DISPCTRL_GET_CC_HUE: {
			dispctr1_debug(("DISPCTR: GET_CC_HUE\r\n"));

			idec_get_color_ctrl_hue(IDE_ID_1, p_disp_param->SEL.GET_CC_HUE.p_hue_tab);
		}
		break;

	case DISPCTRL_GET_CC_INT: {
			dispctr1_debug(("DISPCTR: GET_CC_INT\r\n"));

			idec_get_color_ctrl_int(IDE_ID_1, p_disp_param->SEL.GET_CC_INT.p_int_tab);
		}
		break;

	case DISPCTRL_GET_CC_SAT: {
			dispctr1_debug(("DISPCTR: GET_CC_SAT\r\n"));

			idec_get_color_ctrl_sat(IDE_ID_1, p_disp_param->SEL.GET_CC_SAT.p_sat_tab);
		}
		break;

	case DISPCTRL_GET_CC_DDS: {
			dispctr1_debug(("DISPCTR: GET_CC_DDS\r\n"));

			idec_get_color_ctrl_dds(IDE_ID_1, p_disp_param->SEL.GET_CC_DDS.p_dds_tab);
		}
		break;

	case DISPCTRL_GET_CC_INT_OFS: {
			INT8 iintofs, isatofs;

			dispctr1_debug(("DISPCTR: GET_CC_INT_OFS\r\n"));

			idec_get_color_ctrl_int_sat_ofs(IDE_ID_1, &iintofs, &isatofs);

			p_disp_param->SEL.GET_CC_INT_OFS.iintofs = iintofs;
		}
		break;

	case DISPCTRL_GET_CC_SAT_OFS: {
			INT8 iintofs, isatofs;

			dispctr1_debug(("DISPCTR: GET_CC_SAT_OFS\r\n"));

			idec_get_color_ctrl_int_sat_ofs(IDE_ID_1, &iintofs, &isatofs);

			p_disp_param->SEL.GET_CC_SAT_OFS.isatofs = isatofs;
		}
		break;

	case DISPCTRL_GET_CCA_YCON: {
			UINT8 uiycon;

			dispctr1_debug(("DISPCTR: GET_CCA_YCON\r\n"));

			idec_get_color_comp_ycon(IDE_ID_1, &uiycon);

			p_disp_param->SEL.GET_CCA_YCON.uiycon = uiycon;
		}
		break;

	case DISPCTRL_GET_CCA_CCON: {
			UINT8 uiccon;

			dispctr1_debug(("DISPCTR: GET_CCA_CCON\r\n"));

			idec_get_color_comp_ccon(IDE_ID_1, &uiccon);

			p_disp_param->SEL.GET_CCA_CCON.uiccon = uiccon;
		}
		break;

	case DISPCTRL_GET_CCA_YOFS: {
			INT8 iyofs;

			dispctr1_debug(("DISPCTR: GET_CCA_YOFS\r\n"));

			idec_get_color_comp_yofs(IDE_ID_1, &iyofs);

			p_disp_param->SEL.GET_CCA_YOFS.iyofs = iyofs;
		}
		break;

	case DISPCTRL_GET_CCA_COFS: {
			UINT8 uicbofs, uicrofs;

			dispctr1_debug(("DISPCTR: GET_CCA_COFS\r\n"));

			idec_get_color_comp_cofs(IDE_ID_1, &uicbofs, &uicrofs);

			p_disp_param->SEL.GET_CCA_COFS.uicbofs = uicbofs;
			p_disp_param->SEL.GET_CCA_COFS.uicrofs = uicrofs;
		}
		break;

	//case DISPCTRL_SET_ICST_COEF:  // use 4 fixed coef
	//case DISPCTRL_SET_HLPF_EN:
	//case DISPCTRL_GET_ICST_COEF:
	//case DISPCTRL_GET_HLPF_EN:
	default : {
			DBG_WRN("DISPCTL No Support!(%d)\r\n", (int)disp_ctrl);
			return E_NOSPT;
		}

	}

	return E_OK;
}

/*
    Display layer Control

    This API is used as the display layer control API.
    Such as layer enable/disable, Format select, Buffer/Window dimension,
    ...etc.

    @param[in] layer        Display layer selection
    @param[in] lyr_op        Display layer control command selection. Refer to display.h.
    @param[in] p_lyr_param    Display layer Control parameters according to the layer control command.

    @return
     - @b E_NOEXS:  Display object has not opened.
     - @b E_SYS:    layer selection error.
     - @b E_NOSPT:  layer Control Command no support.
     - @b E_OK:     layer Control Command done.
*/
ER  disp_set_disp1_layer_ctrl(DISPLAYER layer, DISPLAYER_OP lyr_op, PDISPLAYER_PARAM p_lyr_param)
{
	//BOOL    bVdo1Sel,bVdo2Sel;
	//BOOL    bOsd1Sel,bOsd2Sel;

	if (!gui_disp_obj_opened[DISP_1]) {
		return E_NOEXS;
	}

	if ((layer & DISPLAYER_MASK) == 0) {
		return E_PAR;
	}

	switch (lyr_op) {
	///////////////////////
	/* SET control group */
	///////////////////////
	case DISPLAYER_OP_SET_ENABLE: {
			displyr1_debug(("disp_lyr: SET_ENABLE(%d)\r\n", (int)p_lyr_param->SEL.SET_ENABLE.b_en));
			/*if(b_vdo_sel)
			{
			    ide_set_v1_en(p_lyr_param->SEL.SET_ENABLE.b_en);
			}
			else
			{
			    ide_set_o1_en(p_lyr_param->SEL.SET_ENABLE.b_en);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_set_v1_en(p_lyr_param->SEL.SET_ENABLE.b_en);
				break;
			case DISPLAYER_VDO2:
				ide_set_v2_en(p_lyr_param->SEL.SET_ENABLE.b_en);
				break;
			case DISPLAYER_OSD1:
				ide_set_o1_en(p_lyr_param->SEL.SET_ENABLE.b_en);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_set_o2_en(p_lyr_param->SEL.SET_ENABLE.b_en);
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_MODE: {
			IDE_BJMODE  temp1;
			IDE_BUF_NUM temp2;
			IDE_OP_BUF  act_buf;

			displyr1_debug(("disp_lyr: SET_MODE (%d)\r\n", (int)b_vdo_sel));
			/*if (b_vdo_sel) {
				IDE_BJMODE  temp1;
				IDE_BUF_NUM temp2;
				IDE_OP_BUF  act_buf;

				ide_get_v1_buf_op(&temp1, &act_buf, &temp2);
				ide_set_v1_buf_op(p_lyr_param->SEL.SET_MODE.buf_mode, act_buf, p_lyr_param->SEL.SET_MODE.buf_number);
				ide_set_v1_fmt(p_lyr_param->SEL.SET_MODE.buf_format);
			} else {
				ide_set_o1_fmt(p_lyr_param->SEL.SET_MODE.buf_format);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_get_v1_buf_op(&temp1, &act_buf, &temp2);
				ide_set_v1_buf_op(p_lyr_param->SEL.SET_MODE.buf_mode, act_buf, p_lyr_param->SEL.SET_MODE.buf_number);
				ide_set_v1_fmt(p_lyr_param->SEL.SET_MODE.buf_format);
				break;
			case DISPLAYER_VDO2:
				ide_get_v2_buf_op(&temp1, &act_buf, &temp2);
				ide_set_v2_buf_op(p_lyr_param->SEL.SET_MODE.buf_mode, act_buf, p_lyr_param->SEL.SET_MODE.buf_number);
				ide_set_v2_fmt(p_lyr_param->SEL.SET_MODE.buf_format);
				break;
			case DISPLAYER_OSD1:
				ide_set_o1_fmt(p_lyr_param->SEL.SET_MODE.buf_format);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_set_o2_fmt(p_lyr_param->SEL.SET_MODE.buf_format);
				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_SET_BUFSIZE: {
			VOSD_WINDOW_ATTR vosd_win_attr;
			IDE_COLOR_FORMAT ui_fmt;
			UINT32 ui_x, ui_y;

			displyr1_debug(("disp_lyr: SET_BUFSIZE\r\n"));
			/*if (b_vdo_sel) {
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_line_ofs;

				ide_get_v1_fmt(&ui_fmt);
				ide_get_v1_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = ui_x;
				vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_1, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO1);
#else
				ide_set_v1_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W], g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H], (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_VDO1);
			} else {
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_line_ofs;

				ide_get_o1_fmt(&ui_fmt);
				ide_get_o1_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = ui_x;
				vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.high_addr   = 0;// Fix as 0 in current ver.

				ide_set_osd_win_attr_ex(IDE_OSDID_1, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD1);
#else
				ide_set_o1_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W], g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H], (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_OSD1);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_line_ofs;

				ide_get_v1_fmt(&ui_fmt);
				ide_get_v1_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = ui_x;
				vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_1, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO1);
#else
				ide_set_v1_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W], g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H], (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_VDO1);
				break;
			case DISPLAYER_VDO2:
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_line_ofs;

				ide_get_v2_fmt(&ui_fmt);
				ide_get_v2_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = ui_x;
				vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_2, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO2);
#else
				ide_set_v2_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W], g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H], (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_VDO2);
				break;
			case DISPLAYER_OSD1:
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_line_ofs;

				ide_get_o1_fmt(&ui_fmt);
				ide_get_o1_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = ui_x;
				vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.high_addr   = 0;// Fix as 0 in current ver.

				ide_set_osd_win_attr_ex(IDE_OSDID_1, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD1);
#else
				ide_set_o1_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W], g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H], (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_OSD1);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L] = p_lyr_param->SEL.SET_BUFSIZE.ui_buf_line_ofs;

				ide_get_o2_fmt(&ui_fmt);
				ide_get_o2_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = ui_x;
				vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.high_addr   = 0;// Fix as 0 in current ver.

				ide_set_osd_win_attr_ex(IDE_OSDID_2, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD2);
#else
				ide_set_o2_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W], g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H], (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_OSD2);
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_VDOBUFADDR: {
			displyr1_debug(("disp_lyr: SET_VDOBUFADDR\r\n"));
			/*if (b_vdo_sel) {
				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel == DISPBUFADR_ALL) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr2;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_0) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr0;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_1) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr1;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_2) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr2;
				}

				//ide_ch_v1_buf(g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]);
#if 0
				ide_set_v1_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v1_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v1_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO1);
#endif
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel == DISPBUFADR_ALL) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr2;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_0) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr0;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_1) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr1;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_2) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr2;
				}

				//ide_ch_v1_buf(g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]);
#if 0
				ide_set_v1_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v1_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v1_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO1);
#endif
				break;
			case DISPLAYER_VDO2:
				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel == DISPBUFADR_ALL) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr2;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_0) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb0;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr0;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_1) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb1;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr1;
				}

				if (p_lyr_param->SEL.SET_VDOBUFADDR.buf_sel & DISPBUFADR_2) {
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_Y]     = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_y2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_CB]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cb2;
					g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_CR]    = p_lyr_param->SEL.SET_VDOBUFADDR.ui_addr_cr2;
				}

				//ide_ch_v1_buf(g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]);
#if 0
				ide_set_v2_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v2_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v2_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO2);
#endif
				break;
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_OSDBUFADDR: {
			displyr1_debug(("disp_lyr: SET_OSDBUFADDR\r\n"));
			/*if (!b_vdo_sel) {
				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel == DISPBUFADR_ALL) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_0][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf0;
					g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_1][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf1;
					g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_2][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf2;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_0) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_0][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf0;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_1) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_1][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf1;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_2) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_2][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf2;
				}

				ide_set_o1_buf_addr(g_disp1_info.disp_data.p_osd_buf_addr[g_disp1_info.disp_data.act_osd_index[DISP_OSD1]][DISPOSDBUF_PALE]);

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
			}*/
			switch (layer) {
			case DISPLAYER_OSD1:
				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel == DISPBUFADR_ALL) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_0][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf0;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_1][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf1;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_2][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf2;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_0][DISPOSDBUF_ALPHA]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_alpha0;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_1][DISPOSDBUF_ALPHA]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_alpha1;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_2][DISPOSDBUF_ALPHA]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_alpha2;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_0) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_0][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf0;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_0][DISPOSDBUF_ALPHA]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_alpha0;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_1) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_1][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf1;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_1][DISPOSDBUF_ALPHA]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_alpha1;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_2) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_2][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf2;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_2][DISPOSDBUF_ALPHA]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_alpha2;
				}

				ide_set_o1_buf_addr(g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][g_disp1_info.disp_data.act_osd_index[DISP_OSD1]][DISPOSDBUF_PALE]);
				ide_set_o1_buf_alpha_addr(g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][g_disp1_info.disp_data.act_osd_index[DISP_OSD1]][DISPOSDBUF_ALPHA]);
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel == DISPBUFADR_ALL) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_0][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf0;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_1][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf1;
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_2][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf2;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_0) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_0][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf0;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_1) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_1][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf1;
				}

				if (p_lyr_param->SEL.SET_OSDBUFADDR.buf_sel & DISPBUFADR_2) {
					g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_2][DISPOSDBUF_PALE]  = p_lyr_param->SEL.SET_OSDBUFADDR.ui_addr_buf2;
				}

				ide_set_o2_buf_addr(g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][g_disp1_info.disp_data.act_osd_index[DISP_OSD2]][DISPOSDBUF_PALE]);

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD2);
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_ACTBUF: {
			displyr1_debug(("disp_lyr: SET_ACTBUF\r\n"));
			/*if (b_vdo_sel) {
				g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]                      = p_lyr_param->SEL.SET_ACTBUF.active_buf;
				ide_ch_v1_buf(g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]);
			} else {
				g_disp1_info.disp_data.act_osd_index[DISP_OSD1]                      = p_lyr_param->SEL.SET_ACTBUF.active_buf;

#if 0
				ide_set_o1_buf_addr(g_disp1_info.disp_data.p_osd_buf_addr[g_disp1_info.disp_data.act_osd_index[DISP_OSD1]][DISPOSDBUF_PALE]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
#endif
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]                      = p_lyr_param->SEL.SET_ACTBUF.active_buf;
				disp_translate_buf_address(DISPLAYER_VDO1);
				//ide_ch_v1_buf(g_disp1_info.disp_data.act_vdo_index[DISP_VDO1]);
				break;
			case DISPLAYER_VDO2:
				g_disp1_info.disp_data.act_vdo_index[DISP_VDO2]                      = p_lyr_param->SEL.SET_ACTBUF.active_buf;
				disp_translate_buf_address(DISPLAYER_VDO2);
				//ide_ch_v2_buf(g_disp1_info.disp_data.act_vdo_index[DISP_VDO2]);
				break;
			case DISPLAYER_OSD1:
				g_disp1_info.disp_data.act_osd_index[DISP_OSD1]                      = p_lyr_param->SEL.SET_ACTBUF.active_buf;

#if 0
				ide_set_o1_buf_addr(g_disp1_info.disp_data.p_osd_buf_addr[g_disp1_info.disp_data.act_osd_index[DISP_OSD1]][DISPOSDBUF_PALE]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
#endif
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				g_disp1_info.disp_data.act_osd_index[DISP_OSD2]                      = p_lyr_param->SEL.SET_ACTBUF.active_buf;

#if 0
				ide_set_o2_buf_addr(g_disp1_info.disp_data.p_osd_buf_addr[g_disp1_info.disp_data.act_osd_index[DISP_OSD2]][DISPOSDBUF_PALE]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD2);
#endif
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_OUTDIR: {
			displyr1_debug(("disp_lyr: SET_OUTDIR (%d)\r\n", (int)p_lyr_param->SEL.SET_OUTDIR.buf_out_dir));
			/*if (b_vdo_sel) {
				ide_set_v1_read_order(p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x1, ((p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x2) >> 1));

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO1);
			} else {
				ide_set_o1_read_order(p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x1, ((p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x2) >> 1));

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_set_v1_read_order(p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x1, ((p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x2) >> 1));

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO1);
				break;
			case DISPLAYER_VDO2:
				ide_set_v2_read_order(p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x1, ((p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x2) >> 1));

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO2);
				break;
			case DISPLAYER_OSD1:
				ide_set_o1_read_order(p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x1, ((p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x2) >> 1));

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_set_o2_read_order(p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x1, ((p_lyr_param->SEL.SET_OUTDIR.buf_out_dir & 0x2) >> 1));

				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD2);
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_WINSIZE: {
			VOSD_WINDOW_ATTR vosd_win_attr;
			IDE_COLOR_FORMAT ui_fmt;

			displyr1_debug(("disp_lyr: SET_WINSIZE \r\n"));
			/*if (b_vdo_sel) {
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W] = p_lyr_param->SEL.SET_WINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H] = p_lyr_param->SEL.SET_WINSIZE.ui_win_height;

				ide_get_v1_fmt(&ui_fmt);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_1, &vosd_win_attr, FALSE);

				// Translate the buffer W/H/address HERE
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO1);
#endif
			} else {
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W] = p_lyr_param->SEL.SET_WINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H] = p_lyr_param->SEL.SET_WINSIZE.ui_win_height;

				ide_get_o1_fmt(&ui_fmt);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_osd_win_attr_ex(IDE_OSDID_1, &vosd_win_attr, FALSE);

				// Translate the buffer W/H/address HERE
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD1);
#endif
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W] = p_lyr_param->SEL.SET_WINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H] = p_lyr_param->SEL.SET_WINSIZE.ui_win_height;

				ide_get_v1_fmt(&ui_fmt);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_1, &vosd_win_attr, FALSE);

				// Translate the buffer W/H/address HERE
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO1);
#endif

				break;
			case DISPLAYER_VDO2:
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W] = p_lyr_param->SEL.SET_WINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H] = p_lyr_param->SEL.SET_WINSIZE.ui_win_height;

				ide_get_v2_fmt(&ui_fmt);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_2, &vosd_win_attr, FALSE);

				// Translate the buffer W/H/address HERE
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO2);
#endif
				break;
			case DISPLAYER_OSD1:
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W] = p_lyr_param->SEL.SET_WINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H] = p_lyr_param->SEL.SET_WINSIZE.ui_win_height;
				displyr1_debug(("w %d h %d\r\n", (int)g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W], (int)g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H]));
				ide_get_o1_fmt(&ui_fmt);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;
				ide_set_osd_win_attr_ex(IDE_OSDID_1, &vosd_win_attr, FALSE);

				// Translate the buffer W/H/address HERE
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD1);
#endif

				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_W] = p_lyr_param->SEL.SET_WINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_H] = p_lyr_param->SEL.SET_WINSIZE.ui_win_height;

				ide_get_o2_fmt(&ui_fmt);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_WINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_osd_win_attr_ex(IDE_OSDID_2, &vosd_win_attr, FALSE);

				// Translate the buffer W/H/address HERE
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD1);
#endif

				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_SET_PALETTE: {
			displyr1_debug(("disp_lyr: SET_PALETTE \r\n"));
			switch (layer) {
			case DISPLAYER_VDO1:
			case DISPLAYER_VDO2:
				break;
			case DISPLAYER_OSD1:
				ide_set_palette_group(p_lyr_param->SEL.SET_PALETTE.ui_start, p_lyr_param->SEL.SET_PALETTE.ui_number, p_lyr_param->SEL.SET_PALETTE.p_pale_entry);
				//Need Fill Shadow OSD palette at the same time(0x600~0x9FC)
				ide_set_palette_group(p_lyr_param->SEL.SET_PALETTE.ui_start + 256, p_lyr_param->SEL.SET_PALETTE.ui_number, p_lyr_param->SEL.SET_PALETTE.p_pale_entry);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_set_palette_group(p_lyr_param->SEL.SET_PALETTE.ui_start, p_lyr_param->SEL.SET_PALETTE.ui_number, p_lyr_param->SEL.SET_PALETTE.p_pale_entry);
				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_SET_PALETTEACRCBY: {
			displyr1_debug(("disp_lyr: SET_PALETTEACRCBY \r\n"));
			/*if (!b_vdo_sel) {
				ide_set_palette_group(p_lyr_param->SEL.SET_PALETTE.ui_start, p_lyr_param->SEL.SET_PALETTE.ui_number, p_lyr_param->SEL.SET_PALETTE.p_pale_entry);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:

				break;
			case DISPLAYER_VDO2:

				break;
			case DISPLAYER_OSD1:
				ide_set_palette_group_a_cr_cb_y(p_lyr_param->SEL.SET_PALETTE.ui_start, p_lyr_param->SEL.SET_PALETTE.ui_number, p_lyr_param->SEL.SET_PALETTE.p_pale_entry);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_set_palette_group_a_cr_cb_y(p_lyr_param->SEL.SET_PALETTE.ui_start, p_lyr_param->SEL.SET_PALETTE.ui_number, p_lyr_param->SEL.SET_PALETTE.p_pale_entry);
				break;
#endif
			default:
				break;
			}

		}
		break;


	case DISPLAYER_OP_SET_BLEND: {
			IDE_ALPHA_TYPE  ui_alpha_type;

			switch (p_lyr_param->SEL.SET_BLEND.type) {
			default:
			case DISPCTRL_BLEND_TYPE_NOALPHA:
				ui_alpha_type = IDE_NO_ALPHA;
				break;

			case DISPCTRL_BLEND_TYPE_GLOBAL:
				ui_alpha_type = IDE_GLOBAL_ALPHA;
				break;

			case DISPCTRL_BLEND_TYPE_GLOBAL_BACK:
				ui_alpha_type = IDE_GLOBAL_ALPHA_BACK;
				break;

			case DISPCTRL_BLEND_TYPE_SOURCE:
				ui_alpha_type = IDE_SOURCE_ALPHA;
				break;

			case DISPCTRL_BLEND_TYPE_SOURCE_BACK:
				ui_alpha_type = IDE_SOURCE_ALPHA_BACK;
				break;
			}

			switch (layer) {
			case DISPLAYER_VDO1:
				ide_set_alpha_blending(IDE_BLEND_V1, ui_alpha_type, p_lyr_param->SEL.SET_BLEND.ui_global_alpha);
				break;

			case DISPLAYER_VDO2:
				ide_set_alpha_blending(IDE_BLEND_V2, ui_alpha_type, p_lyr_param->SEL.SET_BLEND.ui_global_alpha);
				break;
			//if swap = 0 alpha2 = Line
			//if swap = 1 alpha2 = FD
			case DISPLAYER_FD:
					ide_set_alpha_blending(IDE_BLEND_FD, ui_alpha_type, p_lyr_param->SEL.SET_BLEND.ui_global_alpha);
				break;

			case DISPLAYER_OSD1:
				if (ui_alpha_type == IDE_SOURCE_ALPHA)
					ui_alpha_type -= 1;
				else if (ui_alpha_type == IDE_GLOBAL_ALPHA_BACK)
					ui_alpha_type += 1;

				if (p_lyr_param->SEL.SET_BLEND.b_global_alpha5) {
					ide_set_alpha_blending(IDE_BLEND_O1_GLBALPHA5, ui_alpha_type, p_lyr_param->SEL.SET_BLEND.ui_global_alpha);
				} else {
					ide_set_alpha_blending(IDE_BLEND_O1, ui_alpha_type, p_lyr_param->SEL.SET_BLEND.ui_global_alpha);
				}
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_set_alpha_blending(IDE_BLEND_O2, ui_alpha_type, p_lyr_param->SEL.SET_BLEND.ui_global_alpha);
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_VDOCOLORKEY_SRC: {
			displyr1_debug(("disp_lyr: SET_VDOCOLORKEY_SRC (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_VDO1:
			case DISPLAYER_VDO2:
				idec_set_video_colorkey_sel(IDE_ID_1, p_lyr_param->SEL.SET_VDOCOLORKEY_SRC.ck_src);
				break;

			case DISPLAYER_OSD1:
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_COLORKEY: {
			IDE_VIDEO_COLORKEY_OP ck_op;
			IDE_OSD_COLORKEY_OP   osd_ck_op;
			BOOL                  b_ck_en;

			displyr1_debug(("disp_lyr: SET_COLORKEY (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_VDO1:
			case DISPLAYER_VDO2:
				switch (p_lyr_param->SEL.SET_COLORKEY.ck_op) {
				case DISPCK_OP_YSMALLKEY:
					ck_op = IDE_VIDEO_COLORKEY_YSMALLKEY;
					break;

				case DISPCK_OP_YEQUKEY:
					ck_op = IDE_VIDEO_COLORKEY_YEQUKEY;
					break;

				case DISPCK_OP_YBIGKEY:
					ck_op = IDE_VIDEO_COLORKEY_YBIGKEY;
					break;

				case DISPCK_OP_OFF:
				default:
					ck_op = IDE_VIDEO_COLORKEY_VIDEO1OR2;
					break;
				}
				idec_set_video_colorkey_op(IDE_ID_1, ck_op);
				idec_set_video_colorkey(IDE_ID_1, p_lyr_param->SEL.SET_COLORKEY.ui_ck_y, p_lyr_param->SEL.SET_COLORKEY.ui_ck_cb, p_lyr_param->SEL.SET_COLORKEY.ui_ck_cr);
				break;

			case DISPLAYER_OSD1:
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
#endif
				switch (p_lyr_param->SEL.SET_COLORKEY.ck_op) {
				case DISPCK_OP_YEQUKEY:
					osd_ck_op = IDE_OSD_COLORKEY_EQUAL;
					b_ck_en = TRUE;
					break;

				case DISPCK_OP_YEQUKEY_A:
					osd_ck_op = IDE_OSD_COLORKEY_EQUAL_A;
					b_ck_en = TRUE;
					break;

				case DISPCK_OP_OFF:
				default:
					osd_ck_op = IDE_OSD_COLORKEY_EQUAL;
					b_ck_en = FALSE;
					break;
				}
				//IDE1_OSD2_EXIST
//         if (layer == DISPLAYER_OSD1)
//         {
				idec_set_osd1_colorkey_en(IDE_ID_1, b_ck_en);
				idec_set_osd1_colorkey_op(IDE_ID_1, osd_ck_op);
				idec_set_osd1_colorkey(IDE_ID_1, p_lyr_param->SEL.SET_COLORKEY.ui_ck_y, p_lyr_param->SEL.SET_COLORKEY.ui_ck_cb, p_lyr_param->SEL.SET_COLORKEY.ui_ck_cr, p_lyr_param->SEL.SET_COLORKEY.ui_ck_alpha);
//         }
//         else // DISPLAYER_OSD2
//         {
//             idec_setOsd2ColorKeyEn(IDE_ID_1, b_ck_en);
//             idec_setOsd2ColorKeyOp(IDE_ID_1, osd_ck_op);
//             idec_setOsd2ColorKey(IDE_ID_1, p_lyr_param->SEL.SET_COLORKEY.ui_ck_y, p_lyr_param->SEL.SET_COLORKEY.ui_ck_cb, p_lyr_param->SEL.SET_COLORKEY.ui_ck_cr);
//         }
				break;

			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_BUFXY: {
			displyr1_debug(("disp_lyr: SET_BUFXY (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_VDO1:
				g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO1][DISPBUF_X]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_x;
				g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO1][DISPBUF_Y]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_y;
#if 0
				ide_set_v1_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v1_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v1_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO1);
#endif
				break;
			case DISPLAYER_VDO2:
				g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO2][DISPBUF_X]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_x;
				g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO2][DISPBUF_Y]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_y;
#if 0
				ide_set_v2_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v2_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v2_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_VDO2);
#endif
				break;
			case DISPLAYER_OSD1:
				g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD1][DISPBUF_X]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_x;
				g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD1][DISPBUF_Y]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_y;
#if 0
				ide_set_v1_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v1_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v1_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD1);
#endif
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD2][DISPBUF_X]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_x;
				g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD2][DISPBUF_Y]    = p_lyr_param->SEL.SET_BUFXY.ui_buf_ofs_y;
#if 0
				ide_set_v2_buf0_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR]);
				ide_set_v2_buf1_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR]);
				ide_set_v2_buf2_addr(g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB], g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR]);
#else
				// Translate the buffer address HERE
				disp_translate_buf_address(DISPLAYER_OSD2);
#endif
				break;
#endif
			default:
				break;
			}
		}
		break;
/*
	case DISPLAYER_OP_SET_DMALEN: {
			displyr1_debug(("disp_lyr: SET_DMALEN (%d)\r\n", layer));
			switch (layer) {
			case DISPLAYER_VDO1:
				idec_set_v1_burst_len(IDE_ID_1, p_lyr_param->SEL.SET_DMALEN.DMAY_A, p_lyr_param->SEL.SET_DMALEN.DMAC_RGB);
				break;
			case DISPLAYER_VDO2:
				idec_set_v2_burst_len(IDE_ID_1, p_lyr_param->SEL.SET_DMALEN.DMAY_A, p_lyr_param->SEL.SET_DMALEN.DMAC_RGB);
				break;
			case DISPLAYER_OSD1:
				idec_set_o1_burst_len(IDE_ID_1, p_lyr_param->SEL.SET_DMALEN.DMAY_A, p_lyr_param->SEL.SET_DMALEN.DMAC_RGB);
				break;
			case DISPLAYER_OSD2:
				idec_setO2BurstLen(IDE_ID_1, p_lyr_param->SEL.SET_DMALEN.DMAY_A, p_lyr_param->SEL.SET_DMALEN.DMAC_RGB);
				break;

			default:
				break;
			}
		}
		break;
*/
	case DISPLAYER_OP_SET_FDEN: {
			displyr1_debug(("disp_lyr: SET_FDEN (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_FD:
				if (p_lyr_param->SEL.SET_FDEN.b_en == TRUE) {
					idec_set_fd_en(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.SET_FDEN.fd_num);
				} else {
					idec_set_fd_dis(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.SET_FDEN.fd_num);
				}
				break;

			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_FDSIZE: {
			displyr1_debug(("disp_lyr: SET_FDSIZE (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_FD:
				idec_set_fd_win_pos(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.SET_FDCOLOR.fd_num, p_lyr_param->SEL.SET_FDSIZE.ui_fdx, p_lyr_param->SEL.SET_FDSIZE.ui_fdy);
				idec_set_fd_win_dim(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.SET_FDCOLOR.fd_num, p_lyr_param->SEL.SET_FDSIZE.ui_fdw, p_lyr_param->SEL.SET_FDSIZE.ui_fdh);
				idec_set_fd_win_bord(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.SET_FDCOLOR.fd_num, p_lyr_param->SEL.SET_FDSIZE.ui_fd_bord_w, p_lyr_param->SEL.SET_FDSIZE.ui_fd_bord_h);
				break;

			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_FDCOLOR: {
			displyr1_debug(("disp_lyr: SET_FDCOLOR (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_FD:
				idec_set_fd_color(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.SET_FDCOLOR.fd_num, p_lyr_param->SEL.SET_FDCOLOR.ui_fd_cr_y, p_lyr_param->SEL.SET_FDCOLOR.ui_fd_cr_cb, p_lyr_param->SEL.SET_FDCOLOR.ui_fd_cr_cr);
				break;

			default:
				break;
			}

		}
		break;



	case DISPLAYER_OP_SET_H_BILINEAR: {
			displyr1_debug(("disp_lyr: SET_H_BILINEAR (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_VDO1:
				idec_set_v1_hsm(IDE_ID_1, p_lyr_param->SEL.SET_H_BILINEAR.b_bilinear);
				break;

			case DISPLAYER_VDO2:
				idec_set_v2_hsm(IDE_ID_1, p_lyr_param->SEL.SET_H_BILINEAR.b_bilinear);
				break;

			case DISPLAYER_OSD1:
				idec_set_o1_hsm(IDE_ID_1, p_lyr_param->SEL.SET_H_BILINEAR.b_bilinear);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				idec_set_o2_hsm(IDE_ID_1, p_lyr_param->SEL.SET_H_BILINEAR.b_bilinear);
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_SET_BUFWINSIZE: {
			VOSD_WINDOW_ATTR vosd_win_attr;
			IDE_COLOR_FORMAT ui_fmt;
			//UINT32 ui_x, ui_y;

			displyr1_debug(("disp_lyr: SET_BUFWINSIZE \r\n"));
			switch (layer) {
			case DISPLAYER_VDO1:
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_line_ofs;

				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_height;

				ide_get_v1_fmt(&ui_fmt);
				//ide_get_v1_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				//vosd_win_attr.win_x       = ui_x;
				//vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_1, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO1);
#else
				ide_set_v1_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W], g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H], (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_VDO1);
				break;
			case DISPLAYER_VDO2:
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_line_ofs;

				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_height;

				ide_get_v2_fmt(&ui_fmt);
				//ide_get_v2_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_vdo_win_dim[DISP_VDO2][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				//vosd_win_attr.win_x       = ui_x;
				//vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;

				ide_set_video_win_attr_ex(IDE_VIDEOID_2, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_VDO2);
#else
				ide_set_v2_buf_dim(g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W], g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H], (g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_VDO2);
				break;
			case DISPLAYER_OSD1:
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_line_ofs;

				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_height;

				ide_get_o1_fmt(&ui_fmt);
				//ide_get_o1_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD1][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				//vosd_win_attr.win_x       = ui_x;
				//vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;// Fix as 0 in current ver.

				ide_set_osd_win_attr_ex(IDE_OSDID_1, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD1);
#else
				ide_set_o1_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W], g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H], (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_OSD1);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_width;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_height;
				g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_buf_line_ofs;

				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_W] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_width;
				g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_H] = p_lyr_param->SEL.SET_BUFWINSIZE.ui_win_height;

				ide_get_o2_fmt(&ui_fmt);
				//ide_get_o2_win_pos(&ui_x, &ui_y);
				vosd_win_attr.source_w    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W];
				vosd_win_attr.source_h    = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H];
				vosd_win_attr.des_w       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_W];
				vosd_win_attr.des_h       = g_disp1_info.disp_data.pui_osd_win_dim[DISP_OSD2][DISPDIM_H];
				vosd_win_attr.win_format  = ui_fmt;
				//vosd_win_attr.win_x       = ui_x;
				//vosd_win_attr.win_y       = ui_y;
				vosd_win_attr.win_x       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_x;
				vosd_win_attr.win_y       = p_lyr_param->SEL.SET_BUFWINSIZE.i_win_ofs_y;
				vosd_win_attr.high_addr   = 0;// Fix as 0 in current ver.

				ide_set_osd_win_attr_ex(IDE_OSDID_2, &vosd_win_attr, FALSE);
#if DISP_PATCHBUF_RESIZE
				disp_translate_buf_size(DISPLAYER_OSD2);
#else
				ide_set_o2_buf_dim(g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W], g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H], (g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L] >> 2));
#endif
				disp_translate_buf_address(DISPLAYER_OSD2);
				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_SET_FDLINESWAP: {
			dispdev1_debug(("disp_lyr: SET_OSD_LAYER_SWAP\r\n"));
			idec_set_fd_line_layer_swap(IDE_ID_1, p_lyr_param->SEL.SET_FDLINESWAP.b_en_fd_line_swap);
		}
		break;

	///////////////////////
	/* GET control group */
	///////////////////////

	case DISPLAYER_OP_GET_ENABLE: {
			displyr1_debug(("disp_lyr: GET_ENABLE \r\n"));
			/*if (b_vdo_sel) {
				p_lyr_param->SEL.GET_ENABLE.b_en = ide_get_v1_en();
			} else {
				p_lyr_param->SEL.GET_ENABLE.b_en = ide_get_o1_en();
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				p_lyr_param->SEL.GET_ENABLE.b_en = ide_get_v1_en();
				break;
			case DISPLAYER_VDO2:
				p_lyr_param->SEL.GET_ENABLE.b_en = ide_get_v2_en();
				break;
			case DISPLAYER_OSD1:
				p_lyr_param->SEL.GET_ENABLE.b_en = ide_get_o1_en();
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				p_lyr_param->SEL.GET_ENABLE.b_en = ide_get_o2_en();
				break;
#endif
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_GET_MODE: {
			IDE_COLOR_FORMAT ui_fmt;
			IDE_BJMODE ui_bjmode;
			IDE_OP_BUF ui_opt_buf;
			IDE_BUF_NUM ui_buf_num;

			displyr1_debug(("disp_lyr: GET_MODE \r\n"));
			/*if (b_vdo_sel) {
				ide_get_v1_fmt(&ui_fmt);
				ide_get_v1_buf_op(&ui_bjmode, &ui_opt_buf, &ui_buf_num);

				p_lyr_param->SEL.GET_MODE.buf_format   = ui_fmt;
				p_lyr_param->SEL.GET_MODE.buf_mode     = ui_bjmode;
				p_lyr_param->SEL.GET_MODE.buf_number   = ui_buf_num;
			} else {
				ide_get_o1_fmt(&ui_fmt);
				p_lyr_param->SEL.GET_MODE.buf_format   = ui_fmt;
				p_lyr_param->SEL.GET_MODE.buf_mode     = DISPBUFMODE_BUFFER_REPEAT;
				p_lyr_param->SEL.GET_MODE.buf_number   = DISPBUFNUM_3;
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_get_v1_fmt(&ui_fmt);
				ide_get_v1_buf_op(&ui_bjmode, &ui_opt_buf, &ui_buf_num);

				p_lyr_param->SEL.GET_MODE.buf_format   = ui_fmt;
				p_lyr_param->SEL.GET_MODE.buf_mode     = ui_bjmode;
				p_lyr_param->SEL.GET_MODE.buf_number   = ui_buf_num;

				break;
			case DISPLAYER_VDO2:
				ide_get_v2_fmt(&ui_fmt);
				ide_get_v2_buf_op(&ui_bjmode, &ui_opt_buf, &ui_buf_num);

				p_lyr_param->SEL.GET_MODE.buf_format   = ui_fmt;
				p_lyr_param->SEL.GET_MODE.buf_mode     = ui_bjmode;
				p_lyr_param->SEL.GET_MODE.buf_number   = ui_buf_num;

				break;
			case DISPLAYER_OSD1:
				ide_get_o1_fmt(&ui_fmt);
				p_lyr_param->SEL.GET_MODE.buf_format   = ui_fmt;
				p_lyr_param->SEL.GET_MODE.buf_mode     = DISPBUFMODE_BUFFER_REPEAT;
				p_lyr_param->SEL.GET_MODE.buf_number   = DISPBUFNUM_1;

				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_get_o2_fmt(&ui_fmt);
				p_lyr_param->SEL.GET_MODE.buf_format   = ui_fmt;
				p_lyr_param->SEL.GET_MODE.buf_mode     = DISPBUFMODE_BUFFER_REPEAT;
				p_lyr_param->SEL.GET_MODE.buf_number   = DISPBUFNUM_1;
				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_BUFSIZE: {
			displyr1_debug(("disp_lyr: GET_BUFSIZE \r\n"));
			/*if (b_vdo_sel) {
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_width   = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_height  = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_line_ofs = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L];
			} else {
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_width   = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_height  = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_line_ofs = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L];
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_width   = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_W];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_height  = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_H];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_line_ofs = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO1][DISPDIM_L];
				break;
			case DISPLAYER_VDO2:
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_width   = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_W];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_height  = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_H];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_line_ofs = g_disp1_info.disp_data.pui_vdo_buf_dim[DISP_VDO2][DISPDIM_L];
				break;
			case DISPLAYER_OSD1:
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_width   = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_W];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_height  = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_H];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_line_ofs = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD1][DISPDIM_L];
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_width   = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_W];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_height  = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_H];
				p_lyr_param->SEL.GET_BUFSIZE.ui_buf_line_ofs = g_disp1_info.disp_data.pui_osd_buf_dim[DISP_OSD2][DISPDIM_L];
				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_VDOBUFADDR: {
			displyr1_debug(("disp_lyr: GET_VDOBUFADDR \r\n"));
			/*if (b_vdo_sel) {
				p_lyr_param->SEL.GET_VDOBUFADDR.active_buf = g_disp1_info.disp_data.act_vdo_index[DISP_VDO1];

				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y0  = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb0 = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr0 = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_0][DISPVDOBUF_CR];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y1  = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb1 = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr1 = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_1][DISPVDOBUF_CR];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y2  = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb2 = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr2 = g_disp1_info.disp_data.p_vdo_buf_addr[DISPACTBUF_2][DISPVDOBUF_CR];
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				p_lyr_param->SEL.GET_VDOBUFADDR.active_buf = g_disp1_info.disp_data.act_vdo_index[DISP_VDO1];

				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y0  = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb0 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr0 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_0][DISPVDOBUF_CR];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y1  = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb1 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr1 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_1][DISPVDOBUF_CR];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y2  = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb2 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr2 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO1][DISPACTBUF_2][DISPVDOBUF_CR];
				break;
			case DISPLAYER_VDO2:
				p_lyr_param->SEL.GET_VDOBUFADDR.active_buf = g_disp1_info.disp_data.act_vdo_index[DISP_VDO2];

				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y0  = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb0 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr0 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_0][DISPVDOBUF_CR];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y1  = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb1 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr1 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_1][DISPVDOBUF_CR];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_y2  = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_Y];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cb2 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_CB];
				p_lyr_param->SEL.GET_VDOBUFADDR.ui_addr_cr2 = g_disp1_info.disp_data.p_vdo_buf_addr[DISP_VDO2][DISPACTBUF_2][DISPVDOBUF_CR];

				break;
			case DISPLAYER_OSD1:

				break;
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_OSDBUFADDR: {
			displyr1_debug(("disp_lyr: GET_OSDBUFADDR \r\n"));
			/*if (!b_vdo_sel) {
				p_lyr_param->SEL.GET_OSDBUFADDR.active_buf = g_disp1_info.disp_data.act_osd_index[DISP_OSD1];

				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf0 = g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_0][DISPOSDBUF_PALE];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf1 = g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_1][DISPOSDBUF_PALE];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf2 = g_disp1_info.disp_data.p_osd_buf_addr[DISPACTBUF_2][DISPOSDBUF_PALE];
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:

				break;
			case DISPLAYER_VDO2:

				break;
			case DISPLAYER_OSD1:
				p_lyr_param->SEL.GET_OSDBUFADDR.active_buf = g_disp1_info.disp_data.act_osd_index[DISP_OSD1];

				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf0 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_0][DISPOSDBUF_PALE];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf1 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_1][DISPOSDBUF_PALE];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf2 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_2][DISPOSDBUF_PALE];

				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_alpha0 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_0][DISPOSDBUF_ALPHA];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_alpha1 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_1][DISPOSDBUF_ALPHA];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_alpha2 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD1][DISPACTBUF_2][DISPOSDBUF_ALPHA];

				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				p_lyr_param->SEL.GET_OSDBUFADDR.active_buf = g_disp1_info.disp_data.act_osd_index[DISP_OSD2];

				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf0 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_0][DISPOSDBUF_PALE];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf1 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_1][DISPOSDBUF_PALE];
				p_lyr_param->SEL.GET_OSDBUFADDR.ui_addr_buf2 = g_disp1_info.disp_data.p_osd_buf_addr[DISP_OSD2][DISPACTBUF_2][DISPOSDBUF_PALE];

				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_ACTBUF: {
			displyr1_debug(("disp_lyr: GET_ACTBUF \r\n"));
			/*if (b_vdo_sel) {
				p_lyr_param->SEL.GET_ACTBUF.active_buf = g_disp1_info.disp_data.act_vdo_index[DISP_VDO1];
			} else {
				p_lyr_param->SEL.GET_ACTBUF.active_buf = g_disp1_info.disp_data.act_osd_index[DISP_OSD1];
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				p_lyr_param->SEL.GET_ACTBUF.active_buf = g_disp1_info.disp_data.act_vdo_index[DISP_VDO1];
				break;
			case DISPLAYER_VDO2:
				p_lyr_param->SEL.GET_ACTBUF.active_buf = g_disp1_info.disp_data.act_vdo_index[DISP_VDO2];
				break;
			case DISPLAYER_OSD1:
				p_lyr_param->SEL.GET_ACTBUF.active_buf = g_disp1_info.disp_data.act_osd_index[DISP_OSD1];
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				p_lyr_param->SEL.GET_ACTBUF.active_buf = g_disp1_info.disp_data.act_osd_index[DISP_OSD2];
				break;
#endif
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_OUTDIR: {
			IDE_HOR_READ l2r = IDE_BUFFER_READ_L2R;
			IDE_VER_READ t2b = IDE_BUFFER_READ_T2B;

			displyr1_debug(("disp_lyr: GET_OUTDIR \r\n"));
			/*if (b_vdo_sel) {
				ide_get_v1_read_order(&l2r, &t2b);
			} else {
				ide_get_o1_read_order(&l2r, &t2b);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_get_v1_read_order(&l2r, &t2b);
				break;
			case DISPLAYER_VDO2:
				ide_get_v2_read_order(&l2r, &t2b);
				break;
			case DISPLAYER_OSD1:
				ide_get_o1_read_order(&l2r, &t2b);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_get_o2_read_order(&l2r, &t2b);
				break;
#endif
			default:
				break;
			}
			p_lyr_param->SEL.GET_OUTDIR.buf_out_dir = (l2r + (t2b << 1));
		}
		break;

	case DISPLAYER_OP_GET_WINSIZE: {
			UINT32 ui_win_w, ui_win_h, ui_x, ui_y;

			ui_win_w = ui_win_h = ui_x = ui_y = 0;

			displyr1_debug(("disp_lyr: GET_WINSIZE \r\n"));
			/*if (b_vdo_sel) {
				ide_get_v1_win_dim(&ui_win_w, &ui_win_h);
				ide_get_v1_win_pos(&ui_x, &ui_y);
			} else {
				ide_get_o1_win_dim(&ui_win_w, &ui_win_h);
				ide_get_o1_win_pos(&ui_x, &ui_y);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_get_v1_win_dim(&ui_win_w, &ui_win_h);
				ide_get_v1_win_pos(&ui_x, &ui_y);
				break;
			case DISPLAYER_VDO2:
				ide_get_v2_win_dim(&ui_win_w, &ui_win_h);
				ide_get_v2_win_pos(&ui_x, &ui_y);
				break;
			case DISPLAYER_OSD1:
				ide_get_o1_win_dim(&ui_win_w, &ui_win_h);
				ide_get_o1_win_pos(&ui_x, &ui_y);
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				ide_get_o2_win_dim(&ui_win_w, &ui_win_h);
				ide_get_o2_win_pos(&ui_x, &ui_y);
				break;
#endif
			default:
				break;
			}

			p_lyr_param->SEL.GET_WINSIZE.ui_win_width   = ui_win_w + 1;
			p_lyr_param->SEL.GET_WINSIZE.ui_win_height  = ui_win_h + 1;
			p_lyr_param->SEL.GET_WINSIZE.i_win_ofs_x     = (INT32)ui_x;
			p_lyr_param->SEL.GET_WINSIZE.i_win_ofs_y     = (INT32)ui_y;
		}
		break;

	case DISPLAYER_OP_GET_PALETTE: {
			displyr1_debug(("disp_lyr: GET_PALETTE \r\n"));
			/*if (!b_vdo_sel) {
				ide_get_palette_group(p_lyr_param->SEL.GET_PALETTE.ui_start, p_lyr_param->SEL.GET_PALETTE.ui_number, p_lyr_param->SEL.GET_PALETTE.p_pale_entry);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:

				break;
			case DISPLAYER_VDO2:

				break;
			case DISPLAYER_OSD1:
//          case DISPLAYER_OSD2:
				ide_get_palette_group(p_lyr_param->SEL.GET_PALETTE.ui_start, p_lyr_param->SEL.GET_PALETTE.ui_number, p_lyr_param->SEL.GET_PALETTE.p_pale_entry);
				break;

			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_SHADOW_PALETTE: {
			DBG_IND("disp_lyr: DISPLAYER_OP_GET_SHADOW_PALETTE \r\n");
			switch (layer) {
			case DISPLAYER_OSD1:
//          case DISPLAYER_OSD2:
				ide_get_shadow_palette_group(p_lyr_param->SEL.GET_PALETTE.ui_start, p_lyr_param->SEL.GET_PALETTE.ui_number, p_lyr_param->SEL.GET_PALETTE.p_pale_entry);
				break;
			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_GET_PALETTEACRCBY: {
			displyr1_debug(("disp_lyr: GET_PALETTEACRCBY r\n"));
			/*if (!b_vdo_sel) {
				ide_get_palette_group(p_lyr_param->SEL.GET_PALETTE.ui_start, p_lyr_param->SEL.GET_PALETTE.ui_number, p_lyr_param->SEL.GET_PALETTE.p_pale_entry);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:

				break;
			case DISPLAYER_VDO2:

				break;
			case DISPLAYER_OSD1:
//          case DISPLAYER_OSD2:
				ide_get_palette_group_a_cr_cb_y(p_lyr_param->SEL.GET_PALETTE.ui_start, p_lyr_param->SEL.GET_PALETTE.ui_number, p_lyr_param->SEL.GET_PALETTE.p_pale_entry);
				break;
			default:
				break;
			}

		}
		break;


	case DISPLAYER_OP_SET_VDOBUFCONTENT: {
			/*if (b_vdo_sel) {
				ide_set_video_buffer_content(IDE_VIDEOID_1, p_lyr_param->SEL.SET_VDOBUFCONTENT.buf_id, p_lyr_param->SEL.SET_VDOBUFCONTENT.p_ycbcr);
			}*/
			switch (layer) {
			case DISPLAYER_VDO1:
				ide_set_video_buffer_content(IDE_VIDEOID_1, p_lyr_param->SEL.SET_VDOBUFCONTENT.buf_id, p_lyr_param->SEL.SET_VDOBUFCONTENT.p_ycbcr);
				break;
			case DISPLAYER_VDO2:
				ide_set_video_buffer_content(IDE_VIDEOID_2, p_lyr_param->SEL.SET_VDOBUFCONTENT.buf_id, p_lyr_param->SEL.SET_VDOBUFCONTENT.p_ycbcr);
				break;
			case DISPLAYER_OSD1:

				break;
			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_BLEND: {

			switch (layer) {
			case DISPLAYER_VDO1: {
					IDE_ALPHA_TYPE type;
					UINT8 global_alpha;

					idec_get_alpha_blending(IDE_ID_1, IDE_BLEND_V1, &type, &global_alpha);
					p_lyr_param->SEL.GET_BLEND.type = type;
					p_lyr_param->SEL.GET_BLEND.ui_global_alpha = global_alpha;
				}
				break;
			case DISPLAYER_VDO2: {
					IDE_ALPHA_TYPE type;
					UINT8 global_alpha;

					idec_get_alpha_blending(IDE_ID_1, IDE_BLEND_V2, &type, &global_alpha);
					p_lyr_param->SEL.GET_BLEND.type = type;
					p_lyr_param->SEL.GET_BLEND.ui_global_alpha = global_alpha;
				}
				break;
			case DISPLAYER_FD: {
					IDE_ALPHA_TYPE type;
					UINT8 global_alpha;

					idec_get_alpha_blending(IDE_ID_1, IDE_BLEND_FD, &type, &global_alpha);
					p_lyr_param->SEL.GET_BLEND.type = type;
					p_lyr_param->SEL.GET_BLEND.ui_global_alpha = global_alpha;
				}
				break;

			case DISPLAYER_OSD1: {
					IDE_ALPHA_TYPE type;
					UINT8 global_alpha;

					idec_get_alpha_blending(IDE_ID_1, IDE_BLEND_O1, &type, &global_alpha);
					if (type == 2)
						type++;
					else if (type == 3)
						type--;

					switch (type) {
					case IDE_NO_ALPHA:
						p_lyr_param->SEL.GET_BLEND.type = DISPCTRL_BLEND_TYPE_NOALPHA;
						break;
					case IDE_GLOBAL_ALPHA:
						p_lyr_param->SEL.GET_BLEND.type = DISPCTRL_BLEND_TYPE_GLOBAL;
						break;
					case IDE_GLOBAL_ALPHA_BACK:
						p_lyr_param->SEL.GET_BLEND.type = DISPCTRL_BLEND_TYPE_GLOBAL_BACK;
						break;
					case IDE_SOURCE_ALPHA:
						p_lyr_param->SEL.GET_BLEND.type = DISPCTRL_BLEND_TYPE_SOURCE;
						break;
					case IDE_SOURCE_ALPHA_BACK:
						p_lyr_param->SEL.GET_BLEND.type = DISPCTRL_BLEND_TYPE_SOURCE_BACK;
						break;
					default:
						p_lyr_param->SEL.GET_BLEND.type = DISPCTRL_BLEND_SEL_DEFAULT;
						DBG_ERR("Unknow alpha type = %d, use default no alpha\r\n", (int)type);
						break;
					}
					p_lyr_param->SEL.GET_BLEND.ui_global_alpha = global_alpha;
				}
				break;

			default:
				break;
			}
		}
		break;


	case DISPLAYER_OP_GET_VDOCOLORKEY_SRC: {
			IDE_VIDEO_COLORKEY_SEL ck_src;

			displyr1_debug(("disp_lyr: GET_VDOCOLORKEY_SRC (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_VDO1:
			case DISPLAYER_VDO2:
				idec_get_video_colorkey_sel(IDE_ID_1, &ck_src);
				p_lyr_param->SEL.GET_VDOCOLORKEY_SRC.ck_src = ck_src;
				break;

			case DISPLAYER_OSD1:
//     case DISPLAYER_OSD2:
			default :
				break;
			}
		}
		break;

	case DISPLAYER_OP_GET_COLORKEY: {
			IDE_VIDEO_COLORKEY_OP ck_op;
			IDE_OSD_COLORKEY_OP   osd_ck_op;
			BOOL                  b_ck_en;
			UINT8                 ui_ck_y, ui_ck_cb, ui_ck_cr, ck_alpha;
			DISPCKOP              disp_op;

			displyr1_debug(("disp_lyr: GET_COLORKEY (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_VDO1:
			case DISPLAYER_VDO2:
				idec_get_video_colorkey_op(IDE_ID_1, &ck_op);
				idec_get_video_colorkey(IDE_ID_1, &ui_ck_y, &ui_ck_cb, &ui_ck_cr);
				switch (ck_op) {
				case IDE_VIDEO_COLORKEY_YSMALLKEY:
					disp_op = DISPCK_OP_YSMALLKEY;
					break;

				case IDE_VIDEO_COLORKEY_YEQUKEY:
					disp_op = DISPCK_OP_YEQUKEY;
					break;

				case IDE_VIDEO_COLORKEY_YBIGKEY:
					disp_op = DISPCK_OP_YBIGKEY;
					break;

				case IDE_VIDEO_COLORKEY_VIDEO1OR2:
				default:
					disp_op = DISPCK_OP_OFF;
					break;
				}
				p_lyr_param->SEL.GET_COLORKEY.ck_op = disp_op;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_y = ui_ck_y;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_cb = ui_ck_cb;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_cr = ui_ck_cr;
				break;

			case DISPLAYER_OSD1:
//     case DISPLAYER_OSD2:
//         if (layer == DISPLAYER_OSD1)
//         {
				idec_get_osd1_colorkey_en(IDE_ID_1, &b_ck_en);
				idec_get_osd1_colorkey_op(IDE_ID_1, &osd_ck_op);
				idec_get_osd1_colorkey(IDE_ID_1, &ui_ck_y, &ui_ck_cb, &ui_ck_cr, &ck_alpha);
//         }
#if 0
				else { // DISPLAYER_OSD2
					idec_get_osd2_colorkey_en(IDE_ID_1, &b_ck_en);
					idec_get_osd2_colorkey_op(IDE_ID_1, &osd_ck_op);
					idec_get_osd2_colorkey(IDE_ID_1, &ui_ck_y, &ui_ck_cb, &ui_ck_cr);
				}
#endif
				if (b_ck_en == TRUE) {
					switch (osd_ck_op) {
					case IDE_OSD_COLORKEY_EQUAL:
						disp_op = DISPCK_OP_YEQUKEY;
						break;

					case IDE_OSD_COLORKEY_EQUAL_A:
						disp_op = DISPCK_OP_YEQUKEY_A;
						break;

					default:
						disp_op = DISPCK_OP_OFF;
						break;
					}
				} else {
					disp_op = DISPCK_OP_OFF;
				}
				p_lyr_param->SEL.GET_COLORKEY.ck_op = disp_op;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_y = ui_ck_y;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_cb = ui_ck_cb;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_cr = ui_ck_cr;
				p_lyr_param->SEL.GET_COLORKEY.ui_ck_alpha = ck_alpha;
				break;

			default:
				break;
			}
		}
		break;


	case DISPLAYER_OP_GET_BUFXY: {
			displyr1_debug(("disp_lyr: GET_BUFXY \r\n"));
			switch (layer) {
			case DISPLAYER_VDO1:
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_x   = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO1][DISPBUF_X];
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_y   = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO1][DISPBUF_Y];
				break;
			case DISPLAYER_VDO2:
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_x   = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO2][DISPBUF_X];
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_y   = g_disp1_info.disp_data.pui_vdo_buf_xy[DISP_VDO2][DISPBUF_Y];
				break;
			case DISPLAYER_OSD1:
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_x   = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD1][DISPBUF_X];
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_y   = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD1][DISPBUF_Y];
				break;
#if IDE1_OSD2_EXIST
			case DISPLAYER_OSD2:
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_x   = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD2][DISPBUF_X];
				p_lyr_param->SEL.GET_BUFXY.ui_buf_ofs_y   = g_disp1_info.disp_data.pui_osd_buf_xy[DISP_OSD2][DISPBUF_Y];
				break;
#endif
			default:
				break;
			}

		}
		break;
/*
	case DISPLAYER_OP_GET_DMALEN: {
			IDE_DMA_BURST_LEN uidma_y = 0;
			IDE_DMA_BURST_LEN uidma_c = 0;

			displyr1_debug(("disp_lyr: GET_DMALEN (%d)\r\n", layer));
			switch (layer) {
			case DISPLAYER_VDO1:
				idec_get_v1_burst_len(IDE_ID_1, (IDE_DMA_BURST_LEN *)&uidma_y, (IDE_DMA_BURST_LEN *)&uidma_c);
				break;
			case DISPLAYER_VDO2:
				idec_get_v2_burst_len(IDE_ID_1, (IDE_DMA_BURST_LEN *)&uidma_y, (IDE_DMA_BURST_LEN *)&uidma_c);
				break;
			case DISPLAYER_OSD1:
				idec_get_o1_burst_len(IDE_ID_1, (IDE_DMA_BURST_LEN *)&uidma_y, (IDE_DMA_BURST_LEN *)&uidma_c);
				break;
			case DISPLAYER_OSD2:
				idec_getO2BurstLen(IDE_ID_1, (IDE_DMA_BURST_LEN *)&uidma_y, (IDE_DMA_BURST_LEN *)&uidma_c);
				break;

			default:
				break;
			}
			p_lyr_param->SEL.GET_DMALEN.DMAY_A = uidma_y;
			p_lyr_param->SEL.GET_DMALEN.DMAC_RGB = uidma_c;
		}
		break;
*/
	case DISPLAYER_OP_GET_FDEN: {
			BOOL b_en = 0;
			UINT32 ui_all;

			displyr1_debug(("disp_lyr: GET_FDEN (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_FD:
				ui_all = idec_get_fd_all_en(IDE_ID_1);
				if (ui_all & p_lyr_param->SEL.GET_FDEN.fd_num) {
					b_en = TRUE;
				}
				break;

			default:
				break;
			}
			p_lyr_param->SEL.GET_FDEN.b_en = b_en;
		}
		break;

	case DISPLAYER_OP_GET_FDSIZE: {
			UINT32 ui_x, ui_y, ui_w, ui_h, ui_bord_w, ui_bord_h;

			ui_x = ui_y = ui_w = ui_h = ui_bord_w = ui_bord_h = 0;

			displyr1_debug(("disp_lyr: GET_FDSIZE (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_FD:
				idec_get_fd_win_pos(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.GET_FDCOLOR.fd_num, &ui_x, &ui_y);
				idec_get_fd_win_dim(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.GET_FDCOLOR.fd_num, &ui_w, &ui_h);
				idec_get_fd_win_bord(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.GET_FDCOLOR.fd_num, &ui_bord_w, &ui_bord_h);
				p_lyr_param->SEL.GET_FDSIZE.ui_fdx = ui_x;
				p_lyr_param->SEL.GET_FDSIZE.ui_fdy = ui_y;
				p_lyr_param->SEL.GET_FDSIZE.ui_fdw = ui_w;
				p_lyr_param->SEL.GET_FDSIZE.ui_fdh = ui_h;
				p_lyr_param->SEL.GET_FDSIZE.ui_fd_bord_w = ui_bord_w;
				p_lyr_param->SEL.GET_FDSIZE.ui_fd_bord_h = ui_bord_h;
				break;

			default:
				break;
			}
		}
		break;

	case DISPLAYER_OP_GET_FDCOLOR: {
			UINT8 ui_y, ui_cb, ui_cr;

			ui_y = ui_cb = ui_cr = 0;
			displyr1_debug(("disp_lyr: GET_FDCOLOR (%d)\r\n", (int)layer));
			switch (layer) {
			case DISPLAYER_FD:
				idec_get_fd_color(IDE_ID_1, (IDE_FD_NUM)p_lyr_param->SEL.GET_FDCOLOR.fd_num, &ui_y, &ui_cb, &ui_cr);
				p_lyr_param->SEL.GET_FDCOLOR.ui_fd_cr_y = ui_y;
				p_lyr_param->SEL.GET_FDCOLOR.ui_fd_cr_cb = ui_cb;
				p_lyr_param->SEL.GET_FDCOLOR.ui_fd_cr_cr = ui_cr;
				break;

			default:
				break;
			}

		}
		break;

	case DISPLAYER_OP_GET_FDLINESWAP: {
			dispdev1_debug(("disp_lyr: DISPLAYER_OP_GET_LINESWAP\r\n"));
			p_lyr_param->SEL.GET_FDLINESWAP.b_en_fd_line_swap = idec_get_fd_line_layer_swap(IDE_ID_1);
		}
		break;

	case DISPLAYER_OP_GET_CST_FROM_RGB_TO_YUV: {
		INT32   r_offs;
		INT32   g_offs;
		INT32   b_offs;
		UINT8   ide_cst0_coef0, ide_cst0_coef1, ide_cst0_coef2, ide_cst0_coef3;
		INT32   d1, d2, d3, d0;
		INT32   p0, p1, p2, p3;
		INT32   y_tmp, u_tmp, v_tmp, value_u, value_v;

		UINT8   Y;
		UINT8   U;
		UINT8   V;

		DBG_IND("[DISPLAYER_OP_GET_CST_FROM_RGB_TO_YUV]In R[%d], G[%d], B[%d]\r\n", (int)p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.r_to_y, (int)p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.g_to_u, (int)p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.b_to_v);
		idec_get_fcst_coef(IDE_ID_1, &ide_cst0_coef0, &ide_cst0_coef1, &ide_cst0_coef2, &ide_cst0_coef3);
		DBG_IND("[DISPLAYER_OP_GET_CST_FROM_RGB_TO_YUV]cof0=%d, cof1=%d, cof2=%d, cof3=%d\r\n", (int)ide_cst0_coef0, (int)ide_cst0_coef1, (int)ide_cst0_coef2, (int)ide_cst0_coef3);

		//                  d1 = R-G;
		//                  d2 = B-G
		r_offs = p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.r_to_y;
		g_offs = p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.g_to_u;
		b_offs = p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.b_to_v;
		d1 = r_offs - g_offs;
		d2 = b_offs - g_offs;
		d3 = -d1;
		d0 = -d2;

		DBG_IND("d0=%d, d1=%d, d2=%d, d3=%d\r\n", (int)d0, (int)d1, (int)d2, (int)d3);

		p0 = d0 * ide_cst0_coef0;
		p1 = d1 * ide_cst0_coef1;
		p2 = d2 * ide_cst0_coef2;
		p3 = d3 * ide_cst0_coef3;

		DBG_IND("p0=%d, p1=%d, p2=%d, p3=%d\r\n", (int)p0, (int)p1, (int)p2, (int)p3);

		y_tmp = p1 + p2 + (g_offs << 8) + 128;
		//y_tmp = __builtin_round((float)((float)y_tmp/(float)256));
		y_tmp = (y_tmp >> 8);
		//u_tmp = __builtin_floor((float)((float)d2 + (float)((float)p3/(float)128)));
		u_tmp = d2 + (p3 >> 7);
		value_u = int_clamp((abs(u_tmp)+1)>>1, 0, (u_tmp > 0) ? 127 : 128);
		//v_tmp = __builtin_floor((float)((float)d1 + (float)((float)p0/(float)128)));
		v_tmp = d1 + (p0 >> 7);
		value_v = int_clamp((abs(v_tmp)+1)>>1, 0, (v_tmp > 0) ? 127 : 128);

		DBG_IND("y_tmp = %d, u_tmp = %d, v_tmp = %d\r\n", (int)y_tmp, (int)u_tmp, (int)v_tmp);

		Y = int_clamp(y_tmp, 0, 255);
		/*
		if(value_u < 0) {
			u_tmp = 128 + __builtin_floor((float)((float)u_tmp / (float)2));
		} else {
			u_tmp = 128 + __builtin_round((float)((float)u_tmp / (float)2));
		}

		if(value_v < 0) {
			v_tmp = 128 + __builtin_floor((float)((float)v_tmp / (float)2));
		} else {
			v_tmp = 128 + __builtin_round((float)((float)v_tmp / (float)2));
		}
		*/
		u_tmp = ((u_tmp > 0)?1 : -1) * value_u;
		v_tmp = ((v_tmp > 0)?1 : -1) * value_v;

		U = int_clamp(u_tmp + 128, 0, 255);
		V = int_clamp(v_tmp + 128, 0, 255);

		DBG_IND("  calc Y=%d, U=%d, V=%d\r\n", (int)Y, (int)U, (int)V);
		*(UINT8 *)&p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.r_to_y = Y;
		*(UINT8 *)&p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.g_to_u = U;
		*(UINT8 *)&p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.b_to_v = V;
		DBG_IND("output Y=%d, U=%d, V=%d\r\n", (int)p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.r_to_y, (int)p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.g_to_u, (int)p_lyr_param->SEL.GET_CST_OF_RGB_TO_YUV.b_to_v);
		}
		break;

	default: {
			DBG_WRN("disp_lyr No Support!(0x%08x)\r\n", (int)lyr_op);
			return E_NOSPT;
		}
	}

	return E_OK;
}

/*
    Display device control

    This API is used as the display device control API.
    Such as display device (panel/TV/HDMI) open/close, device register access, ...etc.

    @param[in] dev_ctrl      Display device control command
    @param[in] p_dev_param    Display device control command parameters. Refer to display.h.

    @return
     - @b E_NOEXS:  Display object has not opened.
     - @b E_NOSPT:  Device Control Command no support.
     - @b E_OK:     Device Control Command done.
*/
ER disp_set_disp1_device_ctrl(DISPDEV_OP dev_ctrl, PDISPDEV_PARAM p_dev_param)
{
	UINT32      temp;

	if (!gui_disp_obj_opened[DISP_1]) {
		return E_NOEXS;
	}

	switch (dev_ctrl) {
	///////////////////////
	/* SET control group */
	///////////////////////
	case DISPDEV_OPEN_DEVICE: {
			dispdev1_debug(("DISPDEV: open dev (%d)\r\n", (int)p_dev_param->SEL.OPEN_DEVICE.dev_id));
			if (g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL) {
				DBG_WRN("Previous dev1 has not closed!\r\n");
			}

			temp = p_dev_param->SEL.OPEN_DEVICE.dev_id;
			if (temp) {
				// Because NTSC/PAL share the same device object
				temp--;
			}

			if ((gp_disp_dev[DISP_1][temp]->open != NULL) && (gp_disp_dev[DISP_1][temp] != NULL)) {
				if ((gp_disp_dev[DISP_1][temp]->specific_ctrl.DISPDEVUSERCTRL != NULL)) {
					if (p_dev_param->SEL.OPEN_DEVICE.user_data_en) {
						gp_disp_dev[DISP_1][temp]->specific_ctrl.DISPDEVUSERCTRL(p_dev_param->SEL.OPEN_DEVICE.user_data);
					}
				}

				if (gp_disp_dev[DISP_1][temp]->open() != E_OK) {
					DBG_ERR("Not Support Act1 = %d\r\n", (int)g_disp1_info.disp_data.active_dev);
					g_disp1_info.disp_data.active_dev = DISPDEV_ID_NULL;
					return E_SYS;

				}
				g_disp1_info.disp_data.active_dev = p_dev_param->SEL.OPEN_DEVICE.dev_id;
/*
#if _FPGA_EMULATION_
#if DEV_RGB_SERIAL_FORCE_DIV_ZERO
				else {
					if (p_dev_param->SEL.OPEN_DEVICE.dev_id == DISPDEV_ID_PANEL) {
						if ((ide_get_device() != DISPLAY_DEVICE_MIPIDSI) && (ide_get_device() != DISPLAY_DEVICE_MI)) {
							UINT32 ui_reg;

							ui_reg = *(UINT32 *)0xF0800000;

							if ((ui_reg & 0x400000) == 0x400000) {
								DBG_WRN("FPGA & DEVICE = panel => format = RGB 8bit but RGBD => return \r\n");
								break;
							} else if ((ui_reg & 0x4000000) == 0x4000000) {
								DBG_WRN("FPGA & DEVICE = panel => format = RGB 8bit but RGBThrougth => return \r\n");
								break;
							}
							ui_reg = (ui_reg >> 7) & 0xF;
							if (ui_reg == 0x1) { //RGB serial, patch some FPGA version can not config div != 0
								pll_setClockRate(PLL_CLKSEL_IDE_CLKDIV, PLL_IDE_CLKDIV(0));
								pll_setClockRate(PLL_CLKSEL_IDE_OUTIF_CLKDIV, PLL_IDE_OUTIF_CLKDIV(0));
								DBG_WRN("FPGA & DEVICE = panel => format = RGB 8bit Force clock div = 0 \r\n");
							}
							DBG_WRN("FPGA & DEVICE = panel => Force clock div = 0 0x%08x\r\n", *(UINT32 *)0xF002006C);
						}
					}
				}
#endif
#endif
*/
			} else {
				DBG_ERR("Non of DISP1 DEV (%d)\r\n", (int)p_dev_param->SEL.OPEN_DEVICE.dev_id);
				return E_NOEXS;
			}
		}
		break;

	case DISPDEV_CLOSE_DEVICE: {
			dispdev1_debug(("DISPDEV: close dev (%d)\r\n", (int)g_disp1_info.disp_data.active_dev));
			if (g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL) {
				temp = g_disp1_info.disp_data.active_dev;
				if (temp) {
					// Because NTSC/PAL share the same device object
					temp--;
				}

				if ((gp_disp_dev[DISP_1][temp]->close != NULL) && (gp_disp_dev[DISP_1][temp] != NULL)) {
					if (gp_disp_dev[DISP_1][temp]->close() == E_OK) {
						g_disp1_info.disp_data.active_dev = DISPDEV_ID_NULL;
					}
				}
			}
		}
		break;

	case DISPDEV_HOOK_DEVICE_OBJECT: {
			dispdev1_debug(("DISPDEV: HOOK_DEVICE_OBJECT\r\n"));

			if ((p_dev_param->SEL.HOOK_DEVICE_OBJECT.dev_id >= DISPDEV_ID_MAX) || (p_dev_param->SEL.HOOK_DEVICE_OBJECT.dev_id == g_disp1_info.disp_data.active_dev)) {
				DBG_ERR("Cant chg obj of act dev!\r\n");
				return E_SYS;
			}

			if (p_dev_param->SEL.HOOK_DEVICE_OBJECT.dev_id) {
				gp_disp_dev[DISP_1][p_dev_param->SEL.HOOK_DEVICE_OBJECT.dev_id - 1] = p_dev_param->SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj;
			} else {
				gp_disp_dev[DISP_1][p_dev_param->SEL.HOOK_DEVICE_OBJECT.dev_id] = p_dev_param->SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj;
			}

			p_dev_param->SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj->set_dev_io_ctrl((FP)dispdev1_ioctrl);
		}
		break;

	case DISPDEV_HOOK_PANEL_ADJUST: {
			dispdev1_debug(("DISPDEV: HOOK_PANEL_ADJUST\r\n"));
			g_disp1_info.disp_dev_data.panel_adjust = p_dev_param->SEL.HOOK_PANEL_ADJUST.fp_adjust;
		}
		break;

	case DISPDEV_SET_TVADJUST: {
			dispdev1_debug(("DISPDEV: SET_TVADJUST\r\n"));
			g_disp1_info.disp_dev_data.tv_adjust = p_dev_param->SEL.SET_TVADJUST.tv_adjust;
		}
		break;

	case DISPDEV_SET_REG_IF: {
			dispdev1_debug(("DISPDEV: SET_REG_IF\r\n"));

			g_disp1_info.disp_dev_data.lcd_ctrl      = p_dev_param->SEL.SET_REG_IF.lcd_ctrl;
			g_disp1_info.disp_dev_data.sif_ch        = p_dev_param->SEL.SET_REG_IF.ui_sif_ch;
			g_disp1_info.disp_dev_data.ui_gpio_sif_sen = p_dev_param->SEL.SET_REG_IF.ui_gpio_sen;
			g_disp1_info.disp_dev_data.ui_gpio_sif_clk = p_dev_param->SEL.SET_REG_IF.ui_gpio_clk;
			g_disp1_info.disp_dev_data.ui_gpio_sif_data = p_dev_param->SEL.SET_REG_IF.ui_gpio_data;
		}
		break;

	case DISPDEV_REG_WRITE: {
			dispdev1_debug(("DISPDEV: REG_WRITE\r\n"));

			if (g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL) {
				temp = g_disp1_info.disp_data.active_dev;
				if (temp) {
					// Because NTSC/PAL share the same device object
					temp--;
				}
				if ((gp_disp_dev[DISP_1][temp]->reg_write != NULL) && (gp_disp_dev[DISP_1][temp] != NULL)) {
					gp_disp_dev[DISP_1][temp]->reg_write(p_dev_param->SEL.REG_WRITE.ui_addr, p_dev_param->SEL.REG_WRITE.ui_value);
				} else {
					DBG_WRN("Non of DISP1 REG WRITE (%d)\r\n", (int)g_disp1_info.disp_data.active_dev);
					return E_NOEXS;
				}
			}
		}
		break;

	case DISPDEV_SET_HDMIMODE: {
			dispdev1_debug(("DISPDEV: SET_HDMIMODE\r\n"));

			if (p_dev_param->SEL.SET_HDMIMODE.audio_id != HDMI_AUDIO_NO_CHANGE) {
				g_disp1_info.disp_dev_data.hdmi_aud_fmt= p_dev_param->SEL.SET_HDMIMODE.audio_id;
			}

			if (p_dev_param->SEL.SET_HDMIMODE.video_id != HDMI_VID_NO_CHANGE) {
				g_disp1_info.disp_dev_data.hdmi_vdo_fmt = p_dev_param->SEL.SET_HDMIMODE.video_id;
			}

		}

		break;

	case DISPDEV_SET_LCDMODE: {
			DBG_IND("DISPDEV: SET_LCDMODE[%d]\r\n", (int)p_dev_param->SEL.SET_LCDMODE.mode);
			dispdev1_debug(("DISPDEV: SET_LCDMODE\r\n"));

			if (p_dev_param->SEL.SET_LCDMODE.mode != DISPDEV_LCDMODE_NO_CHANGE) {
				//coverity[mixed_enums]=>checked fully mapped
				//pinmux_setDispMode(PINMUX_FUNC_ID_LCD, (p_dev_param->SEL.SET_LCDMODE.mode));
			}
		}
		break;

	case DISPDEV_SET_ROTATE: {
			dispdev1_debug(("DISPDEV: SET_ROTATE\r\n"));

			if (g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL) {
				temp = g_disp1_info.disp_data.active_dev;
				if (temp) {
					// Because NTSC/PAL share the same device object
					temp--;
				}
				if ((gp_disp_dev[DISP_1][temp]->rotate != NULL) && (gp_disp_dev[DISP_1][temp] == NULL)) {
					DBG_ERR("Non of DISP1 Rotate (%d)\r\n", (int)g_disp1_info.disp_data.active_dev);
					return E_NOEXS;
				}
				return gp_disp_dev[DISP_1][temp]->rotate(p_dev_param->SEL.SET_ROTATE.rot);
			}
		}
		break;

	case DISPDEV_SET_PANEL_BACKLIGHT: {
			dispdev1_debug(("DISPDEV: SET_PANEL_BACKLIGHT\r\n"));

			if ((gp_disp_dev[DISP_1][DISPDEV_ID_PANEL - 1]->specific_ctrl.DISPDEVBLCTRL != NULL) && (gp_disp_dev[DISP_1][DISPDEV_ID_PANEL - 1] != NULL)) {
				ER  ret;

				ret = gp_disp_dev[DISP_1][DISPDEV_ID_PANEL - 1]->specific_ctrl.DISPDEVBLCTRL(p_dev_param->SEL.SET_PANEL_BACKLIGHT.ui_bl_lvl);
				if (ret == E_OK) {
					g_disp1_info.disp_dev_data.ui_backlight = p_dev_param->SEL.SET_PANEL_BACKLIGHT.ui_bl_lvl;
				} else {
					DBG_WRN("Err BL param\r\n");
					return E_PAR;
				}
			} else {
				DBG_WRN("No BL CTL\r\n");
				return E_NOEXS;
			}
		}
		break;

	case DISPDEV_SET_PANEL_POWER: {
			dispdev1_debug(("DISPDEV: SET_PANEL_POWER\r\n"));

			if ((gp_disp_dev[DISP_1][DISPDEV_ID_PANEL - 1]->specific_ctrl.DISPDEVPWRCTRL != NULL) && (gp_disp_dev[DISP_1][DISPDEV_ID_PANEL - 1] != NULL)) {
				ER  ret;

				ret = gp_disp_dev[DISP_1][DISPDEV_ID_PANEL - 1]->specific_ctrl.DISPDEVPWRCTRL(p_dev_param->SEL.SET_PANEL_POWER.ui_pwr_lvl);
				if (ret == E_OK) {
					g_disp1_info.disp_dev_data.ui_power = p_dev_param->SEL.SET_PANEL_POWER.ui_pwr_lvl;
				} else {
					DBG_WRN("Err PWR param\r\n");
					return E_PAR;
				}
			} else {
				DBG_WRN("No PWR CTL\r\n");
				return E_NOEXS;
			}
		}
		break;

	case DISPDEV_SET_POWERDOWN: {
			dispdev1_debug(("DISPDEV: SET_POWERDOWN\r\n"));

			if ((g_disp1_info.disp_data.active_dev == DISPDEV_ID_TVNTSC) || (g_disp1_info.disp_data.active_dev == DISPDEV_ID_TVPAL)) {
				//ide_set_tv_power_down(p_dev_param->SEL.SET_POWERDOWN.b_power_down);
			} else if (g_disp1_info.disp_data.active_dev == DISPDEV_ID_TVHDMI) {
				//hdmitx_set_config(HDMI_CONFIG_ID_AV_MUTE, p_dev_param->SEL.SET_POWERDOWN.b_power_down);
			} else {
				DBG_WRN("Device not support Device Power down\r\n");
			}
		}
		break;

	case DISPDEV_SET_TVPAR: {
			dispdev1_debug(("DISPDEV: SET_TVPAR\r\n"));

			g_disp1_info.disp_dev_data.b_tv_en_user              = p_dev_param->SEL.SET_TVPAR.b_en_user;
			g_disp1_info.disp_dev_data.tv_par_ntsc.ui_bll       = p_dev_param->SEL.SET_TVPAR.ui_ntsc_bll;
			g_disp1_info.disp_dev_data.tv_par_ntsc.ui_brl       = p_dev_param->SEL.SET_TVPAR.ui_ntsc_brl;
			g_disp1_info.disp_dev_data.tv_par_ntsc.ui_setup     = p_dev_param->SEL.SET_TVPAR.ui_ntsc_setup;
			g_disp1_info.disp_dev_data.tv_par_ntsc.ui_y_scaling  = p_dev_param->SEL.SET_TVPAR.ui_ntsc_y_scaling;
			g_disp1_info.disp_dev_data.tv_par_ntsc.ui_cb_scaling = p_dev_param->SEL.SET_TVPAR.ui_ntsc_cb_scaling;
			g_disp1_info.disp_dev_data.tv_par_ntsc.ui_cr_scaling = p_dev_param->SEL.SET_TVPAR.ui_ntsc_cr_scaling;
			g_disp1_info.disp_dev_data.tv_par_pal.ui_bll        = p_dev_param->SEL.SET_TVPAR.ui_pal_bll;
			g_disp1_info.disp_dev_data.tv_par_pal.ui_brl        = p_dev_param->SEL.SET_TVPAR.ui_pal_brl;
			g_disp1_info.disp_dev_data.tv_par_pal.ui_setup      = p_dev_param->SEL.SET_TVPAR.ui_pal_setup;
			g_disp1_info.disp_dev_data.tv_par_pal.ui_y_scaling   = p_dev_param->SEL.SET_TVPAR.ui_pal_y_scaling;
			g_disp1_info.disp_dev_data.tv_par_pal.ui_cb_scaling  = p_dev_param->SEL.SET_TVPAR.ui_pal_cb_scaling;
			g_disp1_info.disp_dev_data.tv_par_pal.ui_cr_scaling  = p_dev_param->SEL.SET_TVPAR.ui_pal_cr_scaling;
		}
		break;


	case DISPDEV_SET_OUTPUT_DRAM: {
			DBG_IND("DISPDEV: DISPDEV_SET_OUTPUT_DRAM\r\n");

			if (idec_get_v2_en(IDE_ID_1)) {
				DBG_ERR("Output dram function & VDO2 can not active at the same time\r\n");
			} else {
				DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
				IDE_DEVICE_TYPE         device;
				BOOL                    icst;

				device = ide_get_device();
				icst = idec_get_icst(IDE_ID_1);

				DBG_IND("DISPDEV: DISPDEV_SET_OUTPUT_DRAM Y[0x%08x] UV[0x%08x] width[0x%08x] height[0x%08x] lineofs[0x%08x]icst[%d]\r\n", (unsigned int)p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.y_addr, (unsigned int)p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.uv_addr, (unsigned int)p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.output_width, (unsigned int)p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.output_height, (unsigned int)p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.output_line_ofs >> 2, (int)icst);
				ide_set_v2_buf0_addr(p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.y_addr, p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.uv_addr, p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.uv_addr);
				ide_set_v2_buf_dim(p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.output_width, p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.output_height, p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.output_line_ofs >> 2);

				if (icst) {
					idec_set_icst(IDE_ID_1, FALSE);
				}

				dev_io_ctrl.SEL.SET_DEVICE.disp_dev_type = DISPDEV_TYPE_OUTPUT_DRAM;
				dispdev1_ioctrl(DISPDEV_IOCTRL_SET_DEVICE, &dev_io_ctrl);
				idec_set_dram_out_format(IDE_ID_1, p_dev_param->SEL.SET_OUTPUT_DRAM_PAR.is_yuv422);
#if 0
				{
					UINT32 ui_reg;

					ui_reg = *(UINT32 *)0xF0800000;
					*(UINT32 *)0xF0800000 = ui_reg | 0x80000000;
					DBG_IND("0x%08x IDE0 = [0x%08x]\r\n", *(UINT32 *)0xF0800000);
				}
#endif
				ide_wait_frame_end(TRUE);
				disp_set_disp1_load(TRUE);
				if (icst) {
					idec_set_icst(IDE_ID_1, TRUE);
				}
				ide_set_device(device);

			}
		}
		break;

	case DISPDEV_SET_TVFULL: {
			dispdev1_debug(("DISPDEV: SET_TVFULL\r\n"));

			g_disp1_info.disp_dev_data.tv_full = p_dev_param->SEL.SET_TVFULL.b_en_full;
		}
		break;


	///////////////////////
	/* GET control group */
	///////////////////////

	case DISPDEV_GET_ACT_DEVICE: {
			dispdev1_debug(("DISPDEV: GET_ACT_DEVICE\r\n"));
			p_dev_param->SEL.GET_ACT_DEVICE.dev_id = g_disp1_info.disp_data.active_dev;
		}
		break;

	case DISPDEV_GET_DISPSIZE: {
			dispdev1_debug(("DISPDEV: GET_DISPSIZE\r\n"));
			p_dev_param->SEL.GET_DISPSIZE.ui_buf_width  = g_disp1_info.disp_dev_data.ui_buf_width;
			p_dev_param->SEL.GET_DISPSIZE.ui_buf_height = g_disp1_info.disp_dev_data.ui_buf_height;
			p_dev_param->SEL.GET_DISPSIZE.ui_win_width  = g_disp1_info.disp_dev_data.ui_win_width;
			p_dev_param->SEL.GET_DISPSIZE.ui_win_height = g_disp1_info.disp_dev_data.ui_win_height;
		}
		break;

	case DISPDEV_GET_PREDISPSIZE: {
			DISPDEV_GET_PRESIZE presize;

			dispdev1_debug(("DISPDEV: DISPDEV_GET_PREDISPSIZE (%d)\r\n", (int)p_dev_param->SEL.OPEN_DEVICE.dev_id));
			//if(g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL)
			//{
			//    DBG_WRN("Previous dev1 has not closed!\r\n");
			//}

			presize.ui_buf_width = 0;
			presize.ui_buf_height = 0;
			presize.ui_win_width = 0;
			presize.ui_win_height = 0;
			p_dev_param->SEL.GET_PREDISPSIZE.ui_buf_width = presize.ui_buf_width;
			p_dev_param->SEL.GET_PREDISPSIZE.ui_buf_height = presize.ui_buf_height;
			p_dev_param->SEL.GET_PREDISPSIZE.ui_win_width = presize.ui_win_width;
			p_dev_param->SEL.GET_PREDISPSIZE.ui_win_height = presize.ui_win_height;

			temp = p_dev_param->SEL.GET_PREDISPSIZE.dev_id;
			presize.ui_dev = temp;
			if (temp) {
				// Because NTSC/PAL share the same device object
				temp--;
			}

			if ((gp_disp_dev[DISP_1][temp]->get_pre_size != NULL) && (gp_disp_dev[DISP_1][temp] != NULL)) {
				//g_disp1_info.disp_data.active_dev= p_dev_param->SEL.OPEN_DEVICE.dev_id;
				//debug_msg("Cur Act1 = %d\r\n", g_disp1_info.disp_data.active_dev);
				if (gp_disp_dev[DISP_1][temp]->get_pre_size(&presize) != E_OK) {
					//g_disp1_info.disp_data.active_dev = DISPDEV_ID_NULL;
					return E_SYS;
				}
				p_dev_param->SEL.GET_PREDISPSIZE.ui_buf_width = presize.ui_buf_width;
				p_dev_param->SEL.GET_PREDISPSIZE.ui_buf_height = presize.ui_buf_height;
				p_dev_param->SEL.GET_PREDISPSIZE.ui_win_width = presize.ui_win_width;
				p_dev_param->SEL.GET_PREDISPSIZE.ui_win_height = presize.ui_win_height;
			} else {
				DBG_ERR("Non of DISP1 DEV (%d)\r\n", (int)p_dev_param->SEL.GET_PREDISPSIZE.dev_id);
				return E_NOEXS;
			}
		}
		break;


	case DISPDEV_GET_PANEL_ADJUST: {
			dispdev1_debug(("DISPDEV: GET_PANEL_ADJUST\r\n"));
			p_dev_param->SEL.GET_PANEL_ADJUST.pfp_adjust = g_disp1_info.disp_dev_data.panel_adjust;
		}
		break;

	case DISPDEV_GET_TVADJUST: {
			dispdev1_debug(("DISPDEV: GET_TVADJUST\r\n"));
			p_dev_param->SEL.GET_TVADJUST.tv_adjust    = g_disp1_info.disp_dev_data.tv_adjust;
		}
		break;

	case DISPDEV_GET_REG_IF: {
			dispdev1_debug(("DISPDEV: GET_REG_IF\r\n"));

			p_dev_param->SEL.GET_REG_IF.lcd_ctrl     = g_disp1_info.disp_dev_data.lcd_ctrl;
			//p_dev_param->SEL.GET_REG_IF.ui_sif_ch     = g_disp1_info.disp_dev_data.sif_ch;
			p_dev_param->SEL.GET_REG_IF.ui_gpio_sen   = g_disp1_info.disp_dev_data.ui_gpio_sif_sen;
			p_dev_param->SEL.GET_REG_IF.ui_gpio_clk   = g_disp1_info.disp_dev_data.ui_gpio_sif_clk;
			p_dev_param->SEL.GET_REG_IF.ui_gpio_data  = g_disp1_info.disp_dev_data.ui_gpio_sif_data;
		}
		break;

	case DISPDEV_REG_READ: {
			dispdev1_debug(("DISPDEV: REG_READ\r\n"));

			if (g_disp1_info.disp_data.active_dev != DISPDEV_ID_NULL) {
				temp = g_disp1_info.disp_data.active_dev;
				if (temp) {
					// Because NTSC/PAL share the same device object
					temp--;
				}

				if ((gp_disp_dev[DISP_1][temp]->reg_read != NULL) && (gp_disp_dev[DISP_1][temp] != NULL)) {
					p_dev_param->SEL.REG_READ.ui_return = gp_disp_dev[DISP_1][temp]->reg_read(p_dev_param->SEL.REG_READ.ui_addr);
				} else {
					DBG_ERR("Non of DISP1 REG READ (%d)\r\n", (int)g_disp1_info.disp_data.active_dev);
					return E_NOEXS;
				}
			}
		}
		break;

	case DISPDEV_GET_HDMIMODE: {
			dispdev1_debug(("DISPDEV: GET_HDMIMODE\r\n"));

			p_dev_param->SEL.GET_HDMIMODE.audio_id = g_disp1_info.disp_dev_data.hdmi_aud_fmt;
			p_dev_param->SEL.GET_HDMIMODE.video_id = g_disp1_info.disp_dev_data.hdmi_vdo_fmt;
		}
		break;

	case DISPDEV_GET_LCDMODE: {
			DBG_IND("DISPDEV: GET_LCDMODE\r\n");

			//coverity[mixed_enums]=>checked fully mapped
			//p_dev_param->SEL.GET_LCDMODE.mode = pinmux_getDispMode(PINMUX_FUNC_ID_LCD);
		}
		break;


	case DISPDEV_GET_PANEL_BACKLIGHT: {
			dispdev1_debug(("DISPDEV: GET_PANEL_BACKLIGHT\r\n"));
			p_dev_param->SEL.GET_PANEL_BACKLIGHT.ui_bl_lvl = g_disp1_info.disp_dev_data.ui_backlight;
		}
		break;

	case DISPDEV_GET_PANEL_POWER: {
			dispdev1_debug(("DISPDEV: GET_PANEL_POWER\r\n"));
			p_dev_param->SEL.GET_PANEL_POWER.ui_pwr_lvl = g_disp1_info.disp_dev_data.ui_power;
		}
		break;

	case DISPDEV_GET_POWERDOWN: {
			dispdev1_debug(("DISPDEV: GET_POWERDOWN\r\n"));
			if ((g_disp1_info.disp_data.active_dev == DISPDEV_ID_TVNTSC) || (g_disp1_info.disp_data.active_dev == DISPDEV_ID_TVPAL)) {
				//p_dev_param->SEL.GET_POWERDOWN.b_power_down = ide_get_tv_power_down();
			} else {
				DBG_WRN("Device not support Device Power down\r\n");
			}
		}
		break;

	case DISPDEV_GET_TVPAR: {
			dispdev1_debug(("DISPDEV: GET_TVPAR\r\n"));

			p_dev_param->SEL.GET_TVPAR.b_en_user         = g_disp1_info.disp_dev_data.b_tv_en_user;
			p_dev_param->SEL.GET_TVPAR.ui_ntsc_bll      = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_bll;
			p_dev_param->SEL.GET_TVPAR.ui_ntsc_brl      = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_brl;
			p_dev_param->SEL.GET_TVPAR.ui_ntsc_setup    = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_setup;
			p_dev_param->SEL.GET_TVPAR.ui_ntsc_y_scaling = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_y_scaling;
			p_dev_param->SEL.GET_TVPAR.ui_ntsc_cb_scaling = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_cb_scaling;
			p_dev_param->SEL.GET_TVPAR.ui_ntsc_cr_scaling = g_disp1_info.disp_dev_data.tv_par_ntsc.ui_cr_scaling;
			p_dev_param->SEL.GET_TVPAR.ui_pal_bll       = g_disp1_info.disp_dev_data.tv_par_pal.ui_bll;
			p_dev_param->SEL.GET_TVPAR.ui_pal_brl       = g_disp1_info.disp_dev_data.tv_par_pal.ui_brl;
			p_dev_param->SEL.GET_TVPAR.ui_pal_setup     = g_disp1_info.disp_dev_data.tv_par_pal.ui_setup;
			p_dev_param->SEL.GET_TVPAR.ui_pal_y_scaling  = g_disp1_info.disp_dev_data.tv_par_pal.ui_y_scaling;
			p_dev_param->SEL.GET_TVPAR.ui_pal_cb_scaling = g_disp1_info.disp_dev_data.tv_par_pal.ui_cb_scaling;
			p_dev_param->SEL.GET_TVPAR.ui_pal_cr_scaling = g_disp1_info.disp_dev_data.tv_par_pal.ui_cr_scaling;
		}
		break;

	default: {
			DBG_WRN("DISPDEV No Support!(%d)\r\n", (int)dev_ctrl);
			return E_NOSPT;
		}

	}

	return E_OK;
}
#endif
