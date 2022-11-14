/*
    @file	nand_reg.h
    @ingroup	mICardSMC

    @brief	Nand flash / Smartmedia Card driver register definition

    @note

    @version	V1.00.000
    @author	Cliff Lin
    @date	2009/05/19

    Copyright	Novatek Microelectronics Corp. 2009.  All rights reserved.

*/

#ifndef _NAND_REG_H
#define _NAND_REG_H

#include <asm/nvt-common/rcw_macro.h>

#define NAND_MODULE0_REG_OFS	0x00
REGDEF_BEGIN(NAND_MODULE0_REG)
	REGDEF_BIT(COL_ADDR              ,2)
	REGDEF_BIT(ROW_ADD               ,2)
	REGDEF_BIT(_PAGE_SIZE            ,4)
	REGDEF_BIT(PRI_ECC_TYPE          ,2)
	REGDEF_BIT(SEC_ECC_TYPE          ,2)
	REGDEF_BIT(                      ,4)
	REGDEF_BIT(LATCH_DLY             ,3)
	REGDEF_BIT(NAND_TYPE             ,1)
	REGDEF_BIT(SPI_FLASH_TYPE        ,1)
	REGDEF_BIT(SPI_NOR_FLASH_OP_MODE ,1)
	REGDEF_BIT(SPI_NAND_SKIP_COLUMN_ADDRESS, 1)
	REGDEF_BIT(                      ,9)
REGDEF_END(NAND_MODULE0_REG)

#define NAND_DLL_PHASE_DLY_REG0_OFS	0x04
REGDEF_BEGIN(NAND_DLL_PHASE_DLY_REG0)
	REGDEF_BIT(PHY_DET_RATIO,32)
REGDEF_END(NAND_DLL_PHASE_DLY_REG0)

#define NAND_DLL_PHASE_DLY_REG1_OFS	0x08
REGDEF_BEGIN(NAND_DLL_PHASE_DLY_REG1)
	REGDEF_BIT(PHY_SW_RESET      ,1)
	REGDEF_BIT(PHY_DET_CLR       ,1)
	REGDEF_BIT(PHY_SAMPCLK_INV   ,1)
	REGDEF_BIT(PHY_SRC_CLK_SEL   ,1)
	REGDEF_BIT(PHY_EGDET_SEL     ,4)
	REGDEF_BIT(PHY_DET_MODE      ,1)
	REGDEF_BIT(PHY_DET_1_0_DIS   ,1)
	REGDEF_BIT(PHY_DET_0_1_DIS   ,1)
	REGDEF_BIT(PHY_DET_AUTO      ,1)
	REGDEF_BIT(PHY_PHASE_SEL     ,4)
	REGDEF_BIT(PHY_DLY_SEL       ,3)
	REGDEF_BIT(PHY_CLK_OUT_INV   ,1)
	REGDEF_BIT(PHY_PAD_CLK_SEL   ,1)
	REGDEF_BIT(                  ,3)
	REGDEF_BIT(PHY_DATA_DLY_SEL  ,5)
	REGDEF_BIT(                  ,3)
REGDEF_END(NAND_DLL_PHASE_DLY_REG1)

#define NAND_DLL_PHASE_DLY_REG2_OFS	0x0C
REGDEF_BEGIN(NAND_DLL_PHASE_DLY_REG2)
	REGDEF_BIT(PHY_DATA_DSW_SEL ,12)
	REGDEF_BIT(PHY_DAT_READ_DLY ,2)
	REGDEF_BIT(PHY_DET_READ_DLY ,2)
	REGDEF_BIT(OUTDLY_SEL       ,6)
	REGDEF_BIT(                 ,2)
	REGDEF_BIT(INDLY_SEL        ,6)
	REGDEF_BIT(                 ,2)
REGDEF_END(NAND_DLL_PHASE_DLY_REG2)

#define NAND_DLL_PHASE_DLY_REG3_OFS	0x10
REGDEF_BEGIN(NAND_DLL_PHASE_DLY_REG3)
  	REGDEF_BIT(PHASE_CMP_EN   ,32)
