#include "./include/ide_platform.h"
#include "./include/ide_reg.h"
#include "./include/ide2_int.h"
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>

#define INREG32(x)          			(*((volatile UINT32*)(x)))
#define OUTREG32(x, y)      			(*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      			OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      			OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#define CLOCK_GEN_ENABLE_REG0       (IOADDR_CG_REG_BASE + 0x70)

#define enableIDEClock()            SETREG32(CLOCK_GEN_ENABLE_REG0, (0x03 << 16))
#define disableIDEClock()           CLRREG32(CLOCK_GEN_ENABLE_REG0, (0x03 << 16))

//
//  PLL Register access definition
//
#define PLL_SETREG(ofs, value)      OUTW((IOADDR_CG_REG_BASE+(ofs)), (value))
#define PLL_GETREG(ofs)             INW(IOADDR_CG_REG_BASE+(ofs))

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
    PLL ID
*/
typedef enum {
	PLL_ID_1        = 1,        ///< PLL1 (internal 480 MHz)
	PLL_ID_3		= 3,		///< PLL3
	PLL_ID_4        = 4,        ///< PLL4 (for SSPLL)
	PLL_ID_6        = 6,        ///< PLL6 (for IDE/eth)
	PLL_ID_9        = 9,        ///< PLL9 (for IDE/eth backup)

	PLL_ID_MAX,
	ENUM_DUMMY4WORD(PLL_ID)
} PLL_ID;

#define PLL_CLKSEL_R1_OFFSET        32
#define PLL_CLKSEL_R2_OFFSET        64
#define PLL_CLKSEL_R3_OFFSET        96
#define PLL_CLKSEL_R4_OFFSET        128
#define PLL_CLKSEL_R5_OFFSET        160
#define PLL_CLKSEL_R6_OFFSET        192
#define PLL_CLKSEL_R7_OFFSET        224
#define PLL_CLKSEL_R8_OFFSET        256
#define PLL_CLKSEL_R9_OFFSET        288
#define PLL_CLKSEL_R10_OFFSET       320
#define PLL_CLKSEL_R11_OFFSET       352
#define PLL_CLKSEL_R12_OFFSET       384
#define PLL_CLKSEL_R13_OFFSET       416
#define PLL_CLKSEL_R14_OFFSET       448
#define PLL_CLKSEL_R15_OFFSET       480
#define PLL_CLKSEL_R16_OFFSET       512
#define PLL_CLKSEL_R17_OFFSET       544
#define PLL_CLKSEL_R18_OFFSET       576
#define PLL_CLKSEL_R19_OFFSET       608
#define PLL_CLKSEL_R20_OFFSET       640
#define PLL_CLKSEL_R21_OFFSET       672

/*
    Clock select ID

    Clock select ID for pll_set_clock_rate() & pll_get_clock_rate().
*/
typedef enum {
	PLL_CLKSEL_IDE_CLKSRC =     PLL_CLKSEL_R4_OFFSET + 16,  //< Clock Select Module ID: IDE clock source

	//Video Clock Divider bit definition
	PLL_CLKSEL_IDE_CLKDIV =     PLL_CLKSEL_R9_OFFSET + 0,   //< Clock Select Module ID: IDE clock divider
	PLL_CLKSEL_IDE_OUTIF_CLKDIV = PLL_CLKSEL_R9_OFFSET + 8, //< Clock Select Module ID: IDE Output Interface clock divider

	ENUM_DUMMY4WORD(PLL_CLKSEL)
} PLL_CLKSEL;

/**
    Clock frequency select ID

    @note This is for pll_set_clock_freq().
*/
typedef enum {

	IDECLK_FREQ,            ///< IDE    CLK freq Select ID
	IDEOUTIFCLK_FREQ,       ///< IDE    output I/F CLK freq Select ID

	PLL_CLKFREQ_MAXNUM
} PLL_CLKFREQ;

#define PLL_CLKSEL_IDE_CLKSRC_OFS	16
#define PLL_CLKSEL_IDE_CLKSRC_480   (0x00 << PLL_CLKSEL_IDE_CLKSRC_OFS)    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL6  (0x01 << PLL_CLKSEL_IDE_CLKSRC_OFS)    //< Select IDE clock source as PLL6 (for IDE/ETH)
#define PLL_CLKSEL_IDE_CLKSRC_PLL4  (0x02 << PLL_CLKSEL_IDE_CLKSRC_OFS)    //< Select IDE clock source as PLL4 (for SSPLL)
#define PLL_CLKSEL_IDE_CLKSRC_PLL9  (0x03 << PLL_CLKSEL_IDE_CLKSRC_OFS)    //< Select IDE clock source as PLL9 (for IDE/ETH backup)

