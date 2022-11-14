
#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <stdlib.h>
#include <linux/arm-smccc.h>
#include "nvt_ivot_tzpc_utils.h"
#include "nvt_ivot_protected.h"

UINT32 dma_getDramBaseAddr(DMA_ID id);
UINT32 dma_getDramCapacity(DMA_ID id);

#define HEAVY_LOAD_CTRL_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_ADDR_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_SIZE_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_WAIT_CYCLE_OFS(ch)   (DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS + ((ch) * 0x10))

#define PROTECT_START_ADDR_OFS(ch)      (DMA_PROTECT_STARTADDR0_REG0_OFS+(ch)*8)
#define PROTECT_END_ADDR_OFS(ch)        (DMA_PROTECT_STOPADDR0_REG0_OFS+(ch)*8)
#define PROTECT_CH_MSK0_OFS(ch)         (DMA_PROTECT_RANGE0_MSK0_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK1_OFS(ch)         (DMA_PROTECT_RANGE0_MSK1_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK2_OFS(ch)         (DMA_PROTECT_RANGE0_MSK2_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK3_OFS(ch)         (DMA_PROTECT_RANGE0_MSK3_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK4_OFS(ch)         (DMA_PROTECT_RANGE0_MSK4_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK5_OFS(ch)         (DMA_PROTECT_RANGE0_MSK5_REG_OFS+(ch)*32)

static UINT32 chip_id = 0x0;

#define INREG32(x)          			(*((volatile UINT32*)(x)))
#define OUTREG32(x, y)      			(*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      			OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      			OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#define _Y_LOG(fmt, args...)         	printf(DBG_COLOR_YELLOW fmt DBG_COLOR_END, ##args)
#define _R_LOG(fmt, args...)         	printf(DBG_COLOR_RED fmt DBG_COLOR_END, ##args)
#define _M_LOG(fmt, args...)         	printf(DBG_COLOR_MAGENTA fmt DBG_COLOR_END, ##args)
#define _G_LOG(fmt, args...)         	printf(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#define _W_LOG(fmt, args...)         	printf(DBG_COLOR_WHITE fmt DBG_COLOR_END, ##args)
#define _X_LOG(fmt, args...)         	printf(DBG_COLOR_HI_GRAY fmt DBG_COLOR_END, ##args)

static DRV_CB pDmaWPCBFunc[DMA_WPSET_TOTAL] = {NULL, NULL, NULL, NULL, NULL, NULL};
static DMA_PROT_ATTR dma_protect_attr[DMA_WPSET_TOTAL];