REGDEF_END(NAND_DLL_PHASE_DLY_REG3)

#define NAND_SPI_CFG_REG_OFS       	0x14
REGDEF_BEGIN(NAND_SPI_CFG_REG)
	REGDEF_BIT(SPI_PKT_LSB_MODE  ,1)
	REGDEF_BIT(SPI_OPERATION_MODE,1)
	REGDEF_BIT(                  ,2)
	REGDEF_BIT(SPI_NAND_CS       ,1)
	REGDEF_BIT(                  ,3)
	REGDEF_BIT(SPI_CS_POL        ,1)
	REGDEF_BIT(SPI_BS_WIDTH      ,2)
	REGDEF_BIT(SPI_IO_ORDER      ,1)
	REGDEF_BIT(SPI_PULL_WPHLD    ,1)
	REGDEF_BIT(                  ,19)
REGDEF_END(NAND_SPI_CFG_REG)

#define NAND_DLL_PHASE_DLY_REG4_OFS	0x18
REGDEF_BEGIN(NAND_DLL_PHASE_DLY_REG4)
	REGDEF_BIT(OUTPUT_DLY_EN     ,1)
	REGDEF_BIT(                  ,3)
	REGDEF_BIT(DATA_OUT_DELAY_INV,1)
	REGDEF_BIT(                  ,3)
	REGDEF_BIT(OUTPUT_DLY_SEL    ,5)
	REGDEF_BIT(                  ,3)
	REGDEF_BIT(CLK_OUT_INV       ,1)
	REGDEF_BIT(                  ,15)
REGDEF_END(NAND_SPI_CFG_REG)

#define NAND_CTRL0_REG_OFS         0x20
REGDEF_BEGIN(NAND_CTRL0_REG)
	REGDEF_BIT(OPER_CMMD    ,6)
	REGDEF_BIT(             ,2)
	REGDEF_BIT(CHIP_EN      ,1)
	REGDEF_BIT(             ,3)
	REGDEF_BIT(OPER_EN      ,1)
	REGDEF_BIT(NAND_WP      ,1)
	REGDEF_BIT(             ,1)
	REGDEF_BIT(SOFT_RESET   ,1)
	REGDEF_BIT(MULTIPAGE_SEL,2)
	REGDEF_BIT(             ,1)
	REGDEF_BIT(TIMEOUT_EN   ,1)
	REGDEF_BIT(PROTECT_AREA1,1)
	REGDEF_BIT(PROTECT_AREA2,1)
	REGDEF_BIT(             ,10)
REGDEF_END(NAND_CTRL0_REG)

#define NAND_TIME2_REG_OFS        0x24
REGDEF_BEGIN(NAND_TIME2_REG)
	REGDEF_BIT(TSLCH      ,4)
	REGDEF_BIT(TSHCH      ,4)
	REGDEF_BIT(TSHSL      ,8)
	REGDEF_BIT(           ,16)
REGDEF_END(NAND_TIME2_REG)

#define NAND_SRAM_ACCESS_REG_OFS  0x28
REGDEF_BEGIN(NAND_SRAM_ACCESS_REG)
	REGDEF_BIT(SPARE_ACC  ,1)
	REGDEF_BIT(ERROR_ACC  ,1)
	REGDEF_BIT(           ,30)
REGDEF_END(NAND_SRAM_ACCESS_REG)

#define NAND_TIME0_REG_OFS	  0x2C
REGDEF_BEGIN(NAND_TIME0_REG)
	REGDEF_BIT(TRP		,4)
	REGDEF_BIT(TREH		,4)
	REGDEF_BIT(TWP		,4)
	REGDEF_BIT(TWH		,4)
	REGDEF_BIT(TCLS		,4)
	REGDEF_BIT(TCLH		,4)
	REGDEF_BIT(TADL		,4)
	REGDEF_BIT(TCLCH	,4)
REGDEF_END(NAND_TIME0_REG)