/*
    @name   IDE clock divider

    IDE clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV).
*/
//@{
#define PLL_IDE_CLKDIV(x)           ((x) << (PLL_CLKSEL_IDE_CLKDIV - PLL_CLKSEL_R9_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV)
//@}

/*
    @name   IDE Output Interface clock divider

    IDE Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R9_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV)
//@}

#if defined __UITRON || defined __ECOS

static const DRV_INT_NUM v_ide_int_en[] = {DRV_INT_IDE, DRV_INT_IDE2};
static ID v_ide_flg_id[] = {FLG_ID_IDE, FLG_ID_IDE2};

#elif defined(__FREERTOS)
static ID v_ide_flg_id[IDE_ID_1+1];

static vk_spinlock_t v_ide_spin_locks[IDE_ID_1+1];

static SEM_HANDLE SEMID_IDE[IDE_ID_1+1];

unsigned int ide_debug_level = NVT_DBG_WRN;

UINT32 IOADDR_IDE2_REG_BASE;

#else

//UINT32 IOADDR_IDE_REG_BASE;
//UINT32 IOADDR_IDE2_REG_BASE;

//static ID v_ide_flg_id[IDE_ID_1+1];

//static spinlock_t v_ide_spin_locks[IDE_ID_1+1];

//static struct clk *ide_clk[IDE_ID_1+1];
//static struct clk *ide_if_clk[IDE_ID_1+1];
//static SEM_HANDLE SEMID_IDE[IDE_ID_1+1];
//static struct tasklet_struct * v_p_ide_tasklet[IDE_ID_1+1];

#endif

#define IDE_REQ_POLL_SIZE	16
static IDE_REQ_LIST_NODE v_req_pool[IDE_ID_1+1][IDE_REQ_POLL_SIZE];
static UINT32 v_req_front[IDE_ID_1+1];
static UINT32 v_req_tail[IDE_ID_1+1];

//Need porting
//PINMUX_LCDINIT ide_platform_get_disp_mode(UINT32 pin_func_id)
//{
//#if defined __UITRON || defined __ECOS
//	return pinmux_getDispMode((PINMUX_FUNC_ID)pin_func_id);
//#else
//	return pinmux_get_dispmode((PINMUX_FUNC_ID) pin_func_id);
//#endif
//}

/**
    Get PLL frequency

    Get PLL frequency (unit:Hz)
    When spread spectrum is enabled (by pll_set_pll_spread_spectrum()), this API will return lower bound frequency of spread spectrum.

    @param[in] id           PLL ID

    @return PLL frequency (unit:Hz)
*/
UINT32 pll_get_pll_freq(PLL_ID id)
{
	UINT64 pll_ratio;
	T_PLL_PLL2_CR0_REG reg0 = {0};
	T_PLL_PLL2_CR1_REG reg1 = {0};
	T_PLL_PLL2_CR2_REG reg2 = {0};
	const UINT32 pll_address[] = {PLL_PLL3_CR0_REG_OFS, PLL_PLL4_CR0_REG_OFS,
								  PLL_PLL5_CR0_REG_OFS, PLL_PLL6_CR0_REG_OFS, PLL_PLL7_CR0_REG_OFS,
								  PLL_PLL8_CR0_REG_OFS, PLL_PLL9_CR0_REG_OFS, PLL_PLL10_CR0_REG_OFS,
								  PLL_PLL11_CR0_REG_OFS, PLL_PLL12_CR0_REG_OFS, PLL_PLL13_CR0_REG_OFS,
								  PLL_PLL14_CR0_REG_OFS, PLL_PLL15_CR0_REG_OFS
								 };
	const UINT32 pll528_address[] = {PLL528_PLL3_CR0_REG_OFS, PLL528_PLL4_CR0_REG_OFS,
								     PLL528_PLL5_CR0_REG_OFS, PLL528_PLL6_CR0_REG_OFS,  PLL528_PLL7_CR0_REG_OFS,
								     PLL528_PLL8_CR0_REG_OFS, PLL528_PLL9_CR0_REG_OFS,  PLL528_PLL10_CR0_REG_OFS,
								     PLL528_PLL11_CR0_REG_OFS,PLL528_PLL12_CR0_REG_OFS, PLL528_PLL13_CR0_REG_OFS,
								     PLL528_PLL14_CR0_REG_OFS,PLL528_PLL15_CR0_REG_OFS, PLL528_PLL16_CR0_REG_OFS,
								     PLL528_PLL17_CR0_REG_OFS,PLL528_PLL18_CR0_REG_OFS
								 };

	if (id == PLL_ID_1) {
		return 480000000;
	}
	if (id == PLL_ID_1) {
		return 480000000;
	}
	if (nvt_get_chip_id() == CHIP_NA51084) {
		reg0.reg = PLL_GETREG(pll528_address[id - PLL_ID_3]);
		reg1.reg = PLL_GETREG(pll528_address[id - PLL_ID_3] + 0x04);
		reg2.reg = PLL_GETREG(pll528_address[id - PLL_ID_3] + 0x08);
	} else if (nvt_get_chip_id() == CHIP_NA51055){
		reg0.reg = PLL_GETREG(pll_address[id - PLL_ID_3]);
		reg1.reg = PLL_GETREG(pll_address[id - PLL_ID_3] + 0x04);
		reg2.reg = PLL_GETREG(pll_address[id - PLL_ID_3] + 0x08);
	} else {
		DBG_ERR("id out of range: PLL%d\r\n", id);
		return E_ID;
	}

	pll_ratio = (reg2.bit.PLL_RATIO2 << 16) | (reg1.bit.PLL_RATIO1 << 8) | (reg0.bit.PLL_RATIO0 << 0);


	return 12000000 * pll_ratio / 131072;
}

