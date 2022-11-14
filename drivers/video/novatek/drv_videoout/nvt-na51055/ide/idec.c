/*
    Public APIs for ide

    Public APIs for ide.

    @file       idec.c
    @ingroup    mIDrvDisp_IDE
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2009.  All rights reserved.
*/
#include "../include/ide_protected.h"
#include "./include/ide_reg.h"
#include "./include/ide2_int.h"

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
@name ide OS Level Functions
*/
//@{
#define HS_DELAY_CNT    5

// Default comparative for video cover
IDE_LINE_COMPARATIVE g_ide_line_comparative = IDE_LINE_COMPARATIVE_GTLE;

static UINT32   gui_const_win_x[] = {0x0, 0x0};
static UINT32   gui_const_win_y[] = {0x0, 0x0};

static UINT32   v_wait_num[] = {0, 0};


/// ide Frame H sync count
static UINT8            v_ide_opened[] = {FALSE, FALSE};
//static ID             v_ide_lock_status[] = {NO_TASK_LOCKED, NO_TASK_LOCKED};
//static UINT32           v_ide_int_status[] = {0x0, 0x0};

//function prototype
//ID idec_get_lock_status(IDE_ID id);
void ide_isr(void);

//extern void idec_dump_info(UINT32 id);
extern UINT32 idec_dump_status(UINT32 *p_data);
/*
static BOOL cmd_ide_dumpinfo(CHAR *strCmd)
{
    idec_dump_info(0);
    idec_dump_info(1);
    return TRUE;
}

static SXCMD_BEGIN(ide, "ide command")
SXCMD_ITEM("dumpinfo", cmd_ide_dumpinfo, "dump ide info")
SXCMD_END()

static void idec_installCmd(void)
{
    static BOOL bInstall = FALSE;

    if (bInstall == FALSE) {
		SxCmd_AddTable(ide);
		bInstall = TRUE;
    }
}
*/

/*
  Get the lock status of ide.

  This function return the lock status of ide.

  @return NO_TASK_LOCKED  :ide is free, no application is using ide
		TASK_LOCKED     :ide is locked by some application
*/
/*
ID idec_get_lock_status(IDE_ID id)
{
	if (id > IDE_ID_2) {
		return E_NOSPT;
	}

	return v_ide_lock_status[id];
}
*/

//}

/*
    Interrupt Handler

    @return void
*/
#if 0
void ide_isr(void)
{
	UINT32 i = 0;
	UINT32 dead_zone = 0;

	v_ide_int_status[IDE_ID_1] = idec_get_reg((UINT32)IDE_ID_1, IDE_INT_OFS);//ide2_getInterruptStatus();

	v_ide_int_status[IDE_ID_1] &= (idec_get_interrupt_en(IDE_ID_1) << IDE_INTSTS_SFT);

	if (v_ide_int_status[IDE_ID_1] != 0) {
#if defined(_BSP_NA51000_)
		if (v_ide_int_status[IDE_ID_1] & IDE_VS_IRQSTS) {

			// Clear Interrupt
			idec_clear_interrupt_status(IDE_ID_1, IDE_VS_IRQSTS);

			while(((idec_get_reg(IDE_ID_1, IDE_CTRL_OFS) >> 28) & 0x1)){
				i++;
				if (i == 5) {
					dead_zone = 1;
					break;
				}
			}

			// Disable interrupt
			if (dead_zone == 0)
				idec_clr_interrupt_en(IDE_ID_1, IDE_VS_IRQEN);

			if (!ide_platform_list_empty(IDE_ID_1)) {
				ide_platform_set_ist_event(IDE_ID_1);
			} else if ((dead_zone == 0) && (v_wait_num[IDE_ID_1] != 0)) {
				ide_platform_flg_set(IDE_ID_1, (FLGPTN)v_wait_num[IDE_ID_1]);
			}
		}

#else
		if (v_ide_int_status[IDE_ID_1] & IDE_FRAME_END_IRQSTS) {

			// Clear Interrupt
			idec_clear_interrupt_status(IDE_ID_1, IDE_FRAME_END_IRQSTS);

			while(((idec_get_reg(IDE_ID_1, IDE_CTRL_OFS) >> 28) & 0x1)){
				i++;
				if (i == 5) {
					dead_zone = 1;
					break;
				}
			}

			// Disable interrupt
			if (dead_zone == 0)
				idec_clr_interrupt_en(IDE_ID_1, IDE_FRAME_END_IRQEN);

			if (!ide_platform_list_empty(IDE_ID_1)) {
				ide_platform_set_ist_event(IDE_ID_1);
			} else if ((dead_zone == 0) && (v_wait_num[IDE_ID_1] != 0)) {
				ide_platform_flg_set(IDE_ID_1, (FLGPTN)v_wait_num[IDE_ID_1]);
			}
		}
#endif

		else if (v_ide_int_status[IDE_ID_1] & IDE_YUV_OUT_DRAM_END_IRQSTS) {
			DBG_IND("ide YUV output done\r\n");
			// Clear Interrupt
			idec_clear_interrupt_status(IDE_ID_1, IDE_YUV_OUT_DRAM_END_IRQSTS);

			ide_platform_flg_set(IDE_ID_1, FLGPTN_IDE_DMA_DONE);         // signal event flag for interrupt notification
		}
        /*
		else if (g_IDEIntStatus & IDE_HS_IRQSTS)
		{
		     g_IDEHsyncCnt ++;
		    // Clear Interrupt
		    ide_clear_interrupt_status(IDE_HS_IRQSTS);

		    iset_flg(FLG_ID_IDE, FLGPTN_IDE);         // signal event flag for interrupt notification
		}
        */
		else if (v_ide_int_status[IDE_ID_1] & IDE_V1BWF_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_1, IDE_V1BWF_IRQSTS);
			#ifndef CONFIG_NVT_FPGA_EMULATION
			DBG_WRN("ide V1 BW not enough!\r\n");
			#endif
		} else if (v_ide_int_status[IDE_ID_1] & IDE_V2BWF_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_1, IDE_V2BWF_IRQSTS);
			DBG_WRN("ide V2 BW not enough!\r\n");
		} else if (v_ide_int_status[IDE_ID_1] & IDE_OSD1BWF_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_1, IDE_OSD1BWF_IRQSTS);
			DBG_WRN("ide O1 BW not enough!\r\n");
		}
		/*
		else if (v_ide_int_status[IDE_ID_1] & IDE_OSD2BWF_IRQSTS)
		{
		    idec_clear_interrupt_status(IDE_ID_1, IDE_OSD2BWF_IRQSTS);
		    DBG_WRN("ide O2 BW not enough!\r\n");
		}
		*/
		else if (v_ide_int_status[IDE_ID_1] & IDE_V1_LINE_START_ERROR_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_1, IDE_V1_LINE_START_ERROR_IRQSTS);
			DBG_WRN("ide V1 Line start error!\r\n");
		} else if (v_ide_int_status[IDE_ID_1] & IDE_V2_LINE_START_ERROR_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_1, IDE_V2_LINE_START_ERROR_IRQSTS);
			DBG_WRN("ide V2 Line start error!\r\n");
		} else if (v_ide_int_status[IDE_ID_1] & IDE_O1_LINE_START_ERROR_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_1, IDE_O1_LINE_START_ERROR_IRQSTS);
			DBG_WRN("ide O1 Line start error!\r\n");
		}


	}
}
#endif

#if 0
/*
    Interrupt Handler

    @return void
*/
void ide2_isr(void)
{
	v_ide_int_status[IDE_ID_2] = idec_get_reg((UINT32)IDE_ID_2, IDE_INT_OFS);//ide2_getInterruptStatus();

	v_ide_int_status[IDE_ID_2] &= (idec_get_interrupt_en(IDE_ID_2) << IDE_INTSTS_SFT);

	if (v_ide_int_status[IDE_ID_2] != 0) {
		if (v_ide_int_status[IDE_ID_2] & IDE_VS_IRQSTS) {

			// Clear Interrupt
			idec_clear_interrupt_status(IDE_ID_2, IDE_VS_IRQSTS);

			// signal event flag for interrupt notification
			ide_platform_flg_set(IDE_ID_2, (FLGPTN)v_wait_num[IDE_ID_2]);

			// Disable interrupt
			idec_clr_interrupt_en(IDE_ID_2, IDE_VS_IRQEN);
		}
		/*
		else if (g_IDEIntStatus & IDE_HS_IRQSTS)
		{
		     g_IDEHsyncCnt ++;
		    // Clear Interrupt
		    ide_clear_interrupt_status(IDE_HS_IRQSTS);

		    iset_flg(FLG_ID_IDE, FLGPTN_IDE);         // signal event flag for interrupt notification
		}
		*/
		else if (v_ide_int_status[IDE_ID_2] & IDE_V1BWF_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_2, IDE_V1BWF_IRQSTS);
			DBG_WRN("IDE2 V1 BW not enough!\r\n");
		}
		/*
		else if (v_ide_int_status[IDE_ID_2] & IDE_V2BWF_IRQSTS)
		{
		    idec_clear_interrupt_status(IDE_ID_2, IDE_V2BWF_IRQSTS);
		    DBG_WRN("IDE2 V2 BW not enough!\r\n");
		}
		*/
		else if (v_ide_int_status[IDE_ID_2] & IDE_OSD1BWF_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_2, IDE_OSD1BWF_IRQSTS);
			DBG_WRN("IDE2 O1 BW not enough!\r\n");
		}
		/*
		else if (v_ide_int_status[IDE_ID_2] & IDE_OSD2BWF_IRQSTS)
		{
		    idec_clear_interrupt_status(IDE_ID_2, IDE_OSD2BWF_IRQSTS);
		    DBG_WRN("IDE2 O2 BW not enough!\r\n");
		}
		*/
		else if (v_ide_int_status[IDE_ID_2] & IDE_V1_LINE_START_ERROR_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_2, IDE_V1_LINE_START_ERROR_IRQSTS);
			DBG_WRN("IDE2 V1 Line start error!\r\n");
		} else if (v_ide_int_status[IDE_ID_2] & IDE_O1_LINE_START_ERROR_IRQSTS) {
			idec_clear_interrupt_status(IDE_ID_2, IDE_O1_LINE_START_ERROR_IRQSTS);
			DBG_WRN("IDE2 O1 Line start error!\r\n");
		}
	}
}
#endif

ER idec_set_config(IDE_ID id, IDE_CONFIG_ID config_id, UINT32 config_context)
{
	if (id > IDE_ID_2) {
		return E_NOSPT;
	}
	switch (config_id) {
	case IDE_CONFIG_DITHER_EN:
		break;

	case IDE_CONFIG_DITHER_FREERUN:

		break;
	case IDE_CONFIG_DISPDEV:

		break;
	case IDE_CONFIG_PDIR:

		break;
	case IDE_CONFIG_LCDL0:

		break;
	case IDE_CONFIG_LCDL1:

		break;
	case IDE_CONFIG_HSINV:

		break;
	case IDE_CONFIG_VSINV:

		break;
	case IDE_CONFIG_HVLDINV:

		break;
	case IDE_CONFIG_VVLDINV:

		break;
	case IDE_CONFIG_CLKINV:

		break;
	case IDE_CONFIG_FLDINV:

		break;
	case IDE_CONFIG_RGBDSEL:

		break;
	case IDE_CONFIG_DEINV:

		break;
	case IDE_CONFIG_OUTDDR:

		break;
	case IDE_CONFIG_THROUGHSEL:

		break;

	//
	case IDE_CONFIG_IDEEN:

		break;
	case IDE_CONFIG_LAYEREN:

		break;

	//TG
	case IDE_CONFIG_TG_HSYNC:

		break;
	case IDE_CONFIG_TG_HTOTAL:

		break;
	case IDE_CONFIG_TG_HSYNCDLY:

		break;
	case IDE_CONFIG_TG_HVALIDST:

		break;
	case IDE_CONFIG_TG_HVALIDED:

		break;
	case IDE_CONFIG_TG_VSYNC:

		break;
	case IDE_CONFIG_TG_VTOTAL:

		break;
	case IDE_CONFIG_TG_VSYNCDLY:

		break;
	case IDE_CONFIG_TG_FLD0_VVALIDST:

		break;
	case IDE_CONFIG_TG_FLD0_VVALIDED:

		break;
	case IDE_CONFIG_TG_FLD1_VVALIDST:

		break;
	case IDE_CONFIG_TG_FLD1_VVALIDED:

		break;
	case IDE_CONFIG_TG_INTERLACE_EN:

		break;
	case IDE_CONFIG_TG_FLD0_CBLKST:

		break;
	case IDE_CONFIG_TG_FLD0_CBLKED:

		break;
	case IDE_CONFIG_TG_FLD1_CBLKST:

		break;
	case IDE_CONFIG_TG_FLD1_CBLKED:

		break;
	case IDE_CONFIG_TG_CFIDST:

		break;
	case IDE_CONFIG_TG_CFIDED:

		break;

	case IDE_CONFIG_LINE_COMPARATIVE_METHOD:
		switch (config_context) {
		case 0:
			g_ide_line_comparative = IDE_LINE_COMPARATIVE_GTLT;
			break;
		case 1:
			g_ide_line_comparative = IDE_LINE_COMPARATIVE_GTLE;
			break;
		case 2:
			g_ide_line_comparative = IDE_LINE_COMPARATIVE_GELT;
			break;
		case 3:
		default:
			g_ide_line_comparative = IDE_LINE_COMPARATIVE_GELE;
			break;
		}
		break;
	default:
		break;
	}

	return E_OK;
}