#define NAND_TIME1_REG_OFS	  0x30
REGDEF_BEGIN(NAND_TIME1_REG)
	REGDEF_BIT(TMPCEH	,8)
	REGDEF_BIT(TWB		,4)
	REGDEF_BIT(TMPRB	,4)
	REGDEF_BIT(TALS		,4)
	REGDEF_BIT(TALH		,4)
	REGDEF_BIT(reserved	,8)
REGDEF_END(NAND_TIME1_REG)


#define NAND_COMMAND_REG_OFS	  0x34
REGDEF_BEGIN(NAND_COMMAND_REG)
	REGDEF_BIT(CMD_CYC_1ST	,8)
	REGDEF_BIT(CMD_CYC_2ND	,8)
	REGDEF_BIT(CMD_CYC_3RD	,8)
	REGDEF_BIT(CMD_CYC_4TH	,8)
REGDEF_END(NAND_COMMAND_REG)

#define NAND_COLADDR_REG_OFS      0x38
REGDEF_BEGIN(NAND_COLADDR_REG)
	REGDEF_BIT(COLADDR_1ST  ,8)
	REGDEF_BIT(COLADDR_2ND  ,8)
	REGDEF_BIT(COLADDR_3RD  ,8)
	REGDEF_BIT(             ,8)
REGDEF_END(NAND_COLADDR_REG)

#define NAND_ROWADDR_REG_OFS	  0x3C
REGDEF_BEGIN(NAND_ROWADDR_REG)
	REGDEF_BIT(ROWADDR_1ST	,8)
	REGDEF_BIT(ROWADDR_2ND	,8)
	REGDEF_BIT(ROWADDR_3RD	,8)
	REGDEF_BIT(ROWADDR_4TH	,8)
REGDEF_END(NAND_ROWADDR_REG)

#define NAND_TIMEOUT_REG_OFS	  0x40
REGDEF_BEGIN(NAND_TIMEOUT_REG)
	REGDEF_BIT(TOUTVALUE	,16)
	REGDEF_BIT(reserved	,16)
REGDEF_END(NAND_TIMEOUT_REG)

#define NAND_INTMASK_REG_OFS	  0x44
REGDEF_BEGIN(NAND_INTMASK_REG)
	REGDEF_BIT(reserved1	,12)
	REGDEF_BIT(COMP_INTEN	,1)
	REGDEF_BIT(PRI_ECC_INTEN,1)
	REGDEF_BIT(STSFAIL_INTEN,1)
	REGDEF_BIT(TOUT_INTEN	,1)
	REGDEF_BIT(SEC_ECC_INTEN,1)
	REGDEF_BIT(PROTECT1_INTEN,1)
	REGDEF_BIT(PROTECT2_INTEN,1)
	REGDEF_BIT(reserved2	,13)
REGDEF_END(NAND_INTMASK_REG)


#define NAND_CTRL_STS_REG_OFS	  0x48
REGDEF_BEGIN(NAND_CTRL_STS_REG)
	REGDEF_BIT(SM_STS	,8)
	REGDEF_BIT(BUSY_STS	,1)
	REGDEF_BIT(reserved1	,3)
	REGDEF_BIT(COMP_STS	,1)
	REGDEF_BIT(PRI_ECC_STS	,1)
	REGDEF_BIT(STSFAIL_STS	,1)
	REGDEF_BIT(TOUT_STS	,1)
	REGDEF_BIT(SEC_ECC_STS	,1)
	REGDEF_BIT(PROTECT1_STS	,1)
	REGDEF_BIT(PROTECT2_STS	,1)
	REGDEF_BIT(reserved	,13)
REGDEF_END(NAND_CTRL_STS_REG)


#define NAND_PAGENUM_REG_OFS	  0x4C
REGDEF_BEGIN(NAND_PAGENUM_REG)
	REGDEF_BIT(PAGENUM	,12)
	REGDEF_BIT(reserved	,20)
REGDEF_END(NAND_PAGENUM_REG)