/**
    Get module clock rate

    Get module clock rate, one module at a time.

    @param[in] ui_num	Module ID(PLL_CLKSEL_*), one module at a time.
						Please refer to pll.h

    @return Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h
*/
UINT32 pll_get_clock_rate(PLL_CLKSEL clk_sel)
{
	UINT32      ui_mask, ui_reg_offset;
	REGVALUE    reg_data;

	ui_reg_offset = 0x20;
	ui_mask = (0x7<<PLL_CLKSEL_IDE_CLKSRC_OFS);

	reg_data = INREG32(IOADDR_CG_REG_BASE + ui_reg_offset);
	reg_data &= ui_mask;

	return (UINT32)reg_data;
}

void pll_set_clock_rate(PLL_CLKSEL clk_sel, UINT32 ui_value)
{
	REGVALUE reg_data;
	UINT32 ui_mask, ui_reg_offset;
	UINT32 ui_temp;
//	unsigned long flags = 0;
	if (clk_sel == PLL_CLKSEL_IDE_CLKSRC) {
		ui_reg_offset = 0x20;
		ui_mask = (0x7<<PLL_CLKSEL_IDE_CLKSRC_OFS);
		ui_temp =  (ui_value << PLL_CLKSEL_IDE_CLKSRC_OFS);

		reg_data = INREG32(IOADDR_CG_REG_BASE + ui_reg_offset);
		reg_data &= ~ui_mask;
		reg_data |= ui_temp;
		OUTREG32(IOADDR_CG_REG_BASE + ui_reg_offset, reg_data);

		ui_reg_offset = 0x0;
		if (ui_temp == PLL_CLKSEL_IDE_CLKSRC_480) {
			ui_mask = 0x0;
			ui_temp = 0x0;
		} else if (ui_temp == PLL_CLKSEL_IDE_CLKSRC_PLL6) {
			ui_mask = (0x1 << PLL_ID_6);
			ui_temp = (0x1 << PLL_ID_6);
		} else if (ui_temp == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
			ui_mask = (0x1 << PLL_ID_4);
			ui_temp = (0x1 << PLL_ID_4);
		} else if (ui_temp == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
			ui_mask = (0x1 << PLL_ID_9);
			ui_temp = (0x1 << PLL_ID_9);
		} 
	}

	if (clk_sel == PLL_CLKSEL_IDE_CLKDIV) {
		ui_reg_offset = 0x34;
		ui_mask = (0xFF);
		ui_temp =  ui_value;
	}

	if (clk_sel == PLL_CLKSEL_IDE_OUTIF_CLKDIV) {
		ui_reg_offset = 0x34;
		ui_mask = (0xFF<<8);
		ui_temp =  ui_value;
	}


	reg_data = INREG32(IOADDR_CG_REG_BASE + ui_reg_offset);
	reg_data &= ~ui_mask;
	reg_data |= ui_temp;
	OUTREG32(IOADDR_CG_REG_BASE + ui_reg_offset, reg_data);

}