ER idec_set_callback(IDE_ID id, KDRV_CALLBACK_FUNC *p_cb_func)
{
	//UINT32 spin_flags;

	if (p_cb_func != NULL) {
		DBG_WRN("not support callback!\r\n");
	/*
	    spin_flags = ide_platform_spin_lock(id);

		if (ide_platform_add_list(id, p_cb_func) != E_OK) {
	        ide_platform_spin_unlock(id, spin_flags);
	        printk("%s: ide %d add list fail\r\n", __func__, id);
	        return E_SYS;
		}

    	ide_platform_spin_unlock(id, spin_flags);
    */
	}

	return E_OK;
}

#if 0
void idec_isr_bottom(IDE_ID id, UINT32 events)
{
	IDE_REQ_LIST_NODE *p_node;
	BOOL is_list_empty;
	UINT32 spin_flags;

	while (1) {
	    p_node = ide_platform_get_head(id);
	    if (p_node == NULL) {
			DBG_ERR("get head fail %d\r\n", (int)id);
			return;
	    }

	   	p_node->callback.callback(&p_node->cb_info, NULL);

		spin_flags = ide_platform_spin_lock(id);
	    ide_platform_del_list(id);
		is_list_empty = ide_platform_list_empty(id);
		ide_platform_spin_unlock(id, spin_flags);

		//printk("%s: %d\r\n", __func__, is_list_empty);

		if (is_list_empty == TRUE) {
			break;
		}
	}
}
#endif

/**
    Open and enable the interrupt(VS) of ide.

    @param[in] id   ide ID
    @return error number.
      - @b E_OK: Success.
      - Others: Error occur
*/
ER idec_open(IDE_ID id)
{
//	ER er_return;

	//idec_installCmd();

	if (id > IDE_ID_1) {
		return E_NOSPT;
	}

	if (v_ide_opened[id]) {
		return E_OK;
	}
#if defined __FREERTOS
	ide_platform_create_resource();
#endif
//	er_return = ide_platform_flg_clear(id, FLGPTN_IDE | FLGPTN_IDE_2 | FLGPTN_IDE_3 | FLGPTN_IDE_4 | FLGPTN_IDE_5);
//	if (er_return != E_OK) {
//		return er_return;
//	}
//	ide_platform_int_enable(id);


	idec_set_interrupt_en(id, IDEDEFAULT_EN_INT);

	v_wait_num[id] = 0;

	// Disable ide's sram shutdown => enable ide sram
	//ide_platform_sram_disable(id);

	idec_set_lb_read_en(id, FALSE);

	// Configure 6 ide channel's burst length as burst 48
	idec_set_v1_burst_len(id, IDE_DMA_BURST_LEN_48, IDE_DMA_BURST_LEN_48);
	if (id == IDE_ID_1)
		idec_set_v2_burst_len(id, IDE_DMA_BURST_LEN_48, IDE_DMA_BURST_LEN_48);
	//                           alpha plane           rgb plane
	//                           Max 64
	idec_set_o1_burst_len(id, IDE_DMA_BURST_LEN_48, IDE_DMA_BURST_LEN_64);
	v_ide_opened[id] = TRUE;

	return E_OK;
}

/**
    Close and disable the interrupt(VS) of ide.

    @param[in] id   ide ID
    @return Always return E_OK
*/
ER idec_close(IDE_ID id)
{
	if (id > IDE_ID_1) {
		return E_NOSPT;
	}

#if defined __FREERTOS
	ide_platform_release_resource();
#endif
//	ide_platform_int_disable(id);
	// Enable ide's sram shutdown => disable ide sram
	//ide_platform_sram_enable(id);

	idec_clr_interrupt_en(id, IDE_INTEN_MSK);

//	if (v_wait_num[id] != 0) {
		// signal event flag for interrupt notification
//		ide_platform_flg_set(id, FLGPTN_IDE | FLGPTN_IDE_2 | FLGPTN_IDE_3 | FLGPTN_IDE_4 | FLGPTN_IDE_5);

//		v_wait_num[id] = 0;
//	}

	v_ide_opened[id] = FALSE;

	return E_OK;
}

/**
    ide open status.

    @param[in] id   ide ID
    @return
		- @b TRUE: ide is opened
		- @b FALSE: ide is closed.
*/
BOOL idec_is_opened(IDE_ID id)
{
	return v_ide_opened[id];
}

//@}

/**
@name ide Public Functions
*/
//@{
/**
    Initialize the starting address of Video1 or Video2.

    @param[in] id   ide ID
    @param[in] video_id the specific Video number.
		- IDE_VIDEOID_1
		- IDE_VIDEOID_2
    @param[in] pv_buf_addr the starting information for the specific Video number.
    @note ui_buf_sel in VIDEO_BUF_ADDR structure must be the OR operation of IDE_VIDEO_ADDR_SEL_BUFFER0, IDE_VIDEO_ADDR_SEL_BUFFER1 and IDE_VIDEO_ADDR_SEL_BUFFER2.
		If ui_buf_sel is 0, Set all buffer0, buffer1 and buffer2 address(for backward compatible)

    @return TRUE or FALSE.
*/
BOOL idec_set_video_buf_addr(IDE_ID id, IDE_VIDEOID video_id, VIDEO_BUF_ADDR *pv_buf_addr)
{
	if ((video_id != IDE_VIDEOID_1) && (video_id != IDE_VIDEOID_2)) {
		return FALSE;
	}

	if (pv_buf_addr->ui_buf_sel > (IDE_VIDEO_ADDR_SEL_BUFFER0 | IDE_VIDEO_ADDR_SEL_BUFFER1 | IDE_VIDEO_ADDR_SEL_BUFFER2)) {
		return FALSE;
	}

	if (video_id == IDE_VIDEOID_1) {
		if (pv_buf_addr->ui_buf_sel == 0) {
			// value 0 for backward compatible
			idec_set_v1_buf0_addr(id, pv_buf_addr->b0_y, pv_buf_addr->b0_cb, pv_buf_addr->b0_cr);
			idec_set_v1_buf1_addr(id, pv_buf_addr->b1_y, pv_buf_addr->b1_cb, pv_buf_addr->b1_cr);
			idec_set_v1_buf2_addr(id, pv_buf_addr->b2_y, pv_buf_addr->b2_cb, pv_buf_addr->b2_cr);
		} else {
			if (pv_buf_addr->ui_buf_sel & IDE_VIDEO_ADDR_SEL_BUFFER0) {
				idec_set_v1_buf0_addr(id, pv_buf_addr->b0_y, pv_buf_addr->b0_cb, pv_buf_addr->b0_cr);
			}
			if (pv_buf_addr->ui_buf_sel & IDE_VIDEO_ADDR_SEL_BUFFER1) {
				idec_set_v1_buf1_addr(id, pv_buf_addr->b1_y, pv_buf_addr->b1_cb, pv_buf_addr->b1_cr);
			}
			if (pv_buf_addr->ui_buf_sel & IDE_VIDEO_ADDR_SEL_BUFFER2) {
				idec_set_v1_buf2_addr(id, pv_buf_addr->b2_y, pv_buf_addr->b2_cb, pv_buf_addr->b2_cr);
			}
		}
	} else {
		if (pv_buf_addr->ui_buf_sel == 0) {
			// value 0 for backward compatible
			idec_set_v2_buf0_addr(id, pv_buf_addr->b0_y, pv_buf_addr->b0_cb, pv_buf_addr->b0_cr);
			idec_set_v2_buf1_addr(id, pv_buf_addr->b1_y, pv_buf_addr->b1_cb, pv_buf_addr->b1_cr);
			idec_set_v2_buf2_addr(id, pv_buf_addr->b2_y, pv_buf_addr->b2_cb, pv_buf_addr->b2_cr);
		} else {
			if (pv_buf_addr->ui_buf_sel & IDE_VIDEO_ADDR_SEL_BUFFER0) {
				idec_set_v2_buf0_addr(id, pv_buf_addr->b0_y, pv_buf_addr->b0_cb, pv_buf_addr->b0_cr);
			}
			if (pv_buf_addr->ui_buf_sel & IDE_VIDEO_ADDR_SEL_BUFFER1) {
				idec_set_v2_buf1_addr(id, pv_buf_addr->b1_y, pv_buf_addr->b1_cb, pv_buf_addr->b1_cr);
			}
			if (pv_buf_addr->ui_buf_sel & IDE_VIDEO_ADDR_SEL_BUFFER2) {
				idec_set_v2_buf2_addr(id, pv_buf_addr->b2_y, pv_buf_addr->b2_cb, pv_buf_addr->b2_cr);
			}
		}
	}
	return TRUE;
}

/**
    Get the starting address of Video1 or Video2.

    @param[in] id   ide ID
    @param[in] video_id the specific Video number.
		IDE_VIDEOID_1
    @param[out] pv_buf_addr the starting information for the specific Video number.
    @note ui_buf_sel in VIDEO_BUF_ADDR structure is dummy return for this function.

    @return TRUE or FALSE.
*/
BOOL idec_get_video_buf_addr(IDE_ID id, IDE_VIDEOID video_id, VIDEO_BUF_ADDR *pv_buf_addr)
{
	if ((video_id != IDE_VIDEOID_1) && (video_id != IDE_VIDEOID_2)) {
		return FALSE;
	}

	if (video_id == IDE_VIDEOID_1) {
		idec_get_v1_buf0_addr(id, &pv_buf_addr->b0_y, &pv_buf_addr->b0_cb, &pv_buf_addr->b0_cr);
		idec_get_v1_buf1_addr(id, &pv_buf_addr->b1_y, &pv_buf_addr->b1_cb, &pv_buf_addr->b1_cr);
		idec_get_v1_buf2_addr(id, &pv_buf_addr->b2_y, &pv_buf_addr->b2_cb, &pv_buf_addr->b2_cr);
	} else {
		idec_get_v2_buf0_addr(id, &pv_buf_addr->b0_y, &pv_buf_addr->b0_cb, &pv_buf_addr->b0_cr);
		idec_get_v2_buf1_addr(id, &pv_buf_addr->b1_y, &pv_buf_addr->b1_cb, &pv_buf_addr->b1_cr);
		idec_get_v2_buf2_addr(id, &pv_buf_addr->b2_y, &pv_buf_addr->b2_cb, &pv_buf_addr->b2_cr);
	}
	return TRUE;
}