#define NAND_HAMERR_STS0_REG_OFS  0x50
REGDEF_BEGIN(NAND_HAMERR_STS0_REG)
	REGDEF_BIT(SEC0F1_ERR_STS ,2)
	REGDEF_BIT(SEC0F2_ERR_STS ,2)
	REGDEF_BIT(SEC1F1_ERR_STS ,2)
	REGDEF_BIT(SEC1F2_ERR_STS ,2)
	REGDEF_BIT(SEC2F1_ERR_STS ,2)
	REGDEF_BIT(SEC2F2_ERR_STS ,2)
	REGDEF_BIT(SEC3F1_ERR_STS ,2)
	REGDEF_BIT(SEC3F2_ERR_STS ,2)
	REGDEF_BIT(SEC4F1_ERR_STS ,2)
	REGDEF_BIT(SEC4F2_ERR_STS ,2)
	REGDEF_BIT(SEC5F1_ERR_STS ,2)
	REGDEF_BIT(SEC5F2_ERR_STS ,2)
	REGDEF_BIT(SEC6F1_ERR_STS ,2)
	REGDEF_BIT(SEC6F2_ERR_STS ,2)
	REGDEF_BIT(SEC7F1_ERR_STS ,2)
	REGDEF_BIT(SEC7F2_ERR_STS ,2)
REGDEF_END(NAND_HAMERR_STS0_REG)

#define NAND_RSERR_STS0_REG_OFS   0x70
REGDEF_BEGIN(NAND_RSERR_STS0_REG)
	REGDEF_BIT(SEC0_ERR_STS ,4)
	REGDEF_BIT(SEC1_ERR_STS ,4)
	REGDEF_BIT(SEC2_ERR_STS ,4)
	REGDEF_BIT(SEC3_ERR_STS ,4)
	REGDEF_BIT(SEC4_ERR_STS ,4)
	REGDEF_BIT(SEC5_ERR_STS ,4)
	REGDEF_BIT(SEC6_ERR_STS ,4)
	REGDEF_BIT(SEC7_ERR_STS ,4)
REGDEF_END(NAND_RSERR_STS0_REG)

#define NAND_SECONDARY_ECC_STS_REG_OFS	0x80
REGDEF_BEGIN(NAND_SECONDARY_ECC_STS_REG)
	REGDEF_BIT(SEC_SEC0_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC1_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC2_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC3_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC4_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC5_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC6_ERR_STS ,2)
	REGDEF_BIT(SEC_SEC7_ERR_STS ,2)
	REGDEF_BIT(reserved	    ,16)
REGDEF_END(NAND_SECONDARY_ECC_STS_REG)


#define NAND_PROTECT_AREA1_ROW_START_ADDR_OFS	0x90

#define NAND_PROTECT_AREA1_ROW_END_ADDR_OFS	0x94

#define NAND_PROTECT_AREA2_ROW_START_ADDR_OFS	0x98

#define NAND_PROTECT_AREA2_ROW_END_ADDR_OFS	0x9C

#define NAND_MULTISPARE_INTERVAL_OFS		0xB0
#define NAND_COMMAND2_REG_OFS      0xB4
REGDEF_BEGIN(NAND_COMMAND2_REG)
	REGDEF_BIT(CMD_CYC_5TH  ,8)
	REGDEF_BIT(CMD_CYC_6TH  ,8)
	REGDEF_BIT(CMD_CYC_7TH  ,8)
	REGDEF_BIT(CMD_CYC_8TH  ,8)
REGDEF_END(NAND_COMMAND2_REG)

#define NAND_STATUS_CHECK2_REG_OFS           0xB8
REGDEF_BEGIN(NAND_STATUS_CHECK2_REG)
	REGDEF_BIT(STATUS_VALUE ,8)
	REGDEF_BIT(STATUS_MASK  ,8)
	REGDEF_BIT(             ,16)
REGDEF_END(NAND_STATUS_CHECK2_REG)

#define NAND_STATUS_CHECK_REG_OFS           0xBC
REGDEF_BEGIN(NAND_STATUS_CHECK_REG)
	REGDEF_BIT(STATUS_VALUE ,8)
	REGDEF_BIT(STATUS_MASK  ,8)
	REGDEF_BIT(             ,16)
REGDEF_END(NAND_STATUS_CHECK_REG)