/**
    Set the module clock frequency

    This api setup the module clock frequency by chnaging module clock divider.
    If the module has multiple source clock choices, user must set the correct
    source clock before calling this API.
\n  If the target frequency can not well divided from source frequency,this api
    would output warning message.

    @param[in] clock_id    Module select ID, refer to structure PLL_CLKFREQ.
    @param[in] ui_freq   Target clock frequency. Unit in Hertz.

    @return
     - @b E_ID:     clock_id is not support in this API.
     - @b E_PAR:    Target frequency can not be divided with no remainder.
     - @b E_OK:     Done and success.
*/
ER pll_set_clock_freq(PLL_CLKFREQ clock_id, UINT32 ui_freq)
{
	UINT32 source_clock, divider, clock_source;

	if (clock_id >= PLL_CLKFREQ_MAXNUM) {
		return E_ID;
	}

	// Get Src Clock Frequency
	switch (clock_id) {
	case IDECLK_FREQ: {
			clock_source = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			if (clock_source == PLL_CLKSEL_IDE_CLKSRC_480) {
				source_clock = pll_get_pll_freq(PLL_ID_1);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL6) {
				source_clock = pll_get_pll_freq(PLL_ID_6);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
				source_clock = pll_get_pll_freq(PLL_ID_4);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
				source_clock = pll_get_pll_freq(PLL_ID_9);
			} else {
				return E_PAR;
			}
		}
		break;

	case IDEOUTIFCLK_FREQ: {
			clock_source = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			if (clock_source == PLL_CLKSEL_IDE_CLKSRC_480) {
				source_clock = pll_get_pll_freq(PLL_ID_1);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL6) {
				source_clock = pll_get_pll_freq(PLL_ID_6);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
				source_clock = pll_get_pll_freq(PLL_ID_4);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
				source_clock = pll_get_pll_freq(PLL_ID_9);
			}

			else {
				return E_PAR;
			}
		}
		break;

	default:
		return E_PAR;
	}


	// Calculate the clock divider value
	divider = (source_clock + ui_freq - 1) / ui_freq;

	// prevent error case
	if (divider == 0) {
		divider = 1;
	}
	// Set Clock divider
	switch (clock_id) {
	case IDECLK_FREQ:
		pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV, PLL_IDE_CLKDIV(divider - 1));
		break;
	case IDEOUTIFCLK_FREQ:
		pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV, PLL_IDE_OUTIF_CLKDIV(divider - 1));
		break;
	//coverity[dead_error_begin]
	default :
		return E_PAR;
	}

	// Output warning msg if target freq can not well divided from source freq
	if (source_clock % ui_freq) {
		UINT32 ui_real_freq = source_clock / divider;

		// Truncate inaccuray under 1000 Hz
		ui_real_freq = (ui_real_freq + 50) / 1000;
		ui_freq /= 1000;
		if (ui_freq != ui_real_freq) {
			DBG_WRN("Target(%d) freq can not be divided with no remainder! Result is %dHz.\r\n", clock_id, (UINT)(source_clock / divider));
			return E_PAR;
		}
	}

	return E_OK;
}