/**
    Initialize the buffer attribute of Video1 or Video2.

    @param[in] id   ide ID
    @param[in] video_id the specific Video number.
		IDE_VIDEOID_1
		IDE_VIDEOID_2
    @param[in] pv_buf_attr the attribute information of buffer for the specific Video number.

    @return TRUE or FALSE.
*/
BOOL idec_set_video_buf_attr(IDE_ID id, IDE_VIDEOID video_id, VIDEO_BUF_ATTR *pv_buf_attr)
{
	if ((video_id != IDE_VIDEOID_1) && (video_id != IDE_VIDEOID_2)) {
		return FALSE;
	}

	if (video_id == IDE_VIDEOID_1) {
		idec_set_v1_buf_dim(id, pv_buf_attr->buf_w, pv_buf_attr->buf_h, pv_buf_attr->buf_lineoffset);
		idec_set_v1_read_order(id, pv_buf_attr->buf_l2r, pv_buf_attr->buf_t2b);
		idec_set_v1_buf_op(id, pv_buf_attr->buf_bjmode, pv_buf_attr->buf_opt, pv_buf_attr->buf_num);
	} else {
		idec_set_v2_buf_dim(id, pv_buf_attr->buf_w, pv_buf_attr->buf_h, pv_buf_attr->buf_lineoffset);
		idec_set_v2_read_order(id, pv_buf_attr->buf_l2r, pv_buf_attr->buf_t2b);
		idec_set_v2_buf_op(id, pv_buf_attr->buf_bjmode, pv_buf_attr->buf_opt, pv_buf_attr->buf_num);
	}
	return TRUE;
}

/**
    Get the buffer attribute of Video1 or Video2.

    @param[in] id   ide ID
    @param[in] video_id the specific Video number.
		- @b IDE_VIDEOID_1
		- @b IDE_VIDEOID_2
    @param[out] pv_buf_attr the attribute information of buffer for the specific Video number.

    @return TRUE or FALSE.
*/
BOOL idec_get_video_buf_attr(IDE_ID id, IDE_VIDEOID video_id, VIDEO_BUF_ATTR *pv_buf_attr)
{
	IDE_HOR_READ b_l2r;
	IDE_VER_READ b_t2b;
	IDE_BJMODE ui_bjmode;
	IDE_OP_BUF ui_optbuf;
	IDE_BUF_NUM ui_buf_num;

	if ((video_id != IDE_VIDEOID_1) && (video_id != IDE_VIDEOID_2)) {
		return FALSE;
	}

	if (video_id == IDE_VIDEOID_1) {
		idec_get_v1_buf_dim(id, &pv_buf_attr->buf_w, &pv_buf_attr->buf_h, &pv_buf_attr->buf_lineoffset);
		idec_get_v1_read_order(id, &b_l2r, &b_t2b);
		idec_get_v1_buf_op(id, &ui_bjmode, &ui_optbuf, &ui_buf_num);
	} else {
		idec_get_v2_buf_dim(id, &pv_buf_attr->buf_w, &pv_buf_attr->buf_h, &pv_buf_attr->buf_lineoffset);
		idec_get_v2_read_order(id, &b_l2r, &b_t2b);
		idec_get_v2_buf_op(id, &ui_bjmode, &ui_optbuf, &ui_buf_num);
	}

	pv_buf_attr->buf_l2r = b_l2r;
	pv_buf_attr->buf_t2b = b_t2b;
	pv_buf_attr->buf_bjmode = ui_bjmode;
	pv_buf_attr->buf_opt = ui_optbuf;
	pv_buf_attr->buf_num = ui_buf_num;

	return TRUE;
}

/**
    Initialize the window attribute of Video1 or Video2.

    @param[in] id   ide ID
    @param[in] video_id      The specific Video number.
		- @b IDE_VIDEOID_1
		- @b IDE_VIDEOID_2
    @param[in] pv_win_attr  The attribute information of window for the specific Video number.
    @param[in] b_load
		- @b TRUE:    Set ide Load at end of this API.
		- @b FALSE:   Do not set ide Load at end of this API.

    @return
		- @b TRUE:    Config success
		- @b FALSE:   Parameter error.
*/
BOOL idec_set_video_win_attr_ex(IDE_ID id, IDE_VIDEOID video_id, VOSD_WINDOW_ATTR *pv_win_attr, BOOL b_load)
{
	UINT32  factor_w, factor_h;
	BOOL    subsample = 0;
	BOOL    vsubsample = 0;
	UINT32  factor_w_dec, factor_h_dec;
	UINT32  w_scale_key, h_scale_key;
	UINT32  des_w, win_x;



	if ((video_id != IDE_VIDEOID_1) && (video_id != IDE_VIDEOID_2)) {
		return FALSE;
	}

	if (idec_get_device(id) == DISPLAY_DEVICE_TV) {
		des_w = pv_win_attr->des_w * 2;
		win_x = (pv_win_attr->win_x + gui_const_win_x[id]) * 2;
	} else {
		des_w = pv_win_attr->des_w;
		win_x = pv_win_attr->win_x + gui_const_win_x[id];
	}

	if (pv_win_attr->source_w == (des_w * 2)) {
		factor_w = 2;
		factor_w = factor_w << IDE_VIDEO_HSF_TOTAL_BITS;
		//debug_err(("Scale Down 2 times! Use Subsample \r\n"));
	} else {
		factor_w = factor_caculate((UINT16) pv_win_attr->source_w, (UINT16) des_w, TRUE);
	}

	if (pv_win_attr->source_h == (pv_win_attr->des_h * 2)) {
		factor_h = 2;
		factor_h = factor_h << IDE_VIDEO_VSF_TOTAL_BITS;
	} else {
		factor_h = factor_caculate((UINT16) pv_win_attr->source_h, (UINT16) pv_win_attr->des_h, FALSE);
	}

	//horizontal
	if (factor_w >= (1 << IDE_VIDEO_HSF_TOTAL_BITS)) {
		//horizontal scaling down
		if (factor_w > (2 << IDE_VIDEO_HSF_TOTAL_BITS)) {
			DBG_ERR("Horizontal Scaling down ratio over 2!  \r\n");
			return FALSE;
		}
		if (factor_w == (2 << IDE_VIDEO_HSF_TOTAL_BITS)) {
			subsample = 1;
			factor_w = 0;
		}

		if (factor_w >= (1 << IDE_VIDEO_HSF_TOTAL_BITS)) {
			factor_w -= (1 << IDE_VIDEO_HSF_TOTAL_BITS);
		}

		w_scale_key = 0;
	} else {
		//horizontal scaling up
		subsample = 0;
		w_scale_key = 1;
	}
	factor_w_dec = factor_w;

	//vertical
	if (factor_h >= (1 << IDE_VIDEO_VSF_TOTAL_BITS)) {
		//vertical scaling down
		if (factor_h > (2 << IDE_VIDEO_VSF_TOTAL_BITS)) {
			DBG_ERR("Vertical Scaling down ratio over 2!  \r\n");
			return FALSE;
		}
		if (factor_h == (2 << IDE_VIDEO_VSF_TOTAL_BITS)) {
			vsubsample = 1;
			factor_h = 0;
		}

		if (factor_h >= (1 << IDE_VIDEO_VSF_TOTAL_BITS)) {
			factor_h -= (1 << IDE_VIDEO_VSF_TOTAL_BITS);
		}

		h_scale_key = 0;
	} else {
		h_scale_key = 1;
	}
	factor_h_dec = factor_h;

	if (video_id == IDE_VIDEOID_1) {
		if (idec_get_rgbd(id) == 1) {
//			idec_set_v1_win_dim(id, (des_w * 4 - 1), (pv_win_attr->des_h - 1));
//			idec_set_v1_win_pos(id, win_x * 4, pv_win_attr->win_y + gui_const_win_y[id]);
			idec_set_v1_win_dim(id, (des_w - 1), (pv_win_attr->des_h - 1));
			idec_set_v1_win_pos(id, win_x, pv_win_attr->win_y + gui_const_win_y[id]);
		} else if (idec_get_through(id) == 1) {
//			idec_set_v1_win_dim(id, (des_w * 3 - 1), (pv_win_attr->des_h - 1));
//			idec_set_v1_win_pos(id, win_x * 3, pv_win_attr->win_y + gui_const_win_y[id]);
			idec_set_v1_win_dim(id, (des_w - 1), (pv_win_attr->des_h - 1));
			idec_set_v1_win_pos(id, win_x, pv_win_attr->win_y + gui_const_win_y[id]);
		} else {
			if (idec_get_interlace(id) == 1) {
				idec_set_v1_win_dim(id, (des_w - 1), (pv_win_attr->des_h / 2 - 1));
			} else {
				idec_set_v1_win_dim(id, (des_w - 1), (pv_win_attr->des_h - 1));
			}
			idec_set_v1_win_pos(id, win_x, pv_win_attr->win_y + gui_const_win_y[id]);
		}

		idec_set_v1_fmt(id, pv_win_attr->win_format);

#if IDE_CMLCOMPARE
		if (factor_w_dec >= 1) {
			factor_w_dec = factor_w_dec - 1;
		}
#endif
		idec_set_v1_scale_factor(id, factor_w_dec, subsample, factor_h_dec, vsubsample);

		if (idec_get_interlace(id) == 1) {
			if (factor_h_dec == 0) {
				if (vsubsample == 1) {
					idec_set_v1_vsf_init(id, 0, (0x1000));
				} else {
					idec_set_v1_vsf_init(id, 0, (0x800));
				}
			} else {
				if (h_scale_key == 0) {
					idec_set_v1_vsf_init(id, 0, (factor_h_dec) | (0x800));
					// for 96650 setting, maybe next chip will be modify
				} else {
					idec_set_v1_vsf_init(id, 0, (factor_h_dec));
				}
			}
		} else {
			idec_set_v1_vsf_init(id, 0, 0);
		}

		idec_set_v1_scale_ctrl(id, w_scale_key, h_scale_key);

	} else {
		if (idec_get_rgbd(id) == 1) {
			//idec_set_v2_win_dim(id, (des_w * 4 - 1), (pv_win_attr->des_h - 1));
			//idec_set_v2_win_pos(id, win_x * 4, pv_win_attr->win_y + gui_const_win_y[id]);
			idec_set_v2_win_dim(id, (des_w - 1), (pv_win_attr->des_h - 1));
			idec_set_v2_win_pos(id, win_x, pv_win_attr->win_y + gui_const_win_y[id]);
		} else if (idec_get_through(id) == 1) {
			//idec_set_v2_win_dim(id, (des_w * 3 - 1), (pv_win_attr->des_h - 1));
			//idec_set_v2_win_pos(id, win_x * 3, pv_win_attr->win_y + gui_const_win_y[id]);
			idec_set_v2_win_dim(id, (des_w - 1), (pv_win_attr->des_h - 1));
			idec_set_v2_win_pos(id, win_x, pv_win_attr->win_y + gui_const_win_y[id]);
		} else {
			if (idec_get_interlace(id) == 1) {
				idec_set_v2_win_dim(id, (des_w - 1), (pv_win_attr->des_h / 2 - 1));
			} else {
				idec_set_v2_win_dim(id, (des_w - 1), (pv_win_attr->des_h - 1));
			}
			idec_set_v2_win_pos(id, win_x, pv_win_attr->win_y + gui_const_win_y[id]);
		}

		idec_set_v2_fmt(id, pv_win_attr->win_format);
		// Set V2 H-Scaling method
		if ((pv_win_attr->win_format == COLOR_ARGB8565) || (pv_win_attr->win_format == COLOR_ARGB4565)) {
			idec_set_v2_hsm(id, FALSE);
		} else {
			idec_set_v2_hsm(id, TRUE);
		}

#if IDE_CMLCOMPARE
		if (factor_w_dec >= 1) {
			factor_w_dec = factor_w_dec - 1;
		}
#endif
		idec_set_v2_scale_factor(id, factor_w_dec, subsample, factor_h_dec, vsubsample);

		if (idec_get_interlace(id) == 1) {
			if (factor_h_dec == 0) {
				if (vsubsample == 1) {
					idec_set_v2_vsf_init(id, 0, (0x1000));
				} else {
					idec_set_v2_vsf_init(id, 0, (0x800));
				}
			} else {
				if (h_scale_key == 0) {
					idec_set_v2_vsf_init(id, 0, (factor_h_dec) | (0x800));
					// for 96650 setting, maybe next chip will be modify
				} else {
					idec_set_v2_vsf_init(id, 0, (factor_h_dec));
				}
			}
		} else {
			idec_set_v2_vsf_init(id, 0, 0);
		}

		idec_set_v2_scale_ctrl(id, w_scale_key, h_scale_key);

	}

	if (b_load) {
		idec_set_load(id);
	}

	return TRUE;
}