#define NAND_DUMMY_CLOCK_NUM_OFS   0xC0
REGDEF_BEGIN(NAND_DUMMY_CLOCK_NUM)
	REGDEF_BIT(DUMMY_CLK ,3)
	REGDEF_BIT(          ,29)
REGDEF_END(NAND_DUMMY_CLOCK_NUM)

#define NAND_DATAPORT_REG_OFS	  0x100
REGDEF_BEGIN(NAND_DATAPORT_REG)
	REGDEF_BIT(DATA , 32)
REGDEF_END(NAND_DATAPORT_REG)

#define NAND_DATALEN_REG_OFS	  0x104
REGDEF_BEGIN(NAND_DATALEN_REG)
	REGDEF_BIT(LENGTH	, 26)
	REGDEF_BIT(reserved	, 6)
REGDEF_END(NAND_DATALEN_REG)

#define NAND_FIFO_STS_REG_OFS	  0x108
REGDEF_BEGIN(NAND_FIFO_STS_REG)
	REGDEF_BIT(FIFO_CNT	, 5)
	REGDEF_BIT(reserved1	, 3)
	REGDEF_BIT(FIFO_EMPTY	, 1)
	REGDEF_BIT(FIFO_FULL	, 1)
	REGDEF_BIT(reserved2	, 22)
REGDEF_END(NAND_FIFO_STS_REG)


#define NAND_FIFO_CTRL_REG_OFS	  0x10C
REGDEF_BEGIN(NAND_FIFO_CTRL_REG)
	REGDEF_BIT(FIFO_EN	, 1)
	REGDEF_BIT(FIFO_MODE	, 1)
	REGDEF_BIT(FIFO_DIR	, 1)
	REGDEF_BIT(reserved	, 29)
REGDEF_END(NAND_FIFO_CTRL_REG)

#define NAND_DMASTART_REG_OFS	  0x110
REGDEF_BEGIN(NAND_DMASTART_REG)
	REGDEF_BIT(DMASTADR	, 30)
	REGDEF_BIT(reserved	, 2)
REGDEF_END(NAND_DMASTART_REG)


#define NAND_DMACURRENT_REG_OFS   0x114
REGDEF_BEGIN(NAND_DMACURRENT_REG)
	REGDEF_BIT(DMACURADR	, 30)
	REGDEF_BIT(reserved	, 2)
REGDEF_END(NAND_DMACURRENT_REG)

#define NAND_DEBUG_SEL_REG_OFS   0x140
REGDEF_BEGIN(NAND_DEBUG_SEL_REG)
	REGDEF_BIT(            ,4)
	REGDEF_BIT(FIX_TCLCH   ,1)
	REGDEF_BIT(debug_sel   ,27)
REGDEF_END(NAND_DEBUG_SEL_REG)



// for page 512/2K
#define NAND_SPARE00_REG_OFS      0x200
REGDEF_BEGIN(NAND_SPARE00_REG)
	REGDEF_BIT(SPARE00   , 32)
REGDEF_END(NAND_SPARE00_REG)

#define NAND_SPARE01_REG_OFS	  0x204
REGDEF_BEGIN(NAND_SPARE01_REG)
	REGDEF_BIT(SPARE01   , 32)
REGDEF_END(NAND_SPARE01_REG)

#define NAND_SPARE02_REG_OFS	  0x208
REGDEF_BEGIN(NAND_SPARE02_REG)
	REGDEF_BIT(SPARE02   , 32)
REGDEF_END(NAND_SPARE02_REG)

#define NAND_SPARE03_REG_OFS	  0x20C
REGDEF_BEGIN(NAND_SPARE03_REG)
	REGDEF_BIT(SPARE03   , 32)
REGDEF_END(NAND_SPARE03_REG)

#define NAND_SPARE04_REG_OFS	  0x210
REGDEF_BEGIN(NAND_SPARE04_REG)
	REGDEF_BIT(SPARE04   , 32)
REGDEF_END(NAND_SPARE04_REG)

#define NAND_SPARE05_REG_OFS	  0x214
REGDEF_BEGIN(NAND_SPARE05_REG)
	REGDEF_BIT(SPARE05   , 32)