/**
    Get the module clock frequency

    This api get the module clock frequency.

    @param[in] clock_id    Module select ID, refer to structure PLL_CLKFREQ.
    @param[out] p_freq   Return clock frequency. Unit in Hertz.

    @return
     - @b E_ID:     clock_id is not support in this API.
     - @b E_PAR:    Target frequency can not be divided with no remainder.
     - @b E_OK:     Done and success.
*/
ER pll_get_clock_freq(PLL_CLKFREQ clock_id, UINT32 *p_freq)
{
	UINT32 source_clock = 0, divider = 0, clock_source = 0;

	if (clock_id >= PLL_CLKFREQ_MAXNUM) {
		return E_ID;
	}

	if (p_freq == NULL) {
		DBG_ERR("input p_freq is NULL\r\n");
		return E_PAR;
	}

	// Get Src Clock Frequency
	switch (clock_id) {
	case IDECLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			divider = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKDIV) >> (PLL_CLKSEL_IDE_CLKDIV & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_IDE_CLKSRC_480:
				source_clock = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL6:
				source_clock = pll_get_pll_freq(PLL_ID_6);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL4:
				source_clock = pll_get_pll_freq(PLL_ID_4);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL9:
				source_clock = pll_get_pll_freq(PLL_ID_9);
				break;

			default:
				return E_PAR;
			}
		}
		break;
	case IDEOUTIFCLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			divider = pll_get_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV) >> (PLL_CLKSEL_IDE_OUTIF_CLKDIV & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_IDE_CLKSRC_480:
				source_clock = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL6:
				source_clock = pll_get_pll_freq(PLL_ID_6);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL4:
				source_clock = pll_get_pll_freq(PLL_ID_4);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL9:
				source_clock = pll_get_pll_freq(PLL_ID_9);
				break;

			default:
				return E_PAR;
			}
		}
		break;

	default:
		return E_PAR;
	}

	*p_freq = source_clock / (divider + 1);

	return E_OK;
}

void ide_platform_delay_ms(UINT32 ms)
{
	udelay(1000 *ms);
}

void ide_platform_delay_us(UINT32 us)
{
	ndelay(1000 * us);
}

#if 0
ER ide_platform_flg_clear(IDE_ID id, FLGPTN flg)
{
	return clr_flg(v_ide_flg_id[id], flg);
}

ER ide_platform_flg_set(IDE_ID id, FLGPTN flg)
{
	 return iset_flg(v_ide_flg_id[id], flg);
}

ER ide_platform_flg_wait(IDE_ID id, FLGPTN flg)
{
	FLGPTN              ui_flag;

	return wai_flg(&ui_flag, v_ide_flg_id[id], flg, TWF_ORW | TWF_CLR);
}

ER ide_platform_sem_set(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(SEMID_IDE[id]);
#else
	SEM_SIGNAL(SEMID_IDE[id]);
	return E_OK;
#endif
}

ER ide_platform_sem_wait(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(SEMID_IDE[id]);
#else
	return SEM_WAIT(SEMID_IDE[id]);
#endif
}
#endif

UINT32 ide_platform_spin_lock(IDE_ID id)
{
	return 0;
}

void ide_platform_spin_unlock(IDE_ID id, UINT32 flag)
{
	return;
}


void ide_platform_sram_enable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	if (id == IDE_ID_1)
		pinmux_disable_sram_shutdown(IDE_SD);
	else
		;//pll_disableSramShutDown(IDE2_RSTN);
#elif defined __FREERTOS
	nvt_enable_sram_shutdown(IDE_SD);
#else
#endif
}

void ide_platform_sram_disable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	if (id == IDE_ID_1)
		pinmux_enable_sram_shutdown(IDE_SD);
	else
		;//pll_enableSramShutDown(IDE2_RSTN);
#elif defined __FREERTOS
	nvt_disable_sram_shutdown(IDE_SD);
#else
#endif
}

void ide_platform_int_enable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	drv_enableInt(v_ide_int_en[id]);
#else
#endif
}

void ide_platform_int_disable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	drv_disableInt(v_ide_int_en[id]);
#else
#endif
}

#define dma_getPhyAddr(addr) 	((((uint32_t)(addr))>=0x60000000UL)?((uint32_t)(addr)-0x60000000UL):(uint32_t)(addr))


UINT32 ide_platform_va2pa(UINT32 addr)
{
	return dma_getPhyAddr(addr);
}

UINT32 factor_caculate(UINT16 x, UINT16 y, BOOL h)
{
	UINT32 a, b, c;
	UINT64 temp;

	if (h == TRUE) {

		a = (x - 1) << 15;
		b = (y - 1);

		temp = (UINT64) a;
//#if defined __UITRON || defined __ECOS || defined __FREERTOS
		temp = temp/b;
//#else
//		do_div(temp, b);
//#endif
		c = (UINT32) temp;
	} else {
		a = (x - 1) << 12;
		b = (y - 1);
		temp = (UINT64) a;
//#if defined __UITRON || defined __ECOS || defined __FREERTOS
		temp = temp/b;
//#else
//		do_div(temp, b);
//#endif
		c = (UINT32) temp;
	}

	//c = temp - (1 << 15);

	return c;
}