/**
    Initialize the window attribute of Video1 or Video2.

    @param[in] id   ide ID
    @param[in] video_id the specific Video number.
		- @b IDE_VIDEOID_1
		- @b IDE_VIDEOID_2
    @param[in] pv_win_attr the attribute information of window for the specific Video number.

    @return TRUE or FALSE.
*/
BOOL idec_set_video_win_attr(IDE_ID id, IDE_VIDEOID video_id, VOSD_WINDOW_ATTR *pv_win_attr)
{
	return idec_set_video_win_attr_ex(id, video_id, pv_win_attr, TRUE);
}

/**
    Set ide VIDEO Vertical scaling factor init for interlace mode

    @param[in] id   ide ID
    @param[in] video_id  Which video to select.
		- @b IDE_VIDEOID_1: Video 1
		- @b IDE_VIDEOID_2: Video 2
    @param[in] ui_init0 init value of field0
    @param[in] ui_init1 init value of field1

    @return void
*/
void idec_set_video_vsf_init(IDE_ID id, IDE_VIDEOID video_id, UINT32 ui_init0, UINT32 ui_init1)
{
	if (video_id == IDE_VIDEOID_1) {
		idec_set_v1_vsf_init(id, ui_init0, ui_init1);
	} else {
		idec_set_v2_vsf_init(id, ui_init0, ui_init1);
	}
}

/**
    Initialize the starting address of OSD1 or OSD2.

    @param[in] id   ide ID
    @param[in] osd_id the specific OSD number.
		- IDE_OSDID_1
		- IDE_OSDID_2
    @param[in] ui_osd_addr the starting information for the specific OSD number.

    @return TRUE or FALSE.
*/
BOOL idec_set_osd_buf_addr(IDE_ID id, IDE_OSDID osd_id, UINT32 ui_osd_addr)
{
	if ((osd_id != IDE_OSDID_1)/* && (osd_id != IDE_OSDID_2)*/) {
		return FALSE;
	}

//  if (osd_id == IDE_OSDID_1)
//  {
	idec_set_o1_buf_addr(id, ui_osd_addr);
//  }
//  else
//  {
//    idec_set_o2_buf_addr(id, ui_osd_addr);
//  }
	return TRUE;
}

/**
    Get the starting address of OSD1 or OSD2.

    @param[in] id   ide ID
    @param[in] osd_id the specific OSD number
		- IDE_OSDID_1
		- IDE_OSDID_2
    @param[out] pui_osd_addr the starting information for the specific OSD number.

    @return TRUE or FALSE.
*/
BOOL idec_get_osd_buf_addr(IDE_ID id, IDE_OSDID osd_id, UINT32 *pui_osd_addr)
{
	if ((osd_id != IDE_OSDID_1)/* && (osd_id != IDE_OSDID_2)*/) {
		return FALSE;
	}

	//if (osd_id == IDE_OSDID_1)
	//{
	idec_get_o1_buf_addr(id, pui_osd_addr);
	//}
	//else
	//{
	//    idec_get_o2_buf_addr(id, pui_osd_addr);
	//}
	return TRUE;
}

/**
    Initialize the buffer attribute of OSD1 or OSD2.

    @param[in] id   ide ID
    @param[in] osd_id the specific OSD number.
		- IDE_OSDID_1
		- IDE_OSDID_2
    @param[in] p_osd_buf_attr the attribute information of buffer for the specific OSD number.

    @return TRUE or FALSE.
*/
BOOL idec_set_osd_buf_attr(IDE_ID id, IDE_OSDID osd_id, OSD_BUF_ATTR *p_osd_buf_attr)
{
	if ((osd_id != IDE_OSDID_1)/* && (osd_id != IDE_OSDID_2)*/) {
		return FALSE;
	}

	//if (osd_id == IDE_OSDID_1)
	//{
	idec_set_o1_buf_dim(id, p_osd_buf_attr->buf_w, p_osd_buf_attr->buf_h, p_osd_buf_attr->buf_lineoffset);
	idec_set_o1_read_order(id, p_osd_buf_attr->buf_l2r, p_osd_buf_attr->buf_t2b);
	//}
	//else
	//{
	//    idec_set_o2_buf_dim(id, p_osd_buf_attr->buf_w, p_osd_buf_attr->buf_h, p_osd_buf_attr->buf_lineoffset);
	//    idec_set_o2_read_order(id, p_osd_buf_attr->buf_l2r, p_osd_buf_attr->buf_t2b);
	//}
	return TRUE;
}

/**
    Get the buffer attribute of OSD1 or OSD2.

    @param[in] id   ide ID
    @param[in] osd_id the specific OSD number.
		- IDE_OSDID_1
		- IDE_OSDID_1
    @param[out] p_osd_buf_attr the attribute information of buffer for the specific OSD number.

    @return TRUE or FALSE.
*/
BOOL idec_get_osd_buf_attr(IDE_ID id, IDE_OSDID osd_id, OSD_BUF_ATTR *p_osd_buf_attr)
{
	IDE_HOR_READ b_l2r;
	IDE_VER_READ b_t2b;

	if ((osd_id != IDE_OSDID_1)/* && (osd_id != IDE_OSDID_2)*/) {
		return FALSE;
	}

	//if (osd_id == IDE_OSDID_1)
	//{
	idec_get_o1_buf_dim(id, &p_osd_buf_attr->buf_w, &p_osd_buf_attr->buf_h, &p_osd_buf_attr->buf_lineoffset);
	idec_get_o1_read_order(id, &b_l2r, &b_t2b);
	//}
	//else
	//{
	//    idec_get_o2_buf_dim(id, &p_osd_buf_attr->buf_w, &p_osd_buf_attr->buf_h, &p_osd_buf_attr->buf_lineoffset);
	//    idec_get_o2_read_order(id, &b_l2r, &b_t2b);
	//}

	p_osd_buf_attr->buf_l2r = b_l2r;
	p_osd_buf_attr->buf_t2b = b_t2b;

	return TRUE;
}

/**
    Initialize the OSD attribute of OSD1 or OSD2.

    @param[in] id   ide ID
    @param[in] osd_id    The specific OSD number.
		- IDE_OSDID_1
		- IDE_OSDID_1
    @param[in] p_osd_win_attr the attribute information of window for the specific OSD number.
    @param[in] b_load
		- @b TRUE:    Set ide Load at end of this API.
		- @b FALSE:   Do not set ide Load at end of this API.

    @return
		- @b TRUE:    Config success
		- @b FALSE:   Parameter error.
*/
BOOL idec_set_osd_win_attr_ex(IDE_ID id, IDE_OSDID osd_id, VOSD_WINDOW_ATTR *p_osd_win_attr, BOOL b_load)
{
	UINT32  factor_w, factor_h;
	BOOL    subsample = 0;
	BOOL    vsubsample = 0;
	UINT32  factor_w_dec, factor_h_dec;
	UINT32  w_scale_key, h_scale_key;
	UINT32  des_w, win_x;
	UINT32  des_h;
	//#NT# Special case for O-project panel optimize.
	UINT8   incre_one = 0x0;


	if ((osd_id != IDE_OSDID_1) && (osd_id != IDE_OSDID_2)) {
		return FALSE;
	}

	if (idec_get_device(id) == DISPLAY_DEVICE_TV) {
		des_w = p_osd_win_attr->des_w * 2;
		win_x = (p_osd_win_attr->win_x + gui_const_win_x[id]) * 2;
	} else {
		des_w = p_osd_win_attr->des_w;
		win_x = (p_osd_win_attr->win_x + gui_const_win_x[id]);
	}
	des_h = p_osd_win_attr->des_h;

	//#NT# Special case for O-project panel optimize.
	if (des_w == (p_osd_win_attr->source_w << 1)) {
		incre_one = 0x01;
	}

	if (p_osd_win_attr->source_w == (des_w * 2)) {
		factor_w = 2 << IDE_OSD_HSF_TOTAL_BITS;
		//debug_err(("Scale Down 2 times! Use Subsample \r\n"));
	} else {
		//factor_w = (float)(p_osd_win_attr->source_w - 1) / (float)(des_w - 1);
		factor_w = factor_caculate((UINT16) p_osd_win_attr->source_w, (UINT16) des_w, TRUE);
	}

	if (p_osd_win_attr->source_h == (des_h * 2)) {
		factor_h = 2 << IDE_OSD_VSF_TOTAL_BITS;
	} else {
		//factor_h = (float)(p_osd_win_attr->source_h - 1) / (float)(p_osd_win_attr->des_h - 1);
		factor_h = factor_caculate((UINT16) p_osd_win_attr->source_h, (UINT16) p_osd_win_attr->des_h, FALSE);
	}

	//horizontal
	if (factor_w >= (1 << IDE_OSD_HSF_TOTAL_BITS)) {
		//horizontal scaling down
		if (factor_w > (2 << IDE_OSD_HSF_TOTAL_BITS)) {
			DBG_ERR("Horizontal Scaling down ratio over 2!\r\n");
			return FALSE;
		}
		if (factor_w == (2 << IDE_OSD_HSF_TOTAL_BITS)) {
			subsample = 1;
			factor_w = 0;
		}

		if (factor_w >= (1 << IDE_OSD_HSF_TOTAL_BITS)) {
			factor_w -= (1 << IDE_OSD_HSF_TOTAL_BITS);
		}

		w_scale_key = 0;
	} else {
		//horizontal scaling up
		subsample = 0;
		w_scale_key = 1;
	}
	factor_w_dec = factor_w;

	//vertical
	if (factor_h >= (1 << IDE_OSD_VSF_TOTAL_BITS)) {
		//vertical scaling down
		if (factor_h > (2 << IDE_OSD_VSF_TOTAL_BITS)) {
			DBG_ERR("Vertical Scaling down ratio over 2!  \r\n");
			return FALSE;
		}
		if (factor_h == (2 << IDE_OSD_VSF_TOTAL_BITS)) {
			vsubsample = 1;
			factor_h = 0;
		}

		if (factor_h >= (1 << IDE_OSD_VSF_TOTAL_BITS)) {
			factor_h -= (1 << IDE_OSD_VSF_TOTAL_BITS);
		}

		h_scale_key = 0;
	} else {
		h_scale_key = 1;
	}
	factor_h_dec = factor_h;

	if (osd_id == IDE_OSDID_1) {
		if (idec_get_rgbd(id) == 1) {
			idec_set_o1_win_dim(id, (des_w * 4 - 1), (p_osd_win_attr->des_h - 1));
			idec_set_o1_win_pos(id, win_x * 4, p_osd_win_attr->win_y + gui_const_win_y[id]);
		} else if (idec_get_through(id) == 1) {
			idec_set_o1_win_dim(id, (des_w * 3 - 1), (p_osd_win_attr->des_h - 1));
			idec_set_o1_win_pos(id, win_x * 3, p_osd_win_attr->win_y + gui_const_win_y[id]);
		} else {
			if (idec_get_interlace(id) == 1) {
				idec_set_o1_win_dim(id, (des_w - 1), (p_osd_win_attr->des_h / 2 - 1));
			} else {
				idec_set_o1_win_dim(id, (des_w - 1), (p_osd_win_attr->des_h - 1));
			}
			idec_set_o1_win_pos(id, win_x, p_osd_win_attr->win_y + gui_const_win_y[id]);
		}

		idec_set_o1_fmt(id, p_osd_win_attr->win_format);

		idec_set_o1_palette_high_addr(id, p_osd_win_attr->high_addr);

#if IDE_CMLCOMPARE
		if (factor_w_dec >= 1) {
			factor_w_dec = factor_w_dec - 1;
		}
#endif

		//#NT# Special case for O-project panel optimize.
		idec_set_o1_scale_factor(id, (factor_w_dec + incre_one), subsample, factor_h_dec, vsubsample);

		if (idec_get_interlace(id) == 1) {
			if (factor_h_dec == 0) {
				if (vsubsample == 1) {
					idec_set_o1_vsf_init(id, 0, (0x1000));
				} else {
					idec_set_o1_vsf_init(id, 0, (0x800));
				}
			} else {
				if (h_scale_key == 0) {
					idec_set_o1_vsf_init(id, 0, (factor_h_dec) | (0x800));    // for 96650 setting, maybe next chip will be modify
				} else {
					idec_set_o1_vsf_init(id, 0, (factor_h_dec));
				}
			}
		} else {
			idec_set_o1_vsf_init(id, 0, 0);
		}

		idec_set_o1_scale_ctrl(id, w_scale_key, h_scale_key);

	}
#if 0
	else {

		if (idec_get_rgbd(id) == 1) {
			idec_set_o2_win_dim(id, (des_w * 4 - 1), (p_osd_win_attr->des_h - 1));
			idec_set_o2_win_pos(id, win_x * 4, p_osd_win_attr->win_y + gui_const_win_y[id]);
		} else if (idec_get_through(id) == 1) {
			idec_set_o2_win_dim(id, (des_w * 3 - 1), (p_osd_win_attr->des_h - 1));
			idec_set_o2_win_pos(id, win_x * 3, p_osd_win_attr->win_y + gui_const_win_y[id]);
		} else {
			if (idec_get_interlace(id) == 1) {
				idec_set_o2_win_dim(id, (des_w - 1), (p_osd_win_attr->des_h / 2 - 1));
			} else {
				idec_set_o2_win_dim(id, (des_w - 1), (p_osd_win_attr->des_h - 1));
			}
			idec_set_o2_win_pos(id, win_x, p_osd_win_attr->win_y + gui_const_win_y[id]);
		}

		idec_set_o2_fmt(id, p_osd_win_attr->win_format);

		idec_set_o2_palette_high_addr(id, p_osd_win_attr->high_addr);
#if IDE_CMLCOMPARE
		if (factor_w_dec >= 1) {
			factor_w_dec = factor_w_dec - 1;
		}
#endif
		//#NT# Special case for O-project panel optimize.
		idec_set_o2_scale_factor(id, (factor_w_dec + incre_one), subsample, factor_h_dec, vsubsample);

		if (idec_get_interlace(id) == 1) {
			if (factor_h_dec == 0) {
				if (vsubsample == 1) {
					idec_set_o2_vsf_init(id, 0, (0x1000));
				} else {
					idec_set_o2_vsf_init(id, 0, (0x800));
				}
			} else {
				if (h_scale_key == 0) {
					idec_set_o2_vsf_init(id, 0, (factor_h_dec) | (0x800));    // for 96650 setting, maybe next chip will be modify
				} else {
					idec_set_o2_vsf_init(id, 0, (factor_h_dec));
				}
			}
		} else {
			idec_set_o2_vsf_init(id, 0, 0);
		}

		idec_set_o2_scale_ctrl(id, w_scale_key, h_scale_key);

	}
#endif
	if (b_load) {
		idec_set_load(id);
	}

	return TRUE;
}

