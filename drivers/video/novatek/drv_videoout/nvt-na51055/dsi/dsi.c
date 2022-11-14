/*
    @file       dsi.c
    @ingroup    mIDrvDisp_DSI

    @brief      Driver file for DSI module
				This file is the driver file that define the driver API for DSI module.

    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#ifdef __KERNEL__
//#include <linux/delay.h>
//#include <linux/clk.h>
//#include <plat-na51055/nvt-sramctl.h>
//#include <kwrap/type.h>
//#include <kwrap/util.h>
//#include "comm/drvdump.h"
#include <asm/nvt-common/nvt_types.h>
#include "include/dsi_int.h"
#include "include/dsi_platform.h"
#else
#if defined(__FREERTOS)
#define __MODULE__    rtos_dsi
#include <kwrap/debug.h>
#include <kwrap/spinlock.h>
#include "kwrap/semaphore.h"
#include "kwrap/flag.h"
unsigned int rtos_dsi_debug_level = NVT_DBG_WRN;
#include "dsi.h"
#include "include/dsi_int.h"
#include "dsi_platform.h"
#include "interrupt.h"
#if defined(_BSP_NA51055_)
#include "nvt-sramctl.h"
#endif
#include "comm/timer.h"
#else
#include "DrvCommon.h"
#include "Utility.h"
#include "dsi_int.h"
#endif
#endif

#ifdef __KERNEL__
#define _FPGA_EMULATION_ 0

#define Delay_DelayMs(ms) udelay(ms*1000)
#define Delay_DelayUs(us) udelay(us)
typedef unsigned int        	FLGPTN;                     ///< Flag patterns
typedef unsigned int        	*PFLGPTN;                   ///< Flag patterns (Pointer)
#define FLGPTN_BIT(n)       	((FLGPTN)(1 << (n)))        ///< Bit of flag pattern
//static SEM_HANDLE SEMID_DSI;
//static FLGPTN     FLG_ID_DSI;

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


#define FLGPTN_DSI_INT_STS          FLGPTN_BIT(0)
#define FLGPTN_DSI_TX_DONE          FLGPTN_BIT(1)
#define FLGPTN_DSI_FRM_END          FLGPTN_BIT(2)
#define FLGPTN_DSI_CLK_ULPS_DONE    FLGPTN_BIT(3)
#define FLGPTN_DSI_DAT0_ESC_DONE    FLGPTN_BIT(4)
#define FLGPTN_DSI_DAT1_ESC_DONE    FLGPTN_BIT(5)
#define FLGPTN_DSI_DAT2_ESC_DONE    FLGPTN_BIT(6)
#define FLGPTN_DSI_DAT3_ESC_DONE    FLGPTN_BIT(7)
#define FLGPTN_DSI_RX_RDY           FLGPTN_BIT(8)

#define drv_enableInt(x)
#define drv_disableInt(x)
#define pll_enableClock(x)
#define pll_disableClock(x)

#else
#if defined(__FREERTOS)
//static  VK_DEFINE_SPINLOCK(my_lock);
//#define loc_cpu(flags) vk_spin_lock_irqsave(&my_lock, flags)
//#define unl_cpu(flags) vk_spin_unlock_irqrestore(&my_lock, flags)
static ID FLG_ID_DSI;

#define FLGPTN_DSI_INT_STS          FLGPTN_BIT(0)
#define FLGPTN_DSI_TX_DONE          FLGPTN_BIT(1)
#define FLGPTN_DSI_FRM_END          FLGPTN_BIT(2)
#define FLGPTN_DSI_CLK_ULPS_DONE    FLGPTN_BIT(3)
#define FLGPTN_DSI_DAT0_ESC_DONE    FLGPTN_BIT(4)
#define FLGPTN_DSI_DAT1_ESC_DONE    FLGPTN_BIT(5)
#define FLGPTN_DSI_DAT2_ESC_DONE    FLGPTN_BIT(6)
#define FLGPTN_DSI_DAT3_ESC_DONE    FLGPTN_BIT(7)
#define FLGPTN_DSI_RX_RDY           FLGPTN_BIT(8)

#define drv_enableInt(x)
#define drv_disableInt(x)
#define Delay_DelayMs(ms) delay_us(ms*1000)
#define Delay_DelayUs(us) delay_us(us)
#endif
#endif

static BOOL         b_dsi_opened = FALSE;
static BOOL         b_lp_trans_starting = FALSE;
//static UINT32       ui_dsi_int_sts;

#define DSI_ESC_SET_TRIGGER(lane, ctx, en);\
	{                                          \
		switch (lane) {                        \
		default:                               \
		case DSI_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_TRIG = en;        \
			break;                             \
		case DSI_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_TRIG = en;        \
			break;                             \
		case DSI_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_TRIG = en;        \
			break;                             \
		case DSI_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_TRIG = en;        \
			break;                             \
		}                                      \
	}
#define DSI_ESC_SET_START(lane, ctx, en);  \
	{                                          \
		switch (lane) {                        \
		default:                               \
		case DSI_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_START = en;       \
			break;                             \
		case DSI_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_START = en;       \
			break;                             \
		case DSI_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_START = en;       \
			break;                             \
		case DSI_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_START = en;       \
			break;                             \
		}                                      \
	}
#define DSI_ESC_SET_STOP(lane, ctx, en);   \
	{                                          \
		switch (lane) {                        \
		default:                               \
		case DSI_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_STOP = en;        \
			break;                             \
		case DSI_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_STOP = en;        \
			break;                             \
		case DSI_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_STOP = en;        \
			break;                             \
		case DSI_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_STOP = en;        \
			break;                             \
		}                                      \
	}
#define DSI_ESC_SET_CMD(lane, ctx, en);    \
	{                                          \
		switch (lane) {                        \
		default:                               \
		case DSI_DATA_LANE_0:                  \
			ctx.bit.DAT0_ESC_CMD = en;         \
			break;                             \
		case DSI_DATA_LANE_1:                  \
			ctx.bit.DAT1_ESC_CMD = en;         \
			break;                             \
		case DSI_DATA_LANE_2:                  \
			ctx.bit.DAT2_ESC_CMD = en;         \
			break;                             \
		case DSI_DATA_LANE_3:                  \
			ctx.bit.DAT3_ESC_CMD = en;         \
			break;                             \
		}                                      \
	}

#ifdef __KERNEL__
void dsi_create_resource(void)
{
	//OS_CONFIG_FLAG(FLG_ID_DSI);
	//SEM_CREATE(SEMID_DSI, 1);
}
//EXPORT_SYMBOL(dsi_create_resource);

void dsi_release_resource(void)
{
	//SEM_DESTROY(SEMID_DSI);
	//rel_flg(FLG_ID_DSI);
}
void dsi_set_base_addr(UINT32 addr)
{
	//_DSI_REG_BASE_ADDR = addr;

}
//EXPORT_SYMBOL(dsi_set_base_addr);
#endif

#if defined(__FREERTOS)
irqreturn_t dsi_isr(int irq, void *devid);
#else
void dsi_isr(void);
#endif

/*
     DSI set phy enable

     Enable / disable of the DSI phy

     @param[in]     b_en
		- @b TRUE   : Enable dsi phy
		- @b FALSE  : Disable dsi phy

     @return
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
static void dsi_set_phy_enable(BOOL b_en)
{
	T_DSI_EN_REG    reg_enable;

	reg_enable.reg = DSI_GETREG(DSI_EN_REG_OFS);
	if (reg_enable.bit.PHY_EN != b_en) {
		reg_enable.bit.PHY_EN = b_en;
		DSI_SETREG(DSI_EN_REG_OFS, reg_enable.reg);
	}
}


/*
    DSI ISR

    Interrupt service routine for DSI driver.

    @return Return void
*/
#if 0
#ifndef __KERNEL__
#if defined(__FREERTOS)
irqreturn_t dsi_isr(int irq, void *devid)
#else
void dsi_isr(void)
#endif
#else
void dsi_isr(void)
#endif
{
	T_DSI_INTSTS_REG    dsi_int_sts;
	UINT32              ui_tmp_sts;
	FLGPTN              sts_flg;

	sts_flg = 0;

	dsi_int_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
	dsi_int_sts.reg &= DSI_GETREG(DSI_INTEN_REG_OFS);

	if (dsi_int_sts.reg != 0) {
		ui_dsi_int_sts = dsi_int_sts.reg;

		DSI_SETREG(DSI_INTSTS_REG_OFS, dsi_int_sts.reg);

		//debug_msg("[DSI_STS][0x%08x]\r\n", ui_dsi_int_sts);
		if (ui_dsi_int_sts & CLK_ULPS_DONE_STS) {
			DBG_IND("[dsi]CLK ULPS done\r\n");
			sts_flg |= FLGPTN_DSI_CLK_ULPS_DONE;
		}
		if (ui_dsi_int_sts & DAT0_ESC_DONE_STS) {
			//DBG_IND("[dsi]DAT0 ESC done\r\n");
			sts_flg |= FLGPTN_DSI_DAT0_ESC_DONE;
		}
		if (ui_dsi_int_sts & DAT1_ESC_DONE_STS) {
			//DBG_IND("[dsi]DAT1 ESC done\r\n");
			sts_flg |= FLGPTN_DSI_DAT1_ESC_DONE;
		}
		if (ui_dsi_int_sts & DAT2_ESC_DONE_STS) {
			//DBG_IND("[dsi]DAT2 ESC done\r\n");
			sts_flg |= FLGPTN_DSI_DAT2_ESC_DONE;
		}
		if (ui_dsi_int_sts & DAT3_ESC_DONE_STS) {
			//DBG_IND("[dsi]DAT3 ESC done\r\n");
			sts_flg |= FLGPTN_DSI_DAT3_ESC_DONE;
		}
		if (ui_dsi_int_sts & DSI_RX_READY_STS) {
			DBG_IND("[dsi]RX RDY\r\n");
			sts_flg |= FLGPTN_DSI_RX_RDY;
		}

		if (ui_dsi_int_sts & DSI_TX_DIS_STS) {
			DBG_IND("[dsi]TX done\r\n");
			sts_flg |= FLGPTN_DSI_TX_DONE;
		}
		if (ui_dsi_int_sts & DSI_FRM_END_STS) {
			//DBG_IND("[dsi]Frame end \r\n");
			sts_flg |= FLGPTN_DSI_FRM_END;
		}
		ui_tmp_sts = ui_dsi_int_sts & ~(CLK_ULPS_DONE_STS | \
										DAT0_ESC_DONE_STS | \
										DAT1_ESC_DONE_STS | \
										DAT2_ESC_DONE_STS | \
										DAT3_ESC_DONE_STS | \
										DSI_RX_READY_STS  | \
										DSI_TX_DIS_STS    | \
										DSI_FRM_END_STS);


		if (ui_tmp_sts != 0) {
			DBG_DUMP("[dsi] total status 0x%08x, event 0x%08x\r\n", (unsigned int)ui_dsi_int_sts, (unsigned int)ui_tmp_sts);
			sts_flg |= FLGPTN_DSI_INT_STS;
		}
		iset_flg(FLG_ID_DSI, sts_flg);
	}
#if defined(__FREERTOS)
		return IRQ_HANDLED;
#endif
}
#endif