void ide_platform_clk_en(IDE_ID id)
{
	DBG_IND("ide clk enable\n");
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	if (id == IDE_ID_1)
		pll_enable_clock(IDE1_CLK);
	else
		DBG_ERR("not support IDE%d\n", id);
#else
	//clk_prepare(ide_clk[id]);

	//clk_enable(ide_clk[id]);
	enableIDEClock();
#endif
	DBG_IND("ide clk enable finished\n");
}

void ide_platform_clk_dis(IDE_ID id)
{
	DBG_IND("ide clk disable\n");
#if defined __UITRON || defined __ECOS || defined __FREERTOS
	if (id == IDE_ID_1)
		pll_disable_clock(IDE1_CLK);
	else
		DBG_ERR("not support IDE%d\n", id);
#else
	//clk_disable(ide_clk[id]);

	//clk_unprepare(ide_clk[id]);
	disableIDEClock();
#endif
	DBG_IND("ide clk disable finished\n");
}

void ide_platform_set_iffreq(IDE_ID id, UINT32 freq)
{
	DBG_IND("ide if clk %d Hz\n", (int)freq);
	pll_set_clock_freq(IDEOUTIFCLK_FREQ, freq);
}

void ide_platform_set_freq(IDE_ID id, UINT32 freq)
{
	DBG_IND("ide clk %d Hz\n", (int)freq);
	pll_set_clock_freq(IDECLK_FREQ, freq);
}


UINT32 ide_platform_get_iffreq(IDE_ID id)
{
	UINT32 rate = 0;

	if (id == IDE_ID_1)
		pll_get_clock_freq(IDEOUTIFCLK_FREQ, &rate);
	else
		DBG_ERR("not support IDE%d\n", id);

	DBG_IND("ide if clk %d Hz\n", (int)rate);

	return rate;
}

UINT32 ide_platform_get_freq(IDE_ID id)
{
	UINT32 rate = 0;

	if (id == IDE_ID_1)
		pll_get_clock_freq(IDECLK_FREQ, &rate);
	else
		DBG_ERR("not support IDE%d\n", id);

	DBG_IND("ide clk %d Hz\n", (int)rate);

	return rate;
}

ER ide_platform_set_clksrc(IDE_ID id, UINT32 src)
{
#if 1//defined __UITRON || defined __ECOS || defined __FREERTOS

	pll_set_clock_rate(PLL_CLKSEL_IDE_CLKSRC, src);

	return E_OK;
#else
	struct clk *source_clk;

	if (src == 0) {
		source_clk = clk_get(NULL, "fix480m");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
	} else if (src == 1) {
		source_clk = clk_get(NULL, "pll6");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
	} else if (src == 2) {
		source_clk = clk_get(NULL, "pll4");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
	} else if (src == 3) {
		source_clk = clk_get(NULL, "pll9");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
	} else {
		DBG_ERR("ide clk source %d not support\n", src);
		return E_SYS;
	}
#endif
	return E_OK;
}

/*
        Check if service queue is empty
*/
BOOL ide_platform_list_empty(IDE_ID id)
{
	if (id > IDE_ID_1) {
                DBG_ERR("invalid id %d\r\n", id);
                return E_SYS;
        }

	if (v_req_front[id] == v_req_tail[id]) {
		// queue empty
		return TRUE;
	} else {
		return FALSE;
	}
}

/**
    Add request descriptor to service queue
*/
ER ide_platform_add_list(IDE_ID id, KDRV_CALLBACK_FUNC *p_callback)
{
	UINT32 next;
	const UINT32 tail = v_req_tail[id];

	if (id > IDE_ID_1) {
		DBG_ERR("invalid id %d\r\n", id);
		return E_SYS;
	}

	next = (tail+1) % IDE_REQ_POLL_SIZE;
	//printk("%s: next %d\r\n", __func__, next);

	if (next == v_req_front[id]) {
		// queue full
		DBG_ERR("queue full, front %d, tail %d\r\n", (int)v_req_front[id], (int)tail);
		return E_SYS;
	}

	if (p_callback) {
		memcpy(&v_req_pool[id][tail].callback,
			p_callback,
			sizeof(KDRV_CALLBACK_FUNC));
	} else {
		memset(&v_req_pool[id][tail].callback,
                        0,
                        sizeof(KDRV_CALLBACK_FUNC));
	}

	v_req_tail[id] = next;

	return E_OK;
}