/**
    Initialize the OSD attribute of OSD1 or OSD2.

    @param[in] id   ide ID
    @param[in] osd_id the specific OSD number.
		- IDE_OSDID_1
		- IDE_OSDID_1
    @param[in] p_osd_win_attr the attribute information of window for the specific OSD number.

    @return TRUE or FALSE.
*/
BOOL idec_set_osd_win_attr(IDE_ID id, IDE_OSDID osd_id, VOSD_WINDOW_ATTR *p_osd_win_attr)
{
	return idec_set_osd_win_attr_ex(id, osd_id, p_osd_win_attr, TRUE);
}

/**
    Set ide OSD Vertical scaling factor init for interlace mode

    @param[in] id   ide ID
    @param[in] osd_id    Which video to select.
		- IDE_OSD_1: OSD 1
		- IDE_OSD_2: OSD 2
    @param[in] ui_init0  Init value of field0
    @param[in] ui_init1  Init value of field1

    @return void
*/
void idec_set_osd_vsf_init(IDE_ID id, IDE_OSDID osd_id, UINT32 ui_init0, UINT32 ui_init1)
{
	if ((osd_id != IDE_OSDID_1)) {
		return;
	}

	idec_set_o1_vsf_init(id, ui_init0, ui_init1);
}

/**
    Set the color, blending and blinking operation of the specific entry of OSD palette.

    @param[in] id   ide ID
    @param[in] p_palette_entry the color elements, blending and blinking operation of the specific entry of OSD palette.

    @return None.
*/
void idec_set_palette_entry(IDE_ID id, PALETTE_ENTRY *p_palette_entry)
{
	idec_set_pal_entry(id, p_palette_entry->entry, p_palette_entry->osd_color.color_y, p_palette_entry->osd_color.color_cb, p_palette_entry->osd_color.color_cr, p_palette_entry->blend_op);
}

/**
    Get the color, blending and blinking operation of the specific entry of OSD palette.

    @param[in] id   ide ID
    @param[out] p_palette_entry the color elements, blending and blinking operation of the specific entry of OSD palette.

    @return void
*/
void idec_get_palette_entry(IDE_ID id, PALETTE_ENTRY *p_palette_entry)
{
	idec_get_pal_entry(id, p_palette_entry->entry, &p_palette_entry->osd_color.color_y, &p_palette_entry->osd_color.color_cb, &p_palette_entry->osd_color.color_cr, &(p_palette_entry->blend_op));
}

/**
    Set a group of palette entries

    Set the color and blending operation of the specific entries of OSD palette.
    The palette entries is set from the start id to the end id.
    The ID range 0~255 is for palette-0 and 256~511 is for palette-1.


    @param[in] id   ide ID
    @param[in]  ui_start         Start entry id. Valid range from 0~511.
    @param[in]  ui_number        Total number of palette entry to be set. Valid range from 1~512.
    @param[in]  p_palette_entry  The queue of palette settings.
		- p_palette_entry[7~0]:      Cr value of the palette.
		- p_palette_entry[15~8]:     Cb value of the palette.
		- p_palette_entry[23~16]:    Y value of the palette.
		- p_palette_entry[31~24]:    Alpha value of the palette.

    @return void

*/
void idec_set_palette_group(IDE_ID id, UINT32 ui_start, UINT32 ui_number, UINT32 *p_palette_entry)
{
	UINT32 i = 0;
	UINT32 current_pal;

	//if((ui_start+ui_number) > IDE_PAL_NUM)
	//{
	//    DBG_WRN("Palette entry exceed the total palette size! ui_start=%d  ui_number=%d\r\n",ui_start,ui_number);
	//}


	if (ui_start < IDE_PAL_NUM) {
		UINT32 end;

		end = ((ui_start + ui_number) > IDE_PAL_NUM) ? IDE_PAL_NUM : (ui_start + ui_number);

		idec_set_pal0_rw(id, 1);

		for (current_pal = ui_start; current_pal < end; current_pal++) {
			idec_set_pal(id, current_pal, p_palette_entry[i++]);
		}

		idec_set_pal0_rw(id, 0);

	}

	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		UINT32 start;

		start = (ui_start >= IDE_PAL_NUM) ? ui_start : IDE_PAL_NUM;

		idec_set_pal0_rw(id, 1);

		for (current_pal = start; current_pal < (ui_start + ui_number); current_pal++) {
			idec_set_pal(id, current_pal, p_palette_entry[i++]);
		}

		idec_set_pal0_rw(id, 0);
	}



}

/**
    Set a group of palette entries

    Set the color and blending operation of the specific entries of OSD palette.
    The palette entries is set from the start id to the end id.
    The ID range 0~255 is for palette-0 and 256~511 is for palette-1.


    @param[in] id   ide ID
    @param[in]  ui_start         Start entry id. Valid range from 0~511.
    @param[in]  ui_number        Total number of palette entry to be set. Valid range from 1~512.
    @param[in]  p_palette_entry  The queue of palette settings.
		- p_palette_entry[7~0]:      Y value of the palette.
		- p_palette_entry[15~8]:     Cb value of the palette.
		- p_palette_entry[23~16]:    Cr value of the palette.
		- p_palette_entry[31~24]:    Alpha value of the palette.

    @return void

*/
void idec_set_palette_group_a_cr_cb_y(IDE_ID id, UINT32 ui_start, UINT32 ui_number, UINT32 *p_palette_entry)
{
	UINT32 i = 0, current_pal;
	UINT32 ui_re_order, ui_re_order_tmp;
	UINT32 ui_re_order_a, ui_re_order_y, ui_re_order_cb, ui_re_order_cr;

	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		DBG_WRN("Palette entry exceed the total palette size! ui_start=%d  ui_number=%d\r\n", (int)ui_start, (int)ui_number);
	}


	if (ui_start < IDE_PAL_NUM) {
		UINT32 end;

		end = ((ui_start + ui_number) > IDE_PAL_NUM) ? IDE_PAL_NUM : (ui_start + ui_number);

		idec_set_pal0_rw(id, 1);

		for (current_pal = ui_start; current_pal < end; current_pal++) {
			ui_re_order_tmp = p_palette_entry[i++];
			ui_re_order_a = (ui_re_order_tmp & 0xFF000000) >> 24;
			ui_re_order_cr = (ui_re_order_tmp & 0x00FF0000) >> 16;
			ui_re_order_cb = (ui_re_order_tmp & 0x0000FF00) >> 8;
			ui_re_order_y = (ui_re_order_tmp & 0x000000FF) >> 0;
			ui_re_order = (ui_re_order_a << 24) + (ui_re_order_y << 16) + (ui_re_order_cb << 8) + (ui_re_order_cr << 0);

			idec_set_pal(id, current_pal, ui_re_order);
		}

		idec_set_pal0_rw(id, 0);
	}

#if 0
	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		UINT32 start;

		start = (ui_start >= IDE_PAL_NUM) ? ui_start : IDE_PAL_NUM;

		idec_set_pal1_rw(id, 1);

		for (current = start; current < (ui_start + ui_number); current++) {
			ui_re_order_tmp = p_palette_entry[i++];
			ui_re_order_a = (ui_re_order_tmp & 0xFF000000) >> 24;
			ui_re_order_cr = (ui_re_order_tmp & 0x00FF0000) >> 16;
			ui_re_order_cb = (ui_re_order_tmp & 0x0000FF00) >> 8;
			ui_re_order_y = (ui_re_order_tmp & 0x000000FF) >> 0;
			ui_re_order = (ui_re_order_a << 24) + (ui_re_order_y << 16) + (ui_re_order_cb << 8) + (ui_re_order_cr << 0);

			idec_set_pal(id, current, ui_re_order);
		}

		idec_set_pal1_rw(id, 0);
	}
#endif
}

/**
    Get a group of palette entries

    Get the color and blending operation of the specific entries of OSD palette.
    The palette entries is set from the start id to the end id.
    The ID range 0~255 is for palette-0 and 256~511 is for palette-1.


    @param[in] id   ide ID
    @param[in]  ui_start         Start entry id. Valid range from 0~511.
    @param[in]  ui_number        Total number of palette entry to be set. Valid range from 1~512.
    @param[in]  p_palette_entry  The queue of palette settings.
		- p_palette_entry[7~0]:      Cr value of the palette.
		- p_palette_entry[15~8]:     Cb value of the palette.
		- p_palette_entry[23~16]:    Y value of the palette.
		- p_palette_entry[31~24]:    Alpha value of the palette.

    @return void

*/
void idec_get_palette_group(IDE_ID id, UINT32 ui_start, UINT32 ui_number, UINT32 *p_palette_entry)
{
	UINT32 i = 0, current_pal;

	//if((ui_start+ui_number) > IDE_PAL_NUM)
	//{
	//    DBG_WRN("Palette entry exceed the total palette size! ui_start=%d  ui_number=%d\r\n",ui_start,ui_number);
	//}

	if (ui_start < IDE_PAL_NUM) {
		UINT32 end;

		end = ((ui_start + ui_number) > IDE_PAL_NUM) ? IDE_PAL_NUM : (ui_start + ui_number);

		idec_set_pal0_rw(id, 1);

		for (current_pal = ui_start; current_pal < end; current_pal++) {
			idec_get_pal(id, current_pal, &(p_palette_entry[i]));
			i++;
		}

		idec_set_pal0_rw(id, 0);
	}

	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		UINT32 start;

		start = (ui_start >= IDE_PAL_NUM) ? ui_start : IDE_PAL_NUM;

		idec_set_pal0_rw(id, 1);

		for (current_pal = start; current_pal < (ui_start + ui_number); current_pal++) {
			idec_get_pal(id, current_pal, &(p_palette_entry[i]));
			i++;
		}

		idec_set_pal0_rw(id, 0);
	}

}