REGDEF_END(NAND_SPARE05_REG)

#define NAND_SPARE06_REG_OFS	  0x218
REGDEF_BEGIN(NAND_SPARE06_REG)
	REGDEF_BIT(SPARE06   , 32)
REGDEF_END(NAND_SPARE06_REG)

#define NAND_SPARE07_REG_OFS	  0x21C
REGDEF_BEGIN(NAND_SPARE07_REG)
	REGDEF_BIT(SPARE07   , 32)
REGDEF_END(NAND_SPARE07_REG)

#define NAND_SPARE08_REG_OFS	  0x220
REGDEF_BEGIN(NAND_SPARE08_REG)
	REGDEF_BIT(SPARE08   , 32)
REGDEF_END(NAND_SPARE08_REG)

#define NAND_SPARE09_REG_OFS	  0x224
REGDEF_BEGIN(NAND_SPARE09_REG)
	REGDEF_BIT(SPARE09   , 32)
REGDEF_END(NAND_SPARE09_REG)

#define NAND_SPARE10_REG_OFS	  0x228
REGDEF_BEGIN(NAND_SPARE10_REG)
	REGDEF_BIT(SPARE10   , 32)
REGDEF_END(NAND_SPARE10_REG)

#define NAND_SPARE11_REG_OFS	  0x22C
REGDEF_BEGIN(NAND_SPARE11_REG)
	REGDEF_BIT(SPARE11   , 32)
REGDEF_END(NAND_SPARE11_REG)

#define NAND_SPARE12_REG_OFS	  0x230
REGDEF_BEGIN(NAND_SPARE12_REG)
	REGDEF_BIT(SPARE12   , 32)
REGDEF_END(NAND_SPARE12_REG)

#define NAND_SPARE13_REG_OFS	  0x234
REGDEF_BEGIN(NAND_SPARE13_REG)
	REGDEF_BIT(SPARE13   , 32)
REGDEF_END(NAND_SPARE13_REG)

#define NAND_SPARE14_REG_OFS	  0x238
REGDEF_BEGIN(NAND_SPARE14_REG)
	REGDEF_BIT(SPARE14   , 32)
REGDEF_END(NAND_SPARE14_REG)

#define NAND_SPARE15_REG_OFS	  0x23C
REGDEF_BEGIN(NAND_SPARE15_REG)
	REGDEF_BIT(SPARE15   , 32)
REGDEF_END(NAND_SPARE15_REG)

#define NAND_SPARE16_REG_OFS	  0x240
REGDEF_BEGIN(NAND_SPARE16_REG)
	REGDEF_BIT(SPARE16   , 32)
REGDEF_END(NAND_SPARE16_REG)

#define NAND_SPARE17_REG_OFS	  0x244
REGDEF_BEGIN(NAND_SPARE17_REG)
	REGDEF_BIT(SPARE17   , 32)
REGDEF_END(NAND_SPARE17_REG)

#define NAND_SPARE18_REG_OFS	  0x248
REGDEF_BEGIN(NAND_SPARE18_REG)
	REGDEF_BIT(SPARE18   , 32)
REGDEF_END(NAND_SPARE18_REG)

#define NAND_SPARE19_REG_OFS	  0x24C
REGDEF_BEGIN(NAND_SPARE19_REG)
	REGDEF_BIT(SPARE19   , 32)
REGDEF_END(NAND_SPARE19_REG)

#define NAND_SPARE20_REG_OFS	  0x250
REGDEF_BEGIN(NAND_SPARE20_REG)
	REGDEF_BIT(SPARE20   , 32)
REGDEF_END(NAND_SPARE20_REG)

#define NAND_SPARE21_REG_OFS	  0x254
REGDEF_BEGIN(NAND_SPARE21_REG)
	REGDEF_BIT(SPARE21   , 32)
REGDEF_END(NAND_SPARE21_REG)

#define NAND_SPARE22_REG_OFS	  0x258
REGDEF_BEGIN(NAND_SPARE22_REG)
	REGDEF_BIT(SPARE22   , 32)
REGDEF_END(NAND_SPARE22_REG)