/*
     DSI wait LP done

     Enable / disable of the DSI module for HS transmission

     @param[in]     b_en
		- @b TRUE   : Enable HS transmission
		- @b FALSE  : Disable HS transmission
     @return
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
static ER dsi_wait_lpdt_done(DSI_LANESEL ui_lane)
{
	//FLGPTN ui_flag;

	//FLGPTN ui_wait_flag;
	T_DSI_INTSTS_REG	dsi_int_sts;
	int timeout = 0;
#if 1
    do {
		dsi_int_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
		Delay_DelayMs(1);
		timeout++;
		if (timeout > 1000) {
			printf("dsi_wait_lpdt_done wait timeout\r\n");
			break;
		}
	} while(((dsi_int_sts.reg & DAT_ESC_DONE_STS_MASK)  == 0));
	
	if (dsi_int_sts.reg != 0) {	
		DSI_SETREG(DSI_INTSTS_REG_OFS, dsi_int_sts.reg);
	}
#else	
	if (ui_lane == DSI_DATA_LANE_0) {
		ui_wait_flag = FLGPTN_DSI_DAT0_ESC_DONE;
	} else if (ui_lane == DSI_DATA_LANE_1) {
		ui_wait_flag = FLGPTN_DSI_DAT1_ESC_DONE;
	} else if (ui_lane == DSI_DATA_LANE_2) {
		ui_wait_flag = FLGPTN_DSI_DAT2_ESC_DONE;
	} else if (ui_lane == DSI_DATA_LANE_3) {
		ui_wait_flag = FLGPTN_DSI_DAT3_ESC_DONE;
	} else {
		ui_wait_flag = FLGPTN_DSI_DAT0_ESC_DONE;
	}
	
	//NEED PORTING
	//wai_flg(&ui_flag, FLG_ID_DSI, ui_wait_flag | FLGPTN_DSI_INT_STS, TWF_ORW | TWF_CLR);
#endif
	return E_OK;
}


/**
    @addtogroup mIDrvDisp_DSI
*/
//@{
/**
     DSI wait TX done

     Wait TX operation done

     @return
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
ER dsi_wait_tx_done(void)
{
#if 1   //Polling mode
#if 1
	T_DSI_INTSTS_REG	dsi_int_sts;
	int timeout = 0;
	
	do {
		dsi_int_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
		Delay_DelayMs(1);
		timeout++;
		if (timeout > 1000) {
			printf("dsi_wait_tx_done wait timeout\r\n");
			break;
		}
	} while(((dsi_int_sts.reg & DSI_TX_DIS_STS)  == 0));//!dsi_int_sts.bit.TX_DIS);
	
	if (dsi_int_sts.reg != 0) { 
		DSI_SETREG(DSI_INTSTS_REG_OFS, dsi_int_sts.reg);
	}
#else

	UINT32  ui_cnt;
	T_DSI_INTSTS_REG   reg_sts;

	reg_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);

	ui_cnt = 0;
	while (!reg_sts.bit.TX_DIS) {
		reg_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
		ui_cnt++;

		if (ui_cnt > 100) {
			break;
		}
	}

	debug_msg("status = 0x%08x\r\n");
	DSI_SETREG(DSI_INTSTS_REG_OFS, reg_sts.reg);
#endif
	return E_OK;
#else
	//NEED PORTING
//	FLGPTN ui_flag;
//	FLGPTN ui_wait_flag = FLGPTN_DSI_TX_DONE;

//	wai_flg(&ui_flag, FLG_ID_DSI, ui_wait_flag, TWF_ORW | TWF_CLR);

	return E_OK;
#endif
}

#define DSI_CLKEN_OFFSET 0xF0020078

/**
    Open DSI driver

    Initial DSI controller related block (clock, enable interrupt/phy etc)

    @return
		- @b E_OK: open sucess

*/
ER dsi_open(void)
{
	UINT32 ui_reg;

	if (b_dsi_opened) {
		return E_OK;
	}
	//nvt_dbg(IND, "%s\n", __FUNCTION__);

#if defined(__FREERTOS)
	static BOOL rtos_init = 0;

	if (!rtos_init) {
		rtos_init = 1;
		cre_flg(&FLG_ID_DSI, NULL, "FLG_ID_DSI");
	}
#endif

#if defined(__FREERTOS)
#if defined(_TC680_) && defined(_NVT_FPGA_)
	dsi_tc680_init();
	dsi_tc680_writereg(0x0000, 0x00000000);
	dsi_tc680_writereg(0x0003, 0x00000802); //?0x00000802
	dsi_tc680_writereg(0x0004, 0x000005b0);

	dsi_tc680_writereg(0x002B, 0x00031031); //switch bank
	dsi_tc680_writereg(0x505F, 0x00000019); //0x19: clk = 150MHz
	//dsi_tc680_writereg(0x505F, 0x00000014); //0x14: clk = 120MHz
#endif
#endif


	// Turn on power
#ifdef __KERNEL__
	//nvt_disable_sram_shutdown(MIPI_SD);
#else
#if defined(__FREERTOS) && defined(_BSP_NA51055_)
	nvt_disable_sram_shutdown(DSI_SD);
#else
	//pmc_turnonPower(PMC_MODULE_MIPI_DSI);
	//pinmux_disable_sram_shutdown(DSI_SD);
#endif
#endif

	//pll_setClockRate(PLL_CLKSEL_DSI_CLKSRC, PLL_CLKSEL_DSI_CLKSRC_480);
#ifndef __KERNEL__
	pll_enable_clock(MIPI_DSI_CLK);
#endif
	ui_reg = INW(DSI_CLKEN_OFFSET);
	ui_reg |= 0x8;
	OUTW(DSI_CLKEN_OFFSET, ui_reg);

	ui_reg = DSI_GETREG(DSI_INTSTS_REG_OFS);

	DSI_SETREG(DSI_INTSTS_REG_OFS, ui_reg);
#if 0
	DSI_SETREG(DSI_INTEN_REG_OFS,   DSI_TX_DIS_INTEN | \
			   DSI_FRM_END_INTEN | \
			   DSI_SRAM_OV_INTEN | \
			   DSI_BTA_TIMEOUT_INTEN | \
			   DSI_BTA_FAIL_INTEN | \
			   DSI_BUS_CONTENTION_INTEN | \
			   DSI_RX_READY_INTEN | \
			   DSI_ERR_REPORT_INTEN | \
			   DSI_RX_ECC_ERR_INTEN | \
			   DSI_RX_CRC_ERR_INTEN | \
			   DSI_RX_STATE_ERR_INTEN | \
			   DSI_RX_INSUFFICIENT_PL_INTEN | \
			   DSI_RX_ALIGN_ERR_INTEN | \
			   DSI_FIFO_UNDER_INTEN | \
			   DSI_HS_FIFO_UNDER_INTEN | \
			   DSI_SYNC_ERR_INTEN | \
			   DSI_RX_UNKNOWN_PKT_INTEN | \
			   DSI_RX_INVALID_ESCCMD_INTEN | \
			   CLK_ULPS_DONE_INTEN | \
			   DAT0_ESC_DONE_INTEN | \
			   DAT1_ESC_DONE_INTEN | \
			   DAT2_ESC_DONE_INTEN | \
			   DAT3_ESC_DONE_INTEN);
	#endif
#ifndef __KERNEL__
#if defined(__FREERTOS)
	// Enable interrupt
	request_irq(INT_ID_DSI, dsi_isr ,IRQF_TRIGGER_HIGH, "dsi", 0);
#else
	// Enable interrupt
	//drv_enableInt(DRV_INT_DSI);
#endif
#endif

	dsi_set_phy_enable(TRUE);
	// Wait PHY Stable
	Delay_DelayUs(10);

	b_dsi_opened = TRUE;
	return E_OK;
}


/**
    Close DSI driver

    @return
		- @b E_OK: open sucess
*/
ER dsi_close(void)
{
	if (!b_dsi_opened) {
		return E_OK;
	}

	dsi_set_phy_enable(FALSE);

#ifdef __KERNEL__

	pll_disableClock(MIPI_DSI_CLK);

	//nvt_enable_sram_shutdown(MIPI_SD);
#else
#if defined(__FREERTOS)
	// Disable interrupt
	free_irq(INT_ID_DSI, 0);
#else
	// Disable interrupt
	drv_disableInt(DRV_INT_DSI);
#endif

	pll_disableClock(MIPI_DSI_CLK);

#if defined(__FREERTOS) && defined(_BSP_NA51055_)
	nvt_enable_sram_shutdown(DSI_SD);
#else
	//pinmux_enable_sram_shutdown(DSI_SD);
#endif
#endif

	b_dsi_opened = FALSE;
	return E_OK;
}


/**
    dsi is open or not

    Check if dsi is open or not

    @return
		- @b TRUE: open
		- @b FALSE: close

*/
BOOL dsi_is_opened(void)
{
	return b_dsi_opened;
}


/**
    dsi wait frame end

    Wait DSI frame end

    @return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
ER dsi_wait_frame_end(void)
{
#if 1
	T_DSI_INTSTS_REG	dsi_int_sts;
	int timeout = 0;
	
	do {
		dsi_int_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
		Delay_DelayMs(1);
		timeout++;
		if (timeout > 1000) {
			printf("dsi_wait_frame_end wait timeout\r\n");
			break;
		}
	} while(((dsi_int_sts.reg & DSI_FRM_END_STS)  == 0));
	
	if (dsi_int_sts.reg != 0) { 
		DSI_SETREG(DSI_INTSTS_REG_OFS, dsi_int_sts.reg);
	}
#else
	FLGPTN ui_flag;

	while (1) {
		ui_dsi_int_sts &=  ~(DSI_FRM_END_STS);
		clr_flg(FLG_ID_DSI, FLGPTN_DSI_FRM_END);
		wai_flg(&ui_flag, FLG_ID_DSI, FLGPTN_DSI_FRM_END | FLGPTN_DSI_INT_STS, TWF_ORW | TWF_CLR);
		if (ui_dsi_int_sts & (DSI_FRM_END_STS)) {
			break;
		}
	}
#endif
	return E_OK;
}


/**
    dsi wait RX ready

    Wait DSI RX ready

    @return
            - @b E_SYS      Status fail
            - @b E_OK       Operation success

*/
ER dsi_wait_RX_ready(void)
{
#if 1
	T_DSI_INTSTS_REG	dsi_int_sts;
	int timeout = 0;
	
	do {
		dsi_int_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
		Delay_DelayMs(1);
		timeout++;
		if (timeout > 1000) {
			printf("dsi_wait_RX_ready wait timeout\r\n");
			break;
		}
	} while(((dsi_int_sts.reg & DSI_RX_READY_STS)  == 0));
	
	if (dsi_int_sts.reg != 0) { 
		DSI_SETREG(DSI_INTSTS_REG_OFS, dsi_int_sts.reg);
	}
#else

    FLGPTN uiFlag;
    //while (1)
    //{
    ui_dsi_int_sts &=  ~(DSI_RX_READY_STS);
    clr_flg(FLG_ID_DSI, FLGPTN_DSI_RX_RDY);
	DBG_IND("dsi_waitRXReady\r\n");
    wai_flg(&uiFlag, FLG_ID_DSI, FLGPTN_DSI_RX_RDY, TWF_ORW | TWF_CLR);
	DBG_IND("dsi_waitRXReady done\r\n");
    //}
#endif
    return E_OK;
}