/**
    Get a group of palette entries

    Get the color and blending operation of the specific entries of OSD palette.
    The palette entries is set from the start id to the end id.
    The ID range 0~255 is for palette-0 and 256~511 is for palette-1.


    @param[in] id   ide ID
    @param[in]  ui_start         Start entry id. Valid range from 0~511.
    @param[in]  ui_number        Total number of palette entry to be set. Valid range from 1~512.
    @param[in]  p_palette_entry  The queue of palette settings.
		- p_palette_entry[7~0]:      Y value of the palette.
		- p_palette_entry[15~8]:     Cb value of the palette.
		- p_palette_entry[23~16]:    Cr value of the palette.
		- p_palette_entry[31~24]:    Alpha value of the palette.

    @return void

*/
void idec_get_palette_group_a_cr_cb_y(IDE_ID id, UINT32 ui_start, UINT32 ui_number, UINT32 *p_palette_entry)
{
	UINT32 i = 0, current_pal;
	UINT32 ui_re_order, ui_re_order_tmp;
	UINT32 ui_re_order_a, ui_re_order_y, ui_re_order_cb, ui_re_order_cr;

	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		DBG_WRN("Palette entry exceed the total palette size! ui_start=%d  ui_number=%d\r\n", (int)ui_start, (int)ui_number);
	}

	if (ui_start < IDE_PAL_NUM) {
		UINT32 end;

		end = ((ui_start + ui_number) > IDE_PAL_NUM) ? IDE_PAL_NUM : (ui_start + ui_number);

		idec_set_pal0_rw(id, 1);

		for (current_pal = ui_start; current_pal < end; current_pal++) {
			idec_get_pal(id,  current_pal, &ui_re_order_tmp);

			ui_re_order_a = (ui_re_order_tmp & 0xFF000000) >> 24;
			ui_re_order_y = (ui_re_order_tmp & 0x00FF0000) >> 16;
			ui_re_order_cb = (ui_re_order_tmp & 0x0000FF00) >> 8;
			ui_re_order_cr = (ui_re_order_tmp & 0x000000FF) >> 0;
			ui_re_order = (ui_re_order_a << 24) + (ui_re_order_cr << 16) + (ui_re_order_cb << 8) + (ui_re_order_y << 0);
			p_palette_entry[i] =  ui_re_order;

			i++;
		}

		idec_set_pal0_rw(id, 0);
	}

#if 0
	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		UINT32 start;

		start = (ui_start >= IDE_PAL_NUM) ? ui_start : IDE_PAL_NUM;

		idec_set_pal1_rw(id, 1);

		for (current = start; current < (ui_start + ui_number); current++) {
			idec_get_pal(id,  current, &ui_re_order_tmp);

			ui_re_order_a = (ui_re_order_tmp & 0xFF000000) >> 24;
			ui_re_order_y = (ui_re_order_tmp & 0x00FF0000) >> 16;
			ui_re_order_cb = (ui_re_order_tmp & 0x0000FF00) >> 8;
			ui_re_order_cr = (ui_re_order_tmp & 0x000000FF) >> 0;
			ui_re_order = (ui_re_order_a << 24) + (ui_re_order_cr << 16) + (ui_re_order_cb << 8) + (ui_re_order_y << 0);
			p_palette_entry[i] =  ui_re_order;

			i++;
		}

		idec_set_pal1_rw(id, 0);
	}
#endif
}

#if 0
/**
    Get a group of shadow palette entries

    Get the color and blending operation of the specific entries of OSD palette.
    The palette entries is set from the start id to the end id.
    The ID range 0~255 is for palette-0 and 256~511 is for palette-1.


    @param[in] id   ide ID
    @param[in]  ui_start         Start entry id. Valid range from 0~511.
    @param[in]  ui_number        Total number of palette entry to be set. Valid range from 1~512.
    @param[in]  p_palette_entry  The queue of palette settings.
		- p_palette_entry[7~0]:      Cr value of the palette.
		- p_palette_entry[15~8]:     Cb value of the palette.
		- p_palette_entry[23~16]:    Y value of the palette.
		- p_palette_entry[31~24]:    Alpha value of the palette.

    @return void

*/
void idec_get_shadow_palette_group(IDE_ID id, UINT32 ui_start, UINT32 ui_number, UINT32 *p_palette_entry)
{
	UINT32 i = 0, current;

	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		DBG_WRN("Palette entry exceed the total palette size! ui_start=%d  ui_number=%d\r\n", ui_start, ui_number);
	}

	if (ui_start < IDE_PAL_NUM) {
		UINT32 end;

		end = ((ui_start + ui_number) > IDE_PAL_NUM) ? IDE_PAL_NUM : (ui_start + ui_number);

		//idec_set_o1_pal_shw_en(id, 1);

		idec_set_pal0_rw(id, 1);

		for (current = ui_start; current < end; current++) {
			idec_get_pal(id, current, &(p_palette_entry[i]));
			i++;
		}

		idec_set_pal0_rw(id, 0);

		//idec_set_o1_pal_shw_en(id, 0);
	}

#if 0
	if ((ui_start + ui_number) > IDE_PAL_NUM) {
		UINT32 start;

		start = (ui_start >= IDE_PAL_NUM) ? ui_start : IDE_PAL_NUM;

		idec_set_o1_pal_shw_en(id, 1);

		idec_set_pal1_rw(id, 1);

		for (current = start; current < (ui_start + ui_number); current++) {
			idec_get_pal(id, current, &(p_palette_entry[i]));
			i++;
		}

		idec_set_pal1_rw(id, 0);
		idec_set_o1_pal_shw_en(id, 1);
	}
#endif
}
#endif

/**
    ide wait frame end

    @param[in] id   ide ID
    @return void
*/
void idec_wait_frame_end(IDE_ID id, BOOL wait)
{
	if (wait == FALSE) {
		DBG_WRN("parameter wait must be true! NOT SUPPORT NON BLOCKING(auto change parameter wait to TRUE)\r\n");
		wait = TRUE;
	}
	if (id > IDE_ID_1) {
		DBG_WRN("error id\r\n");
		return;
	}

	if (idec_get_en(id) == 1) {
		if ((idec_get_interrupt_en(id) & IDE_FRAME_END_IRQEN) != IDE_FRAME_END_IRQEN) {
			idec_clear_interrupt_status(id, IDE_FRAME_END_IRQSTS);
			//Do not set interrupt en
			//idec_set_interrupt_en(id, IDE_FRAME_END_IRQEN);
			idec_clr_interrupt_en(id, IDE_FRAME_END_IRQEN);
		}
		if (wait) {
			while(!((idec_get_interrupt_status(id) & IDE_FRAME_END_IRQSTS) == IDE_FRAME_END_IRQSTS)) {};
			idec_clear_interrupt_status(id, IDE_FRAME_END_IRQSTS);
		}
	} else {
		// When ide is set disabled, it is activate after frame-end.
		// So we add 40ms delay to ensure the ide disable is activate.
		ide_platform_delay_ms(40);//25Hz
	}
}

/**
    ide wait YUV output done

    @param[in] id   ide ID
    @return void
*/
void idec_wait_yuv_output_done(IDE_ID id)
{

	if (id == IDE_ID_2) {
		DBG_ERR("YUV output only available at IDE1\r\n");
		return;
	}

	if (idec_get_en(id) == 1) {

//		idec_set_interrupt_en(id, IDE_YUV_OUT_DRAM_END_IRQEN);
		idec_clr_interrupt_en(id, IDE_YUV_OUT_DRAM_END_IRQEN);

		//ide_platform_flg_wait(id, FLGPTN_IDE_DMA_DONE);
		while(!((idec_get_interrupt_status(id) & IDE_YUV_OUT_DRAM_END_IRQSTS) == IDE_YUV_OUT_DRAM_END_IRQSTS)) {};

		//idec_clr_interrupt_en(id, IDE_YUV_OUT_DRAM_END_IRQEN);

		idec_clear_interrupt_status(id, IDE_YUV_OUT_DRAM_END_IRQSTS);
	}
}


/**
    Disable video window

    @param[in] id   ide ID
    @param[in] video_id Which video to disable
		- IDE_VIDEOID_1: Video 1
		- IDE_VIDEOID_2: Video 2

    @return void
*/
void idec_disable_video(IDE_ID id, IDE_VIDEOID video_id)
{
	if (video_id == IDE_VIDEOID_1) {
		idec_set_v1_en(id, FALSE);
	} else {
		idec_set_v2_en(id, FALSE);
	}
	idec_set_load(id);

	// Wait for video is really disabled
	if (idec_get_en(id)) {
		idec_wait_frame_end(id, TRUE);
	}
}

/**
    Enable video window

    @param[in] id   ide ID
    @param[in] video_id Which video to enable.
		- IDE_VIDEOID_1: Video 1
		- IDE_VIDEOID_2: Video 2

    @return void
*/
void idec_enable_video(IDE_ID id, IDE_VIDEOID video_id)
{
	if (video_id == IDE_VIDEOID_1) {
		idec_set_v1_en(id, TRUE);
	} else {
		idec_set_v2_en(id, TRUE);
	}
	idec_set_load(id);

	// Wait for video is really enabled
	if (idec_get_en(id)) {
		idec_wait_frame_end(id, TRUE);
	}
}
#if 0
/**
  Get Video window enable state

  @param[in] video_id Which video to enable.
		- IDE_VIDEOID_1: Video 1
		- IDE_VIDEOID_2: Video 2

  @return TRUE if enabled, FALSE if disabled
*/
BOOL ide_get_video_enable(IDE_VIDEOID video_id)
{
	if (video_id == IDE_VIDEOID_1) {
		return (ide_get_window_en() & DISPLAY_VIDEO1_EN) != 0;
	} else if (video_id == IDE_VIDEOID_2) {
		return (ide_get_window_en() & DISPLAY_VIDEO2_EN) != 0;
	}
	return FALSE;
}


/**
    Disable OSD window

    @param[in] osd_id Which OSD to disable
		- IDE_OSDID_1: OSD 1
		- IDE_OSDID_2: OSD 2

    @return void
*/
void ide_disable_osd(IDE_OSDID osd_id)
{
	if (osd_id == IDE_OSDID_1) {
		ide_set_o1_en(FALSE);
	} else {
		ide_set_o2_en(FALSE);
	}
	ide_set_load();

	// Wait for Osd is really disabled
	if (ide_get_en()) {
		ide_wait_frame_end();
	}
}

/**
  Enable OSD window

  @param[in] osd_id Which OSD to enable
		- IDE_OSDID_1: OSD 1
		- IDE_OSDID_2: OSD 2

  @return void
*/
void ide_enable_osd(IDE_OSDID osd_id)
{
	if (osd_id == IDE_OSDID_1) {
		ide_set_o1_en(TRUE);
	} else {
		ide_set_o2_en(TRUE);
	}
	ide_set_load();

	// Wait for Osd is really disabled
	if (ide_get_en()) {
		ide_wait_frame_end();
	}
}