#define NAND_SPARE23_REG_OFS	  0x25C
REGDEF_BEGIN(NAND_SPARE23_REG)
	REGDEF_BIT(SPARE23   , 32)
REGDEF_END(NAND_SPARE23_REG)

#define NAND_SPARE24_REG_OFS	  0x260
REGDEF_BEGIN(NAND_SPARE24_REG)
	REGDEF_BIT(SPARE24   , 32)
REGDEF_END(NAND_SPARE24_REG)

#define NAND_SPARE25_REG_OFS	  0x264
REGDEF_BEGIN(NAND_SPARE25_REG)
	REGDEF_BIT(SPARE25   , 32)
REGDEF_END(NAND_SPARE25_REG)

#define NAND_SPARE26_REG_OFS	  0x268
REGDEF_BEGIN(NAND_SPARE26_REG)
	REGDEF_BIT(SPARE26   , 32)
REGDEF_END(NAND_SPARE26_REG)

#define NAND_SPARE27_REG_OFS	  0x26C
REGDEF_BEGIN(NAND_SPARE27_REG)
	REGDEF_BIT(SPARE27   , 32)
REGDEF_END(NAND_SPARE27_REG)

#define NAND_SPARE28_REG_OFS	  0x270
REGDEF_BEGIN(NAND_SPARE28_REG)
	REGDEF_BIT(SPARE28   , 32)
REGDEF_END(NAND_SPARE28_REG)

#define NAND_SPARE29_REG_OFS	  0x274
REGDEF_BEGIN(NAND_SPARE29_REG)
	REGDEF_BIT(SPARE29   , 32)
REGDEF_END(NAND_SPARE29_REG)

#define NAND_SPARE30_REG_OFS	  0x278
REGDEF_BEGIN(NAND_SPARE30_REG)
	REGDEF_BIT(SPARE30   , 32)
REGDEF_END(NAND_SPARE30_REG)

#define NAND_SPARE31_REG_OFS	  0x27C
REGDEF_BEGIN(NAND_SPARE31_REG)
	REGDEF_BIT(SPARE31   , 32)
REGDEF_END(NAND_SPARE31_REG)

#define NAND_SEC0_EADDR0_REG_OFS	0x400
REGDEF_BEGIN(NAND_SEC0_EADDR0_REG)
	REGDEF_BIT(SEC0_EADDR0	 , 32)
REGDEF_END(NAND_SEC0_EADDR0_REG)

#define NAND_SEC0_EADDR1_REG_OFS	0x404
REGDEF_BEGIN(NAND_SEC0_EADDR1_REG)
	REGDEF_BIT(SEC0_EADDR1	 , 32)
REGDEF_END(NAND_SEC0_EADDR1_REG)

#define NAND_SEC0_EADDR2_REG_OFS	0x408
REGDEF_BEGIN(NAND_SEC0_EADDR2_REG)
	REGDEF_BIT(SEC0_EADDR2   , 32)
REGDEF_END(NAND_SEC0_EADDR2_REG)

#define NAND_SEC0_EADDR3_REG_OFS	0x40C
REGDEF_BEGIN(NAND_SEC0_EADDR3_REG)
	REGDEF_BIT(SEC0_EADDR3   , 32)
REGDEF_END(NAND_SEC0_EADDR3_REG)


REGDEF_BEGIN(NAND_TBL_TIMING1_REG)
	REGDEF_BIT(Timing1          ,24)    // bit[23..0] timing1
	REGDEF_BIT(InternalClock    ,2)     // bit[25..24]clock
                                        //  00:48 MHz
                                        //  01:60 MHz
                                        //  02:96 MHz
	REGDEF_BIT(DelayLatch       ,1)     //  0: disable
                                        //  1: enable
	REGDEF_BIT(CacheWrite       ,1)     // bit[27] 1: cache write / 0: no cache write
	REGDEF_BIT(CacheRead        ,1)     // bit[28] 1: cache read / 0 : no cache read
	REGDEF_BIT(Reserved         ,3)
REGDEF_END(NAND_TBL_TIMING1_REG)
#endif