/*
	Get head request descriptor from service queue
*/
IDE_REQ_LIST_NODE* ide_platform_get_head(IDE_ID id)
{
	IDE_REQ_LIST_NODE *p_node;

	p_node = &v_req_pool[id][v_req_front[id]];

	if (id > IDE_ID_1) {
		DBG_ERR("invalid id %d\r\n", id);
		return NULL;
	}

	if (v_req_front[id] == v_req_tail[id]) {
		// queue empty
		DBG_ERR("queue empty\r\n");
		return NULL;
	}

	return p_node;

//	memcpy(p_param, &p_node->trig_param, sizeof(KDRV_GRPH_TRIGGER_PARAM));

//	return E_OK;
}

/*
	Delete request descriptor from service queue
*/
ER ide_platform_del_list(IDE_ID id)
{
	if (id > IDE_ID_1) {
		DBG_ERR("invalid id %d\r\n", id);
		return E_SYS;
	}

	if (v_req_front[id] == v_req_tail[id]) {
		DBG_ERR("queue already empty, front %d, tail %d\r\n", (int)v_req_front[id], (int)v_req_tail[id]);
		return E_SYS;
	}

	v_req_front[id] = (v_req_front[id]+1) % IDE_REQ_POLL_SIZE;

	return E_OK;
}


void ide_platform_set_ist_event(IDE_ID id)
{
	/*  Tasklet for bottom half mechanism */
#if defined __KERNEL__
//        tasklet_schedule(v_p_ide_tasklet[id]);
#endif
}

int ide_platform_ist(IDE_ID id, UINT32 event)
{
	idec_isr_bottom(id, event);

	return 0;
}

#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
static int is_create = 0;
irqreturn_t ide_platform_isr(int irq, void *devid)
{
	ide_isr();
	return IRQ_HANDLED;
}


void ide_platform_create_resource(void)
{
	if (!is_create) {
		OS_CONFIG_FLAG(v_ide_flg_id[0]);
		SEM_CREATE(SEMID_IDE[0], 1);
		vk_spin_lock_init(&v_ide_spin_locks[0]);

		request_irq(INT_ID_IDE, ide_platform_isr, IRQF_TRIGGER_HIGH, "ide", 0);
		is_create = 1;
	}
}
void ide_platform_release_resource(void)
{
	is_create = 0;
	rel_flg(v_ide_flg_id[0]);
	SEM_DESTROY(SEMID_IDE[0]);
}
#else
#if 0
void ide_platform_create_resource(MODULE_INFO *pmodule_info)
{
	IOADDR_IDE_REG_BASE = (UINT32)pmodule_info->io_addr[0];
	OS_CONFIG_FLAG(v_ide_flg_id[0]);
	SEM_CREATE(SEMID_IDE[0], 1);
	spin_lock_init(&v_ide_spin_locks[0]);

	ide_clk[0] = pmodule_info->pclk[0];
	ide_if_clk[0] = pmodule_info->ifclk[0];

	v_p_ide_tasklet[IDE_ID_1] = &pmodule_info->ide_tasklet[IDE_ID_1];
	v_req_front[IDE_ID_1] = 0;
	v_req_tail[IDE_ID_1] = 0;

	//printk("ide addr 0x%x\r\n",
	//	IOADDR_IDE_REG_BASE);
}

void ide_platform_release_resource(void)
{
	rel_flg(v_ide_flg_id[0]);
	SEM_DESTROY(SEMID_IDE[0]);
}

EXPORT_SYMBOL(ide_platform_set_clksrc);
EXPORT_SYMBOL(ide_platform_set_freq);
EXPORT_SYMBOL(ide_platform_set_iffreq);
EXPORT_SYMBOL(ide_platform_get_freq);
EXPORT_SYMBOL(ide_platform_get_iffreq);
EXPORT_SYMBOL(ide_platform_clk_en);
EXPORT_SYMBOL(ide_platform_clk_dis);
#endif
#endif
#endif


