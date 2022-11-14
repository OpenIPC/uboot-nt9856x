#include "./include/ide_platform.h"

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

void idec_dump_info(UINT32 id);

void idec_dump_info(UINT32 id)
{
#if 0
	UINT32           fps = 0;
	UINT32           v1_bw = 0, v2_bw = 0, o1_bw = 0, bw_total = 0;
	IDE_COLOR_FORMAT v1_fmt, v2_fmt, o1_fmt;
	UINT32          out_htotal, out_hsync, out_hstart, out_hend;
	UINT32          out_vtotal, out_vsync, out_vodd_start, out_vodd_end, out_veven_start, out_veven_end;
	UINT32          v1_width, v1_height, v1_loff, v1_win_width, v1_win_height;
	UINT32          v2_width, v2_height, v2_loff, v2_win_width, v2_win_height;
	UINT32          o1_width, o1_height, o1_loff, o1_win_width, o1_win_height;
	UINT32			fd_width, fd_height, fd_hor, fd_ver;
	UINT8			fd_y, fd_u, fd_v;

	UINT32          v1x, v1y, v2x, v2y, o1x, o1y, fdx, fdy;
	//UINT32          clksrc = 0;
	//UINT32          clksrc_freq = 0;
	UINT32			clkin = 0;//, clkin_div = 0;
	UINT32			pixclk = 0;//, clkout_div = 0;
	UINT32          device = 0;
	UINT32          mode = 0;
	UINT32          pinmux = 0;
	UINT32			fd = 0, quad = 0;
	BOOL			fd_en = FALSE, quad_en = FALSE;
	UINT8			line_alpha, line_y, line_u, line_v;
	BOOL			sign_a[4], sign_b[4];
	UINT32			a[4], b[4], slope[4], compare[4];

	static UINT32 v1_fmt_bw[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1.5};
	static UINT32 v2_fmt_bw[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1.5};
	static UINT32 o1_fmt_bw[14] = {0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 4, 2, 2};

	if (id == 0) {
		id = IDE_ID_1;
		pinmux = PINMUX_FUNC_ID_LCD;
	} else if (id == 1) {
		id = IDE_ID_2;
		pinmux = PINMUX_FUNC_ID_LCD2;
	}

	DBG_DUMP("=================================================================\r\n");
	DBG_DUMP("ide %d                                                           \r\n", (int)id + 1);
	DBG_DUMP("=================================================================\r\n");
	//DBG_DUMP("IDE_CLK=%d\r\n", pll_isClockEnabled((id == IDE_ID_1) ? IDE1_CLK : IDE2_CLK));
	DBG_DUMP("IDE_EN=%d\r\n", idec_get_en(id));
	DBG_DUMP("-----------------------------------------------------------------\r\n");

	if (!idec_get_en(id)) {
		return;
	}

	idec_get_hor_timing(id, &out_hsync, &out_htotal, &out_hstart, &out_hend);
	idec_get_ver_timing(id, &out_vsync, &out_vtotal, &out_vodd_start, &out_vodd_end, &out_veven_start, &out_veven_end);

	device = idec_get_device(id);
	switch (device) {
	case DISPLAY_DEVICE_CASIO2G:
	case DISPLAY_DEVICE_AU:
		mode = ide_platform_get_disp_mode(pinmux);
		break;
	case DISPLAY_DEVICE_TOPPOLY: //
	case DISPLAY_DEVICE_CCIR656: //
	case DISPLAY_DEVICE_CCIR601: //
		mode = ide_platform_get_disp_mode(pinmux);
		break;
	case DISPALY_DEVICE_PARALLEL:
	case DISPLAY_DEVICE_CCIR656_16BIT:
	case DISPLAY_DEVICE_CCIR601_16BIT:
	case DISPLAY_DEVICE_RGB_16BIT:
	case DISPLAY_DEVICE_MI:
	case DISPLAY_DEVICE_MIPIDSI:
		mode = ide_platform_get_disp_mode(PINMUX_FUNC_ID_LCD);
		break;

	case DISPLAY_DEVICE_TV:
		mode = 0; //NTSC or PAL
		break;

	case DISPLAY_DEVICE_HDMI_24BIT:
	case DISPLAY_DEVICE_HDMI_16BIT:
		mode = ide_platform_get_disp_mode(PINMUX_FUNC_ID_HDMI) & (PINMUX_HDMIMODE_OFFSET - 1);
		break;
	}

	clkin = ide_platform_get_freq(id);
	pixclk = ide_platform_get_iffreq(id);

	fps = clkin / (out_htotal + 1) / (out_vtotal + 1);
#if 0
	if (id == IDE_ID_1) {
		clkin_div = pll_getClockRate(PLL_CLKSEL_IDE_CLKDIV) >> (PLL_CLKSEL_IDE_CLKDIV - PLL_CLKSEL_R23_OFFSET);
		clkout_div = pll_getClockRate(PLL_CLKSEL_IDE_OUTIF_CLKDIV) >> (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R23_OFFSET);
		switch (pll_getClockRate(PLL_CLKSEL_IDE_CLKSRC)) {
		case PLL_CLKSEL_IDE_CLKSRC_480:
			clksrc = 1;
			clksrc_freq = pll_getPLLFreq(PLL_ID_1) / 1000000;
			break;
		case PLL_CLKSEL_IDE_CLKSRC_PLL2:
			clksrc = 2;
			clksrc_freq = pll_getPLLFreq(PLL_ID_2) / 1000000;
			break;
		case PLL_CLKSEL_IDE_CLKSRC_PLL4:
			clksrc = 4;
			clksrc_freq = pll_getPLLFreq(PLL_ID_4) / 1000000;
			break;
		case PLL_CLKSEL_IDE_CLKSRC_PLL14:
			clksrc = 14;
			clksrc_freq = pll_getPLLFreq(PLL_ID_14) / 1000000;
			break;
		}
		pll_getClockFreq(IDECLK_FREQ, &clkin);
		pll_getClockFreq(IDEOUTIFCLK_FREQ, &pixclk);

		fps = (float)clkin / (float)(out_htotal + 1) / (float)(out_vtotal + 1);
	}
	if (id == IDE_ID_2) {
		clkin_div = pll_getClockRate(PLL_CLKSEL_IDE2_CLKDIV) >> (PLL_CLKSEL_IDE2_CLKDIV - PLL_CLKSEL_R23_OFFSET);
		clkout_div = pll_getClockRate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV) >> (PLL_CLKSEL_IDE2_OUTIF_CLKDIV - PLL_CLKSEL_R23_OFFSET);
		switch (pll_getClockRate(PLL_CLKSEL_IDE2_CLKSRC)) {
		case PLL_CLKSEL_IDE2_CLKSRC_480:
			clksrc = 1;
			clksrc_freq = pll_getPLLFreq(PLL_ID_1) / 1000000;
			break;
		case PLL_CLKSEL_IDE2_CLKSRC_PLL2:
			clksrc = 2;
			clksrc_freq = pll_getPLLFreq(PLL_ID_2) / 1000000;
			break;
		case PLL_CLKSEL_IDE2_CLKSRC_PLL4:
			clksrc = 4;
			clksrc_freq = pll_getPLLFreq(PLL_ID_4) / 1000000;
			break;
		case PLL_CLKSEL_IDE2_CLKSRC_PLL14:
			clksrc = 14;
			clksrc_freq = pll_getPLLFreq(PLL_ID_14) / 1000000;
			break;
		}
		pll_getClockFreq(IDE2CLK_FREQ, &clkin);
		pll_getClockFreq(IDE2OUTIFCLK_FREQ, &pixclk);

		fps = (float)clkin / (float)(out_htotal + 1) / (float)(out_vtotal + 1);
	}