static DMA_WP_STS_TYPE gDmaProtectChSts[DMA_WPSET_TOTAL][DMA_PROT_RGN_TOTAL] = {
	{ { 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	{ { 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	{ { 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	{ { 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	{ { 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	{ { 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
};

/*
    Get DMA controller register.

    @param[in] id           graphic controller ID
    @param[in] offset       register offset in DMA controller (word alignment)

    @return register value
*/
static REGVALUE dma_getReg(DMA_ID id, UINT32 offset)
{
	if (id == DMA_ID_1)
		return DMA_GETREG(offset);
	else
		return DMA2_GETREG(offset);
}


/*
    Check if DMA controller is active

    Check if DMA controller is active

    @param[in] id           DMA controller ID

    @return BOOL
        - @b TRUE:  active
        - @b FALSE: inactive
*/
static BOOL dma_chkDramActive(DMA_ID id)
{
	union T_DMA_CONTROL_REG   dmaCtrlReg;

	dmaCtrlReg.Reg = dma_getReg(id, DMA_CONTROL_REG_OFS);
	// assume auto refresh is identical to DMA controller EN
	if (dmaCtrlReg.Bit.AUTO_REFRESH_CTRL == 0) {
		return FALSE;
	}

	return TRUE;
}


/*
    Config DMA write protection function.

    This function is used to config DMA write protection function.
    The using right of write protect function must have been gotten before calling this function,

    @param[in] WpSet        Write protect function set
    @param[in] PprotectAttr Configuration for write protect function
    @param[in] pDrvcb       callback handler for write protect function

    @return void
*/
void dma_config_wp_func(DMA_WRITEPROT_SET wp_set, PDMA_PROT_ATTR p_protect_attr, DRV_CB p_drv_cb)
{
	UINT32 i = 0;

	memcpy(&dma_protect_attr[wp_set].ch_en_mask, &p_protect_attr->ch_en_mask, sizeof(DMA_CH_MSK));
	dma_protect_attr[wp_set].protect_level = p_protect_attr->protect_level;
	dma_protect_attr[wp_set].protect_mode = p_protect_attr->protect_mode;

//#if (DRV_SUPPORT_IST == ENABLE)
//    pfIstCB_DMA[WpSet] = pDrvcb;
//#else
	pDmaWPCBFunc[wp_set] = p_drv_cb;
//#endif

	// NT96520 DMA supports DDR2 and DDR3
	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (p_protect_attr->protect_rgn_attr[i].en) {
			//for DDR3 the starting address and size must be 4 word alignment
			if ((p_protect_attr->protect_rgn_attr[i].starting_addr & 0x0000000f) != 0x00000000) {
				printf("DMA_WP: starting address isn't 4 words alignment!0x%08x\r\n", (int)p_protect_attr->protect_rgn_attr[i].starting_addr);
			}
			if ((p_protect_attr->protect_rgn_attr[i].size & 0x0000000f) != 0x00000000) {
				printf("DMA_WP: protecting size isn't 4 words alignment!0x%08x\r\n", p_protect_attr->protect_rgn_attr[i].size);
			}
			dma_protect_attr[wp_set].protect_rgn_attr[i].en = p_protect_attr->protect_rgn_attr[i].en;
			dma_protect_attr[wp_set].protect_rgn_attr[i].starting_addr = p_protect_attr->protect_rgn_attr[i].starting_addr & 0xFFFFFFF0;
			dma_protect_attr[wp_set].protect_rgn_attr[i].size = p_protect_attr->protect_rgn_attr[i].size - 1;
		}
	}
}

/*
    Config DMA write protection function.

    This function is used to config DMA write protection function.
    The using right of write protect function must have been gotten before calling this function,

    @param[in] WpSet        Write protect function set
    @param[in] PprotectAttr Configuration for write protect function
    @param[in] pDrvcb       callback handler for write protect function

    @return void
*/
void dma_configWPFunc(DMA_WRITEPROT_SET WpSet, PDMA_WRITEPROT_ATTR PprotectAttr, DRV_CB pDrvcb)
{
	DMA_PROT_ATTR p_protect_attr = {0};

	memcpy(&p_protect_attr.ch_en_mask, &PprotectAttr->chEnMask, sizeof(DMA_CH_MSK));
	p_protect_attr.protect_level = PprotectAttr->uiProtectlel;
	p_protect_attr.protect_mode = DMA_PROT_IN;
	p_protect_attr.protect_rgn_attr[DMA_PROT_RGN0].en = ENABLE;
	p_protect_attr.protect_rgn_attr[DMA_PROT_RGN0].starting_addr = PprotectAttr->uiStartingAddr;
	p_protect_attr.protect_rgn_attr[DMA_PROT_RGN0].size = PprotectAttr->uiSize;

	dma_config_wp_func(WpSet, &p_protect_attr, pDrvcb);
}

/*
    Enable DMA write protect function.

    Enable DMA write protect function.

    @param[in] WpSet  Write protect function set

    @return void
*/
void dma_enableWPFunc(DMA_WRITEPROT_SET WpSet)
{
	union T_DMA_PROTECT_STARTADDR0_REG0 uiStartingAddr0 = {0};
	union T_DMA_PROTECT_STOPADDR0_REG0 uiStopAddr0 = {0};
	union T_DMA_PROTECT_INTCTRL_REG dma1WpIntCtrl;
	union T_DMA_PROTECT_INTCTRL_REG dma2WpIntCtrl;
	union T_DMA_PROTECT_REGION_EN_REG0 region_en0 = {0};
	union T_DMA_PROTECT_REGION_EN_REG0 region_en0_2 = {0};
	union T_DMA_PROTECT_REGION_EN_REG1 region_en1 = {0};
	union T_DMA_PROTECT_REGION_EN_REG1 region_en1_2 = {0};
	union T_DMA_PROTECT_CTRL_REG dma1WpCtrl;
	union T_DMA_PROTECT_CTRL_REG dma2WpCtrl;
	UINT32 i = 0;
	UINT32 vChMask[DMA_CH_GROUP_CNT];
    UINT32 uiStartPhyAddr, uiEndPhyAddr, uiDma2EndPhy;
	BOOL protect_en = DISABLE;

	dma1WpCtrl.Reg = PROTECT_GETREG(DMA_PROTECT_CTRL_REG_OFS);
	dma2WpCtrl.Reg = PROTECT2_GETREG(DMA_PROTECT_CTRL_REG_OFS);
	if (WpSet < 4) {
		region_en0.Reg = PROTECT_GETREG(DMA_PROTECT_REGION_EN_REG0_OFS);
		region_en0_2.Reg = PROTECT2_GETREG(DMA_PROTECT_REGION_EN_REG0_OFS);
	} else {
		region_en1.Reg = PROTECT_GETREG(DMA_PROTECT_REGION_EN_REG1_OFS);
		region_en1_2.Reg = PROTECT2_GETREG(DMA_PROTECT_REGION_EN_REG1_OFS);
	}
	dma1WpIntCtrl.Reg = DMA_GETREG(DMA_PROTECT_INTCTRL_REG_OFS);
	dma2WpIntCtrl.Reg = DMA2_GETREG(DMA_PROTECT_INTCTRL_REG_OFS);

	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (dma_protect_attr[WpSet].protect_rgn_attr[i].en) {
			protect_en |= ENABLE;
			uiStartPhyAddr = dma_getPhyAddr(dma_protect_attr[WpSet].protect_rgn_attr[i].starting_addr);
    		uiEndPhyAddr = dma_getPhyAddr(dma_protect_attr[WpSet].protect_rgn_attr[i].starting_addr + dma_protect_attr[WpSet].protect_rgn_attr[i].size);
    		uiDma2EndPhy = dma_getPhyAddr(dma_getDramBaseAddr(DMA_ID_2)) + dma_getDramCapacity(DMA_ID_2);

			//set starting address and stop address
			// 1. check if protect range is in DMA controller 1
			if (uiStartPhyAddr < dma_getDramCapacity(DMA_ID_1)) {
				if (uiEndPhyAddr >= dma_getDramCapacity(DMA_ID_1)) {
					uiStopAddr0.Bit.STP_ADDR = dma_getDramCapacity(DMA_ID_1) - 1;
				} else {
					uiStopAddr0.Bit.STP_ADDR = uiEndPhyAddr;
				}

				uiStartingAddr0.Bit.STA_ADDR = uiStartPhyAddr;

				PROTECT_SETREG(PROTECT_START_ADDR_OFS(WpSet * 4 + i), uiStartingAddr0.Reg);
				PROTECT_SETREG(PROTECT_END_ADDR_OFS(WpSet * 4 + i), uiStopAddr0.Reg);

				dma1WpIntCtrl.Reg |= 1 << (WpSet * 4 + i);

				if (WpSet < 4) {
					region_en0.Reg |= 1 << (WpSet * 8 + i);
				} else {
					region_en1.Reg |= 1 << ((WpSet - 4) * 8 + i);
				}

				dma1WpCtrl.Reg |= 1 << WpSet;
			} else {
				if (!protect_en) {
					dma1WpIntCtrl.Reg &= ~(1 << (WpSet * 4 + i));
					dma1WpCtrl.Reg &= ~(1 << WpSet);
				}
			}
			// 2. check if protect range is in DMA controller 2
			if (uiEndPhyAddr > dma_getPhyAddr(dma_getDramBaseAddr(DMA_ID_2))) {
				if (uiStartPhyAddr < dma_getPhyAddr(dma_getDramBaseAddr(DMA_ID_2))) {
					uiStartingAddr0.Bit.STA_ADDR = dma_getPhyAddr(dma_getDramBaseAddr(DMA_ID_2));
				} else {
					uiStartingAddr0.Bit.STA_ADDR = uiStartPhyAddr;
				}

				if (uiEndPhyAddr >= uiDma2EndPhy) {
					uiStopAddr0.Bit.STP_ADDR = uiDma2EndPhy - 1;
				} else {
					uiStopAddr0.Bit.STP_ADDR = uiEndPhyAddr;
				}

				PROTECT2_SETREG(PROTECT_START_ADDR_OFS(WpSet * 4 + i), uiStartingAddr0.Reg);
				PROTECT2_SETREG(PROTECT_END_ADDR_OFS(WpSet * 4 + i), uiStopAddr0.Reg);

				dma2WpIntCtrl.Reg |= 1 << (WpSet * 4 + i);

				if (WpSet < 4) {
					region_en0_2.Reg |= 1 << (WpSet * 8 + i);
				} else {
					region_en1_2.Reg |= 1 << ((WpSet - 4) * 8 + i);
				}

				dma2WpCtrl.Reg |= 1 << WpSet;
			} else {
				if (!protect_en) {
					dma2WpIntCtrl.Reg &= ~(1 << (WpSet * 4 + i));
					dma2WpCtrl.Reg &= ~(1 << WpSet);
				}
			}

		} else {
			dma1WpIntCtrl.Reg &= ~(1 << (WpSet * 4 + i));
			dma2WpIntCtrl.Reg &= ~(1 << (WpSet * 4 + i));

			if (WpSet < 4) {
				region_en0.Reg &= ~(1 << (WpSet * 8 + i));
				region_en0_2.Reg &= ~(1 << (WpSet * 8 + i));
			} else {
				region_en1.Reg &= ~(1 << ((WpSet - 4) * 8 + i));
				region_en1_2.Reg &= ~(1 << ((WpSet - 4) * 8 + i));
			}
			if (!protect_en) {
				dma1WpCtrl.Reg &= ~(1 << WpSet);
				dma2WpCtrl.Reg &= ~(1 << WpSet);
			}
		}

		memset(gDmaProtectChSts[WpSet][i].uiChannelGroup, 0, sizeof(DMA_WP_STS_TYPE));
	}



//#if _FPGA_EMULATION_
//	DBG_IND("%s: sizeof vChMask %d, sizeof DMA_CH_MSK %d\r\n", __func__, sizeof(vChMask), sizeof(DMA_CH_MSK));
//#endif
	memcpy(vChMask, &dma_protect_attr[WpSet].ch_en_mask, sizeof(DMA_CH_MSK));

	PROTECT_SETREG(PROTECT_CH_MSK0_OFS(WpSet), vChMask[DMA_CH_GROUP0]);
	PROTECT_SETREG(PROTECT_CH_MSK1_OFS(WpSet), vChMask[DMA_CH_GROUP1]);
	PROTECT_SETREG(PROTECT_CH_MSK2_OFS(WpSet), vChMask[DMA_CH_GROUP2]);
	PROTECT_SETREG(PROTECT_CH_MSK3_OFS(WpSet), vChMask[DMA_CH_GROUP3]);
	PROTECT_SETREG(PROTECT_CH_MSK4_OFS(WpSet), vChMask[DMA_CH_GROUP4]);
	PROTECT_SETREG(PROTECT_CH_MSK5_OFS(WpSet), vChMask[DMA_CH_GROUP5]);
	PROTECT2_SETREG(PROTECT_CH_MSK0_OFS(WpSet), vChMask[DMA_CH_GROUP0]);
	PROTECT2_SETREG(PROTECT_CH_MSK1_OFS(WpSet), vChMask[DMA_CH_GROUP1]);
	PROTECT2_SETREG(PROTECT_CH_MSK2_OFS(WpSet), vChMask[DMA_CH_GROUP2]);
	PROTECT2_SETREG(PROTECT_CH_MSK3_OFS(WpSet), vChMask[DMA_CH_GROUP3]);
	PROTECT2_SETREG(PROTECT_CH_MSK4_OFS(WpSet), vChMask[DMA_CH_GROUP4]);
	PROTECT2_SETREG(PROTECT_CH_MSK5_OFS(WpSet), vChMask[DMA_CH_GROUP5]);

	dma1WpCtrl.Reg &= ~(0x3 << (16 + 2 * WpSet));
	dma1WpCtrl.Reg |= dma_protect_attr[WpSet].protect_level << (16 + 2 * WpSet);

	dma1WpCtrl.Reg &= ~(0x1 << (8 + WpSet));
	dma1WpCtrl.Reg |= dma_protect_attr[WpSet].protect_mode << (8 + WpSet);

	dma2WpCtrl.Reg &= ~(0x3 << (16 + 2 * WpSet));
	dma2WpCtrl.Reg |= dma_protect_attr[WpSet].protect_level << (16 + 2 * WpSet);

	dma2WpCtrl.Reg &= ~(0x1 << (8 + WpSet));
	dma2WpCtrl.Reg |= dma_protect_attr[WpSet].protect_mode << (8 + WpSet);

	//loc_cpu();

	//Clear interrupt status
	DMA_SETREG(DMA_PROTECT_INTSTS_REG_OFS, 0xffffff);
	DMA2_SETREG(DMA_PROTECT_INTSTS_REG_OFS, 0xffffff);

	DMA_SETREG(DMA_PROTECT_INTCTRL_REG_OFS, dma1WpIntCtrl.Reg);
	DMA2_SETREG(DMA_PROTECT_INTCTRL_REG_OFS, dma2WpIntCtrl.Reg);

	PROTECT_SETREG(DMA_PROTECT_CTRL_REG_OFS, dma1WpCtrl.Reg);
	PROTECT2_SETREG(DMA_PROTECT_CTRL_REG_OFS, dma2WpCtrl.Reg);
	if (WpSet < 4) {
		PROTECT_SETREG(DMA_PROTECT_REGION_EN_REG0_OFS, region_en0.Reg);
		PROTECT2_SETREG(DMA_PROTECT_REGION_EN_REG0_OFS, region_en0_2.Reg);
	} else {
		PROTECT_SETREG(DMA_PROTECT_REGION_EN_REG1_OFS, region_en1.Reg);
		PROTECT2_SETREG(DMA_PROTECT_REGION_EN_REG1_OFS, region_en1_2.Reg);
	}

	//unl_cpu();
}

/*
    Disable specific set of DMA write protect function.

    Disable specific set of DMA write protect function.

    @param[in] WpSet  Write protect function set

    @return void
*/
void dma_disableWPFunc(DMA_WRITEPROT_SET WpSet)
{
	union T_DMA_PROTECT_INTCTRL_REG dma1WpIntCtrl;
	union T_DMA_PROTECT_INTCTRL_REG dma2WpIntCtrl;
	union T_DMA_PROTECT_CTRL_REG dma1WpCtrl;
	union T_DMA_PROTECT_CTRL_REG dma2WpCtrl;
 	UINT32 i = 0;

	dma1WpCtrl.Reg = PROTECT_GETREG(DMA_PROTECT_CTRL_REG_OFS);
	dma2WpCtrl.Reg = PROTECT2_GETREG(DMA_PROTECT_CTRL_REG_OFS);
	dma1WpIntCtrl.Reg = DMA_GETREG(DMA_PROTECT_INTCTRL_REG_OFS);
	dma2WpIntCtrl.Reg = DMA2_GETREG(DMA_PROTECT_INTCTRL_REG_OFS);


	for (i = 0; i < DMA_PROT_RGN_TOTAL; i++) {
		if (dma_protect_attr[WpSet].protect_rgn_attr[i].en) {
			dma1WpIntCtrl.Reg &= ~(1 << (WpSet * 4 + i));
			dma2WpIntCtrl.Reg &= ~(1 << (WpSet * 4 + i));
		}
	}
	dma1WpCtrl.Reg &= ~(1 << WpSet);
	dma2WpCtrl.Reg &= ~(1 << WpSet);

//    drv_disableInt(DRV_INT_DMA);

	//loc_cpu();

	PROTECT_SETREG(DMA_PROTECT_CTRL_REG_OFS, dma1WpCtrl.Reg);
	PROTECT2_SETREG(DMA_PROTECT_CTRL_REG_OFS, dma2WpCtrl.Reg);
	DMA_SETREG(DMA_PROTECT_INTCTRL_REG_OFS, dma1WpIntCtrl.Reg);
	DMA2_SETREG(DMA_PROTECT_INTCTRL_REG_OFS, dma2WpIntCtrl.Reg);

	//unl_cpu();
}

/**
    Return DRAM starting address

    Get DRAM starting address for usage (always return non cache area)

    @return DRAM starting address(0xA0000000)
*/
UINT32 dma_getDramBaseAddr(DMA_ID id)
{
	if (id == DMA_ID_1) {
		return dma_getNonCacheAddr(0x0);
	} else {
		return dma_getNonCacheAddr(0x40000000);
	}
}

/*
    Get dram capacity

    Get dram capacity of DMA controller configuration

    @note in DMA controller 0xC000_0000 bit[2..0] where
        => Dram capacity = 1 << (20 + 4 + reg[2..0])
        => 2   512Mb   ==> 64MB    ==> 2 ^ 6 => 1 << 26
        => 3   1Gb     ==>128MB    ==> 2 ^ 7 => 1 << 27
        => 4   2Gb     ==>256MB    ==> 2 ^ 8 => 1 << 28
        => 5   4Gb     ==>512MB    ==> 2 ^ 9 => 1 << 29

    @return DRAM capacity (unit: byte)

    Example:
    @code
    {
       UINT32 uiDramCapacity = dma_getDramCapacity();
       UINT32 uiDramEndAddress = dma_getDramCapacity + 0x80000000 (or 0xa0000000);
    }
    @endcode
*/
UINT32 dma_getDramCapacity(DMA_ID id)
{
	union T_DMA_CONFIG_REG    dmaCfgReg;
	UINT32 uiDramCapacity = 0;

	if (dma_chkDramActive(id) == FALSE) {
		return 0;
	}

	dmaCfgReg.Reg = dma_getReg(id, DMA_CONFIG_REG_OFS);
	uiDramCapacity = 1UL << (24 + dmaCfgReg.Bit.SDRAM_CAPACITY);
	uiDramCapacity <<= dmaCfgReg.Bit.SDRAM_COUNT;

	return uiDramCapacity;
}

static int do_tzasc(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	UINT32  selectionBit = 0;
	UINT32  element;
	UINT32  address, address_ofs, address_bit, tzpc_address;
	UINT32  reg;

	chip_id = INREG32(0xF00100F0);
	printf("do_tzpc argc = %d\r\n", argc);
	if (argc < 2) {
		printf("argc %d error\n", argc);
		return -1;
	}

	printf("cmd = %s\r\n", argv[1]);

	printf("chip id = 0x%08x\r\n", chip_id);

	if (strncmp(argv[1], "mem_prot_in_1", 13) == 0) {
		DMA_WRITEPROT_ATTR  ProtectAttr = {0};
		BOOL                bResult;
		UINT32              uiSet;
		UINT32              i;
		UINT32              protect_mode;
		UINT32				uiTestSize;
		UINT32				uiTestAddrStart;


		uiTestSize = 0x4000;

		uiTestAddrStart = (UINT32)malloc(0x4000);

		if(uiTestAddrStart % 16)
			uiTestAddrStart = (uiTestAddrStart + 15) & 0xFFFFFFF0;


		_M_LOG(" Memory address = 0x%08x size = 0x4000\r\n", (int)uiTestAddrStart);

		for (uiSet = 0; uiSet < 6; uiSet++) {
			for (protect_mode = 0; protect_mode < 4; protect_mode++) {
				if(protect_mode == 0) {
					_W_LOG("====================set[%d] mode[%d][   write protected only]====================\r\n", uiSet, protect_mode);
				} else if(protect_mode == 1) {
					_W_LOG("====================set[%d] mode[%d][    write detected only]====================\r\n", uiSet, protect_mode);
				} else if(protect_mode == 2) {
					_W_LOG("====================set[%d] mode[%d][     read detected only]====================\r\n", uiSet, protect_mode);
				} else if(protect_mode == 3) {
					_W_LOG("====================set[%d] mode[%d][ read & write protected]====================\r\n", uiSet, protect_mode);
				}
                ProtectAttr.chEnMask.bCPU_NS = TRUE;
//              ProtectAttr.chEnMask.bCPU = TRUE;
				ProtectAttr.uiProtectlel = protect_mode;
				ProtectAttr.uiStartingAddr = uiTestAddrStart;
				ProtectAttr.uiSize = uiTestSize * 2;

				//fLib_PutSerialStr("1. CPU write protect test start, write original pattern, test set %d, test mode %d\r\n", uiSet, protect_mode);

				_Y_LOG("address[0x%08x] size[0x%08x]\r\n", (int)uiTestAddrStart, (int)uiTestSize);

				memset((void *)uiTestAddrStart, 0x96, uiTestSize);
				memset((void *)uiTestAddrStart + uiTestSize, 0xff, uiTestSize);

				flush_dcache_range((ulong)uiTestAddrStart, (ulong)uiTestAddrStart + roundup(uiTestSize, ARCH_DMA_MINALIGN));

				if (uiSet == DMA_WPSET_0) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet0*/);
				} else if (uiSet == DMA_WPSET_1) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet1*/);
				} else if (uiSet == DMA_WPSET_2) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet2*/);
				} else if (uiSet == DMA_WPSET_3) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet3*/);
				} else if (uiSet == DMA_WPSET_4) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet4*/);
				} else {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet5*/);
				}


				dma_enableWPFunc(uiSet);

				//pinmux_select_debugport(PINMUX_DEBUGPORT_DDR);
				//0x0 : Write protection only
				//0x1 : Write detection only
				//0x2 : Read protection only
				//0x3 : Read & write protection


				//   CPU read				CPU read write
				if ((protect_mode == 2) || (protect_mode == 3)) {
					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x96969696) {
							_R_LOG("cpu read fail = 0x%08x", *(volatile UINT32 *)(uiTestAddrStart + i));
						} else {
							_G_LOG("cpu read = 0x%08x", *(volatile UINT32 *)(uiTestAddrStart + i));
							if(*(volatile UINT32 *)(uiTestAddrStart + i) == 0x55aa55aa)
								_G_LOG(" ==> corrected");
							else
								_R_LOG(" ==>     error");
							printf("\r\n");
						}
					}
				}

				for (i = 0; i < ARCH_DMA_MINALIGN; i += 4) {
					*(volatile UINT32 *)(uiTestAddrStart + i) = 0x69696969;
				}

				flush_dcache_range((ulong)uiTestAddrStart, (ulong)uiTestAddrStart + roundup(ARCH_DMA_MINALIGN, ARCH_DMA_MINALIGN));

				//cpu_cleanInvalidateDCacheBlock(uiTestAddrStart, uiTestAddrStart + 8);

				if ((protect_mode == 0) || (protect_mode == 3)) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x69696969) {
							bResult = FALSE;
							_R_LOG("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						_G_LOG("CPU write protect test success\r\n");
					}
				} else if (protect_mode == 1) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) != 0x69696969) {
							bResult = FALSE;
							_R_LOG("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						_G_LOG("CPU write protect test success\r\n");
					}
				}

				dma_disableWPFunc(uiSet);
			}
		}
	} else if (strncmp(argv[1], "mem_prot_in_2", 13) == 0) {
		DMA_WRITEPROT_ATTR  ProtectAttr = {0};
		BOOL                bResult;
		UINT32              uiSet;
		UINT32              i;
		UINT32              protect_mode;
		UINT32				uiTestSize;
		UINT32				uiTestAddrStart;


		uiTestSize = 0x4000;

		uiTestAddrStart = (UINT32)malloc(0x4000);

		uiTestAddrStart += 0x40000000;

		if(uiTestAddrStart % 16)
			uiTestAddrStart = (uiTestAddrStart + 15) & 0xFFFFFFF0;


		_M_LOG(" Memory address = 0x%08x size = 0x4000\r\n", (int)uiTestAddrStart);

		for (uiSet = 0; uiSet < 6; uiSet++) {
			for (protect_mode = 0; protect_mode < 4; protect_mode++) {
				if(protect_mode == 0) {
					_W_LOG("====================set[%d] mode[%d][   write protected only]====================\r\n", uiSet, protect_mode);
				} else if(protect_mode == 1) {
					_W_LOG("====================set[%d] mode[%d][    write detected only]====================\r\n", uiSet, protect_mode);
				} else if(protect_mode == 2) {
					_W_LOG("====================set[%d] mode[%d][     read detected only]====================\r\n", uiSet, protect_mode);
				} else if(protect_mode == 3) {
					_W_LOG("====================set[%d] mode[%d][ read & write protected]====================\r\n", uiSet, protect_mode);
				}
                ProtectAttr.chEnMask.bCPU_NS = TRUE;
                //ProtectAttr.chEnMask.bCPU = TRUE;
				ProtectAttr.uiProtectlel = protect_mode;
				ProtectAttr.uiStartingAddr = uiTestAddrStart;
				ProtectAttr.uiSize = uiTestSize * 2;

				//fLib_PutSerialStr("1. CPU write protect test start, write original pattern, test set %d, test mode %d\r\n", uiSet, protect_mode);

				_Y_LOG("address[0x%08x] size[0x%08x]\r\n", (int)uiTestAddrStart, (int)uiTestSize);

				memset((void *)uiTestAddrStart, 0x96, uiTestSize);
				memset((void *)uiTestAddrStart + uiTestSize, 0xff, uiTestSize);

				flush_dcache_range((ulong)uiTestAddrStart, (ulong)uiTestAddrStart + roundup(uiTestSize, ARCH_DMA_MINALIGN));

				if (uiSet == DMA_WPSET_0) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet0*/);
				} else if (uiSet == DMA_WPSET_1) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet1*/);
				} else if (uiSet == DMA_WPSET_2) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet2*/);
				} else if (uiSet == DMA_WPSET_3) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet3*/);
				} else if (uiSet == DMA_WPSET_4) {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet4*/);
				} else {
					dma_configWPFunc(uiSet, &ProtectAttr, NULL/*emu_wpCallBackSet5*/);
				}


				dma_enableWPFunc(uiSet);

				//pinmux_select_debugport(PINMUX_DEBUGPORT_DDR);
				//0x0 : Write protection only
				//0x1 : Write detection only
				//0x2 : Read protection only
				//0x3 : Read & write protection


				//   CPU read				CPU read write
				if ((protect_mode == 2) || (protect_mode == 3)) {
					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x96969696) {
							_R_LOG("cpu read fail = 0x%08x", *(volatile UINT32 *)(uiTestAddrStart + i));
						} else {
							_G_LOG("cpu read = 0x%08x", *(volatile UINT32 *)(uiTestAddrStart + i));
							if(*(volatile UINT32 *)(uiTestAddrStart + i) == 0x55aa55aa)
								_G_LOG(" ==> corrected");
							else
								_R_LOG(" ==>     error");
							printf("\r\n");
						}
					}
				}

				for (i = 0; i < ARCH_DMA_MINALIGN; i += 4) {
					*(volatile UINT32 *)(uiTestAddrStart + i) = 0x69696969;
				}

				flush_dcache_range((ulong)uiTestAddrStart, (ulong)uiTestAddrStart + roundup(ARCH_DMA_MINALIGN, ARCH_DMA_MINALIGN));

				//cpu_cleanInvalidateDCacheBlock(uiTestAddrStart, uiTestAddrStart + 8);

				if ((protect_mode == 0) || (protect_mode == 3)) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) == 0x69696969) {
							bResult = FALSE;
							_R_LOG("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						_G_LOG("CPU write protect test success\r\n");
					}
				} else if (protect_mode == 1) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) != 0x69696969) {
							bResult = FALSE;
							_R_LOG("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						_G_LOG("CPU write protect test success\r\n");
					}
				}
				dma_disableWPFunc(uiSet);
			}
		}
	}
	return 0;
}

U_BOOT_CMD(nvt_tzasc, 3, 0, do_tzasc,
		   "tzasc emulation cmd:",
		   "[Option] \n"
		   "       [mem_prot_in_1] :  mem protect in @ dram1 @ non secure world\n"
		   "       [mem_prot_in_2] :  mem protect in @ dram2 @ non secure world\n"
		  );