/**
	Get OSD window enable state

	@param[in] osd_id Which OSD to enable
		- IDE_OSDID_1: OSD 1
		- IDE_OSDID_2: OSD 2

	@return TRUE if enabled, FALSE if disabled
*/
BOOL ide_get_osd_enable(IDE_OSDID osd_id)
{
	if (osd_id == IDE_OSDID_1) {
		return (ide_get_window_en() & DISPLAY_OSD1_EN) != 0;
	} else if (osd_id == IDE_OSDID_2) {
		return (ide_get_window_en() & DISPLAY_OSD2_EN) != 0;
	}
	return FALSE;
}
#endif
/**
    Fill YCbCr color to specific Video Buffer

    @param[in] id   ide ID
    @param[in] video_id Determine which video to work
      - VIDEOID_1: Video1
      - VIDEOID_2: Video2
    @param[in] buffer_id: Determine which buffer to work
      - BUFFERID_0: Buffer0
      - BUFFERID_1: Buffer1
      - BUFFERID_2: Buffer2
      - BUFFERID_CURRENT: Current operating buffer
    @param[in] p_ycbcr: The color which will be filled up with the buffer

    @return void
*/
void idec_set_video_buffer_content(IDE_ID id, IDE_VIDEOID video_id, IDE_BUFFERID buffer_id, PYUVCOLOR p_ycbcr)
{
#if 0
	//#NT#2015/10/07#Steven Wang -begin
	//#NT#Coverity 44377,44379, 44380
	UINT32  ui_bw = 0, ui_bh = 0, ui_lineoffset = 0;
	//#NT#2015/10/07#Steven Wang -end
	//#NT#2015/10/07#Steven Wang -begin
	//#NT#Coverity 44375
	IDE_COLOR_FORMAT ui_format = COLOR_1_BIT;
	//#NT#2015/10/07#Steven Wang -end
	UINT32  ui_total_cbcr_length = 0;
	UINT8   uc_buffer_selected;
	UINT32  ui_y_addr, ui_cb_addr, ui_cr_addr;
	IDE_BJMODE ui_bjmode;
	IDE_OP_BUF ui_optbuf;
	IDE_BUF_NUM ui_buf_num;
	UINT32  uii;
	UINT16 *xs16;

	ui_bjmode = IDE_VIDEO_BJMODE_CANT_CROSS_WRITE;
	ui_optbuf = IDE_VIDEO_BUFFER_OPT_0;
	ui_buf_num = IDE_VIDEO_BUFFER_NUM_1;
	ui_y_addr = 0;
	ui_cb_addr = 0;
	ui_cr_addr = 0;


	if (video_id == IDE_VIDEOID_1) {
		if (buffer_id == IDE_VIDEO_BUFFERID_CURRENT) {
			idec_get_v1_buf_op(id, &ui_bjmode, &ui_optbuf, &ui_buf_num);
			uc_buffer_selected = ui_optbuf;
		} else {
			uc_buffer_selected = buffer_id;
		}

		// Get buffer's address
		switch (uc_buffer_selected) {
		case IDE_VIDEO_BUFFER_OPT_0:
			idec_get_v1_buf0_addr(id, &ui_y_addr, &ui_cb_addr, &ui_cr_addr);
			break;
		case IDE_VIDEO_BUFFER_OPT_1:
			idec_get_v1_buf1_addr(id, &ui_y_addr, &ui_cb_addr, &ui_cr_addr);
			break;
		case IDE_VIDEO_BUFFER_OPT_2:
			idec_get_v1_buf2_addr(id, &ui_y_addr, &ui_cb_addr, &ui_cr_addr);
			break;
		}

		// Get the dimension of current buffer
		idec_get_v1_buf_dim(id, &ui_bw, &ui_bh, &ui_lineoffset);
		// Get current window format
		idec_get_v1_fmt(id, &ui_format);
	} else if (video_id == IDE_VIDEOID_2) {
		if (buffer_id == IDE_VIDEO_BUFFERID_CURRENT) {
			idec_get_v2_buf_op(id, &ui_bjmode, &ui_optbuf, &ui_buf_num);
			uc_buffer_selected = ui_optbuf;
		} else {
			uc_buffer_selected = buffer_id;
		}

		// Get buffer's address

		switch (uc_buffer_selected) {
		//43337
		case IDE_VIDEO_BUFFERID_0:
			idec_get_v2_buf0_addr(id, &ui_y_addr, &ui_cb_addr, &ui_cr_addr);
			break;
		case IDE_VIDEO_BUFFERID_1:
			idec_get_v2_buf1_addr(id, &ui_y_addr, &ui_cb_addr, &ui_cr_addr);
			break;
		//coverity[mixed_enums]
		case IDE_VIDEO_BUFFERID_2:
			idec_get_v2_buf2_addr(id, &ui_y_addr, &ui_cb_addr, &ui_cr_addr);
			break;
		}
		// Get the dimension of current buffer
		idec_get_v2_buf_dim(id, &ui_bw, &ui_bh, &ui_lineoffset);
		// Get current window format
		idec_get_v2_fmt(id, &ui_format);
	}

	// If the buffer is not configured, return
	if ((ui_y_addr == 0) || (ui_cb_addr == 0) || (ui_cr_addr == 0) || (ui_lineoffset == 0)) {
		return ;
	}

	switch (ui_format) {
	case COLOR_YCBCR444:
		ui_total_cbcr_length = ui_bw * ui_bh;
		break;
	case COLOR_YCBCR422:
		ui_total_cbcr_length = (ui_bw * ui_bh >> 1);
		break;
	case COLOR_YCBCR420:
		ui_total_cbcr_length = (ui_bw * ui_bh >> 2);
		break;
	case COLOR_YCC422P:
		ui_total_cbcr_length = (ui_bw * ui_bh);
		break;
	case COLOR_YCC420P:
		ui_total_cbcr_length = (ui_bw * ui_bh >> 1);
		break;
	default:
		ui_total_cbcr_length = 0;
		DBG_WRN("Non-supported data format!\r\n");
		break;
	}

	memset((UINT8 *)ui_y_addr, p_ycbcr->color_y, ui_bw * ui_bh);
	if ((ui_format == COLOR_YCC422P) || (ui_format == COLOR_YCC420P)) {
		xs16 = (UINT16 *)ui_cb_addr;
		for (uii = 0; uii < ui_total_cbcr_length / 2; uii++) {
			*xs16++ = ((p_ycbcr->color_cb + (p_ycbcr->color_cr << 8)) & 0xFFFF);
		}
	} else {
		memset((UINT8 *)ui_cb_addr, p_ycbcr->color_cb, ui_total_cbcr_length);
		memset((UINT8 *)ui_cr_addr, p_ycbcr->color_cr, ui_total_cbcr_length);
	}

	idec_set_load(id);
	idec_wait_frame_end(id);
#endif
}

/**
  Get ide palette capability

  @param[in] id   ide ID
  @return
    - IDE_PAL_CAP_16X2  supports 16x2 palette entries
    - IDE_PAL_CAP_256X1 supports 256x1 palette entries
    - IDE_PAL_CAP_256X2 supports 256x2 palette entries
*/
IDE_PAL_CAP idec_get_pal_capability(IDE_ID id)
{
	if (id == IDE_ID_1) {
		return IDE_PAL_CAP_256X2;
	} else {
		return IDE_PAL_CAP_256X1;
	}
}


#if 0
/**
    Convert RGB color space to YCbCr(YUV) color space

    Convert RGB color space to YCbCr(YUV) color space for ide display.

    @param[in] p_rgb - RGB color to be converted
    @param[in] p_ycbcr - output YCbCr color

    @return void
*/
void idec_convert_rgb2ycbcrr(PRGBCOLOR p_rgb, PYUVCOLOR p_ycbcr)
{
	INT32 rmg, bmg;

	rmg = p_rgb->color_r - p_rgb->color_g;
	bmg = p_rgb->color_b - p_rgb->color_g;

	p_ycbcr->color_y = (UINT8)(p_rgb->color_g + ((19 * (rmg + (49 * bmg >> 7))) >> 6));
	p_ycbcr->color_cb = (UINT8)(((bmg - ((43 * rmg) >> 7)) >> 1) + 128);
	p_ycbcr->color_cr = (UINT8)(((rmg - ((21 * bmg) >> 7)) >> 1) + 128);
}
#endif
/*
    YCbCr -> R'G'B'

    R' = (Y - 0) + 1.4020(Cr - 128)
    G' = (Y - 0) - 0.3441(Cb - 128) - 0.7141(Cr - 128)
    B' = (Y - 0) + 1.7720(Cb - 128)
*/
/*static UINT32 icst_coef[9] =
{
    0x100, 0x000, 0x166,
    0x100, 0xFA8, 0xF4A,
    0x100, 0x1C5, 0x000
};
static UINT32 ICSTPostOfs[3] = { 0x00, 0x00, 0x00 };
static UINT32 ICSTPreOfs[3] =  { 0x00, 0x80, 0x80 };*/

/*
    YCbCr -> RGB

*/
static UINT32 icst_coef[4] = {
	0x92, 0x30, 0x77, 0xDB
};

/*
    R'G'B' -> YCbCr

    Y  =  0.299R'   + 0.587G'   + 0.114B'   + 0
    Cb = -0.16857R' - 0.33126G' + 0.5B'     + 128
    Cr =  0.5R'     - 0.41869G' - 0.08131B' + 128
*/
/*
static UINT32 CSTCoef[9] =
{
    0x04C, 0x096, 0x01D,
    0x7D5, 0x7AC, 0x080,
    0x080, 0x795, 0x7EC
};

static UINT32 CSTPostOfs[3] = { 0x00, 0x80, 0x80 };
static UINT32 CSTPreOfs[3] =  { 0x00, 0x00, 0x00 };
*/
//static UINT32 ZeroOfs[3] = { 0x0, 0x0, 0x0 };

static UINT8 default_limit[] = { 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF };
static UINT8 ccir656_limit[] = { 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE };


/**
  Configure display ICST

  Enable display color space transform to convert YCbCr to RGB

  @param[in] id   ide ID
  @param[in] b_en 0:disable icst 1:enable icst
  @param[in] SEL
    - @b CST_RGB2YCBCR
    - @b CST_YCBCR2RGB

  @return void
*/
void idec_config_icst(IDE_ID id, BOOL b_en, CST_SEL SEL)
{
	idec_set_icst(id, b_en);

	if (b_en) {
		idec_set_icst_coef(id, icst_coef);
		//ide_setIcstPreOffset(ICSTPreOfs[0], ICSTPreOfs[1], ICSTPreOfs[2]);
		//ide_set_out_offset(ICSTPostOfs[0], ICSTPostOfs[1], ICSTPostOfs[2]);
		//ide_set_tv_out_offset(ICSTPostOfs[0], ICSTPostOfs[1], ICSTPostOfs[2]);
	} else {
		//ide_set_out_offset(ZeroOfs[0], ZeroOfs[1], ZeroOfs[2]);
		//ide_set_out_offset(ZeroOfs[0], ZeroOfs[1], ZeroOfs[2]);
		//ide_set_tv_out_offset(ZeroOfs[0], ZeroOfs[1], ZeroOfs[2]);
	}
	idec_set_load(id);
}

/**
  Get Configure display ICST

  Enable display color space transform to convert YCbCr to RGB

  @param[in] id   ide ID
  @param[in] b_en 0:disable icst 1:enable icst
  @param[in] SEL
    - @b CST_RGB2YCBCR
    - @b CST_YCBCR2RGB

  @return void
*/
void idec_getconfig_icst(IDE_ID id, BOOL *b_en, CST_SEL *SEL)
{
	*b_en = idec_get_icst(id);
	*SEL = CST_RGB2YCBCR;
}


/**
    Set display output limit

    @param[in] id   ide ID
    @param[in] b_sel
      - @b FALSE:default limit
      - @b TRUE:CCIR656 limit

    @return void
*/
void idec_config_output_limit(IDE_ID id, BOOL b_sel)
{
	if (b_sel == 0) {
		// default limit
		idec_set_out_limit(id, default_limit[0], default_limit[1], default_limit[2], default_limit[3], default_limit[4], default_limit[5]);
	} else if (b_sel == 1) {
		// CCIR656 output limit
		idec_set_out_limit(id, ccir656_limit[0], ccir656_limit[1], ccir656_limit[2], ccir656_limit[3], ccir656_limit[4], ccir656_limit[5]);
	}
}