#endif
	DBG_DUMP("OUTPUT DEVICE = %d\r\n", (int)device);
	DBG_DUMP("OUTPUT mode = %d\r\n", (int)mode);
	DBG_DUMP("-----------------------------------------------------------------\r\n");
	//DBG_DUMP("ide%d-src_clk:PLL%d=%d(Mhz)\r\n", id + 1, clksrc, clksrc_freq);
	DBG_DUMP("ide%d-module clk:%d(hz)\r\n", (int)id + 1, (int)clkin);
	//DBG_DUMP("ide%d-ClkDiv=%d\r\n", id + 1, clkin_div);
	//DBG_DUMP("ide%d-PxlDiv=%d\r\n", id + 1, clkout_div);
	DBG_DUMP("-----------------------------------------------------------------\r\n");
	DBG_DUMP("PxlCLK: %d(Hz) \r\n", (int)pixclk);
	DBG_DUMP("-----------------------------------------------------------------\r\n");
	DBG_DUMP("Output Timing: H-Sync=%d  H-Total=%d  H-Start=%d  H-End=%d      \r\n", (int)out_hsync, (int)out_htotal, (int)out_hstart, (int)out_hend);
	DBG_DUMP("Output Timing: V-Sync=%d  V-Total=%d  Vodd-Start=%d  Vodd-End=%d \r\n", (int)out_vsync, (int)out_vtotal, (int)out_vodd_start, (int)out_vodd_end);
	DBG_DUMP("                                      Veven-Start=%d  Veven-End=%d \r\n", (int)out_veven_start, (int)out_veven_end);
	DBG_DUMP("-----------------------------------------------------------------\r\n");
	DBG_DUMP("Frame Rate: %d(fps)                 \r\n", (int)fps);
	DBG_DUMP("-----------------------------------------------------------------\r\n");

	idec_get_v1_fmt(id, &v1_fmt);
	idec_get_v2_fmt(id, &v2_fmt);
	idec_get_o1_fmt(id, &o1_fmt);

	idec_get_v1_buf_dim(id, &v1_width, &v1_height, &v1_loff);
	v1_loff = v1_loff << 2;
	idec_get_v2_buf_dim(id, &v2_width, &v2_height, &v2_loff);
	v2_loff = v2_loff << 2;
	idec_get_o1_buf_dim(id, &o1_width, &o1_height, &o1_loff);
	o1_loff = o1_loff << 2;

	idec_get_v1_win_dim(id, &v1_win_width, &v1_win_height);
	v1_win_width++;
	v1_win_height++;
	idec_get_v2_win_dim(id, &v2_win_width, &v2_win_height);
	v2_win_width++;
	v2_win_height++;
	idec_get_o1_win_dim(id, &o1_win_width, &o1_win_height);
	o1_win_width++;
	o1_win_height++;


	idec_get_v1_win_pos(id, &v1x, &v1y);
	idec_get_v2_win_pos(id, &v2x, &v2y);
	idec_get_o1_win_pos(id, &o1x, &o1y);


	if (idec_get_v1_en(id)) {
		v1_bw = fps * v1_width * v1_height / 1048576 * v1_fmt_bw[v1_fmt];
	}
	if (idec_get_v2_en(id)) {
		v2_bw = fps * v2_width * v2_height / 1048576 * v2_fmt_bw[v2_fmt];
	}
	if (idec_get_o1_en(id)) {
		o1_bw = fps * o1_width * o1_height / 1048576 * o1_fmt_bw[o1_fmt];
	}
	bw_total = v1_bw + v2_bw + o1_bw;

	DBG_DUMP("\r\n");
	DBG_DUMP("========================================================================\r\n");
	DBG_DUMP("==               V1          V2(ide2 not support)          O1         ==\r\n");
	DBG_DUMP("========================================================================\r\n");
	DBG_DUMP("Format           %4d                 %4d                   %4d          \r\n", v1_fmt, v2_fmt, o1_fmt);
	DBG_DUMP("------------------------------------------------------------------------\r\n");
	DBG_DUMP("Enable           %4d                 %4d                   %4d          \r\n", idec_get_v1_en(id), idec_get_v2_en(id), idec_get_o1_en(id));
	DBG_DUMP("------------------------------------------------------------------------\r\n");
	DBG_DUMP("Buf Width(pix)   %4d                 %4d                   %4d          \r\n", (int)v1_width, (int)v2_width, (int)o1_width);
	DBG_DUMP("Buf Height       %4d                 %4d                   %4d          \r\n", (int)v1_height, (int)v2_height, (int)o1_height);
	DBG_DUMP("Buf Loff         %4d                 %4d                   %4d          \r\n", (int)v1_loff, (int)v2_loff, (int)o1_loff);
	DBG_DUMP("------------------------------------------------------------------------\r\n");
	DBG_DUMP("Win Width        %4d                 %4d                   %4d          \r\n", (int)v1_win_width, (int)v2_win_width, (int)o1_win_width);
	DBG_DUMP("Win Height       %4d                 %4d                   %4d          \r\n", (int)v1_win_height, (int)v2_win_height, (int)o1_win_height);
	DBG_DUMP("Win (x,y)     (%4d,%4d)           (%4d,%4d)             (%4d,%4d)       \r\n", (int)v1x, (int)v1y, (int)v2x, (int)v2y, (int)o1x, (int)o1y);
	DBG_DUMP("------------------------------------------------------------------------\r\n");
	DBG_DUMP("BandWidth        %4d                 %4d                   %4d          \r\n", (int)v1_bw, (int)v2_bw, (int)o1_bw);
	DBG_DUMP("BandWidth Total  %4d (MB/s)\r\n", (int)bw_total);
	DBG_DUMP("------------------------------------------------------------------------\r\n");
	DBG_DUMP("\r\n");

	if (id == IDE_ID_1) {
		//FD
		fd_en = idec_get_fd_all_en(id);
		DBG_DUMP("============");
		for (fd = 0; fd < 32; fd++) {
			if (fd_en & (1 << fd)) {
				DBG_DUMP("|     FD %2d    ", (int)fd + 1);
			}
		}
		DBG_DUMP("|=====\r\n");

		DBG_DUMP("Win (x,y)   ");
		for (fd = 0; fd < 32; fd++) {
			if (fd_en & (1 << fd)) {
				idec_get_fd_win_pos(id, (1 << fd), &fdx, &fdy);
				DBG_DUMP("|   %4d,%4d  ", (int)fdx, (int)fdy);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("Win Dim(w,h)");
		for (fd = 0; fd < 32; fd++) {
			if (fd_en & (1 << fd)) {
				idec_get_fd_win_dim(id, (1 << fd), &fd_width, &fd_height);
				DBG_DUMP("|   %4d,%4d  ", (int)fd_width, (int)fd_height);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("Bor Dim(h,v)");
		for (fd = 0; fd < 32; fd++) {
			if (fd_en & (1 << fd)) {
				idec_get_fd_win_bord(id, (1 << fd), &fd_hor, &fd_ver);
				DBG_DUMP("|   %4d,%4d  ", (int)fd_hor, (int)fd_ver);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("color(y,u,v)");
		for (fd = 0; fd < 32; fd++) {
			if (fd_en & (1 << fd)) {
				idec_get_fd_color(id, (1 << fd), &fd_y, &fd_u, &fd_v);
				DBG_DUMP("|%4d,%4d,%4d", (int)fd_y, (int)fd_u, (int)fd_v);
			}
		}
		DBG_DUMP("|     \r\n");
		DBG_DUMP("\r\n");

		// QUAD
		quad_en = idec_get_line_all_en(id);
		DBG_DUMP("======");
		for (quad = 0; quad < 16; quad++) {
			if (quad_en & (1 << quad)) {
				DBG_DUMP("|    Quadrangle %2d     ", (int)quad + 1);
			}
		}
		DBG_DUMP("|=====\r\n");

		DBG_DUMP("line 0");
		for (quad = 0; quad < 16; quad++) {
			if (quad_en & (1 << quad)) {
				idec_get_line_para(id, 0x1 << quad, 0, &sign_a[0], &sign_b[0], &a[0], &b[0], &slope[0], &compare[0]);
				DBG_DUMP("|%s%4dx%s%4dy%2s%8d", sign_a[0] ? "-" : " ", (int)a[0], sign_b[0] ? "-" : "+", (int)b[0], compare[0] ? (compare[0] == 1 ? "<=" : (compare[0] == 2 ? ">" : "<")) : ">=", (int)slope[0]);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("line 1");
		for (quad = 0; quad < 16; quad++) {
			if (quad_en & (1 << quad)) {
				idec_get_line_para(id, 0x1 << quad, 1, &sign_a[1], &sign_b[1], &a[1], &b[1], &slope[1], &compare[1]);
				DBG_DUMP("|%s%4dx%s%4dy%2s%8d", sign_a[1] ? "-" : " ", (int)a[1], sign_b[0] ? "-" : "+", (int)b[1], compare[1] ? (compare[1] == 1 ? "<=" : (compare[1] == 2 ? ">" : "<")) : ">=", (int)slope[1]);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("line 2");
		for (quad = 0; quad < 16; quad++) {
			if (quad_en & (1 << quad)) {
				idec_get_line_para(id, 0x1 << quad, 2, &sign_a[2], &sign_b[2], &a[2], &b[2], &slope[2], &compare[2]);
				DBG_DUMP("|%s%4dx%s%4dy%2s%8d", sign_a[2] ? "-" : " ", (int)a[2], sign_b[0] ? "-" : "+", (int)b[2], compare[2] ? (compare[2] == 1 ? "<=" : (compare[2] == 2 ? ">" : "<")) : ">=", (int)slope[2]);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("line 3");
		for (quad = 0; quad < 16; quad++) {
			if (quad_en & (1 << quad)) {
				idec_get_line_para(id, 0x1 << quad, 3, &sign_a[3], &sign_b[3], &a[3], &b[3], &slope[3], &compare[3]);
				DBG_DUMP("|%s%4dx%s%4dy%2s%8d", sign_a[3] ? "-" : " ", (int)a[3], sign_b[0] ? "-" : "+", (int)b[3], compare[3] ? (compare[3] == 1 ? "<=" : (compare[3] == 2 ? ">" : "<")) : ">=", (int)slope[3]);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("color ");
		for (quad = 0; quad < 16; quad++) {
			if (quad_en & (1 << quad)) {
				idec_get_line_color(id, 0x1 << quad, &line_alpha, &line_y, &line_u, &line_v);
				DBG_DUMP("|A=%x, Y=%x, U=%x, V=%x ", (unsigned int)line_alpha, (unsigned int)line_y, (unsigned int)line_u, (unsigned int)line_v);
			}
		}
		DBG_DUMP("|     \r\n");

		DBG_DUMP("\r\n");
	}


	DBG_DUMP("Note: Format Code (0): not support  (1): not support  (2): not support \r\n");
	DBG_DUMP("Note: Format Code (3): Palette-8bit (4): not support  (5): not support \r\n");
	DBG_DUMP("Note: Format Code (6): not support  (7): not support  (8): ARGB8565    \r\n");
	DBG_DUMP("Note: Format Code (9): YUV422PACK   (10): YUV420PACK  (11): ARGB8888   \r\n");
	DBG_DUMP("Note: Format Code (12): ARGB4444    (13): ARGB1555                     \r\n");
#endif
}
//#if !(defined __UITRON || defined __ECOS || defined __FREERTOS)
//EXPORT_SYMBOL(idec_dump_info);
//#endif