/**
    dsi issue bus turn arround

    dsi issue bus turn arround protocal

    @return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
ER dsi_issue_bta(void)
{
	DSI_CMD_RW_CTRL_PARAM   dcs_context;
	DSI_CMD_CTRL_PARAM      dcs_ctrl_context;

	dcs_context.b_bta_en = TRUE;
	dcs_context.b_bta_only = TRUE;
	dcs_context.ui_cmd_no = 1;
	dcs_context.ui_sram_ofs = 0x80;
	dcs_context.b_eot_en = 0;

	dcs_ctrl_context.ui_data_type = DATA_TYPE_SHORT_READ_NO_PARAM;
	dcs_ctrl_context.ui_packet_type = DSI_SHORT_PACKET;
	dcs_ctrl_context.ui_virtual_channel = 0;
	dcs_ctrl_context.ui_dcs_cmd = 0x0C;
	dcs_ctrl_context.ui_param = NULL;
	dcs_ctrl_context.ui_param_cnt = 0;

	dcs_context.p_dsi_cmd_ctx = &dcs_ctrl_context;
	return dsi_set_hs_dcs_command(&dcs_context);
}


/**
    dsi_get_error_report

    Wait DSI frame end

    @return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
UINT32 dsi_get_error_report(void)
{
	return DSI_GETREG(DSI_ERREPORT_REG_OFS);
}



/**
     DSI set TX enable

     Enable / disable of the DSI module for HS transmission

     @param[in]     b_en
		- @b TRUE   : Enable HS transmission
		- @b FALSE  : Disable HS transmission

     @param[in]     b_wait  wait TX done or not

     @return
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
ER dsi_set_tx_en(BOOL b_en, BOOL b_wait)
{
	T_DSI_EN_REG	reg_enable;
	ER				ret;

	ret = E_OK;
	reg_enable.reg = DSI_GETREG(DSI_EN_REG_OFS);

	if (dsi_get_mode() == DSI_MODE_MANUAL_MODE) {
		if (!b_en) {
			DBG_ERR("dsi_set_tx_en in manual mode useless operation %d\r\n", b_en);
			//return E_SYS;
			ret = E_SYS;
			goto dsi_set_tx_en_exit;
		}
		if (reg_enable.bit.TX_EN) {
			DBG_ERR("dsi_set_tx_en in manual mode under transmission\r\n");
			//return E_SYS;
			ret = E_SYS;
			goto dsi_set_tx_en_exit;
		} else {
			reg_enable.bit.TX_EN = b_en;
		}
		DSI_SETREG(DSI_EN_REG_OFS, reg_enable.reg);
	} else {
		if (reg_enable.bit.TX_EN == b_en) {
			if (reg_enable.bit.TX_EN == TRUE) {
				DBG_ERR("dsi_set_tx_en is under transmission\r\n");
			} else {
				DBG_ERR("dsi_set_tx_en in useless operation %d\r\n", b_en);
			}
			//return E_SYS;
			ret = E_SYS;
			goto dsi_set_tx_en_exit;
		} else {
			reg_enable.bit.TX_EN = b_en;
			DSI_SETREG(DSI_EN_REG_OFS, reg_enable.reg);
			if (b_en == FALSE && b_wait == TRUE) {
				dsi_wait_tx_done();
			}
		}
	}

dsi_set_tx_en_exit:

	return ret;
}


/**
    DSI set escape operation control

    Enable / disable of the DSI module for LP transmission

    @param[in]     ui_data_lane
		- @b DSI_DATA_LANE_0    : Data lane 0's low power transmission
		- @b DSI_DATA_LANE_1    : Data lane 1's low power transmission

    @param[in]     ui_esc_op
		- @b DSI_ESCAPE_TRIGGER : Escape command trigger
		- @b DSI_ESCAPE_START   : Escape command start
		- @b DSI_ESCAPE_STOP    : Escape command stop

    @param[in]     b_en
		- @b TRUE   : Enable
		- @b FALSE  : Disable

    @return
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
ER dsi_set_escape_control(DSI_LANESEL ui_data_lane, DSI_ESC_OP ui_esc_op, BOOL b_en)
{
	T_DSI_EN_REG        reg_enable;
	T_DSI_ESCCTRL0_REG  reg_esc_ctrl;
	UINT32              esc_op = ui_esc_op;

	reg_enable.reg = DSI_GETREG(DSI_EN_REG_OFS);
	reg_esc_ctrl.reg = DSI_GETREG(DSI_ESCCTRL0_REG_OFS);


	if (dsi_get_mode() == DSI_MODE_MANUAL_MODE) {
		switch (esc_op) {
		case DSI_ESCAPE_TRIGGER: {
				if (reg_enable.bit.TX_EN == 1) {
					DBG_ERR("Lower power mode but TXEN = 1 ==> illegal\r\n");
					return E_SYS;
				}
				DSI_ESC_SET_TRIGGER(ui_data_lane, reg_enable, b_en);
			}
			break;

		case DSI_ESCAPE_START:
			DSI_ESC_SET_START(ui_data_lane, reg_esc_ctrl, b_en);
			break;

		case DSI_ESCAPE_STOP:
			DSI_ESC_SET_STOP(ui_data_lane, reg_esc_ctrl, b_en);
			break;
		}
	} else {
		//TODO
	}
	DSI_SETREG(DSI_ESCCTRL0_REG_OFS, reg_esc_ctrl.reg);

	DSI_SETREG(DSI_EN_REG_OFS, reg_enable.reg);

	return E_OK;
}


/**
     DSI control related configuration.

     DSI control related configuration.
     @note DSI_CONFIG_ID

     @param[in]     cfg_id   Configure control event, define in DSI_CONFIG_ID
     @param[in,out] ui_config in/out type depend on specific event
     @return Description of data returned.
		- @b E_OK   : set config operation success
		- @b E_SYS  : set config operation fail
*/