/**
    Set the window constant offset value

    This api is designed for TV-HDMI Overscan function, which makes overscan can be achieved by
    ide constant win-X/Y and without modifying buffer content.

    @param[in] id   ide ID
    @param[in] ui_x  Window X value.
    @param[in] ui_y  Window Y value.

    @return void
*/
void idec_set_constant_window_offset(IDE_ID id, UINT32 ui_x, UINT32 ui_y)
{
	gui_const_win_x[id] = ui_x;
	gui_const_win_y[id] = ui_y;
}

/**
    Get the window constant offset value

    This api is designed for TV-HDMI Overscan function, which makes overscan can be achieved by
    ide constant win-X/Y and without modifying buffer content.

    @param[in] id   ide ID
    @param[in] ui_x  Window X value.
    @param[in] ui_y  Window Y value.

    @return void
*/
void idec_get_constant_window_offset(IDE_ID id, UINT32 *ui_x, UINT32 *ui_y)
{
	*ui_x = gui_const_win_x[id];
	*ui_y = gui_const_win_y[id];
}

#if (0)//def __KERNEL__
EXPORT_SYMBOL(idec_open);
EXPORT_SYMBOL(idec_set_color_comp_ccon_en);
EXPORT_SYMBOL(idec_set_color_ctrl_hue);
EXPORT_SYMBOL(idec_wait_frame_end);
EXPORT_SYMBOL(idec_fill_rgb_gamma);
EXPORT_SYMBOL(idec_get_background);
EXPORT_SYMBOL(idec_set_dithering);
EXPORT_SYMBOL(idec_set_color_ctrl_int);
EXPORT_SYMBOL(idec_get_window_en);
EXPORT_SYMBOL(idec_get_color_ctrl_int_sat_ofs);
EXPORT_SYMBOL(idec_get_v2_read_order);
EXPORT_SYMBOL(idec_set_ver_timing);
EXPORT_SYMBOL(idec_set_cst1);
EXPORT_SYMBOL(idec_get_video_colorkey_op);
EXPORT_SYMBOL(idec_set_hs_inv);
EXPORT_SYMBOL(idec_get_cst1);
EXPORT_SYMBOL(idec_set_fd_line_layer_swap);
EXPORT_SYMBOL(idec_set_interlace);
EXPORT_SYMBOL(idec_set_video_colorkey);
EXPORT_SYMBOL(idec_set_video_colorkey_op);
EXPORT_SYMBOL(idec_get_color_comp_ccon_en);
EXPORT_SYMBOL(idec_set_v1_en);
EXPORT_SYMBOL(idec_set_color_comp_ycon_en);
EXPORT_SYMBOL(idec_set_video_buffer_content);
EXPORT_SYMBOL(idec_set_sync_delay);
EXPORT_SYMBOL(idec_get_fd_win_bord);
EXPORT_SYMBOL(idec_get_v2_buf_op);
EXPORT_SYMBOL(idec_set_even);
EXPORT_SYMBOL(idec_get_v2_en);
EXPORT_SYMBOL(idec_get_palette_group_a_cr_cb_y);
EXPORT_SYMBOL(idec_set_odd);
EXPORT_SYMBOL(idec_get_o1_read_order);
EXPORT_SYMBOL(idec_set_fld_inv);
EXPORT_SYMBOL(idec_set_out_limit);
EXPORT_SYMBOL(idec_set_osd1_colorkey);
EXPORT_SYMBOL(idec_set_subpixel);
EXPORT_SYMBOL(idec_set_color_comp_cofs);
EXPORT_SYMBOL(idec_set_fd_win_dim);
EXPORT_SYMBOL(idec_get_color_ctrl_hue_adj_en);
EXPORT_SYMBOL(idec_set_o1_buf_dim);
EXPORT_SYMBOL(idec_get_device);
EXPORT_SYMBOL(idec_set_background);
EXPORT_SYMBOL(idec_set_v1_buf_op);
EXPORT_SYMBOL(idec_set_color_ctrl_hue_adj_en);
EXPORT_SYMBOL(idec_set_all_window_en);
EXPORT_SYMBOL(idec_set_vs_inv);
EXPORT_SYMBOL(idec_set_v1_read_order);
EXPORT_SYMBOL(idec_set_through);
EXPORT_SYMBOL(idec_set_icst0_pre_offset);
EXPORT_SYMBOL(idec_set_color_comp_ccon);
EXPORT_SYMBOL(idec_set_v2_buf_op);
EXPORT_SYMBOL(idec_get_video_colorkey);
EXPORT_SYMBOL(idec_set_fd_color);
EXPORT_SYMBOL(idec_get_fd_win_dim);
EXPORT_SYMBOL(idec_set_palette_group);
EXPORT_SYMBOL(idec_get_v1_fmt);
EXPORT_SYMBOL(idec_set_fd_en);
EXPORT_SYMBOL(idec_get_color_comp_ycon_en);
EXPORT_SYMBOL(idec_set_ycex);
EXPORT_SYMBOL(idec_set_o1_read_order);
EXPORT_SYMBOL(idec_get_en);
EXPORT_SYMBOL(idec_set_v2_buf0_addr);
EXPORT_SYMBOL(idec_get_icst0);
EXPORT_SYMBOL(idec_set_dram_out_format);
EXPORT_SYMBOL(idec_get_v1_read_order);
EXPORT_SYMBOL(idec_set_osd_win_attr_ex);
EXPORT_SYMBOL(idec_set_out_comp);
EXPORT_SYMBOL(idec_set_o1_en);
EXPORT_SYMBOL(idec_set_v2_read_order);
EXPORT_SYMBOL(idec_set_all_window_dis);
EXPORT_SYMBOL(idec_get_o1_en);
EXPORT_SYMBOL(idec_get_color_comp_yofs);
EXPORT_SYMBOL(idec_set_o1_hsm);
EXPORT_SYMBOL(idec_get_osd1_colorkey);
EXPORT_SYMBOL(idec_set_color_comp_adj_en);
EXPORT_SYMBOL(idec_get_v1_win_dim);
EXPORT_SYMBOL(idec_set_osd1_colorkey_en);
EXPORT_SYMBOL(idec_get_out_offset);
EXPORT_SYMBOL(idec_set_hor_timing);
EXPORT_SYMBOL(idec_get_hor_timing);
EXPORT_SYMBOL(idec_get_ver_timing);
EXPORT_SYMBOL(idec_get_color_comp_ccon);
EXPORT_SYMBOL(idec_get_constant_window_offset);
EXPORT_SYMBOL(idec_get_v2_fmt);
EXPORT_SYMBOL(idec_set_v2_buf_dim);
EXPORT_SYMBOL(idec_get_o1_win_pos);
EXPORT_SYMBOL(idec_get_fcst_coef);
EXPORT_SYMBOL(idec_get_osd1_colorkey_en);
EXPORT_SYMBOL(idec_set_color_ctrl_int_sat_ofs);
EXPORT_SYMBOL(idec_set_alpha_blending);
EXPORT_SYMBOL(idec_get_fd_color);
EXPORT_SYMBOL(idec_get_v1_win_pos);
EXPORT_SYMBOL(idec_set_clamp);
EXPORT_SYMBOL(idec_get_icst0_pre_offset);
EXPORT_SYMBOL(idec_get_o1_fmt);
EXPORT_SYMBOL(idec_set_color_ctrl_sat);
EXPORT_SYMBOL(idec_set_v2_hsm);
EXPORT_SYMBOL(idec_set_color_comp_yofs);
EXPORT_SYMBOL(idec_is_opened);
EXPORT_SYMBOL(idec_get_v2_win_pos);
EXPORT_SYMBOL(idec_set_constant_window_offset);
EXPORT_SYMBOL(idec_get_color_comp_ycon);
EXPORT_SYMBOL(idec_get_v2_win_dim);
EXPORT_SYMBOL(idec_set_device);
EXPORT_SYMBOL(idec_get_color_ctrl_hue);
EXPORT_SYMBOL(idec_set_video_colorkey_sel);
EXPORT_SYMBOL(idec_set_v1_buf0_addr);
EXPORT_SYMBOL(idec_getconfig_icst);
EXPORT_SYMBOL(idec_set_color_ctrl_dds);
EXPORT_SYMBOL(idec_get_color_comp_adj_en);
//EXPORT_SYMBOL(ide_set_tv_power_down);
//EXPORT_SYMBOL(ide_set_tv_src);
EXPORT_SYMBOL(idec_set_o1_buf_alpha_addr);
EXPORT_SYMBOL(idec_set_fd_win_pos);
EXPORT_SYMBOL(idec_get_alpha_blending);
EXPORT_SYMBOL(idec_set_icst0_coef);
EXPORT_SYMBOL(idec_get_rgb_gamma);
EXPORT_SYMBOL(idec_set_en);
EXPORT_SYMBOL(idec_set_fd_win_bord);
EXPORT_SYMBOL(idec_get_color_ctrl_en);
EXPORT_SYMBOL(idec_get_palette_group);
//EXPORT_SYMBOL(ide_get_tv_power_down);
EXPORT_SYMBOL(idec_set_v1_hsm);
EXPORT_SYMBOL(idec_get_icst);
EXPORT_SYMBOL(idec_set_gamma_en);
EXPORT_SYMBOL(idec_set_v2_en);
EXPORT_SYMBOL(idec_set_clk_inv);
EXPORT_SYMBOL(idec_get_color_ctrl_dds);
EXPORT_SYMBOL(idec_get_v1_buf_op);
EXPORT_SYMBOL(idec_set_pdir);
EXPORT_SYMBOL(idec_get_color_comp_cofs);
EXPORT_SYMBOL(idec_get_o1_win_dim);
EXPORT_SYMBOL(idec_set_icst0);
EXPORT_SYMBOL(idec_get_fd_win_pos);
EXPORT_SYMBOL(idec_set_o1_buf_addr);
EXPORT_SYMBOL(idec_set_out_offset);
EXPORT_SYMBOL(idec_wait_yuv_output_done);
EXPORT_SYMBOL(idec_set_v1_fmt);
EXPORT_SYMBOL(idec_get_color_ctrl_int);
EXPORT_SYMBOL(idec_set_clk1_2);
EXPORT_SYMBOL(idec_set_v1_buf_dim);
EXPORT_SYMBOL(idec_set_o1_fmt);
EXPORT_SYMBOL(idec_set_hvld_inv);
EXPORT_SYMBOL(idec_config_icst);
EXPORT_SYMBOL(idec_set_video_win_attr_ex);
EXPORT_SYMBOL(idec_set_osd1_colorkey_op);
EXPORT_SYMBOL(idec_get_v1_en);
EXPORT_SYMBOL(idec_set_palette_group_a_cr_cb_y);
EXPORT_SYMBOL(idec_get_gamma_en);
EXPORT_SYMBOL(idec_set_color_ctrl_en);
EXPORT_SYMBOL(idec_set_color_comp_ycon);
EXPORT_SYMBOL(idec_get_color_ctrl_sat);
EXPORT_SYMBOL(idec_set_rgbd);
EXPORT_SYMBOL(idec_set_fd_dis);
EXPORT_SYMBOL(idec_set_digital_timing);
EXPORT_SYMBOL(idec_get_fd_line_layer_swap);
EXPORT_SYMBOL(idec_set_start_field);
EXPORT_SYMBOL(idec_set_icst);
EXPORT_SYMBOL(idec_get_osd1_colorkey_op);
EXPORT_SYMBOL(idec_set_cex);
EXPORT_SYMBOL(idec_set_vvld_inv);
EXPORT_SYMBOL(idec_set_de_inv);
EXPORT_SYMBOL(idec_set_dither_vbits);
EXPORT_SYMBOL(idec_get_video_colorkey_sel);
EXPORT_SYMBOL(idec_set_load);
EXPORT_SYMBOL(idec_get_icst0_coef);
EXPORT_SYMBOL(idec_get_fd_all_en);
EXPORT_SYMBOL(idec_set_v2_fmt);
EXPORT_SYMBOL(idec_set_callback);
EXPORT_SYMBOL(idec_set_rgbd_swap);
#endif


//@}

//@}