ER dsi_set_config(DSI_CONFIG_ID cfg_id, UINT32 ui_config)
{
	T_DSI_CTRL0_REG         dsi_ctrl_reg;
	T_DSI_BUS_TIME0_REG     dsi_bus_timing_ctrl_reg0;
	T_DSI_BUS_TIME1_REG     dsi_bus_timing_ctrl_reg1;
	T_DSI_BUS_TIME2_REG     dsi_bus_timing_ctrl_reg2;
	T_DSI_BTA_CTRL_REG      dsi_bta_ctrl_reg;
	T_DSI_PIXTYPE_REG       dsi_pixel_type_reg;
	T_DSI_HORI_CTRL0_REG    dsi_hor_ctrl0_reg;
	T_DSI_HORI_CTRL1_REG    dsi_hor_ctrl1_reg;
	T_DSI_SYNCEVT_CTRL_REG  dsi_sync_ctrl_reg;

	T_DSI_VERT_CTRL0_REG    dsi_vert_ctrl_reg0;
	T_DSI_VERT_CTRL1_REG    dsi_vert_ctrl_reg1;

	T_DSI_HORI_CTRL2_REG    dsi_hor_ctrl2_reg;
	T_DSI_SYNC_CTRL_REG     dsi_sync_ctrl_reg2;

	T_DSI_IDEHORT_CTRL0_REG dsi_ide_hort_ctrl_reg0;

	T_DSI_DEBUG3_REG        dsi_debug3_reg;
	T_DSI_DEBUG4_REG        dsi_debug4_reg;

#ifdef __KERNEL__
	//struct clk *dsipll_clk;
#endif

	/*
	#ifdef _FPGA_EMULATION_
	    FLOAT dsi_src_clk = 24.0;
	#else
	    FLOAT dsi_src_clk = 480.0;
	#endif
	    UINT32 div;
	*/

	if (cfg_id <= DSI_CONFIG_ID_SRC) {
		dsi_ctrl_reg.reg = DSI_GETREG(DSI_CTRL0_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_TLPX && cfg_id <= DSI_CONFIG_ID_BTA_TA_GET) {
		dsi_bus_timing_ctrl_reg0.reg = DSI_GETREG(DSI_BUS_TIME0_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_THS_PREPARE && cfg_id <= DSI_CONFIG_ID_TWAKEUP) {
		dsi_bus_timing_ctrl_reg1.reg = DSI_GETREG(DSI_BUS_TIME1_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_TCLK_PREPARE && cfg_id <= DSI_CONFIG_ID_TCLK_TRAIL) {
		dsi_bus_timing_ctrl_reg2.reg = DSI_GETREG(DSI_BUS_TIME2_REG_OFS);
	} else if ((cfg_id >= DSI_CONFIG_ID_BTA_TMOUT_VAL && cfg_id <= DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL)) {
		dsi_bta_ctrl_reg.reg = DSI_GETREG(DSI_BTA_CTRL_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_PIXPKT_PH_DT && cfg_id <= DSI_CONFIG_ID_DCS_CT1) {
		dsi_pixel_type_reg.reg = DSI_GETREG(DSI_PIXTYPE_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_VSA && cfg_id <= DSI_CONFIG_ID_VTOTAL) {
		dsi_vert_ctrl_reg0.reg = DSI_GETREG(DSI_VERT_CTRL0_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_VVALID_START && cfg_id <= DSI_CONFIG_ID_VVALID_END) {
		dsi_vert_ctrl_reg1.reg = DSI_GETREG(DSI_VERT_CTRL1_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_HSA && cfg_id <= DSI_CONFIG_ID_BLLP) {
		dsi_hor_ctrl0_reg.reg = DSI_GETREG(DSI_HORI_CTRL0_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_SYNCEVT_SLICE_NO && cfg_id <= DSI_CONFIG_ID_SYNCEVT_NULL_LEN) {
		dsi_sync_ctrl_reg.reg = DSI_GETREG(DSI_SYNCEVT_CTRL_REG_OFS);
	} else if (cfg_id == DSI_CONFIG_ID_HACT) {
		dsi_hor_ctrl2_reg.reg = 0;
		dsi_hor_ctrl2_reg.bit.HACT = ui_config;
		DSI_SETREG(DSI_HORI_CTRL2_REG_OFS, dsi_hor_ctrl2_reg.reg);
		return E_OK;
	} else if (cfg_id >= DSI_CONFIG_ID_HBP && cfg_id <= DSI_CONFIG_ID_HFP) {
		dsi_hor_ctrl1_reg.reg = DSI_GETREG(DSI_HORI_CTRL1_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_SYNC_POL && cfg_id <= DSI_CONFIG_ID_TE_BTA_INTERVAL) {
		dsi_sync_ctrl_reg2.reg = DSI_GETREG(DSI_SYNC_CTRL_REG_OFS);
	} else if (cfg_id >= DSI_CONFIG_ID_CLK_PHASE_OFS && cfg_id <= DSI_CONFIG_ID_PHASE_DELAY_ENABLE_OFS) {
		dsi_debug3_reg.reg = DSI_GETREG(DSI_DEBUG3_REG_OFS);
	} else if (cfg_id == DSI_CONFIG_ID_IDEHVALID) {
		dsi_ide_hort_ctrl_reg0.reg = DSI_GETREG(DSI_IDEHORT_CTRL0_REG_OFS);
	} else if ((cfg_id >= DSI_CONFIG_ID_LANSEL_D0) && (cfg_id <= DSI_CONFIG_ID_LANSEL_D3)) {
		dsi_debug3_reg.reg = DSI_GETREG(DSI_DEBUG3_REG_OFS);
	} else if (cfg_id == DSI_CONFIG_ID_FREQ) {

	} else if (cfg_id == DSI_CONFIG_ID_LPFREQ) {

	} else if (cfg_id == DSI_CONFIG_ID_PHY_LP_RX_DAT0) {
		dsi_debug4_reg.reg = DSI_GETREG(DSI_DEBUG4_REG_OFS);
	} else {
		DBG_ERR("dsi_set_config error unknow configure event[%d]\r\n", cfg_id);
		return E_NOSPT;
	}
	switch (cfg_id) {
	case DSI_CONFIG_ID_FREQ: {
#if 0
#ifdef __KERNEL__
			dsipll_clk = clk_get(NULL, "pll11");
			clk_prepare(dsipll_clk);
			//clk_disable(dsipll_clk);
			clk_set_rate(dsipll_clk, ui_config); //unit: Hz
			clk_enable(dsipll_clk);
			clk_put(dsipll_clk);
#else
			/*div = (UINT32) (dsi_src_clk/(ui_config/1000000));
			if(div == 0)
			{
			    div++;
			}*/
			pll_setPLLEn(PLL_ID_11, FALSE);
			pll_setPLL(PLL_ID_11, (ui_config / 1000000) / 12 * 131072);
			pll_setPLLEn(PLL_ID_11, TRUE);
			DBG_ERR("PLL11 = 0x%x\r\n", (unsigned int)(ui_config / 1000000) / 12 * 131072);
#endif
#else
			dsi_platform_clk_set_freq(ui_config);
#endif

			return E_OK;

		}
		break;

	case DSI_CONFIG_ID_DATALANE_NO:
		if (ui_config > DSI_DATA_LANE_CNT) {
			DBG_ERR("dsi_set_config error lane number exceed [%d]\r\n", (int)ui_config);
			return E_SYS;
		}
		dsi_ctrl_reg.bit.DATA_LANE_NO = ui_config;
		break;

	case DSI_CONFIG_ID_MODE: {
			if (ui_config >= DSI_MODE_CNT) {
				DBG_ERR("dsi_set_config error unknow mode[%d]\r\n", (int)ui_config);
				return E_SYS;
			}
			dsi_ctrl_reg.bit.MODE = ui_config;
		}
		break;

	case DSI_CONFIG_ID_PIXEL_FMT: {
			if (ui_config >= DSI_PIXEL_FMT_CNT) {
				DBG_ERR("dsi_set_config error unknow pixel mode [%d]\r\n", (int)ui_config);
				return E_SYS;
			}
			dsi_ctrl_reg.bit.PIXEL_FMT = ui_config;
		}
		break;

	case DSI_CONFIG_ID_PIXPKT_MODE: {
			if (ui_config >= DSI_PIXMODE_CNT) {
				DBG_ERR("dsi_set_config error unknow packet mode[%d]\r\n", (int)ui_config);
				return E_SYS;
			}
			dsi_ctrl_reg.bit.PIXPKT_MODE = ui_config;
		}
		break;

	case DSI_CONFIG_ID_FRMEND_BTA_EN: {
			dsi_ctrl_reg.bit.FRM_END_BTA_EN = ui_config;
		}
		break;

	case DSI_CONFIG_ID_VDOPKT_TYPE: {
			dsi_ctrl_reg.bit.VDOPKT_TYPE = ui_config;
		}
		break;

	case DSI_CONFIG_ID_EOT_PKT_EN: {
			dsi_ctrl_reg.bit.EOT_PKT_EN = ui_config;
		}
		break;

	case DSI_CONFIG_ID_BLANK_CTRL:
		dsi_ctrl_reg.bit.BLANK_CTRL = ui_config;
		break;

	case DSI_CONFIG_ID_INTER_PKT_LP:
		dsi_ctrl_reg.bit.INTER_PKT_LP = ui_config;
		break;

	case DSI_CONFIG_ID_CLK_LP_CTRL:
		dsi_ctrl_reg.bit.CLK_LP_CTRL = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_EN:
		dsi_ctrl_reg.bit.SYNC_EN = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_SRC:
		dsi_ctrl_reg.bit.SYNC_SRC = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_WITH_SETTEON:
		dsi_ctrl_reg.bit.SYNC_WITH_SETTEON = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_WITH_SETTEON_RTY:
		dsi_ctrl_reg.bit.SYNC_WITH_SETTEON_RTY = ui_config;
		break;

	case DSI_CONFIG_ID_RGB_SWAP:
		dsi_ctrl_reg.bit.RGB_SWAP = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_WITH_SETTEON_RTY_TWICEBTA:
		dsi_ctrl_reg.bit.SYNC_WITH_SETTEON_RTY_TWICEBTA = ui_config;
		break;

	case DSI_CONFIG_ID_SRC:
		dsi_ctrl_reg.bit.DSI_SRC_SEL = ui_config;
		break;

	case DSI_CONFIG_ID_VSA: {
			dsi_vert_ctrl_reg0.bit.VSA = ui_config;
		}
		break;

	case DSI_CONFIG_ID_VTOTAL: {
			dsi_vert_ctrl_reg0.bit.VTOTAL = ui_config;
		}
		break;

	case DSI_CONFIG_ID_HSA:
		dsi_hor_ctrl0_reg.bit.HSA = ui_config;
		break;

	case DSI_CONFIG_ID_BLLP:
		dsi_hor_ctrl0_reg.bit.BLLP = ui_config;
		break;

	case DSI_CONFIG_ID_HBP: {
			dsi_hor_ctrl1_reg.bit.HBP = ui_config;
		}
		break;

	case DSI_CONFIG_ID_HFP: {
			dsi_hor_ctrl1_reg.bit.HFP = ui_config;
		}
		break;

	case DSI_CONFIG_ID_VVALID_START:
		dsi_vert_ctrl_reg1.bit.VVALID_START = ui_config;
		break;

	case DSI_CONFIG_ID_VVALID_END:
		dsi_vert_ctrl_reg1.bit.VVALID_END = ui_config;
		break;

	case DSI_CONFIG_ID_PIXPKT_PH_DT:
		dsi_pixel_type_reg.bit.PIXPKT_PH_DT = ui_config;
		break;

	case DSI_CONFIG_ID_PIXPKT_PH_VC:
		dsi_pixel_type_reg.bit.PIXPKT_PH_VC = ui_config;
		break;

	case DSI_CONFIG_ID_DCS_CT0:
		dsi_pixel_type_reg.bit.DCS_CT0 = ui_config;
		break;

	case DSI_CONFIG_ID_DCS_CT1:
		dsi_pixel_type_reg.bit.DCS_CT1 = ui_config;
		break;

	case DSI_CONFIG_ID_SYNCEVT_NULL_LEN:
		dsi_sync_ctrl_reg.bit.NULL_LEN = ui_config;
		break;

	case DSI_CONFIG_ID_SYNCEVT_SLICE_NO:
		dsi_sync_ctrl_reg.bit.SLICE_NO = ui_config;
		break;

	case DSI_CONFIG_ID_TLPX: {
			if (ui_config > 0xF) {
				DBG_WRN("DSI_CONFIG_ID_TLPX value exceed 0xF, cast to 0xF\r\n");
			}
			dsi_bus_timing_ctrl_reg0.bit.TLPX = ui_config;
		}
		break;

	case DSI_CONFIG_ID_BTA_TA_GO: {
			if (ui_config > 0x3F) {
				DBG_WRN("DSI_CONFIG_ID_BTA_TA_GO value exceed 0x3F, cast to 0x3F\r\n");
			}
			dsi_bus_timing_ctrl_reg0.bit.BTA_TA_GO = ui_config;
		}
		break;

	case DSI_CONFIG_ID_BTA_TA_SURE: {
			if (ui_config > 0x1F) {
				DBG_WRN("DSI_CONFIG_ID_BTA_TA_SURE value exceed 0x1F, cast to 0x1F\r\n");
			}
			dsi_bus_timing_ctrl_reg0.bit.BTA_TA_SURE = ui_config;
		}
		break;

	case DSI_CONFIG_ID_BTA_TA_GET: {
			if (ui_config > 0x7F) {
				DBG_WRN("DSI_CONFIG_ID_BTA_TA_GET value exceed 0x7F, cast to 0x7F\r\n");
			}
			dsi_bus_timing_ctrl_reg0.bit.BTA_TA_GET = ui_config;
		}
		break;

	case DSI_CONFIG_ID_THS_PREPARE:
		dsi_bus_timing_ctrl_reg1.bit.THS_PREPARE = ui_config;
		break;

	case DSI_CONFIG_ID_THS_ZERO:
		dsi_bus_timing_ctrl_reg1.bit.THS_ZERO = ui_config;
		break;

	case DSI_CONFIG_ID_THS_TRAIL:
		dsi_bus_timing_ctrl_reg1.bit.THS_TRAIL = ui_config;
		break;

	case DSI_CONFIG_ID_THS_EXIT:
		dsi_bus_timing_ctrl_reg1.bit.THS_EXIT = ui_config;
		break;

	case DSI_CONFIG_ID_TWAKEUP:
		dsi_bus_timing_ctrl_reg1.bit.TWAKEUP = ui_config;
		break;

	case DSI_CONFIG_ID_TCLK_PREPARE:
		dsi_bus_timing_ctrl_reg2.bit.TCLK_PREPARE = ui_config;
		break;

	case DSI_CONFIG_ID_TCLK_ZERO:
		dsi_bus_timing_ctrl_reg2.bit.TCLK_ZERO = ui_config;
		break;

	case DSI_CONFIG_ID_TCLK_POST:
		dsi_bus_timing_ctrl_reg2.bit.TCLK_POST = ui_config;
		break;

	case DSI_CONFIG_ID_TCLK_PRE:
		dsi_bus_timing_ctrl_reg2.bit.TCLK_PRE = ui_config;
		break;

	case DSI_CONFIG_ID_TCLK_TRAIL:
		dsi_bus_timing_ctrl_reg2.bit.TCLK_TRAIL = ui_config;
		break;

	case DSI_CONFIG_ID_BTA_TMOUT_VAL:
		dsi_bta_ctrl_reg.bit.BTA_TIMEOUT_VAL = ui_config;
		break;

	case DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL:
		dsi_bta_ctrl_reg.bit.BTA_HANDSK_TIMEOUT = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_POL:
		dsi_sync_ctrl_reg2.bit.SYNC_POL = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_SEL:
		dsi_sync_ctrl_reg2.bit.SYNC_SEL = ui_config;
		break;

	case DSI_CONFIG_ID_SYNC_DLY_CNT:
		dsi_sync_ctrl_reg2.bit.SYNC_DLY_CLK = ui_config;
		break;

	case DSI_CONFIG_ID_TE_BTA_INTERVAL:
		dsi_sync_ctrl_reg2.bit.TE_BTA_INTERVAL = ui_config;
		break;

	case DSI_CONFIG_ID_CLK_PHASE_OFS:
		dsi_debug3_reg.bit.CLK_PHASE_OFS = ui_config;
		break;

	case DSI_CONFIG_ID_DAT0_PHASE_OFS:
		dsi_debug3_reg.bit.DAT0_PHASE_OFS = ui_config;
		break;

	case DSI_CONFIG_ID_DAT1_PHASE_OFS:
		dsi_debug3_reg.bit.DAT1_PHASE_OFS = ui_config;
		break;

	case DSI_CONFIG_ID_DAT2_PHASE_OFS:
		dsi_debug3_reg.bit.DAT2_PHASE_OFS = ui_config;
		break;

	case DSI_CONFIG_ID_DAT3_PHASE_OFS:
		dsi_debug3_reg.bit.DAT3_PHASE_OFS = ui_config;
		break;

	case DSI_CONFIG_ID_PHASE_DELAY_ENABLE_OFS:
		dsi_debug3_reg.bit.PHY_DELAY_EN = ui_config;
		break;

	case DSI_CONFIG_ID_IDEHVALID:
		dsi_ide_hort_ctrl_reg0.bit.HVLD = ui_config;
		break;

	case DSI_CONFIG_ID_LANSEL_D0:
		dsi_debug3_reg.bit.DAT0_LAND_SEL = ui_config;
		break;

	case DSI_CONFIG_ID_LANSEL_D1:
		dsi_debug3_reg.bit.DAT1_LAND_SEL = ui_config;
		break;

	case DSI_CONFIG_ID_LANSEL_D2:
		dsi_debug3_reg.bit.DAT2_LAND_SEL = ui_config;
		break;

	case DSI_CONFIG_ID_LANSEL_D3:
		dsi_debug3_reg.bit.DAT3_LAND_SEL = ui_config;
		break;

	case DSI_CONFIG_ID_PHY_LP_RX_DAT0:
		dsi_debug4_reg.bit.PHY_LP_RX_DAT0_EN = ui_config;
		break;

	default:
		DBG_ERR("dsi_set_config error unknow configure event[%d]\r\n", (int)cfg_id);
		return E_SYS;
		//break;
	}

	if (cfg_id <= DSI_CONFIG_ID_SRC) {
		DSI_SETREG(DSI_CTRL0_REG_OFS, dsi_ctrl_reg.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_TLPX && cfg_id <= DSI_CONFIG_ID_BTA_TA_GET) {
		DSI_SETREG(DSI_BUS_TIME0_REG_OFS, dsi_bus_timing_ctrl_reg0.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_THS_PREPARE && cfg_id <= DSI_CONFIG_ID_TWAKEUP) {
		DSI_SETREG(DSI_BUS_TIME1_REG_OFS, dsi_bus_timing_ctrl_reg1.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_TCLK_PREPARE && cfg_id <= DSI_CONFIG_ID_TCLK_TRAIL) {
		DSI_SETREG(DSI_BUS_TIME2_REG_OFS, dsi_bus_timing_ctrl_reg2.reg);
	} else if ((cfg_id >= DSI_CONFIG_ID_BTA_TMOUT_VAL && cfg_id <= DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL)) {
		DSI_SETREG(DSI_BTA_CTRL_REG_OFS, dsi_bta_ctrl_reg.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_PIXPKT_PH_DT && cfg_id <= DSI_CONFIG_ID_DCS_CT1) {
		DSI_SETREG(DSI_PIXTYPE_REG_OFS, dsi_pixel_type_reg.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_HSA && cfg_id <= DSI_CONFIG_ID_BLLP) {
		DSI_SETREG(DSI_HORI_CTRL0_REG_OFS, dsi_hor_ctrl0_reg.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_HBP && cfg_id <= DSI_CONFIG_ID_HFP) {
		DSI_SETREG(DSI_HORI_CTRL1_REG_OFS, dsi_hor_ctrl1_reg.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_VSA && cfg_id <= DSI_CONFIG_ID_VTOTAL) {
		DSI_SETREG(DSI_VERT_CTRL0_REG_OFS, dsi_vert_ctrl_reg0.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_VVALID_START && cfg_id <= DSI_CONFIG_ID_VVALID_END) {
		DSI_SETREG(DSI_VERT_CTRL1_REG_OFS, dsi_vert_ctrl_reg1.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_SYNCEVT_SLICE_NO && cfg_id <= DSI_CONFIG_ID_SYNCEVT_NULL_LEN) {
		DSI_SETREG(DSI_SYNCEVT_CTRL_REG_OFS, dsi_sync_ctrl_reg.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_SYNC_POL && cfg_id <= DSI_CONFIG_ID_TE_BTA_INTERVAL) {
		DSI_SETREG(DSI_SYNC_CTRL_REG_OFS, dsi_sync_ctrl_reg2.reg);
	} else if (cfg_id >= DSI_CONFIG_ID_CLK_PHASE_OFS && cfg_id <= DSI_CONFIG_ID_PHASE_DELAY_ENABLE_OFS) {
		DSI_SETREG(DSI_DEBUG3_REG_OFS, dsi_debug3_reg.reg);
	} else if (cfg_id == DSI_CONFIG_ID_IDEHVALID) {
		DSI_SETREG(DSI_IDEHORT_CTRL0_REG_OFS, dsi_ide_hort_ctrl_reg0.reg);
	} else if ((cfg_id >= DSI_CONFIG_ID_LANSEL_D0) && (cfg_id <= DSI_CONFIG_ID_LANSEL_D3)) {
		DSI_SETREG(DSI_DEBUG3_REG_OFS, dsi_debug3_reg.reg);
	} else if (cfg_id == DSI_CONFIG_ID_PHY_LP_RX_DAT0) {
		DSI_SETREG(DSI_DEBUG4_REG_OFS, dsi_debug4_reg.reg);
	}
	/*else if(cfg_id == DSI_CONFIG_ID_FREQ)
	{
	    pll_setPLL(PLL_ID_11, ui_config/1000000*131072/12);
	}*/
	else {
		DBG_ERR("dsi_set_config error unknow configure event[%d]\r\n", (int)cfg_id);
		return E_NOSPT;
	}
	return E_OK;
}



/**
    Get dsi configuration

    Get dsi configuration.

    @param[in] config_id     config ID

    @return Value of the specific configuration ID
*/
UINT32 dsi_get_config(DSI_CONFIG_ID config_id)
{
	T_DSI_CTRL0_REG         dsi_ctrl_reg;
	T_DSI_BUS_TIME0_REG     dsi_bus_timing_ctrl_reg0;
	T_DSI_BUS_TIME1_REG     dsi_bus_timing_ctrl_reg1;
	T_DSI_BUS_TIME2_REG     dsi_bus_timing_ctrl_reg2;
	T_DSI_BTA_CTRL_REG      dsi_bta_ctrl_reg;
	T_DSI_DEBUG3_REG        dsi_debug3_reg;
	T_DSI_DEBUG2_REG        dsi_debug2_reg;
	T_DSI_DEBUG4_REG        dsi_debug4_reg;
#ifdef __KERNEL__
	//struct clk *parent;
	//struct clk *dsipll_clk;
	//struct clk *dsilp_clk;
	//unsigned long parent_rate = 0;
	//unsigned long dsi_src_clk;
#else
//#ifdef _FPGA_EMULATION_
#if defined(_NVT_FPGA_)
	//FLOAT dsi_src_clk; //dsi_src_clk = 24.0
#else
	//FLOAT dsi_src_clk; //dsi_src_clk = 480.0
#endif
#endif


	//UINT32 div;

	if (config_id >= DSI_CONFIG_ID_TLPX && config_id <= DSI_CONFIG_ID_BTA_TA_GET) {
		dsi_bus_timing_ctrl_reg0.reg = DSI_GETREG(DSI_BUS_TIME0_REG_OFS);
	} else if (config_id >= DSI_CONFIG_ID_THS_PREPARE && config_id <= DSI_CONFIG_ID_TWAKEUP) {
		dsi_bus_timing_ctrl_reg1.reg = DSI_GETREG(DSI_BUS_TIME1_REG_OFS);
	} else if (config_id >= DSI_CONFIG_ID_TCLK_PREPARE && config_id <= DSI_CONFIG_ID_TCLK_TRAIL) {
		dsi_bus_timing_ctrl_reg2.reg = DSI_GETREG(DSI_BUS_TIME2_REG_OFS);
	} else if ((config_id >= DSI_CONFIG_ID_BTA_TMOUT_VAL && config_id <= DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL)) {
		dsi_bta_ctrl_reg.reg = DSI_GETREG(DSI_BTA_CTRL_REG_OFS);
	} else if (config_id >= DSI_CONFIG_ID_CLK_PHASE_OFS && config_id <= DSI_CONFIG_ID_PHASE_DELAY_ENABLE_OFS) {
		dsi_debug3_reg.reg = DSI_GETREG(DSI_DEBUG3_REG_OFS);
	} else if (config_id >= DSI_CONFIG_ID_PHY_DRVING && config_id <= DSI_CONFIG_ID_PHY_DRVING) {
		dsi_debug2_reg.reg = DSI_GETREG(DSI_DEBUG2_REG_OFS);
	} else if (config_id == DSI_CONFIG_ID_PHY_LP_RX_DAT0) {
		dsi_debug4_reg.reg = DSI_GETREG(DSI_DEBUG4_REG_OFS);
	}

	switch (config_id) {
	case DSI_CONFIG_ID_FREQ:
#if 0
#ifdef __KERNEL__
		dsipll_clk = clk_get(NULL, "pll11");
		clk_prepare(dsipll_clk);
		dsi_src_clk = clk_get_rate(dsipll_clk);
		div = 1;
		return (dsi_src_clk / div);
#else
		dsi_src_clk = pll_getPLLFreq(PLL_ID_11);
		div = 1;
		return (dsi_src_clk / div);
#endif
#else
		return dsi_platform_clk_get_freq();
#endif
		//break;

	case DSI_CONFIG_ID_LPFREQ:
#if 0
#ifdef __KERNEL__
		dsilp_clk = clk_get(NULL, "f0840000.dsi");
		parent = clk_get_parent(dsilp_clk);
		parent_rate = clk_get_rate(parent);
		return parent_rate;
#else
		if (pll_getClockRate(PLL_CLKSEL_DSI_LPSRC)) {
			return 80000000;
		} else {
			return 60000000;
		}
#endif
#else
		return dsi_platform_clk_get_lp_freq();
#endif
		break;

	case DSI_CONFIG_ID_PIXPKT_MODE:
		dsi_ctrl_reg.reg = DSI_GETREG(DSI_CTRL0_REG_OFS);
		return dsi_ctrl_reg.bit.PIXPKT_MODE;
		//break;

	case DSI_CONFIG_ID_TLPX:
		return dsi_bus_timing_ctrl_reg0.bit.TLPX;
		//break;
	case DSI_CONFIG_ID_BTA_TA_GO:
		return dsi_bus_timing_ctrl_reg0.bit.BTA_TA_GO;
		//break;
	case DSI_CONFIG_ID_BTA_TA_SURE:
		return dsi_bus_timing_ctrl_reg0.bit.BTA_TA_SURE;
		//break;
	case DSI_CONFIG_ID_BTA_TA_GET:
		return dsi_bus_timing_ctrl_reg0.bit.BTA_TA_GET;
		//break;
	case DSI_CONFIG_ID_THS_PREPARE:
		return dsi_bus_timing_ctrl_reg1.bit.THS_PREPARE;
		//break;
	case DSI_CONFIG_ID_THS_ZERO:
		return dsi_bus_timing_ctrl_reg1.bit.THS_ZERO;
		//break;
	case DSI_CONFIG_ID_THS_TRAIL:
		return dsi_bus_timing_ctrl_reg1.bit.THS_TRAIL;
		//break;
	case DSI_CONFIG_ID_THS_EXIT:
		return dsi_bus_timing_ctrl_reg1.bit.THS_EXIT;
		//break;
	case DSI_CONFIG_ID_TWAKEUP:
		return dsi_bus_timing_ctrl_reg1.bit.TWAKEUP;
		//break;
	case DSI_CONFIG_ID_TCLK_PREPARE:
		return dsi_bus_timing_ctrl_reg2.bit.TCLK_PREPARE;
		//break;
	case DSI_CONFIG_ID_TCLK_ZERO:
		return dsi_bus_timing_ctrl_reg2.bit.TCLK_ZERO;
		//break;
	case DSI_CONFIG_ID_TCLK_POST:
		return dsi_bus_timing_ctrl_reg2.bit.TCLK_POST;
		//break;
	case DSI_CONFIG_ID_TCLK_PRE:
		return dsi_bus_timing_ctrl_reg2.bit.TCLK_PRE;
		//break;
	case DSI_CONFIG_ID_TCLK_TRAIL:
		return dsi_bus_timing_ctrl_reg2.bit.TCLK_TRAIL;
		//break;
	case DSI_CONFIG_ID_BTA_TMOUT_VAL:
		return dsi_bta_ctrl_reg.bit.BTA_TIMEOUT_VAL;
		//break;
	case DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL:
		return dsi_bta_ctrl_reg.bit.BTA_HANDSK_TIMEOUT;
		//break;
	case DSI_CONFIG_ID_CLK_PHASE_OFS:
		return dsi_debug3_reg.bit.CLK_PHASE_OFS;
		//break;
	case DSI_CONFIG_ID_DAT0_PHASE_OFS:
		return dsi_debug3_reg.bit.DAT0_PHASE_OFS;
		//break;
	case DSI_CONFIG_ID_DAT1_PHASE_OFS:
		return dsi_debug3_reg.bit.DAT1_PHASE_OFS;
		//break;
	case DSI_CONFIG_ID_DAT2_PHASE_OFS:
		return dsi_debug3_reg.bit.DAT2_PHASE_OFS;
		//break;
	case DSI_CONFIG_ID_DAT3_PHASE_OFS:
		return dsi_debug3_reg.bit.DAT3_PHASE_OFS;
		//break;
	case DSI_CONFIG_ID_PHASE_DELAY_ENABLE_OFS:
		return dsi_debug3_reg.bit.PHY_DELAY_EN;
		//break;
	case DSI_CONFIG_ID_PHY_DRVING:
		return dsi_debug2_reg.bit.PHY_DRVING;
		//break;

	case DSI_CONFIG_ID_PHY_LP_RX_DAT0:
		return dsi_debug4_reg.bit.PHY_LP_RX_DAT0_EN;
		//break;

	case DSI_CONFIG_ID_BTA_VALUE:
		{
//			T_DSI_INTSTS_REG	uiDsiIntStsCtrl;
    		T_DSI_CMDCTRL_REG   uiDsiCmdCtrl;
			UINT32  uiReadCnt, uiRdCnt;
//			UINT32	returnValue;

			uiDsiCmdCtrl.reg = DSI_GETREG(DSI_CMDCTRL_REG_OFS);
	        if(uiDsiCmdCtrl.bit.SRAM_RD_CNT > 0) {
	            DBG_IND("BTA : read count = %02d\r\n", uiDsiCmdCtrl.bit.SRAM_RD_CNT);

	            uiReadCnt = uiDsiCmdCtrl.bit.SRAM_RD_CNT / 4;
	            if(uiDsiCmdCtrl.bit.SRAM_RD_CNT % 4 != 0)
	                uiReadCnt++;

//				uiDsiIntStsCtrl.Reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
//				uiDsiIntStsCtrl.Bit.

	            for(uiRdCnt = 0; uiRdCnt < uiReadCnt; uiRdCnt++)
	            {
	                DBG_IND("=>[0x%08x]\r\n", DSI_GETREG(DSI_SRAM_REG_OFS + uiDsiCmdCtrl.bit.SRAM_RD_OFS + uiRdCnt * 4));
	            }
				return DSI_GETREG(DSI_SRAM_REG_OFS + uiDsiCmdCtrl.bit.SRAM_RD_OFS);
	        } else {
				return 0;
	        }
		}
		break;

	case DSI_CONFIG_ID_CHIP_VER:
#ifdef __KERNEL__
		return 0;
#else
		//return pinmux_getChipVersion();
		return 0;
#endif
		//break;
	default :
		DBG_ERR("config_id[%02d] not reconganized\r\n", (int)config_id);
		return 0;
	}
}



// -----------------------------------------------------------------------------
// 0x2C : DSI command RW control
// -----------------------------------------------------------------------------
/**
     DSI command RW control

     Configure DSI command r/w control

     @param[in]     ui_cmd   Emulation of DSI_CFG_CMD_RW_CTRL
     @param[in,out] param   Depend on specific command
     @return Description of data returned.
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
ER dsi_set_cmd_rw_ctrl(DSI_CFG_CMD_RW_CTRL ui_cmd, UINT32 param)
{
	T_DSI_CMDCTRL_REG   cmd_ctrl;

	cmd_ctrl.reg = DSI_GETREG(DSI_CMDCTRL_REG_OFS);

	switch (ui_cmd) {
	case DSI_SET_CMD_NUMBER:
		if (param > 8 || param < 1) {
			DBG_ERR("dsi_set_cmd_rw_ctrl error command number [%d]\r\n", (int)param);
			return E_SYS;
		}
		cmd_ctrl.bit.CMD_NO = param - 1;
		DSI_LOG_MSG("dsi_set_cmd_rw_ctrl[DSI_SET_CMD_NUMBER][%d]\r\n", (int)(param - 1));
		break;

	case DSI_SET_BTA_EN:
		if (param > 1) {
			DBG_ERR("dsi_set_cmd_rw_ctrl error BTA EN [%d]\r\n", (int)param);
			return E_SYS;
		}
		cmd_ctrl.bit.CMD_BTA_EN = (BOOL)param;
		DSI_LOG_MSG("dsi_set_cmd_rw_ctrl[DSI_SET_BTA_EN][%d]\r\n", (int)param);
		break;

	case DSI_SET_BTA_ONLY:
		cmd_ctrl.bit.CMD_BTA_ONLY = (BOOL)param;
		DSI_LOG_MSG("dsi_set_cmd_rw_ctrl[DSI_SET_BTA_ONLY][%d]\r\n", (int)param);
		break;
	case DSI_SET_SRAM_READ_OFS:
		//if(param > 1)
		//{
		//    DBG_ERR("dsi_set_cmd_rw_ctrl error set sram read ofs [%d]\r\n", (int)param);
		//    return E_SYS;
		//}
		cmd_ctrl.bit.SRAM_RD_OFS = param;
		DSI_LOG_MSG("dsi_set_cmd_rw_ctrl[DSI_SET_SRAM_READ_OFS][%d]\r\n", (int)param);
		break;

	case DSI_GET_SRAM_READ_CNT:
		*(UINT32 *)param = cmd_ctrl.bit.SRAM_RD_CNT;
		DSI_LOG_MSG("dsi_set_cmd_rw_ctrl[DSI_GET_SRAM_READ_CNT][%d]\r\n", *(UINT32 *)param);
		return E_OK;
		//break;

	default :
		DBG_ERR("dsi_set_cmd_rw_ctrl error command[%d]\r\n", (int)ui_cmd);
		return E_SYS;
		//break;
	}

	DSI_SETREG(DSI_CMDCTRL_REG_OFS, cmd_ctrl.reg);
	return E_OK;
}


// -----------------------------------------------------------------------------
// 0x30,0x34 : DSI command Register0,1
// -----------------------------------------------------------------------------
/**
     DSI command register0,1

     Configure DSI command register 0 and 1

     @param[in]     ui_cmd_reg_no  No. of command register set
     @param[in]     ui_cmd_reg    Emulation of DSI_CFG_CMD_REG
     @param[in,out] param       Depend on specific Emulation
     @return Description of data returned.
		- @b E_OK:     Success
		- @b E_SYS:    fail
*/
ER dsi_set_cmd_register(DSI_CMD_REG_NUM ui_cmd_reg_no, DSI_CFG_CMD_REG ui_cmd_reg, UINT32 param)
{
	UINT32          ui_reg_ofs;
	T_DSI_CMD0_REG  cmd_reg;

	ui_reg_ofs = ui_cmd_reg_no * 4;

	cmd_reg.reg = DSI_GETREG(DSI_CMD0_REG_OFS + ui_reg_ofs);

	switch (ui_cmd_reg) {
	case DSI_SET_CMD_DT:
		cmd_reg.bit.CMD0_DT = param;
		DSI_LOG_MSG("dsi_set_cmd_register[DSI_SET_CMD_DT][%d]\r\n", (int)param);
		break;

	case DSI_SET_CMD_VC:
		cmd_reg.bit.CMD0_VC = param;
		DSI_LOG_MSG("dsi_set_cmd_register[DSI_SET_CMD_VC][%d]\r\n", (int)param);
		break;

	case DSI_SET_CMD_WC:
		cmd_reg.bit.CMD0_WC = param;
		DSI_LOG_MSG("dsi_set_cmd_register[DSI_SET_CMD_WC(DATA if short pkt)][0x%08x]\r\n", (unsigned int)param);
		break;

	case DSI_SET_CMD_PT:
		cmd_reg.bit.CMD0_LPKT = param;
		DSI_LOG_MSG("dsi_set_cmd_register[DSI_SET_CMD_PT][%d]\r\n", (int)param);
		break;

	default:
		DBG_ERR("dsi_set_cmd_rw_ctrl error command[%d]\r\n", (int)ui_cmd_reg);
		return E_SYS;
		//break;
	}

	DSI_SETREG(DSI_CMD0_REG_OFS + ui_reg_ofs, cmd_reg.reg);
	return E_OK;
}


/**
     DSI set DCS command

     DSI set specific DCS command routing

     @param[in]     p_dcs_ctx     PDSI_DCS_CMD_PARAM

     @return Description of data returned.
		- @b E_OK   : set Data ID success
		- @b E_SYS  : set Data ID fail
*/
ER dsi_set_hs_dcs_command(PDSI_CMD_RW_CTRL_PARAM p_dcs_ctx)
{
	UINT32  ui_cmd_loop;
	UINT32  ui_long_pkt_data;
	UINT32  ui_word_data;
	UINT32  ui_param_cnt_no;
	UINT32  ui_read_cnt;
	T_DSI_CMDCTRL_REG   ui_dsi_cmd_ctrl;

	UINT32  ui_rd_cnt;

	PDSI_CMD_CTRL_PARAM p_tmp_dsi_cmd_ctx;

	dsi_set_cmd_rw_ctrl(DSI_SET_CMD_NUMBER, p_dcs_ctx->ui_cmd_no);

	dsi_set_cmd_rw_ctrl(DSI_SET_BTA_EN, p_dcs_ctx->b_bta_en);

	dsi_set_cmd_rw_ctrl(DSI_SET_SRAM_READ_OFS, p_dcs_ctx->ui_sram_ofs);

	if (dsi_set_config(DSI_CONFIG_ID_EOT_PKT_EN, p_dcs_ctx->b_eot_en) != E_OK) {
		DBG_ERR("dsi_set_hs_dcs_command error: DSI_CONFIG_ID_EOT_PKT_EN \r\n");
	}

	dsi_set_cmd_rw_ctrl(DSI_SET_BTA_ONLY, p_dcs_ctx->b_bta_only);

	for (ui_cmd_loop = 0; ui_cmd_loop < p_dcs_ctx->ui_cmd_no; ui_cmd_loop++) {
		p_tmp_dsi_cmd_ctx = (PDSI_CMD_CTRL_PARAM)&p_dcs_ctx->p_dsi_cmd_ctx[ui_cmd_loop];

		// Set Data Type as DCS short read
		dsi_set_cmd_register(ui_cmd_loop, DSI_SET_CMD_DT, p_tmp_dsi_cmd_ctx->ui_data_type);

		dsi_set_cmd_register(ui_cmd_loop, DSI_SET_CMD_VC, p_tmp_dsi_cmd_ctx->ui_virtual_channel);

		if (p_tmp_dsi_cmd_ctx->ui_packet_type == DSI_SHORT_PACKET) {
			if (p_tmp_dsi_cmd_ctx->ui_param_cnt == 0) {
				dsi_set_cmd_register(ui_cmd_loop, DSI_SET_CMD_DATA, p_tmp_dsi_cmd_ctx->ui_dcs_cmd);
			} else if (p_tmp_dsi_cmd_ctx->ui_param_cnt == 1) {
				dsi_set_cmd_register(ui_cmd_loop, DSI_SET_CMD_DATA, (p_tmp_dsi_cmd_ctx->ui_dcs_cmd & 0xFF) | ((p_tmp_dsi_cmd_ctx->ui_param[0]) << 8));
			} else {
				DBG_ERR("dsi_setDcsCommand error short pkt but exceed 1 param[%d]\r\n", (int)(p_tmp_dsi_cmd_ctx->ui_param_cnt));
			}

		} else {
			dsi_set_cmd_register(ui_cmd_loop, DSI_SET_CMD_WC, (p_tmp_dsi_cmd_ctx->ui_param_cnt + 1));

			ui_word_data = p_tmp_dsi_cmd_ctx->ui_dcs_cmd;

			ui_param_cnt_no = 0;
			if (p_tmp_dsi_cmd_ctx->ui_param_cnt <= 3) {
				for (ui_long_pkt_data = 1; ui_long_pkt_data <= p_tmp_dsi_cmd_ctx->ui_param_cnt; ui_long_pkt_data++) {
					ui_word_data |= ((p_tmp_dsi_cmd_ctx->ui_param[ui_param_cnt_no]) << (8 * ui_long_pkt_data));
					ui_param_cnt_no++;
				}
				DSI_SETREG(DSI_SRAM_REG_OFS, ui_word_data);
			} else {
				ui_word_data |= ((p_tmp_dsi_cmd_ctx->ui_param[0] << 8) | (p_tmp_dsi_cmd_ctx->ui_param[1] << 16) | (p_tmp_dsi_cmd_ctx->ui_param[2] << 24));

				DSI_SETREG(DSI_SRAM_REG_OFS, ui_word_data);

				ui_param_cnt_no = 4;
				for (ui_long_pkt_data = 3; ui_long_pkt_data <= p_tmp_dsi_cmd_ctx->ui_param_cnt; ui_long_pkt_data += 4) {
					ui_word_data = 0;
					ui_word_data |= ((p_tmp_dsi_cmd_ctx->ui_param[ui_long_pkt_data]) | (p_tmp_dsi_cmd_ctx->ui_param[ui_long_pkt_data + 1] << 8) | (p_tmp_dsi_cmd_ctx->ui_param[ui_long_pkt_data + 2] << 16) | (p_tmp_dsi_cmd_ctx->ui_param[ui_long_pkt_data + 3] << 24));
					DSI_SETREG(DSI_SRAM_REG_OFS + ui_param_cnt_no, ui_word_data);
					ui_param_cnt_no += 4;
				}
			}
		}
		dsi_set_cmd_register(ui_cmd_loop, DSI_SET_CMD_PT, p_tmp_dsi_cmd_ctx->ui_packet_type);
	}

	// TX enable
	dsi_set_tx_en(TRUE, TRUE);

	while (dsi_wait_tx_done() != E_OK)
		;

	if ((p_dcs_ctx->b_bta_en == TRUE || p_dcs_ctx->b_bta_only == TRUE)) {
		ui_dsi_cmd_ctrl.reg = DSI_GETREG(DSI_CMDCTRL_REG_OFS);
		if (ui_dsi_cmd_ctrl.bit.SRAM_RD_CNT > 0) {
			DBG_DUMP("BTA : read count = %02d\r\n", (int)(ui_dsi_cmd_ctrl.bit.SRAM_RD_CNT));

			ui_read_cnt = ui_dsi_cmd_ctrl.bit.SRAM_RD_CNT / 4;
			//ui_read_cnt = ui_dsi_cmd_ctrl.bit.SRAM_RD_CNT >> 2;
			if (ui_dsi_cmd_ctrl.bit.SRAM_RD_CNT % 4 != 0) {
				//if ((ui_dsi_cmd_ctrl.bit.SRAM_RD_CNT & 0x3) != 0) {
				ui_read_cnt++;
			}

			for (ui_rd_cnt = 0; ui_rd_cnt < ui_read_cnt; ui_rd_cnt++) {
				DBG_DUMP("=>[0x%08x]\r\n", (unsigned int)DSI_GETREG(DSI_SRAM_REG_OFS + ui_dsi_cmd_ctrl.bit.SRAM_RD_OFS + ui_rd_cnt * 4));
			}

			ui_dsi_cmd_ctrl.bit.CMD_BTA_EN = 0;
			ui_dsi_cmd_ctrl.bit.CMD_BTA_ONLY = 0;
			DSI_SETREG(DSI_CMDCTRL_REG_OFS, ui_dsi_cmd_ctrl.reg);
		}

	}
	return E_OK;
}


/**
     DSI set DCS command BTA

     DSI set specific DCS cmd issue BTA

     @param[in]     pDcsCtx     PDSI_DCS_CMD_PARAM

     @return Description of data returned.
            - @b E_OK   : set Data ID success
            - @b E_SYS  : set Data ID fail
*/
ER dsi_set_hs_dcs_command_BTA(PDSI_CMD_RW_CTRL_PARAM pDcsCtx)
{
    UINT32  uiCmdLoop;
    UINT32  uiLongPktData;
    UINT32  uiWordData;
    UINT32  uiParamCntNo;
//	UINT32  uiReadCnt;
//	T_DSI_INTEN_REG		uiDsiIntEnCtrl;
//	T_DSI_CMDCTRL_REG   uiDsiCmdCtrl;

//	UINT32  uiRdCnt;

    PDSI_CMD_CTRL_PARAM ptmpDSICmdCtx;

    dsi_set_cmd_rw_ctrl(DSI_SET_CMD_NUMBER, pDcsCtx->ui_cmd_no);

    dsi_set_cmd_rw_ctrl(DSI_SET_BTA_EN, pDcsCtx->b_bta_en);

    dsi_set_cmd_rw_ctrl(DSI_SET_SRAM_READ_OFS, pDcsCtx->ui_sram_ofs);

    //dsi_setConfig(DSI_CONFIG_ID_EOT_PKT_EN, pDcsCtx->ubEOTEN);

    //dsi_setCmdRWCtrl(DSI_SET_BTA_ONLY, pDcsCtx->ubBTAOnly);

    for(uiCmdLoop = 0; uiCmdLoop < pDcsCtx->ui_cmd_no; uiCmdLoop ++)
    {
        ptmpDSICmdCtx = (PDSI_CMD_CTRL_PARAM)&pDcsCtx->p_dsi_cmd_ctx[uiCmdLoop];

        // Set Data Type as DCS short read
        dsi_set_cmd_register(uiCmdLoop, DSI_SET_CMD_DT, ptmpDSICmdCtx->ui_data_type);

        dsi_set_cmd_register(uiCmdLoop, DSI_SET_CMD_VC, ptmpDSICmdCtx->ui_virtual_channel);

        if(ptmpDSICmdCtx->ui_packet_type == DSI_SHORT_PACKET)
        {
            if(ptmpDSICmdCtx->ui_param_cnt == 0)
            {
                dsi_set_cmd_register(uiCmdLoop, DSI_SET_CMD_DATA, ptmpDSICmdCtx->ui_dcs_cmd);
            }
            else if(ptmpDSICmdCtx->ui_param_cnt == 1)
            {
                dsi_set_cmd_register(uiCmdLoop, DSI_SET_CMD_DATA, (ptmpDSICmdCtx->ui_dcs_cmd&0xFF) | ((ptmpDSICmdCtx->ui_param[0])<<8));
            }
            else
            {
                DBG_ERR("dsi_setDcsCommand error short pkt but exceed 1 param[%d]\r\n", (int)ptmpDSICmdCtx->ui_param_cnt);
            }

        }
        else
        {
            dsi_set_cmd_register(uiCmdLoop, DSI_SET_CMD_WC, (ptmpDSICmdCtx->ui_param_cnt + 1));

            uiWordData = ptmpDSICmdCtx->ui_dcs_cmd;

            uiParamCntNo = 0;
            if(ptmpDSICmdCtx->ui_param_cnt <= 3)
            {
                for(uiLongPktData = 1; uiLongPktData <= ptmpDSICmdCtx->ui_param_cnt; uiLongPktData++)
                {
                    uiWordData |= ((ptmpDSICmdCtx->ui_param[uiParamCntNo]) << (8 * uiLongPktData));
                    uiParamCntNo ++;
                }
                DSI_SETREG(DSI_SRAM_REG_OFS, uiWordData);
            }
            else
            {
                uiWordData |= ((ptmpDSICmdCtx->ui_param[0] << 8) | (ptmpDSICmdCtx->ui_param[1] << 16) | (ptmpDSICmdCtx->ui_param[2] << 24));

                DSI_SETREG(DSI_SRAM_REG_OFS, uiWordData);

                uiParamCntNo = 4;
                for(uiLongPktData = 3; uiLongPktData <= ptmpDSICmdCtx->ui_param_cnt; uiLongPktData+=4)
                {
                    uiWordData = 0;
                    uiWordData |= ((ptmpDSICmdCtx->ui_param[uiLongPktData]) | (ptmpDSICmdCtx->ui_param[uiLongPktData+1] << 8) | (ptmpDSICmdCtx->ui_param[uiLongPktData+2] << 16) | (ptmpDSICmdCtx->ui_param[uiLongPktData+3] << 24));
                    DSI_SETREG(DSI_SRAM_REG_OFS + uiParamCntNo, uiWordData);
                    uiParamCntNo +=4;
                }
            }
        }
        dsi_set_cmd_register(uiCmdLoop, DSI_SET_CMD_PT, ptmpDSICmdCtx->ui_packet_type);
    }

#if 0
    // TX enable
    //dsi_setTXEN(TRUE, TRUE);

    //while(dsi_waitTXDone() != E_OK);

    if((pDcsCtx->ubBTAEN == TRUE || pDcsCtx->ubBTAOnly == TRUE))
    {
        uiDsiCmdCtrl.Reg = DSI_GETREG(DSI_CMDCTRL_REG_OFS);
        if(uiDsiCmdCtrl.Bit.SRAM_RD_CNT > 0)
        {
            DBG_DUMP("BTA : read count = %02d\r\n", uiDsiCmdCtrl.Bit.SRAM_RD_CNT);

            uiReadCnt = uiDsiCmdCtrl.Bit.SRAM_RD_CNT / 4;
            if(uiDsiCmdCtrl.Bit.SRAM_RD_CNT % 4 != 0)
                uiReadCnt++;

            for(uiRdCnt = 0; uiRdCnt < uiReadCnt; uiRdCnt++)
            {
                DBG_DUMP("=>[0x%08x]\r\n", DSI_GETREG(DSI_SRAM_REG_OFS + uiDsiCmdCtrl.Bit.SRAM_RD_OFS + uiRdCnt * 4));
            }

             uiDsiCmdCtrl.Bit.CMD_BTA_EN = 0;
             uiDsiCmdCtrl.Bit.CMD_BTA_ONLY = 0;
             DSI_SETREG(DSI_CMDCTRL_REG_OFS, uiDsiCmdCtrl.Reg);
        }

    }
#endif

	//uiDsiIntEnCtrl.Reg = DSI_GETREG(DSI_INTEN_REG_OFS);
	//uiDsiIntEnCtrl.Bit.RX_RDY = 0;
	//DSI_SETREG(DSI_INTEN_REG_OFS, uiDsiIntEnCtrl.Reg);

    return E_OK;
}

/**
    DSI escape mode entry

    DSI escape mode entry flow Entry signal + LPDT command + ...

    @param[in] ui_data_lane   data lane number

    @param[in] ui_entry_cmd   LPDT entry command pattern

    @param[in] b_stop        Issue stop command

    @return Description of data returned.
		- @b E_OK   : set Data ID success
		- @b E_SYS  : set Data ID fail
*/
ER dsi_set_escape_entry(DSI_LANESEL ui_data_lane, UINT32 ui_entry_cmd, BOOL b_stop)
{
	T_DSI_ESCCTRL0_REG  esc_ctrl_reg0;
	ER  ret;

	ret = E_OK;
	if (b_stop == FALSE) {
		if (b_lp_trans_starting == TRUE) {
			DBG_ERR("Escape command start already\r\n");
			//return E_SYS;
			ret = E_SYS;
		} else {
			b_lp_trans_starting = TRUE;
		}
	}

	if (ret == E_OK) {
		esc_ctrl_reg0.reg = DSI_GETREG(DSI_ESCCTRL0_REG_OFS);

		DSI_ESC_SET_START(ui_data_lane, esc_ctrl_reg0, TRUE);

		DSI_ESC_SET_STOP(ui_data_lane, esc_ctrl_reg0, b_stop);

		DSI_SETREG(DSI_ESCCTRL0_REG_OFS, esc_ctrl_reg0.reg);

		dsi_set_lp_dcs_command(ui_data_lane, ui_entry_cmd);

		// Trigger Escape command
		dsi_set_escape_control(ui_data_lane, DSI_ESCAPE_TRIGGER, TRUE);

		ret = dsi_wait_lpdt_done(ui_data_lane);
	}
	return ret;
}


/**
    DSI escape mode data transmission

    DSI escape mode data transmission flow

    @param[in] ui_data_lane  data lane number
    @param[in] ui_cmd        LPDT last command before exit escape mode pattern
    @param[in] exit          Determine stop operation
		-@b DSI_SET_ESC_NOT_STOP                Escape command not stop
		-@b DSI_SET_ESC_STOP_WITH_ESC_CMD       Escape command stop but not send exit cmd
		-@b DSI_SET_ESC_STOP_WITHOUT_ESC_CMD,   Escape command stopp and send exit cmd


    @return Description of data returned.
		- @b E_OK   : set Data ID success
		- @b E_SYS  : set Data ID fail
*/
ER dsi_set_escape_transmission(DSI_LANESEL ui_data_lane, UINT32 ui_cmd, DSI_CFG_ESCAPE_CMD_STOP_TYPE exit)
{
	T_DSI_ESCCTRL0_REG  esc_ctrl_reg0;
	ER  ret;

	ret = E_OK;
	if (exit != 0) {
		if (b_lp_trans_starting == FALSE) {
			DBG_ERR("Escape command stop already\r\n");
			//return E_OK;
			ret = E_OK;
			goto dsi_set_escape_trans_exit;
		} else {
			b_lp_trans_starting = FALSE;
		}
	}

	esc_ctrl_reg0.reg = DSI_GETREG(DSI_ESCCTRL0_REG_OFS);

	DSI_ESC_SET_START(ui_data_lane, esc_ctrl_reg0, FALSE);

	DSI_ESC_SET_STOP(ui_data_lane, esc_ctrl_reg0, exit);

	DSI_SETREG(DSI_ESCCTRL0_REG_OFS, esc_ctrl_reg0.reg);

	dsi_set_lp_dcs_command(ui_data_lane, ui_cmd);

	// Trigger Escape command
	dsi_set_escape_control(ui_data_lane, DSI_ESCAPE_TRIGGER, TRUE);

	return dsi_wait_lpdt_done(ui_data_lane);

dsi_set_escape_trans_exit:
	return ret;

}


/**
     DSI set LP command

     DSI set Low power command

     @param[in] ui_data_lane  data lane number
     @param[in] ui_lp_cmd     LPDT command

     @return Description of data returned.
		- @b E_OK   : set Data ID success
		- @b E_SYS  : set Data ID fail
*/
ER dsi_set_lp_dcs_command(DSI_LANESEL ui_data_lane, UINT32 ui_lp_cmd)
{
	T_DSI_ESCCTRL1_REG  esc_cmd_reg1;

	esc_cmd_reg1.reg = DSI_GETREG(DSI_ESCCTRL1_REG_OFS);

	DSI_ESC_SET_CMD(ui_data_lane, esc_cmd_reg1, ui_lp_cmd);

	DSI_SETREG(DSI_ESCCTRL1_REG_OFS, esc_cmd_reg1.reg);

	return E_OK;
}


/**
	DSI set clock ULPS select

	DSI set clock ULPS select

	@param[in] ulp_sel  Enter / Exit ULP selection

	@return Description of data returned.
		- @b E_OK   : Exter / Exit ULP success
		- @b E_SYS  : Exter / Exit ULP fail
*/
ER dsi_set_lps_clock_sel(DSI_ULP_SEL ulp_sel)
{
	T_DSI_ESCCTRL0_REG  esc_cmd_reg0;

	esc_cmd_reg0.reg = 0x0;//DSI_GETREG(DSI_ESCCTRL0_REG_OFS);

	esc_cmd_reg0.bit.CLK_ULP_SEL = ulp_sel;

	DSI_SETREG(DSI_ESCCTRL0_REG_OFS, esc_cmd_reg0.reg);

	return E_OK;
}


/**
     Trigger DSI set clock ULPS select

     DSI set clock ULPS select

     @return Description of data returned.
		- @b E_OK   : Exter / Exit ULP success
		- @b E_SYS  : Exter / Exit ULP fail
*/
ER dsi_ulps_trigger(void)
{
//	FLGPTN ui_flag;
	T_DSI_EN_REG  reg_enable;
#if 1
	T_DSI_INTSTS_REG	dsi_int_sts;
	int timeout = 0;
#endif

	reg_enable.reg = DSI_GETREG(DSI_EN_REG_OFS);

	reg_enable.bit.CLK_ULP_TRIG = 1;

	DSI_SETREG(DSI_EN_REG_OFS, reg_enable.reg);
#if 1	
	do {
		dsi_int_sts.reg = DSI_GETREG(DSI_INTSTS_REG_OFS);
		Delay_DelayMs(1);
		timeout++;
		if (timeout > 1000) {
			printf("dsi_ulps_trigger wait timeout\r\n");
			break;
		}
	} while(((dsi_int_sts.reg & CLK_ULPS_DONE_STS)  == 0));
	
	if (dsi_int_sts.reg != 0) { 
		DSI_SETREG(DSI_INTSTS_REG_OFS, dsi_int_sts.reg);
	}
#else
	while (1) {
		wai_flg(&ui_flag, FLG_ID_DSI, FLGPTN_DSI_CLK_ULPS_DONE, TWF_ORW | TWF_CLR);
		if (ui_dsi_int_sts & (CLK_ULPS_DONE_STS)) {
			break;
		}
	}
#endif
	return E_OK;
}


/*
     DSI phy get phase API

     Get DSI phy analog related phase param

     @param[out] p_ui_clk_phase    clock phase
     @param[out] p_ui_d0_phase     data0 phase
     @param[out] p_ui_d1_phase     data1 phase
     @return Phase delay enable or not
		- @b TRUE:   Phase delay enabled
		- @b FALSE:  Phase delay disabled
*/
BOOL dsi_get_phase_delay_info(UINT32 *p_ui_clk_phase, UINT32 *p_ui_d0_phase, UINT32 *p_ui_d1_phase)
{
	T_DSI_DEBUG3_REG  dsi_debug3_reg = {0};

	dsi_debug3_reg.reg = DSI_GETREG(DSI_DEBUG3_REG_OFS);

	*p_ui_clk_phase = dsi_debug3_reg.bit.CLK_PHASE_OFS;
	*p_ui_d0_phase = dsi_debug3_reg.bit.DAT0_PHASE_OFS;
	*p_ui_d1_phase = dsi_debug3_reg.bit.DAT1_PHASE_OFS;

	return (BOOL)dsi_debug3_reg.bit.PHY_DELAY_EN;
}

void dsi_dump_info(void)
{
	UINT32 ui_get = 0;

	ui_get = dsi_get_config(DSI_CONFIG_ID_PHASE_DELAY_ENABLE_OFS);
	DBG_DUMP("DSI_phase_dly_en = %c\r\n", (ui_get == TRUE) ? 'Y' : 'N');

	ui_get = dsi_get_config(DSI_CONFIG_ID_CLK_PHASE_OFS);
	DBG_DUMP("DSI_clock_dly = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_DAT0_PHASE_OFS);
	DBG_DUMP("DSI_data0_dly = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_DAT1_PHASE_OFS);
	DBG_DUMP("DSI_data1_dly = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_DAT2_PHASE_OFS);
	DBG_DUMP("DSI_data2_dly = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_DAT3_PHASE_OFS);
	DBG_DUMP("DSI_data3_dly = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TLPX);
	DBG_DUMP("DSI_TLPX = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_BTA_TA_GO);
	DBG_DUMP("DSI_TTA-GO = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_BTA_TA_SURE);
	DBG_DUMP("DSI_TTA-SURE = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_BTA_TA_GET);
	DBG_DUMP("DSI_TTA-GET = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_THS_PREPARE);
	DBG_DUMP("DSI_THS-PREPARE = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_THS_ZERO);
	DBG_DUMP("DSI_THS-ZERO = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_THS_TRAIL);
	DBG_DUMP("DSI_THS-TRIAL = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_THS_EXIT);
	DBG_DUMP("DSI_THS-EXIT = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TWAKEUP);
	DBG_DUMP("DSI_TWAKEUP = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TCLK_PREPARE);
	DBG_DUMP("DSI_TCLK-PREPARE = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TCLK_ZERO);
	DBG_DUMP("DSI_TCLK-ZERO = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TCLK_POST);
	DBG_DUMP("DSI_TCLK-POST = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TCLK_PRE);
	DBG_DUMP("DSI_TCLK-PRE = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_TCLK_TRAIL);
	DBG_DUMP("DSI_TCLK-TRIAL = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_BTA_TMOUT_VAL);
	DBG_DUMP("DSI_BTA_TIMEOUT = %d\r\n", (int)ui_get);

	ui_get = dsi_get_config(DSI_CONFIG_ID_BTA_HANDSK_TMOUT_VAL);
	DBG_DUMP("DSI_BTA_HANDSHAKE_TIMEOUT = %d\r\n", (int)ui_get);

	//ui_get = dsi_get_config(DSI_CONFIG_ID_FREQ);
	//DBG_DUMP("DSI_CLK = %dMHz\r\n", (int)(ui_get / 1000000));

	//ui_get = dsi_get_config(DSI_CONFIG_ID_LPFREQ);
	//DBG_DUMP("DSI_LP_CLK = %dMHz\r\n", (int)(ui_get / 1000000));

	ui_get = dsi_get_config(DSI_CONFIG_ID_PHY_DRVING);
	DBG_DUMP("DSI_PHY_driving = %d\r\n", (int)ui_get);

}

#if 0
#ifdef __KERNEL__
EXPORT_SYMBOL(dsi_isr);
EXPORT_SYMBOL(dsi_open);
EXPORT_SYMBOL(dsi_close);
EXPORT_SYMBOL(dsi_is_opened);
EXPORT_SYMBOL(dsi_set_tx_en);
EXPORT_SYMBOL(dsi_wait_tx_done);
EXPORT_SYMBOL(dsi_wait_frame_end);
EXPORT_SYMBOL(dsi_issue_bta);
EXPORT_SYMBOL(dsi_ulps_trigger);
EXPORT_SYMBOL(dsi_get_error_report);
EXPORT_SYMBOL(dsi_set_lps_clock_sel);
EXPORT_SYMBOL(dsi_set_hs_dcs_command);
EXPORT_SYMBOL(dsi_set_config);
EXPORT_SYMBOL(dsi_set_cmd_rw_ctrl);
EXPORT_SYMBOL(dsi_set_lp_dcs_command);
EXPORT_SYMBOL(dsi_set_escape_entry);
EXPORT_SYMBOL(dsi_set_escape_control);
EXPORT_SYMBOL(dsi_set_cmd_register);
EXPORT_SYMBOL(dsi_set_escape_transmission);
EXPORT_SYMBOL(dsi_get_phase_delay_info);
EXPORT_SYMBOL(dsi_get_config);
EXPORT_SYMBOL(dsi_dump_info);
#endif
#endif
//@}
