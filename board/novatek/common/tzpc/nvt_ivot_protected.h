#ifndef _DMA_PROTECTED_H_
#define _DMA_PROTECTED_H_

#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/rcw_macro.h>


#ifdef __DBG_COLOR_MONO__
#define DBG_COLOR_GRAY ""
#define DBG_COLOR_RED ""
#define DBG_COLOR_YELLOW ""
#define DBG_COLOR_CYAN ""
#define DBG_COLOR_GREEN ""
#define DBG_COLOR_MAGENTA ""
#define DBG_COLOR_BLUE ""
#define DBG_COLOR_WHITE ""
#define DBG_COLOR_HI_GRAY ""
#define DBG_COLOR_HI_RED ""
#define DBG_COLOR_HI_YELLOW ""
#define DBG_COLOR_HI_CYAN ""
#define DBG_COLOR_HI_GREEN ""
#define DBG_COLOR_HI_MAGENTA ""
#define DBG_COLOR_HI_BLUE ""
#define DBG_COLOR_HI_WHITE ""
#define DBG_COLOR_FATAL ""
#define DBG_COLOR_ERR ""
#define DBG_COLOR_WRN ""
#define DBG_COLOR_UNIT ""
#define DBG_COLOR_FUNC ""
#define DBG_COLOR_IND ""
#define DBG_COLOR_MSG ""
#define DBG_COLOR_VALUE ""
#define DBG_COLOR_USER ""
#define DBG_COLOR_END ""
#else
#define DBG_COLOR_GRAY "\033[0;30m"
#define DBG_COLOR_RED "\033[0;31m"
#define DBG_COLOR_YELLOW "\033[0;33m"
#define DBG_COLOR_CYAN "\033[0;36m"
#define DBG_COLOR_GREEN "\033[0;32m"
#define DBG_COLOR_MAGENTA "\033[0;35m"
#define DBG_COLOR_BLUE "\033[0;34m"
#define DBG_COLOR_WHITE "\033[0;37m"
#define DBG_COLOR_HI_GRAY "\033[1;30m"
#define DBG_COLOR_HI_RED "\033[1;31m"
#define DBG_COLOR_HI_YELLOW "\033[1;33m"
#define DBG_COLOR_HI_CYAN "\033[1;36m"
#define DBG_COLOR_HI_GREEN "\033[1;32m"
#define DBG_COLOR_HI_MAGENTA "\033[1;35m"
#define DBG_COLOR_HI_BLUE "\033[1;34m"
#define DBG_COLOR_HI_WHITE "\033[1;37m"
#define DBG_COLOR_FATAL "\033[1;31m"
#define DBG_COLOR_ERR "\033[1;31m"
#define DBG_COLOR_WRN "\033[1;33m"
#define DBG_COLOR_UNIT "\033[0;32m"
#define DBG_COLOR_FUNC "\033[0;36m"
#define DBG_COLOR_IND ""
#define DBG_COLOR_MSG ""
#define DBG_COLOR_VALUE ""
#define DBG_COLOR_USER ""
#define DBG_COLOR_END "\033[0m"
#endif

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif
/*
    Translate DRAM address to physical address.

    Translate DRAM address to physical address.

    @param[in] addr     DRAM address

    @return physical DRAM address
*/
#define         dma_getPhyAddr(addr)            ((((UINT32)(addr))>=0x60000000UL)?((UINT32)(addr)-0x60000000UL):(UINT32)(addr))

/*
    Translate DRAM address to non-cacheable address.

    Translate DRAM address to non-cacheable address.

    @param[in] addr     DRAM address

    @return non-cacheable DRAM address
*/
#define         dma_getNonCacheAddr(addr)       ((((UINT32)(addr))<0x60000000UL)?((UINT32)(addr)+0x60000000UL):(UINT32)(addr))


/**
    DMA controller ID

*/
typedef enum {
	DMA_ID_1,                           ///< DMA Controller
	DMA_ID_2,                           ///< DMA Controller 2

	DMA_ID_COUNT,                       //< DMA controller count

	ENUM_DUMMY4WORD(DMA_ID)
} DMA_ID;

/**
    Driver callback function

    uiEvent is bitwise event, please refer to each module's document.

*/
typedef void (*DRV_CB)(UINT32 uiEvent);
/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/
typedef struct {
	// ch 0
	UINT32 bReserved0: 1;                       //< bit0: reserved (auto refresh)
	UINT32 bCPU_NS: 1;                          //< CPU
	UINT32 bUSB: 1;                             //< USB_0
	UINT32 bUSB_1: 1;                           //< USB_1
	UINT32 bDCE_0: 1;                           //< DCE_0 (in)
	UINT32 bDCE_1: 1;                           //< DCE_1 (in)
	UINT32 bDCE_2: 1;                           //< DCE_2 (out)
	UINT32 bDCE_3: 1;                           //< DCE_3 (out)
	UINT32 bDCE_4: 1;                           //< DCE_4 (out)
	UINT32 bDCE_5: 1;                           //< DCE_5 (in)
	UINT32 bDCE_6: 1;                           //< DCE_6 (out)
	UINT32 bGRA_0: 1;                           //< GRA_0 (in)
	UINT32 bGRA_1: 1;                           //< GRA_1 (in)
	UINT32 bGRA_2: 1;                           //< GRA_2 (in)
	UINT32 bGRA_3: 1;                           //< GRA_3 (out)
	UINT32 bGRA_4: 1;                           //< GRA_4 (out)
	// ch 16
	UINT32 bGRA2_0: 1;                          //< GRA2_0 (in)
	UINT32 bGRA2_1: 1;                          //< GRA2_1 (in)
	UINT32 bGRA2_2: 1;                          //< GRA2_2 (out)
	UINT32 bJPG_0: 1;                           //< JPG IMG
	UINT32 bJPG_1: 1;                           //< JPG BS
	UINT32 bJPG_2: 1;                           //< JPG Enc DC out
	UINT32 bIPE_0: 1;                           //< IPE_0 (in)
	UINT32 bIPE_1: 1;                           //< IPE_1 (in)
	UINT32 bIPE_2: 1;                           //< IPE_2 (out)
	UINT32 bIPE_3: 1;                           //< IPE_3 (out)
	UINT32 bIPE_4: 1;                           //< IPE_4 (out)
	UINT32 bIPE_5: 1;                           //< IPE_5 (out)
	UINT32 bIPE_6: 1;                           //< IPE_6 (in)
	UINT32 bSIE_0: 1;                           //< SIE_0 (out)
	UINT32 bSIE_1: 1;                           //< SIE_1 (out)
	UINT32 bSIE_2: 1;                           //< SIE_2 (in)
	// ch 32
	UINT32 bSIE_3: 1;                           //< SIE_3 (in)
	UINT32 bSIE2_0: 1;                          //< SIE2_0 (out)
	UINT32 bSIE2_1: 1;                          //< SIE2_1 (out)
	UINT32 bSIE2_2: 1;                          //< SIE2_2 (in)
	UINT32 bSIE2_3: 1;                          //< SIE2_3 (in)
	UINT32 bSIE3_0: 1;                          //< SIE3_0 (out)
	UINT32 bSIE3_1: 1;                          //< SIE3_1 (out)
	UINT32 bDIS_0: 1;                           //< DIS_0 (in/out)
	UINT32 bDIS_1: 1;                           //< DIS_1 (in)
	UINT32 bLARB: 1;                            //< Local arbiter (SIF/BMC/I2C/UART/SPI)
	UINT32 bDAI: 1;                             //< DAI
	UINT32 bIFE_0: 1;                           //< IFE_0 (in)
	UINT32 bIFE_1: 1;                           //< IFE_1 (in)
	UINT32 bIFE_2: 1;                           //< IFE_2 (out)
	UINT32 bIME_0: 1;                           //< IME_0 (in)
	UINT32 bIME_1: 1;                           //< IME_1 (in)
	// ch 48
	UINT32 bIME_2: 1;                           //< IME_2 (in)
	UINT32 bIME_3: 1;                           //< IME_3 (out)
	UINT32 bIME_4: 1;                           //< IME_4 (out)
	UINT32 bIME_5: 1;                           //< IME_5 (out)
	UINT32 bIME_6: 1;                           //< IME_6 (out)
	UINT32 bIME_7: 1;                           //< IME_7 (out)
	UINT32 bIME_8: 1;                           //< IME_8 (out)
	UINT32 bIME_9: 1;                           //< IME_9 (out)
	UINT32 bIME_A: 1;                           //< IME_A (out)
	UINT32 bIME_B: 1;                           //< IME_B (out)
	UINT32 bIME_C: 1;                           //< IME_C (in)
	UINT32 bIME_D: 1;                           //< IME_D (out)
	UINT32 bIME_E: 1;                           //< IME_E (in)
	UINT32 bIME_F: 1;                           //< IME_F (in)
	UINT32 bIME_10: 1;                          //< IME_10 (in)
	UINT32 bIME_11: 1;                          //< IME_11 (in)
	// ch 64
	UINT32 bIME_12: 1;                          //< IME_12 (out)
	UINT32 bIME_13: 1;                          //< IME_13 (out)
	UINT32 bIME_14: 1;                          //< IME_14 (in)
	UINT32 bIME_15: 1;                          //< IME_15 (out)
	UINT32 bIME_16: 1;                          //< IME_16 (in/out)
	UINT32 bIME_17: 1;                          //< IME_17 (out)
	UINT32 bISE_0: 1;                           //< ISE_0 (in)
	UINT32 bISE_1: 1;                           //< ISE_1 (out)
	UINT32 bIDE_0: 1;                           //< IDE_0 (in)
	UINT32 bIDE_1: 1;                           //< IDE_1 (in)
	UINT32 bIDE_2: 1;                           //< IDE_2 (in/out)
	UINT32 bIDE_3: 1;                           //< IDE_3 (in/out)
	UINT32 bIDE_4: 1;                           //< IDE_4 (in)
	UINT32 bIDE_5: 1;                           //< IDE_5 (in)
	UINT32 bSDIO: 1;                            //< SDIO
	UINT32 bSDIO2: 1;                           //< SDIO2
	// ch 80
	UINT32 bSDIO3: 1;                           //< SDIO3
	UINT32 bNAND: 1;                            //< NAND
	UINT32 bH264_0: 1;                          //< H.264_0
	UINT32 bH264_1: 1;                          //< H.264_1
	UINT32 bH264_3: 1;                          //< H.264_3
	UINT32 bH264_4: 1;                          //< H.264_4
	UINT32 bH264_5: 1;                          //< H.264_5
	UINT32 bH264_6: 1;                          //< H.264_6
	UINT32 bH264_7: 1;                          //< H.264_7
	UINT32 bH264_8: 1;                          //< H.264_8
	UINT32 bH264_9: 1;                          //< H.264_9 (COE)
	UINT32 bIFE2_0: 1;                          //< IFE2_0 (in)
	UINT32 bIFE2_1: 1;                          //< IFE2_1 (out)
	UINT32 bETHERNET: 1;                        //< Ethernet
	UINT32 bTSE: 1;                             //< TSE input
	UINT32 bTSE_1: 1;                           //< TSE output
	// ch 96
	UINT32 bCRYPTO: 1;                          //< CRYPTO (in/out)
	UINT32 bHASH: 1;                            //< Hash (in/out)
	UINT32 bCNN_0: 1;                           //< CNN_0 (in)
	UINT32 bCNN_1: 1;                           //< CNN_1 (in)
	UINT32 bCNN_2: 1;                           //< CNN_2 (in)
	UINT32 bCNN_3: 1;                           //< CNN_3 (in)
	UINT32 bCNN_4: 1;                           //< CNN_4 (out)
	UINT32 bCNN_5: 1;                           //< CNN_5 (out)
	UINT32 bNUE_0: 1;                           //< NUE_0 (in)
	UINT32 bNUE_1: 1;                           //< NUE_1 (in)
	UINT32 bNUE_2: 1;                           //< NUE_2 (out)
	UINT32 bNUE2_0: 1;                          //< NUE2_0 (in)
	UINT32 bNUE2_1: 1;                          //< NUE2_1 (in)
	UINT32 bNUE2_2: 1;                          //< NUE2_2 (in)
	UINT32 bNUE2_3: 1;                          //< NUE2_3 (out)
	UINT32 bNUE2_4: 1;							//< NUE2_4 (out)
	// ch 112
	UINT32 bNUE2_5: 1;							//< NUE2_5 (out)
	UINT32 bMDBC_0: 1;                          //< MDBC_0 (in)
	UINT32 bMDBC_1: 1;							//< MDBC_1 (in)
	UINT32 bMDBC_2: 1;							//< MDBC_2 (in)
	UINT32 bMDBC_3: 1;							//< MDBC_3 (in)
	UINT32 bMDBC_4: 1;							//< MDBC_4 (in)
	UINT32 bMDBC_5: 1;							//< MDBC_5 (in)
	UINT32 bMDBC_6: 1;							//< MDBC_6 (out)
	UINT32 bMDBC_7: 1;							//< MDBC_7 (out)
	UINT32 bMDBC_8: 1;							//< MDBC_8 (in)
	UINT32 bMDBC_9: 1;							//< MDBC_9 (out)
	UINT32 bHLOAD_0: 1;                         //< Heavy load
	UINT32 bHLOAD_1: 1;							//< Heavy load
	UINT32 bHLOAD_2: 1;							//< Heavy load
	UINT32 bCNN2_0: 1;
	UINT32 bCNN2_1: 1;
	// ch 128
	UINT32 bCNN2_2: 1;
	UINT32 bCNN2_3: 1;
	UINT32 bCNN2_4: 1;
	UINT32 bCNN2_5: 1;
	UINT32 bDCE_7: 1;
	UINT32 bAFN_0: 1;
	UINT32 bAFN_1: 1;
	UINT32 bIVE_0: 1;
	UINT32 bIVE_1: 1;
	UINT32 bSIE4_0: 1; // new channel in 528(137)
	UINT32 bSIE4_1: 1;
	UINT32 bSIE4_2: 1;
	UINT32 bSIE4_3: 1;
	UINT32 bSIE5_0: 1;
	UINT32 bSIE5_1: 1;
	UINT32 bSIE5_2: 1;
	// ch 144
	UINT32 bSIE5_3: 1;
	UINT32 bSDE_0: 1;
	UINT32 bSDE_1: 1;
	UINT32 bSDE_2: 1;
	UINT32 bSDE_3: 1;
	UINT32 bCNN_6: 1;
	UINT32 bCNN2_6: 1;
	UINT32 bNUE_3: 1;
	UINT32 bNUE2_6: 1;
	UINT32 bISE_2: 1;
	UINT32 bLARB_2: 1;
	UINT32 bVPE_0: 1;
	UINT32 bVPE_1: 1;
	UINT32 bVPE_2: 1;
	UINT32 bVPE_3: 1;
	UINT32 bVPE_4: 1;
	// ch 160
	UINT32 bCPU: 1;
	UINT32 bSDE_4: 1;
	UINT32 reserved2: 14;
} DMA_CH_MSK, *PDMA_CH_MSK;

/*
    DRAM protect set

    DMA controller support max 3 write protect/detect set.
    Each set has individual protected/detected DRAM range
    and DMA channels to be protected/detected.

    @note Used in dma_chkDmaWR2ProtectAddr(), dma_clrWPStatus(), dma_getWPStatus(),
                  dma_configWPFunc(), dma_enableWPFunc(), dma_disableWPFunc()
*/
typedef enum {
	DMA_WPSET_0,            // Write protect function set 0
	DMA_WPSET_1,            // Write protect function set 1
	DMA_WPSET_2,            // Write protect function set 2
	DMA_WPSET_3,            // Write protect function set 3
	DMA_WPSET_4,            // Write protect function set 4
	DMA_WPSET_5,            // Write protect function set 5
	DMA_WPSET_TOTAL,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

/*
    DRAM protect region in a set
*/
typedef enum {
	DMA_PROT_RGN0,
	DMA_PROT_RGN1,
	DMA_PROT_RGN2,
	DMA_PROT_RGN3,
	DMA_PROT_RGN_TOTAL,
	ENUM_DUMMY4WORD(DMA_PROT_REGION)
} DMA_PROT_REGION;

/*
    DRAM write protect source
    (OBSOLETE)
*/
typedef enum {
	DMA_WPSRC_CPU,          // from CPU, including AMBA DMA, AHBC.
	DMA_WPSRC_DMA,          // DMA
	DMA_WPSRC_ALL,          // CPU+DMA

	ENUM_DUMMY4WORD(DMA_WRITEPROT_SRC)
} DMA_WRITEPROT_SRC;

/*
    DRAM write protect detect level
*/
typedef enum {
	DMA_WPLEL_UNWRITE,      // Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       // Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       // Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        // Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

/*
    DRAM protect detect level(NEW)
*/
typedef enum {
	DMA_PROT_WP,            // Not only detect write action but also denial access.
	DMA_PROT_WD,            // Only detect write action but allow write access.
	DMA_PROT_RP,
	DMA_PROT_RWP,
	ENUM_DUMMY4WORD(DMA_PROT_LEVEL)
} DMA_PROT_LEVEL;

/*
    DRAM protect mode(NEW)
*/
typedef enum {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;


/*
    DMA controller channel group number

    Each group represent 32 bit for 32 channel, there are 9x channel => need 4 group
*/
typedef enum {
	DMA_CH_GROUP0 = 0x0,    // represent channel 00-31
	DMA_CH_GROUP1,          // represent channel 32-63
	DMA_CH_GROUP2,          // represent channel 64-95
	DMA_CH_GROUP3,          // represent channel 96-127
    DMA_CH_GROUP4,          // represent channel 128-159
    DMA_CH_GROUP5,          // represent channel 160-191

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;

/*
    @name DMA channel encoding

    DMA channel setting

    @note Used in dma_setChannelPriority(), dma_getChannelPriority(),
                  dma_enableChannel(), dma_disableChannel()
*/
typedef enum {
	DMA_CH_RSV = 0x00,
	DMA_CH_FIRST_CHANNEL,
	DMA_CH_CPU = DMA_CH_FIRST_CHANNEL,
	DMA_CH_USB,
	DMA_CH_USB_1,
	DMA_CH_DCE_0,           // DCE input
	DMA_CH_DCE_1,           // DCE input
	DMA_CH_DCE_2,           // DCE output
	DMA_CH_DCE_3,           // DCE output
	DMA_CH_DCE_4,           // DCE output
	DMA_CH_DCE_5,           // DCE input
	DMA_CH_DCE_6,           // DCE output
	DMA_CH_GRA_0,           // Graphic Input 0
	DMA_CH_GRA_1,           // Graphic Input 1
	DMA_CH_GRA_2,           // Graphic Input 2
	DMA_CH_GRA_3,           // Graphic output
	DMA_CH_GRA_4,           // Graphic output
	// Ctrl 0

	DMA_CH_GRA2_0,          // Graphic2 input
	DMA_CH_GRA2_1,          // Graphic2 input
	DMA_CH_GRA2_2,          // Graphic2 output
	DMA_CH_JPG0,            // JPG IMG
	DMA_CH_JPG1,            // JPG BS
	DMA_CH_JPG2,            // JPG Encode mode DC output
	DMA_CH_IPE0,            // IPE input
	DMA_CH_IPE1,            // IPE input
	DMA_CH_IPE2,            // IPE output
	DMA_CH_IPE3,            // IPE output
	DMA_CH_IPE4,            // IPE output
	DMA_CH_IPE5,            // IPE output
	DMA_CH_IPE6,            // IPE input
	DMA_CH_SIE_0,           // SIE output
	DMA_CH_SIE_1,           // SIE output
	DMA_CH_SIE_2,           // SIE output
	// Ctrl 1

	DMA_CH_SIE_3,           // SIE input
	DMA_CH_SIE2_0,          // SIE2 output
	DMA_CH_SIE2_1,          // SIE2 output
	DMA_CH_SIE2_2,          // SIE2 input
	DMA_CH_SIE2_3,          // SIE2 input
	DMA_CH_SIE3_0,          // SIE3 output
	DMA_CH_SIE3_1,          // SIE3 output
	DMA_CH_DIS0,            // DIS
	DMA_CH_DIS1,            // DIS input
	DMA_CH_LARB,            // Local Arbit for SIF/BMC/I2C/UART/SPI
	DMA_CH_DAI,             // DAI
	DMA_CH_IFE_0,           // IFE input
	DMA_CH_IFE_1,           // IFE input
	DMA_CH_IFE_2,           // IFE output
	DMA_CH_IME_0,           // IME input
	DMA_CH_IME_1,           // IME input
	// Ctrl 2

	DMA_CH_IME_2,           // IME input
	DMA_CH_IME_3,           // IME output
	DMA_CH_IME_4,           // IME output
	DMA_CH_IME_5,           // IME output
	DMA_CH_IME_6,           // IME output
	DMA_CH_IME_7,           // IME output
	DMA_CH_IME_8,           // IME output
	DMA_CH_IME_9,           // IME output
	DMA_CH_IME_A,           // IME output
	DMA_CH_IME_B,           // IME output
	DMA_CH_IME_C,           // IME input
	DMA_CH_IME_D,           // IME output
	DMA_CH_IME_E,           // IME input
	DMA_CH_IME_F,           // IME input
	DMA_CH_IME_10,          // IME input
	DMA_CH_IME_11,          // IME input

	// Ctrl 3

	DMA_CH_IME_12,          // IME output
	DMA_CH_IME_13,          // IME output
	DMA_CH_IME_14,          // IME input
	DMA_CH_IME_15,          // IME output
	DMA_CH_IME_16,          // IME
	DMA_CH_IME_17,          // IME output
	DMA_CH_ISE_a0,          // ISE input
	DMA_CH_ISE_a1,          // ISE output
	DMA_CH_IDE_a0,          // IDE V1 Y (in)
	DMA_CH_IDE_b0,          // IDE V1 C (in)
	DMA_CH_IDE_a1,          // IDE V2 Y (in/out)
	DMA_CH_IDE_b1,          // IDE V2 C (in/out)
	DMA_CH_IDE_6,           // IDE O1 PAL/A (in)
	DMA_CH_IDE_7,           // IDE O1 RGB (in)
	DMA_CH_SDIO,            // SDIO
	DMA_CH_SDIO2,           // SDIO2
	// Ctrl 4

	DMA_CH_SDIO3,           // SDIO3
	DMA_CH_NAND,            // NAND
	DMA_CH_H264_0,          // H.264 (input)
	DMA_CH_H264_1,          // H.264 (input)
	DMA_CH_H264_3,          // H.264 (input/output)
	DMA_CH_H264_4,          // H.264 (input)
	DMA_CH_H264_5,          // H.264 (output)
	DMA_CH_H264_6,          // H.264 (input)
	DMA_CH_H264_7,          // H.264 (input)
	DMA_CH_H264_8,          // H.264 (output)
	DMA_CH_H264_9,          // H.264 (COE input)
	DMA_CH_IFE2_0,          // IFE2 input
	DMA_CH_IFE2_1,          // IFE2 output
	DMA_CH_Ethernet,        // Ethernet
	DMA_CH_TSE_0,           // TSE input
	DMA_CH_TSE_1,           // TSE output
	// Ctrl 5

	DMA_CH_CRYPTO,          // CRYPTO
	DMA_CH_HASH,            // Hash
	DMA_CH_CNN_0,           // CNN input
	DMA_CH_CNN_1,           // CNN input
	DMA_CH_CNN_2,           // CNN input
	DMA_CH_CNN_3,           // CNN input
	DMA_CH_CNN_4,           // CNN output
	DMA_CH_CNN_5,           // CNN output
	DMA_CH_NUE_0,           // NUE input
	DMA_CH_NUE_1,           // NUE input
	DMA_CH_NUE_2,           // NUE output
	DMA_CH_NUE2_0,          // NUE input
	DMA_CH_NUE2_1,          // NUE input
	DMA_CH_NUE2_2,          // NUE input
	DMA_CH_NUE2_3,          // NUE output
	DMA_CH_NUE2_4,			// NUE output
	// Ctrl 6

	DMA_CH_NUE2_5,			// NUE output
	DMA_CH_MDBC_0,          // MDBC input
	DMA_CH_MDBC_1,			// MDBC input
	DMA_CH_MDBC_2,			// MDBC input
	DMA_CH_MDBC_3,			// MDBC input
	DMA_CH_MDBC_4,			// MDBC input
	DMA_CH_MDBC_5,			// MDBC input
	DMA_CH_MDBC_6,			// MDBC output
	DMA_CH_MDBC_7,			// MDBC output
	DMA_CH_MDBC_8,			// MDBC input
	DMA_CH_MDBC_9,			// MDBC output
	DMA_CH_HLOAD_0,
	DMA_CH_HLOAD_1,
	DMA_CH_HLOAD_2,
	DMA_CH_CNN2_0,
	DMA_CH_CNN2_1,
	// Ctrl7

	DMA_CH_CNN2_2,
	DMA_CH_CNN2_3,
	DMA_CH_CNN2_4,
	DMA_CH_CNN2_5,
    DMA_CH_COUNT,

    DMA_CH_CPU_NS = 160,
	DMA_CH_ALL = DMA_CH_COUNT,
	ENUM_DUMMY4WORD(DMA_CH)
} DMA_CH;

/*
    @name DMA protect channel name
*/
typedef enum {
	DMA_WPCH_FIRST_CHANNEL = 1,

	DMA_WPCH_CPU = DMA_WPCH_FIRST_CHANNEL,
	DMA_WPCH_USB,
	DMA_WPCH_USB_1,
	DMA_WPCH_DCE_0,           // DCE input
	DMA_WPCH_DCE_1,           // DCE input
	DMA_WPCH_DCE_2,           // DCE output
	DMA_WPCH_DCE_3,           // DCE output
	DMA_WPCH_DCE_4,           // DCE input
	DMA_WPCH_DCE_5,           // DCE output
	DMA_WPCH_DCE_6,           // DCE output
	DMA_WPCH_GRA_0,           // Graphic Input 0
	DMA_WPCH_GRA_1,           // Graphic Input 1
	DMA_WPCH_GRA_2,           // Graphic Input 2
	DMA_WPCH_GRA_3,           // Graphic output
	DMA_WPCH_GRA_4,           // Graphic output
	// Ctrl 0

	DMA_WPCH_GRA2_0,          // Graphic2 input
	DMA_WPCH_GRA2_1,          // Graphic2 input
	DMA_WPCH_GRA2_2,          // Graphic2 output
	DMA_WPCH_JPG0,            // JPG IMG
	DMA_WPCH_JPG1,            // JPG BS
	DMA_WPCH_JPG2,            // JPG Encode mode DC output
	DMA_WPCH_IPE0,            // IPE input
	DMA_WPCH_IPE1,            // IPE input
	DMA_WPCH_IPE2,            // IPE output
	DMA_WPCH_IPE3,            // IPE output
	DMA_WPCH_IPE4,            // IPE output
	DMA_WPCH_IPE5,            // IPE output
	DMA_WPCH_IPE6,            // IPE output
	DMA_WPCH_SIE_0,           // SIE output
	DMA_WPCH_SIE_1,           // SIE output
	DMA_WPCH_SIE_2,           // SIE output
	// Ctrl 1

	DMA_WPCH_SIE_3,           // SIE input
	DMA_WPCH_SIE2_0,          // SIE2 output
	DMA_WPCH_SIE2_1,          // SIE2 output
	DMA_WPCH_SIE2_2,          // SIE2 output
	DMA_WPCH_SIE2_3,          // SIE2 input
	DMA_WPCH_SIE3_0,          // SIE2 output
	DMA_WPCH_SIE3_1,          // SIE2 output
	DMA_WPCH_DIS0,            // DIS input/output
	DMA_WPCH_DIS1,            // DIS input
	DMA_WPCH_LARB,            // Local Arbit for SIF/BMC/I2C/UART/SPI
	DMA_WPCH_DAI,             // DAI
	DMA_WPCH_IFE_0,           // IFE input
	DMA_WPCH_IFE_1,           // IFE output
	DMA_WPCH_IFE_2,           // IFE output
	DMA_WPCH_IME_0,           // IME input
	DMA_WPCH_IME_1,           // IME input
	// Ctrl 2

	DMA_WPCH_IME_2,           // IME input
	DMA_WPCH_IME_3,           // IME output
	DMA_WPCH_IME_4,           // IME output
	DMA_WPCH_IME_5,           // IME output
	DMA_WPCH_IME_6,           // IME output
	DMA_WPCH_IME_7,           // IME output
	DMA_WPCH_IME_8,           // IME output
	DMA_WPCH_IME_9,           // IME output
	DMA_WPCH_IME_A,           // IME output
	DMA_WPCH_IME_B,           // IME output
	DMA_WPCH_IME_C,           // IME input
	DMA_WPCH_IME_D,           // IME output
	DMA_WPCH_IME_E,           // IME input
	DMA_WPCH_IME_F,           // IME input
	DMA_WPCH_IME_10,           // IME input
	DMA_WPCH_IME_11,           // IME input

	// Ctrl 3

	DMA_WPCH_IME_12,           // IME input
	DMA_WPCH_IME_13,           // IME output
	DMA_WPCH_IME_14,           // IME input
	DMA_WPCH_IME_15,           // IME input
	DMA_WPCH_IME_16,           // IME input
	DMA_WPCH_IME_17,           // IME input
	DMA_WPCH_ISE_a0,          // ISE input
	DMA_WPCH_ISE_a1,          // ISE output
	DMA_WPCH_IDE_a0,          // IDE V1 Y
	DMA_WPCH_IDE_b0,          // IDE V1 C
	DMA_WPCH_IDE_a1,          // IDE V2 Y (in/out)
	DMA_WPCH_IDE_b1,          // IDE V2 C (in/out)
	DMA_WPCH_IDE_6,           // IDE O1 PAL/A
	DMA_WPCH_IDE_7,           // IDE O1 RGB
	DMA_WPCH_SDIO,            // SDIO
	DMA_WPCH_SDIO2,           // SDIO2
	// Ctrl 4

	DMA_WPCH_SDIO3,           // SDIO3
	DMA_WPCH_NAND,            // NAND
	DMA_WPCH_H264_0,          // H.264 (input)
	DMA_WPCH_H264_1,          // H.264 (input)
	DMA_WPCH_H264_3,          // H.264 (output)
	DMA_WPCH_H264_4,          // H.264 (in/out)
	DMA_WPCH_H264_5,          // H.264 (output)
	DMA_WPCH_H264_6,          // H.264 (input)
	DMA_WPCH_H264_7,          // H.264 (input)
	DMA_WPCH_H264_8,          // H.264 (output)
	DMA_WPCH_H264_9,          // H.264 (COE input)
	DMA_WPCH_IFE2_0,          // IFE2 input
	DMA_WPCH_IFE2_1,          // IFE2 output
	DMA_WPCH_Ethernet,        // Ethernet
	DMA_WPCH_TSE_0,             // TSE input
	DMA_WPCH_TSE_1,           // TSE output
	// Ctrl 5

	DMA_WPCH_CRYPTO,          // CRYPTO input/output
	DMA_WPCH_HASH,            // Hash input/output
	DMA_WPCH_CNN_0,          // IME
	DMA_WPCH_CNN_1,          // IME
	DMA_WPCH_CNN_2,          // IME
	DMA_WPCH_CNN_3,          // IME
	DMA_WPCH_CNN_4,          // IME
	DMA_WPCH_CNN_5,          // IME
	DMA_WPCH_NUE_0,          // IME
	DMA_WPCH_NUE_1,          // IME
	DMA_WPCH_NUE_2,          // IME
	DMA_WPCH_NUE2_0,          // IME
	DMA_WPCH_NUE2_1,          // IME
	DMA_WPCH_NUE2_2,          // JPG2 IMG
	DMA_WPCH_NUE2_3,          // JPG2 BS
	DMA_WPCH_NUE2_4,
	// Ctrl 6

	DMA_WPCH_NUE2_5,
	DMA_WPCH_MDBC_0,          // JPG2 Encode mode DC output
	DMA_WPCH_MDBC_1,
	DMA_WPCH_MDBC_2,
	DMA_WPCH_MDBC_3,
	DMA_WPCH_MDBC_4,
	DMA_WPCH_MDBC_5,
	DMA_WPCH_MDBC_6,
	DMA_WPCH_MDBC_7,
	DMA_WPCH_MDBC_8,
	DMA_WPCH_MDBC_9,
	DMA_WPCH_HLOAD_0,
	DMA_WPCH_HLOAD_1,
	DMA_WPCH_HLOAD_2,
	DMA_WPCH_CNN2_0,          // JPG2 Encode mode DC output
	DMA_WPCH_CNN2_1,
	// Ctrl 7

	DMA_WPCH_CNN2_2,
	DMA_WPCH_CNN2_3,
	DMA_WPCH_CNN2_4,
	DMA_WPCH_CNN2_5,
    DMA_WPCH_COUNT,

    DMA_WPCH_CPU_NS = 160,

	DMA_WPCH_ALL = DMA_WPCH_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_CH)
} DMA_WRITEPROT_CH;
//STATIC_ASSERT((DMA_WPCH_COUNT == DMA_CH_COUNT));

/*
    DRAM direction

    @note for dma_configMonitor()
*/
typedef enum {
	DMA_DIRECTION_READ,     // DMA read (DRAM -> Module)
	DMA_DIRECTION_WRITE,    // DMA write (DRAM <- Module)
	DMA_DIRECTION_BOTH,     // DMA read and write

	ENUM_DUMMY4WORD(DMA_DIRECTION)
} DMA_DIRECTION;

/*
    DMA controller write protect function configuration

    DMA controller write protect function configuration
*/
typedef struct {
	DMA_WRITEPROT_SRC   uiProtectSrc __attribute__((deprecated));   // (OBSOLETE) protect source
	DMA_CH_MSK          chEnMask;       // DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL uiProtectlel;   // protect level
	UINT32              uiStartingAddr; // DDR3:must be 4 words alignment
	UINT32              uiSize;         // DDR3:must be 4 words alignment
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;
/*
    DMA controller protect function configuration(NEW)

    DMA controller protect function configuration
*/
typedef struct {
	BOOL                en;            // enable this region
	UINT32              starting_addr; // DDR3:must be 4 words alignment
	UINT32              size;          // DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

#if 0
/*
    DMA controller protect function configuration(NEW)

    DMA controller protect function configuration
*/
typedef struct {
	DMA_CH_MSK          ch_en_mask;       // DMA channel masks to be protected/detected
	DMA_PROT_MODE       protect_mode;
	DMA_PROT_LEVEL      protect_level;    // protect level
	DMA_PROT_RGN_ATTR   protect_rgn_attr[DMA_PROT_RGN_TOTAL];
} DMA_PROT_ATTR, *PDMA_PROT_ATTR;
#endif

/*
    DMA controller protect function configuration(NEW)

    DMA controller protect function configuration
*/
typedef struct {
	DMA_CH_MSK          ch_en_mask;       // DMA channel masks to be protected/detected
	DMA_PROT_MODE       protect_mode;
	DMA_PROT_LEVEL      protect_level;    // protect level
	DMA_PROT_RGN_ATTR   protect_rgn_attr[DMA_PROT_RGN_TOTAL];
} DMA_PROT_ATTR, *PDMA_PROT_ATTR;

/*
    DMA controller write protect channel group

    DMA controller write protect channel group
*/
typedef struct {
	UINT32  uiChannelGroup[DMA_CH_GROUP_CNT];
} DMA_WP_STS_TYPE, *PDMA_WP_STS_TYPE;
/*
    Config DMA write protection function.

    This function is used to config DMA write protection function.
    The using right of write protect function must have been gotten before calling this function,

    @param[in] WpSet        Write protect function set
    @param[in] PprotectAttr Configuration for write protect function
    @param[in] pDrvcb       callback handler for write protect function

    @return void
*/
extern void     dma_configWPFunc(DMA_WRITEPROT_SET WpSet, PDMA_WRITEPROT_ATTR PprotectAttr, DRV_CB pDrvcb);
extern void     dma_config_wp_func(DMA_WRITEPROT_SET wp_set, PDMA_PROT_ATTR p_protect_attr, DRV_CB p_drv_cb);
/*
    Enable DMA write protect function.

    Enable DMA write protect function.

    @param[in] WpSet  Write protect function set

    @return void
*/
extern void     dma_enableWPFunc(DMA_WRITEPROT_SET WpSet);

/*
    Disable specific set of DMA write protect function.

    Disable specific set of DMA write protect function.

    @param[in] WpSet  Write protect function set

    @return void
*/
extern void     dma_disableWPFunc(DMA_WRITEPROT_SET WpSet);

/*
    Default API to show write protect channel detected

    Call dma_getWPStatus and send status into dma_showWPChannel, after this
    function called, write protect status will be cleared.

    @param[in] WpSet  Write protect function set
    @param[in] WpSts  Write protect function status

    @return void
*/
//extern UINT32 dma_getDramBaseAddr(DMA_ID id);
//extern UINT32 dma_getDramCapacity(DMA_ID id);

#define DMA_REG_ADDR(ofs)       (IOADDR_DRAM_REG_BASE+(ofs))
#define DMA_SETREG(ofs,value)   OUTW(DMA_REG_ADDR(ofs), (value))
#define DMA_GETREG(ofs)         INW(DMA_REG_ADDR(ofs))

#define PROTECT_REG_ADDR(ofs)       (IOADDR_DRAM_REG_BASE+0xFE0000+(ofs))
#define PROTECT_SETREG(ofs,value)   OUTW(PROTECT_REG_ADDR(ofs), (value))
#define PROTECT_GETREG(ofs)         INW(PROTECT_REG_ADDR(ofs))

#define DMA2_REG_ADDR(ofs)       (IOADDR_DRAM2_REG_BASE+(ofs))
#define DMA2_SETREG(ofs,value)   OUTW(DMA2_REG_ADDR(ofs), (value))
#define DMA2_GETREG(ofs)         INW(DMA2_REG_ADDR(ofs))

#define PROTECT2_REG_ADDR(ofs)       (IOADDR_DRAM_REG_BASE+0xFD0000+(ofs))
#define PROTECT2_SETREG(ofs,value)   OUTW(PROTECT2_REG_ADDR(ofs), (value))
#define PROTECT2_GETREG(ofs)         INW(PROTECT2_REG_ADDR(ofs))


#define DMA_PRI_BIT_MASK        (0x03)


////// PROTECT //////
#define DMA_PROTECT_CTRL_REG_OFS   0x00
REGDEF_BEGIN(DMA_PROTECT_CTRL_REG)    /* declare register cache type "DMA_PROTECT_CTRL_REG" begin */
REGDEF_BIT(DMA_WPWD0_EN, 1)
REGDEF_BIT(DMA_WPWD1_EN, 1)
REGDEF_BIT(DMA_WPWD2_EN, 1)
REGDEF_BIT(DMA_WPWD3_EN, 1)
REGDEF_BIT(DMA_WPWD4_EN, 1)
REGDEF_BIT(DMA_WPWD5_EN, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DMA_WPWD0_MODE, 1)
REGDEF_BIT(DMA_WPWD1_MODE, 1)
REGDEF_BIT(DMA_WPWD2_MODE, 1)
REGDEF_BIT(DMA_WPWD3_MODE, 1)
REGDEF_BIT(DMA_WPWD4_MODE, 1)
REGDEF_BIT(DMA_WPWD5_MODE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DMA_WPWD0_SEL, 2)
REGDEF_BIT(DMA_WPWD1_SEL, 2)
REGDEF_BIT(DMA_WPWD2_SEL, 2)
REGDEF_BIT(DMA_WPWD3_SEL, 2)
REGDEF_BIT(DMA_WPWD4_SEL, 2)
REGDEF_BIT(DMA_WPWD5_SEL, 2)
REGDEF_BIT(, 4)
REGDEF_END(DMA_PROTECT_CTRL_REG)      /* declare register cache type "DMA_PROTECT_CTRL_REG" end */


#define DMA_PROTECT_REGION_EN_REG0_OFS   0x40
REGDEF_BEGIN(DMA_PROTECT_REGION_EN_REG0)    /* declare register cache type "DMA_PROTECT_CTRL_REG" begin */
REGDEF_BIT(DRAM_PROTECT0_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT1_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT2_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT3_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_END(DMA_PROTECT_REGION_EN_REG0)      /* declare register cache type "DMA_PROTECT_CTRL_REG" end */

#define DMA_PROTECT_REGION_EN_REG1_OFS   0x44
REGDEF_BEGIN(DMA_PROTECT_REGION_EN_REG1)    /* declare register cache type "DMA_PROTECT_CTRL_REG" begin */
REGDEF_BIT(DRAM_PROTECT4_REGIONS_EN, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(DRAM_PROTECT5_REGIONS_EN, 4)
REGDEF_BIT(, 20)
REGDEF_END(DMA_PROTECT_REGION_EN_REG1)      /* declare register cache type "DMA_PROTECT_CTRL_REG" end */

#define DMA_PROTECT_STARTADDR0_REG0_OFS  0x100
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG0_OFS  0x104
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR0_REG1_OFS  0x108
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG1_OFS  0x10C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR0_REG2_OFS  0x110
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG2_OFS   0x114
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR0_REG3_OFS  0x118
REGDEF_BEGIN(DMA_PROTECT_STARTADDR0_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR0_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR0_REG3_OFS   0x11C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR0_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR0_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG0_OFS  0x120
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */



#define DMA_PROTECT_STOPADDR1_REG0_OFS  0x124
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG1_OFS  0x128
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */


#define DMA_PROTECT_STOPADDR1_REG1_OFS  0x12C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG2_OFS  0x130
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR1_REG2_OFS   0x134
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR1_REG3_OFS  0x138
REGDEF_BEGIN(DMA_PROTECT_STARTADDR1_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR1_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR1_REG3_OFS   0x13C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR1_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR1_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG0_OFS  0x140
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG0_OFS  0x144
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG1_OFS  0x148
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG1_OFS  0x14C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG2_OFS  0x150
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG2_OFS   0x154
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR2_REG3_OFS  0x158
REGDEF_BEGIN(DMA_PROTECT_STARTADDR2_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR2_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR2_REG3_OFS   0x15C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR2_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR2_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG0_OFS  0x160
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */


#define DMA_PROTECT_STOPADDR3_REG0_OFS  0x164
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG1_OFS  0x168
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG1_OFS  0x16C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */
#if 1
#define DMA_PROTECT_STARTADDR3_REG2_OFS  0x170
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG2_OFS   0x174
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR3_REG3_OFS  0x178
REGDEF_BEGIN(DMA_PROTECT_STARTADDR3_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR3_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR3_REG3_OFS   0x17C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR3_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR3_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG0_OFS  0x180
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG0_OFS  0x184
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG1_OFS  0x188
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG1_OFS  0x18C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG2_OFS  0x190
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG2_OFS   0x194
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR4_REG3_OFS  0x198
REGDEF_BEGIN(DMA_PROTECT_STARTADDR4_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR4_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR4_REG3_OFS   0x19C
REGDEF_BEGIN(DMA_PROTECT_STOPADDR4_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR4_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG0_OFS  0x1A0
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG0_OFS  0x1A4
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG0)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG0)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG1_OFS  0x1A8
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG1_OFS  0x1AC
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG1)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG1)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG2_OFS  0x1B0
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG2)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG2)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG2_OFS   0x1B4
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG2)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG2)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STARTADDR5_REG3_OFS  0x1B8
REGDEF_BEGIN(DMA_PROTECT_STARTADDR5_REG3)    /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STA_ADDR, 32)
REGDEF_END(DMA_PROTECT_STARTADDR5_REG3)      /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */

#define DMA_PROTECT_STOPADDR5_REG3_OFS   0x1BC
REGDEF_BEGIN(DMA_PROTECT_STOPADDR5_REG3)     /* declare register cache type "DMA_PROTECT_STAADR0_REG" begin */
REGDEF_BIT(STP_ADDR, 32)
REGDEF_END(DMA_PROTECT_STOPADDR5_REG3)       /* declare register cache type "DMA_PROTECT_STAADR0_REG" end */
#endif
// 0x88 ~ 0x8C: Reserved

#define DMA_PROTECT_RANGE0_MSK0_REG_OFS 0x200
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK0_REG" end */

#define DMA_PROTECT_RANGE0_MSK1_REG_OFS 0x204
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK1_REG" end */

#define DMA_PROTECT_RANGE0_MSK2_REG_OFS 0x208
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK2_REG" end */

#define DMA_PROTECT_RANGE0_MSK3_REG_OFS 0x20C
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK3_REG" end */

#define DMA_PROTECT_RANGE0_MSK4_REG_OFS 0x210
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK4_REG" end */

#define DMA_PROTECT_RANGE0_MSK5_REG_OFS 0x214
REGDEF_BEGIN(DMA_PROTECT_RANGE0_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE0_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE0_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE0_MSK5_REG" end */

#define DMA_PROTECT_RANGE1_MSK0_REG_OFS 0x220
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK0_REG" end */

#define DMA_PROTECT_RANGE1_MSK1_REG_OFS 0x224
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK1_REG" end */

#define DMA_PROTECT_RANGE1_MSK2_REG_OFS 0x228
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK2_REG" end */

#define DMA_PROTECT_RANGE1_MSK3_REG_OFS 0x22C
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK3_REG" end */

#define DMA_PROTECT_RANGE1_MSK4_REG_OFS 0x230
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK4_REG" end */

#define DMA_PROTECT_RANGE1_MSK5_REG_OFS 0x234
REGDEF_BEGIN(DMA_PROTECT_RANGE1_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE1_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE1_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE1_MSK5_REG" end */

#define DMA_PROTECT_RANGE2_MSK0_REG_OFS 0x240
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE2_MSK1_REG_OFS 0x244
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE2_MSK2_REG_OFS 0x248
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE2_MSK3_REG_OFS 0x24C
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE2_MSK4_REG_OFS 0x250
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE2_MSK5_REG_OFS 0x254
REGDEF_BEGIN(DMA_PROTECT_RANGE2_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE2_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

#define DMA_PROTECT_RANGE3_MSK0_REG_OFS 0x260
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE3_MSK1_REG_OFS 0x264
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE3_MSK2_REG_OFS 0x268
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE3_MSK3_REG_OFS 0x26C
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE3_MSK4_REG_OFS 0x270
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE3_MSK5_REG_OFS 0x274
REGDEF_BEGIN(DMA_PROTECT_RANGE3_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE3_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

#define DMA_PROTECT_RANGE4_MSK0_REG_OFS 0x280
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE4_MSK1_REG_OFS 0x284
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE4_MSK2_REG_OFS 0x288
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE4_MSK3_REG_OFS 0x28C
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE4_MSK4_REG_OFS 0x290
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE4_MSK5_REG_OFS 0x294
REGDEF_BEGIN(DMA_PROTECT_RANGE4_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE4_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

#define DMA_PROTECT_RANGE5_MSK0_REG_OFS 0x2A0
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK0_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK0_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK0_REG" end */

#define DMA_PROTECT_RANGE5_MSK1_REG_OFS 0x2A4
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK1_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK1_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK1_REG" end */

#define DMA_PROTECT_RANGE5_MSK2_REG_OFS 0x2A8
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK2_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK2_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK2_REG" end */

#define DMA_PROTECT_RANGE5_MSK3_REG_OFS 0x2AC
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK3_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK3_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK3_REG" end */

#define DMA_PROTECT_RANGE5_MSK4_REG_OFS 0x2B0
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK4_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK4_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK4_REG" end */

#define DMA_PROTECT_RANGE5_MSK5_REG_OFS 0x2B4
REGDEF_BEGIN(DMA_PROTECT_RANGE5_MSK5_REG)   /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PROTECT_RANGE5_MSK5_REG)     /* declare register cache type "DMA_PROTECT_RANGE2_MSK5_REG" end */

/////////////////////

//
// 0x00 ~ 0xFFC: DDR TOP region
//
#define DMA_CONFIG_REG_OFS      0x00
REGDEF_BEGIN(DMA_CONFIG_REG)        /* declare register cache type "DMA_CONFIG_REG" begin */
REGDEF_BIT(SDRAM_CAPACITY, 3)
REGDEF_BIT(SDRAM_COUNT, 1)
REGDEF_BIT(CL, 4)               /* CAS latency */
REGDEF_BIT(DDR_TYPE, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(WCL, 4)
REGDEF_BIT(ADDR_EXTEND, 1)      /* 2T command */
REGDEF_BIT(, 3)
REGDEF_BIT(ADDR_BIT30_CMP, 1)
REGDEF_BIT(, 11)
REGDEF_END(DMA_CONFIG_REG)          /* declare register cache type "DMA_CONFIG_REG" end */

#define DMA_CONTROL_REG_OFS     0x04
REGDEF_BEGIN(DMA_CONTROL_REG)       /* declare register cache type "DMA_CONTROL_REG" begin */
REGDEF_BIT(INIT_EN, 1)
REGDEF_BIT(PRECHARGE_ALL_CMD, 1)
REGDEF_BIT(MRS_EMRS_CMD, 1)
REGDEF_BIT(AUTO_REFRESH_CMD, 1)
REGDEF_BIT(SELF_REFRESH_CMD, 1)
REGDEF_BIT(AUTO_REFRESH_CTRL, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(SELF_REFRESH_STS, 1)
REGDEF_BIT(INIT_NOP_RDY, 1)
REGDEF_BIT(DDR_DLL_LOCK_RDY, 1)
REGDEF_BIT(PHY_DLL_RDY, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(MODE_REG, 19)
REGDEF_BIT(, 1)
REGDEF_END(DMA_CONTROL_REG)         /* declare register cache type "DMA_CONTROL_REG" end */

#define DMA_TIMING0_REG_OFS     0x08
REGDEF_BEGIN(DMA_TIMING0_REG)       /* declare register cache type "DMA_TIMING0_REG" begin */
REGDEF_BIT(T_RCD, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(T_RRD, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(T_RP, 5)
REGDEF_BIT(, 3)
REGDEF_BIT(T_WR, 5)
REGDEF_BIT(, 3)
REGDEF_BIT(DDR_TXP_EXT, 3)
REGDEF_BIT(, 1)
REGDEF_END(DMA_TIMING0_REG)         /* declare register cache type "DMA_TIMING0_REG" end */

#define DMA_TIMING1_REG_OFS     0x0C
REGDEF_BEGIN(DMA_TIMING1_REG)       /* declare register cache type "DMA_TIMING1_REG" begin */
REGDEF_BIT(T_RFC, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(T_REFI, 13)
REGDEF_BIT(, 3)
REGDEF_END(DMA_TIMING1_REG)         /* declare register cache type "DMA_TIMING1_REG" end */

#define DMA_TIMING2_REG_OFS     0x10
REGDEF_BEGIN(DMA_TIMING2_REG)       /* declare register cache type "DMA_TIMING2_REG" begin */
REGDEF_BIT(T_RC, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(T_FAW, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(T_RAS, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(T_DLLK, 2)
REGDEF_BIT(, 6)
REGDEF_END(DMA_TIMING2_REG)         /* declare register cache type "DMA_TIMING2_REG" end */

#define DMA_TIMING3_REG_OFS     0x14
REGDEF_BEGIN(DMA_TIMING3_REG)       /* declare register cache type "DMA_TIMING3_REG" begin */
REGDEF_BIT(T_RTP, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(T_WTR, 4)
REGDEF_BIT(, 20)
REGDEF_END(DMA_TIMING3_REG)         /* declare register cache type "DMA_TIMING3_REG" end */


// 0x18 ~ 0x1C: Reserved

#define DMA_CONTROL1_REG_OFS 0x20
REGDEF_BEGIN(DMA_CONTROL1_REG)      /* declare register cache type "DMA_CONTROL1_REG" begin */
REGDEF_BIT(ZQ_CALI_CMD, 1)
REGDEF_BIT(ZQ_CALI_MODE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(MRESET, 1)
REGDEF_BIT(WRL_ACT_L, 1)
REGDEF_BIT(WRL_DONE_L, 1)
REGDEF_BIT(WRL_ACT_H, 1)
REGDEF_BIT(WRL_DONE_H, 1)
REGDEF_BIT(, 7)
REGDEF_BIT(SW_RST, 1)
REGDEF_END(DMA_CONTROL1_REG)        /* declare register cache type "DMA_CONTROL1_REG" end */

#define DMA_ODT_CTRL_REG_OFS 0x24
REGDEF_BEGIN(DMA_ODT_CTRL_REG)      /* declare register cache type "DMA_ODT_CTRL_REG" begin */
REGDEF_BIT(DRAM_ODT_SEL, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(PHY_DQS_ODT_SEL, 2)
REGDEF_BIT(, 3)
REGDEF_BIT(PHY_DQ_ODT_SEL, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(PHY_DQS_ODT_PRE_EXTEND, 2)
REGDEF_BIT(, 1)
REGDEF_BIT(PHY_DQS_ODT_POS_EXTEND, 2)
REGDEF_BIT(, 1)
REGDEF_BIT(PHY_DQ_ODT_PRE_EXTEND, 2)
REGDEF_BIT(, 1)
REGDEF_BIT(PHY_DQ_ODT_POS_EXEND, 2)
REGDEF_BIT(, 5)
REGDEF_END(DMA_ODT_CTRL_REG)        /* declare register cache type "DMA_ODT_CTRL_REG" end */

// 0x28 ~ 0x2C: Reserved

#define DMA_DDRIO_TEST0_REG_OFS     0x30
REGDEF_BEGIN(DMA_DDRIO_TEST0_REG)   /* declare register cache type "DMA_DDRIO_TEST_REG" begin */
REGDEF_BIT(DDR_CMD_DC_TEST, 1)
REGDEF_BIT(DDR_DATA_DC_TEST, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DDR_DOUT, 1)
REGDEF_BIT(DDR_PHY_TEST_EN, 1)
REGDEF_BIT(DDR_RDQS_MSK_1T8T_EN, 1)
REGDEF_BIT(DDR_RDQS_MSK_ALL_EN, 1)
REGDEF_BIT(DDR_PHY_LOAD_DUTY, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(DDR_PHY_DUTY_PULSE_WIDTH, 4)
REGDEF_BIT(DDR_PIN_MAP, 2)
REGDEF_BIT(, 14)
REGDEF_END(DMA_DDRIO_TEST0_REG)     /* declare register cache type "DMA_DDRIO_TEST_REG" end */

#define DMA_DDRIO_TEST1_REG_OFS     0x34
REGDEF_BEGIN(DMA_DDRIO_TEST1_REG)   /* declare register cache type "DMA_DDRIO_TEST_REG" begin */
REGDEF_BIT(DDR_DC_CS0_OUTPUT_VALUE, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(DDR_DC_CLK_OUTPUT_VALUE, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(DDR_DC_CKE_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_RST_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_RAS_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_CAS_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_WE_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_ODT_OUTPUT_VALUE, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(DDR_DC_BA0_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_BA1_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_BA2_OUTPUT_VALUE, 1)
REGDEF_BIT(DDR_DC_ADDR_OUTPUT_VALUE, 16)
REGDEF_END(DMA_DDRIO_TEST1_REG)     /* declare register cache type "DMA_DDRIO_TEST_REG" end */

// 0x34 ~ 0x3C: Reserved

#define DMA_TIMING4_REG_OFS             0x40
REGDEF_BEGIN(DMA_TIMING4_REG)       /* declare register cache type "DMA_TIMING4_REG" begin */
REGDEF_BIT(, 12)
REGDEF_BIT(DQ_WEN_PRE_EXT, 1)
REGDEF_BIT(, 7)
REGDEF_BIT(W2R_EXT, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(R2W_EXT, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(DDR_R2R_EXT_1T_2_2T, 1)
REGDEF_BIT(, 3)
REGDEF_END(DMA_TIMING4_REG)     /* declare register cache type "DMA_TIMING4_REG" end */

// 0x44 ~ 0x4C: Reserved

#define DMA_ENGINEER_RESERVE_REG_OFS    0x50
REGDEF_BEGIN(DMA_ENGINEER_RESERVE_REG)  /* declare register cache type "DMA_ENGINEER_RESERVE_REG" begin */
REGDEF_BIT(PRECHARGE_PD, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DQS_IN_CYCLE, 1)
REGDEF_BIT(, 4)
REGDEF_BIT(DQS_INEN_DELAY, 3)
REGDEF_BIT(AF_DOUT_FF, 1)
REGDEF_BIT(CF_DOUT_FF, 1)
REGDEF_BIT(AF_HALF_DEPTH, 1)
REGDEF_BIT(CF_HALF_DEPTH, 1)
REGDEF_BIT(HOLD_SBSR, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(DDR_PHY_SLEEP, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(AUTO_REFRESH_APPROACH, 1)
REGDEF_BIT(, 6)
REGDEF_BIT(DEBUG_SEL, 4)
REGDEF_END(DMA_ENGINEER_RESERVE_REG)    /* declare register cache type "DMA_ENGINEER_RESERVE_REG" end */

#define DMA_BANDWIDTH_REG_OFS   0x54
REGDEF_BEGIN(DMA_BANDWIDTH_REG)         /* declare register cache type "DMA_BANDWIDTH_REG" begin */
REGDEF_BIT(ACT_CYCLE, 26)
REGDEF_BIT(, 6)
REGDEF_END(DMA_BANDWIDTH_REG)           /* declare register cache type "DMA_BANDWIDTH_REG" end */

///// PROTECT /////
#define DMA_PROTECT_INTCTRL_REG_OFS   0x64
REGDEF_BEGIN(DMA_PROTECT_INTCTRL_REG)    /* declare register cache type "DMA_PROTECT_INTCTRL_REG_OFS" begin */
REGDEF_BIT(DMA_WPWD0_INTEN0, 1)
REGDEF_BIT(DMA_WPWD0_INTEN1, 1)
REGDEF_BIT(DMA_WPWD0_INTEN2, 1)
REGDEF_BIT(DMA_WPWD0_INTEN3, 1)
REGDEF_BIT(DMA_WPWD1_INTEN0, 1)
REGDEF_BIT(DMA_WPWD1_INTEN1, 1)
REGDEF_BIT(DMA_WPWD1_INTEN2, 1)
REGDEF_BIT(DMA_WPWD1_INTEN3, 1)
REGDEF_BIT(DMA_WPWD2_INTEN0, 1)
REGDEF_BIT(DMA_WPWD2_INTEN1, 1)
REGDEF_BIT(DMA_WPWD2_INTEN2, 1)
REGDEF_BIT(DMA_WPWD2_INTEN3, 1)
REGDEF_BIT(DMA_WPWD3_INTEN0, 1)
REGDEF_BIT(DMA_WPWD3_INTEN1, 1)
REGDEF_BIT(DMA_WPWD3_INTEN2, 1)
REGDEF_BIT(DMA_WPWD3_INTEN3, 1)
REGDEF_BIT(DMA_WPWD4_INTEN0, 1)
REGDEF_BIT(DMA_WPWD4_INTEN1, 1)
REGDEF_BIT(DMA_WPWD4_INTEN2, 1)
REGDEF_BIT(DMA_WPWD4_INTEN3, 1)
REGDEF_BIT(DMA_WPWD5_INTEN0, 1)
REGDEF_BIT(DMA_WPWD5_INTEN1, 1)
REGDEF_BIT(DMA_WPWD5_INTEN2, 1)
REGDEF_BIT(DMA_WPWD5_INTEN3, 1)
REGDEF_BIT(AUTO_REFRESH_TIMEOUT_INTEN, 1)
REGDEF_BIT(DMA_OUTRANGE_INTEN, 1)
REGDEF_BIT(DMA_UPDATE_USAGE_INTEN, 1)
REGDEF_BIT(, 5)
REGDEF_END(DMA_PROTECT_INTCTRL_REG)      /* declare register cache type "DMA_PROTECT_INTCTRL_REG_OFS" end */

#define DMA_PROTECT_INTSTS_REG_OFS   0x68
REGDEF_BEGIN(DMA_PROTECT_INTSTS_REG)    /* declare register cache type "DMA_PROTECT_INTSTS_REG_OFS" begin */
REGDEF_BIT(DMA_WPWD0_STS0, 1)
REGDEF_BIT(DMA_WPWD0_STS1, 1)
REGDEF_BIT(DMA_WPWD0_STS2, 1)
REGDEF_BIT(DMA_WPWD0_STS3, 1)
REGDEF_BIT(DMA_WPWD1_STS0, 1)
REGDEF_BIT(DMA_WPWD1_STS1, 1)
REGDEF_BIT(DMA_WPWD1_STS2, 1)
REGDEF_BIT(DMA_WPWD1_STS3, 1)
REGDEF_BIT(DMA_WPWD2_STS0, 1)
REGDEF_BIT(DMA_WPWD2_STS1, 1)
REGDEF_BIT(DMA_WPWD2_STS2, 1)
REGDEF_BIT(DMA_WPWD2_STS3, 1)
REGDEF_BIT(DMA_WPWD3_STS0, 1)
REGDEF_BIT(DMA_WPWD3_STS1, 1)
REGDEF_BIT(DMA_WPWD3_STS2, 1)
REGDEF_BIT(DMA_WPWD3_STS3, 1)
REGDEF_BIT(DMA_WPWD4_STS0, 1)
REGDEF_BIT(DMA_WPWD4_STS1, 1)
REGDEF_BIT(DMA_WPWD4_STS2, 1)
REGDEF_BIT(DMA_WPWD4_STS3, 1)
REGDEF_BIT(DMA_WPWD5_STS0, 1)
REGDEF_BIT(DMA_WPWD5_STS1, 1)
REGDEF_BIT(DMA_WPWD5_STS2, 1)
REGDEF_BIT(DMA_WPWD5_STS3, 1)
REGDEF_BIT(AUTO_REFRESH_TIMEOUT_STS, 1)
REGDEF_BIT(DMA_OUTRANGE_STS, 1)
REGDEF_BIT(DMA_UPDATE_USAGE_STS, 1)
REGDEF_BIT(, 5)
REGDEF_END(DMA_PROTECT_INTSTS_REG)      /* declare register cache type "DMA_PROTECT_INTSTS_REG_OFS" end */

#define DMA_PROTECT0_CHSTS_REG_OFS   0x6C
REGDEF_BEGIN(DMA_PROTECT0_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT0_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT1_CHSTS_REG_OFS   0x70
REGDEF_BEGIN(DMA_PROTECT1_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT1_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT2_CHSTS_REG_OFS   0x74
REGDEF_BEGIN(DMA_PROTECT2_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT2_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT3_CHSTS_REG_OFS   0x78
REGDEF_BEGIN(DMA_PROTECT3_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT3_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT4_CHSTS_REG_OFS   0x7C
REGDEF_BEGIN(DMA_PROTECT4_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT4_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT5_CHSTS_REG_OFS   0x80
REGDEF_BEGIN(DMA_PROTECT5_CHSTS_REG)    /* declare register cache type "DMA_PROTECT_CHSTS_REG" begin */
REGDEF_BIT(CHSTS0, 8)
REGDEF_BIT(CHSTS1, 8)
REGDEF_BIT(CHSTS2, 8)
REGDEF_BIT(CHSTS3, 8)
REGDEF_END(DMA_PROTECT5_CHSTS_REG)      /* declare register cache type "DMA_PROTECT_CHSTS_REG" end */

#define DMA_PROTECT0_DETECT_ADDR0_REG_OFS    0x600
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT0_DETECT_ADDR1_REG_OFS    0x604
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT0_DETECT_ADDR2_REG_OFS    0x608
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT0_DETECT_ADDR3_REG_OFS    0x60C
REGDEF_BEGIN(DMA_PROTECT0_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT0_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT1_DETECT_ADDR0_REG_OFS    0x610
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT1_DETECT_ADDR1_REG_OFS    0x614
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT1_DETECT_ADDR2_REG_OFS    0x618
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT1_DETECT_ADDR3_REG_OFS    0x61C
REGDEF_BEGIN(DMA_PROTECT1_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT1_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT2_DETECT_ADDR0_REG_OFS    0x620
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT2_DETECT_ADDR1_REG_OFS    0x624
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT2_DETECT_ADDR2_REG_OFS    0x628
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT2_DETECT_ADDR3_REG_OFS    0x62C
REGDEF_BEGIN(DMA_PROTECT2_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT2_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT3_DETECT_ADDR0_REG_OFS    0x630
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT3_DETECT_ADDR1_REG_OFS    0x634
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT3_DETECT_ADDR2_REG_OFS    0x638
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT3_DETECT_ADDR3_REG_OFS    0x63C
REGDEF_BEGIN(DMA_PROTECT3_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT3_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT4_DETECT_ADDR0_REG_OFS    0x640
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT4_DETECT_ADDR1_REG_OFS    0x644
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT4_DETECT_ADDR2_REG_OFS    0x648
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT4_DETECT_ADDR3_REG_OFS    0x64C
REGDEF_BEGIN(DMA_PROTECT4_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT4_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT5_DETECT_ADDR0_REG_OFS    0x650
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR0_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR0_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR0_REG" end */

#define DMA_PROTECT5_DETECT_ADDR1_REG_OFS    0x654
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR1_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR1_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR1_REG" end */

#define DMA_PROTECT5_DETECT_ADDR2_REG_OFS    0x658
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR2_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR2_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */

#define DMA_PROTECT5_DETECT_ADDR3_REG_OFS    0x65C
REGDEF_BEGIN(DMA_PROTECT5_DETECT_ADDR3_REG)  /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" begin */
REGDEF_BIT(addr, 32)
REGDEF_END(DMA_PROTECT5_DETECT_ADDR3_REG)    /* declare register cache type "DMA_PROTECT_DETECT_ADDR2_REG" end */
///////////////////////////

#define DMA_OUT_RANGE_CHANNEL_REG_OFS   0x100
REGDEF_BEGIN(DMA_OUT_RANGE_CHANNEL_REG)     /* declare register cache type "DMA_OUT_RANGE_CHANNEL_REG" begin */
REGDEF_BIT(DRAM_OUTRANGE_CH, 8)
REGDEF_BIT(, 24)
REGDEF_END(DMA_OUT_RANGE_CHANNEL_REG)       /* declare register cache type "DMA_OUT_RANGE_CHANNEL_REG" end */

#define DMA_OUT_RANGE_ADDR_REG_OFS      0x104
REGDEF_BEGIN(DMA_OUT_RANGE_ADDR_REG)        /* declare register cache type "DMA_OUT_RANGE_ADDR_REG" begin */
REGDEF_BIT(DRAM_OUTRANGE_ADDR, 32)
REGDEF_END(DMA_OUT_RANGE_ADDR_REG)          /* declare register cache type "DMA_OUT_RANGE_ADDR_REG" end */

// 0x1000 ~ 0x7FFC: Reserve for DDR PHY (by IP team)

//
// 0x8000 ~ 0xFFFC: DDR Arbiter region
//
#define DMA_SDRAM_CTRL_REG0_OFS  0x8000
REGDEF_BEGIN(DMA_SDRAM_CTRL_REG0)  /* declare register cache type "DMA_SDRAM_CTRL_REG0" begin */
REGDEF_BIT(DMA_CH_EN_SWITCH, 1)
REGDEF_BIT(DMA_CH_DIS_DONE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(CA9_REQ_NUM, 2)
REGDEF_BIT(, 24)
REGDEF_END(DMA_SDRAM_CTRL_REG0)    /* declare register cache type "DMA_SDRAM_CTRL_REG0" end */

// 0x8004 ~ 0x800C: Reserved

#define ARBIT_OFFSET            0x8000
#define DMA_CHANNEL_ENABLE0_REG_OFS  (0x10 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE0_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_CHANNEL_ENABLE0_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE0_REG" end */

#define DMA_CHANNEL_ENABLE1_REG_OFS    (0x14 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE1_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_CHANNEL_ENABLE1_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE1_REG" end */

#define DMA_CHANNEL_ENABLE2_REG_OFS    (0x18 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE2_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_CHANNEL_ENABLE2_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE2_REG" end */

#define DMA_CHANNEL_ENABLE3_REG_OFS    (0x1C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE3_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE3_REG" begin */
REGDEF_BIT(reserved, 32)
REGDEF_END(DMA_CHANNEL_ENABLE3_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE3_REG" end */

#define DMA_CHANNEL_ENABLE4_REG_OFS    (0x20 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE4_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE4_REG" begin */
REGDEF_BIT(reserved, 32)
REGDEF_END(DMA_CHANNEL_ENABLE4_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE4_REG" end */
#if 0

#define DMA_CHANNEL_ENABLE5_REG_OFS    (0x24 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE5_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE5_REG" begin */
REGDEF_BIT(reserved, 32)
REGDEF_END(DMA_CHANNEL_ENABLE5_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE5_REG" end */

#define DMA_CHANNEL_ENABLE6_REG_OFS    (0x28 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE6_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE6_REG" begin */
REGDEF_BIT(reserved, 32)
REGDEF_END(DMA_CHANNEL_ENABLE6_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE6_REG" end */

#define DMA_CHANNEL_ENABLE7_REG_OFS    (0x2C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_ENABLE7_REG)  /* declare register cache type "DMA_CHANNEL_ENABLE7_REG" begin */
REGDEF_BIT(reserved, 32)
REGDEF_END(DMA_CHANNEL_ENABLE7_REG)    /* declare register cache type "DMA_CHANNEL_ENABLE7_REG" end */
#endif

#define DMA_REQ_FLG0_REG_OFS        (0x30 + ARBIT_OFFSET)
#define DMA_REQ_FLG1_REG_OFS        (0x34 + ARBIT_OFFSET)
#define DMA_REQ_FLG2_REG_OFS        (0x38 + ARBIT_OFFSET)
#define DMA_REQ_FLG3_REG_OFS        (0x3C + ARBIT_OFFSET)
#define DMA_REQ_FLG4_REG_OFS        (0x40 + ARBIT_OFFSET)
#if 0
#define DMA_REQ_FLG5_REG_OFS        (0x44 + ARBIT_OFFSET)
#define DMA_REQ_FLG6_REG_OFS        (0x48 + ARBIT_OFFSET)
#define DMA_REQ_FLG7_REG_OFS        (0x4C + ARBIT_OFFSET)
#endif
#define DMA_ACK_FLG0_REG_OFS        (0xB0 + ARBIT_OFFSET)
#define DMA_ACK_FLG1_REG_OFS        (0xB4 + ARBIT_OFFSET)
#define DMA_ACK_FLG2_REG_OFS        (0xB8 + ARBIT_OFFSET)
#define DMA_ACK_FLG3_REG_OFS        (0xBC + ARBIT_OFFSET)
#define DMA_ACK_FLG4_REG_OFS        (0xC0 + ARBIT_OFFSET)
#if 0
#define DMA_ACK_FLG5_REG_OFS        (0xC4 + ARBIT_OFFSET)
#define DMA_ACK_FLG6_REG_OFS        (0xC8 + ARBIT_OFFSET)
#define DMA_ACK_FLG7_REG_OFS        (0xCC + ARBIT_OFFSET)
#endif
#define DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS    (0x60 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG)  /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG" begin */
REGDEF_BIT(CH0_TEST_START, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CH0_SKIP_COMPARE, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CH0_TEST_METHOD, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(CH0_BURST_LEN, 7)
REGDEF_BIT(, 1)
REGDEF_BIT(CH0_TEST_TIMES, 16)
REGDEF_END(DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG)    /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG" end */

#define DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS      (0x64 + ARBIT_OFFSET)
#define DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS        (0x68 + ARBIT_OFFSET)

#define DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS      (0x6C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE)    /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE" begin */
REGDEF_BIT(INTERVAL_REQ, 10)
REGDEF_BIT(, 6)
REGDEF_BIT(CHECKSUM, 16)
REGDEF_END(DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE)      /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE" end */



#define DMA_CHANNEL1_HEAVY_LOAD_CTRL_OFS    (0x70 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL1_HEAVY_LOAD_CTRL_REG)  /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG" begin */
REGDEF_BIT(CH1_TEST_START, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CH1_SKIP_COMPARE, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CH1_TEST_METHOD, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(CH1_BURST_LEN, 7)
REGDEF_BIT(, 1)
REGDEF_BIT(CH1_TEST_TIMES, 16)
REGDEF_END(DMA_CHANNEL1_HEAVY_LOAD_CTRL_REG)    /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG" end */


#define DMA_CHANNEL1_HEAVY_LOAD_START_ADDR_OFS      (0x74 + ARBIT_OFFSET)
#define DMA_CHANNEL1_HEAVY_LOAD_DMA_SIZE_OFS        (0x78 + ARBIT_OFFSET)

#define DMA_CHANNEL1_HEAVY_LOAD_WAIT_CYCLE_OFS      (0x7C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL1_HEAVY_LOAD_WAIT_CYCLE)    /* declare register cache type "DMA_CHANNEL1_HEAVY_LOAD_WAIT_CYCLE" begin */
REGDEF_BIT(INTERVAL_REQ, 10)
REGDEF_BIT(, 6)
REGDEF_BIT(CHECKSUM, 16)
REGDEF_END(DMA_CHANNEL1_HEAVY_LOAD_WAIT_CYCLE)      /* declare register cache type "DMA_CHANNEL1_HEAVY_LOAD_WAIT_CYCLE" end */

#define DMA_CHANNEL2_HEAVY_LOAD_CTRL_OFS            (0x80 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL2_HEAVY_LOAD_CTRL_REG)  /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG" begin */
REGDEF_BIT(CH2_TEST_START, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CH2_SKIP_COMPARE, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CH2_TEST_METHOD, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(CH2_BURST_LEN, 7)
REGDEF_BIT(, 1)
REGDEF_BIT(CH2_TEST_TIMES, 16)
REGDEF_END(DMA_CHANNEL2_HEAVY_LOAD_CTRL_REG)    /* declare register cache type "DMA_CHANNEL0_HEAVY_LOAD_CTRL_REG" end */

#define DMA_CHANNEL2_HEAVY_LOAD_START_ADDR_OFS      (0x84 + ARBIT_OFFSET)
#define DMA_CHANNEL2_HEAVY_LOAD_DMA_SIZE_OFS        (0x88 + ARBIT_OFFSET)

#define DMA_CHANNEL2_HEAVY_LOAD_WAIT_CYCLE_OFS      (0x8C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL2_HEAVY_LOAD_WAIT_CYCLE)    /* declare register cache type "DMA_CHANNEL2_HEAVY_LOAD_WAIT_CYCLE" begin */
REGDEF_BIT(INTERVAL_REQ, 10)
REGDEF_BIT(, 6)
REGDEF_BIT(CHECKSUM, 16)
REGDEF_END(DMA_CHANNEL2_HEAVY_LOAD_WAIT_CYCLE)      /* declare register cache type "DMA_CHANNEL2_HEAVY_LOAD_WAIT_CYCLE" end */

#define DMA_HEAVY_LOAD_TEST_PATTERN0_OFS            (0x90 + ARBIT_OFFSET)
#define DMA_HEAVY_LOAD_TEST_PATTERN1_OFS            (0x94 + ARBIT_OFFSET)
#define DMA_HEAVY_LOAD_TEST_PATTERN2_OFS            (0x98 + ARBIT_OFFSET)
#define DMA_HEAVY_LOAD_TEST_PATTERN3_OFS            (0x9C + ARBIT_OFFSET)

#define DMA_HEAVY_LOAD_STS_OFS    (0xA0 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_HEAVY_LOAD_STS_REG)  /* declare register cache type "DMA_HEAVY_LOAD_STS_REG" begin */
REGDEF_BIT(HEAVY_LOAD_COMPLETE_STS, 1)
REGDEF_BIT(HEAVY_LOAD_ERROR_STS, 1)
REGDEF_BIT(, 30)
REGDEF_END(DMA_HEAVY_LOAD_STS_REG)    /* declare register cache type "DMA_HEAVY_LOAD_STS_REG" end */

#define DMA_HEAVY_LOAD_ERROR_DATA_OFS               (0xA4 + ARBIT_OFFSET)
#define DMA_HEAVY_LOAD_CORRECT_DATA_OFS             (0xA8 + ARBIT_OFFSET)
#define DMA_HEAVY_LOAD_CURRENT_ADDR_OFS             (0xAC + ARBIT_OFFSET)

#define DMA_PRI_CONTROL0_REG_OFS    (0x100 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL0_REG)  /* declare register cache type "DMA_PRI_CONTROL0_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL0_REG)    /* declare register cache type "DMA_PRI_CONTROL0_REG" end */

#define DMA_PRI_CONTROL1_REG_OFS    (0x104 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL1_REG)  /* declare register cache type "DMA_PRI_CONTROL1_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL1_REG)    /* declare register cache type "DMA_PRI_CONTROL1_REG" end */

#define DMA_PRI_CONTROL2_REG_OFS    (0x108 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL2_REG)  /* declare register cache type "DMA_PRI_CONTROL2_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL2_REG)    /* declare register cache type "DMA_PRI_CONTROL2_REG" end */

#define DMA_PRI_CONTROL3_REG_OFS    (0x10C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL3_REG)  /* declare register cache type "DMA_PRI_CONTROL3_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL3_REG)    /* declare register cache type "DMA_PRI_CONTROL3_REG" end */

#define DMA_PRI_CONTROL4_REG_OFS    (0x110 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL4_REG)  /* declare register cache type "DMA_PRI_CONTROL4_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL4_REG)    /* declare register cache type "DMA_PRI_CONTROL4_REG" end */

#define DMA_PRI_CONTROL5_REG_OFS    (0x114 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL5_REG)  /* declare register cache type "DMA_PRI_CONTROL5_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL5_REG)    /* declare register cache type "DMA_PRI_CONTROL5_REG" end */

#define DMA_PRI_CONTROL6_REG_OFS    (0x118 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL6_REG)  /* declare register cache type "DMA_PRI_CONTROL6_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL6_REG)    /* declare register cache type "DMA_PRI_CONTROL6_REG" end */

#define DMA_PRI_CONTROL7_REG_OFS    (0x11C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL7_REG)  /* declare register cache type "DMA_PRI_CONTROL7_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL7_REG)    /* declare register cache type "DMA_PRI_CONTROL7_REG" end */

#define DMA_PRI_CONTROL8_REG_OFS    (0x120 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL8_REG)  /* declare register cache type "DMA_PRI_CONTROL8_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL8_REG)    /* declare register cache type "DMA_PRI_CONTROL8_REG" end */

#define DMA_PRI_CONTROL9_REG_OFS    (0x124 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL9_REG)  /* declare register cache type "DMA_PRI_CONTROL9_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL9_REG)    /* declare register cache type "DMA_PRI_CONTROL9_REG" end */
#if 0
#define DMA_PRI_CONTROL10_REG_OFS   (0x128 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL10_REG) /* declare register cache type "DMA_PRI_CONTROL10_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL10_REG)   /* declare register cache type "DMA_PRI_CONTROL10_REG" end */

#define DMA_PRI_CONTROL11_REG_OFS   (0x12C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL11_REG) /* declare register cache type "DMA_PRI_CONTROL11_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL11_REG)   /* declare register cache type "DMA_PRI_CONTROL11_REG" end */

#define DMA_PRI_CONTROL12_REG_OFS   (0x130 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL12_REG) /* declare register cache type "DMA_PRI_CONTROL12_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL12_REG)   /* declare register cache type "DMA_PRI_CONTROL12_REG" end */

#define DMA_PRI_CONTROL13_REG_OFS   (0x134 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL13_REG) /* declare register cache type "DMA_PRI_CONTROL13_REG" begin */
REGDEF_BIT(, 32)
REGDEF_END(DMA_PRI_CONTROL13_REG)   /* declare register cache type "DMA_PRI_CONTROL13_REG" end */

#define DMA_PRI_CONTROL14_REG_OFS   (0x138 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL14_REG) /* declare register cache type "DMA_PRI_CONTROL14_REG" begin */
REGDEF_BIT(reserve1, 32)
REGDEF_END(DMA_PRI_CONTROL14_REG)   /* declare register cache type "DMA_PRI_CONTROL14_REG" end */

#define DMA_PRI_CONTROL15_REG_OFS   (0x13C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_PRI_CONTROL15_REG) /* declare register cache type "DMA_PRI_CONTROL15_REG" begin */
REGDEF_BIT(reserve1, 32)
REGDEF_END(DMA_PRI_CONTROL15_REG)   /* declare register cache type "DMA_PRI_CONTROL15_REG" end */
#endif

#define DMA_BOUNDING_CTRL_REG_OFS   (0x200 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_BOUNDING_CTRL_REG) /* declare register cache type "DMA_BOUNDING_CTRL_REG" begin */
REGDEF_BIT(DATA_BOUND_TEST_EN,  1)
REGDEF_BIT(ADDR_BOUND_TEST_EN,  1)
REGDEF_BIT(,  30)
REGDEF_END(DMA_BOUNDING_CTRL_REG)   /* declare register cache type "DMA_BOUNDING_CTRL_REG" end */

#define DMA_BOUNDING_CFG_REG_OFS    (0x204 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_BOUNDING_CFG_REG)  /* declare register cache type "DMA_BOUNDING_CFG_REG" begin */
REGDEF_BIT(ADDR_BOUND_TEST_SEL, 1)
REGDEF_BIT(,  31)
REGDEF_END(DMA_BOUNDING_CFG_REG)    /* declare register cache type "DMA_BOUNDING_CFG_REG" end */

#define DMA_BOUNDING_STS_REG_OFS    (0x208 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_BOUNDING_STS_REG)  /* declare register cache type "DMA_BOUNDING_STS_REG" begin */
REGDEF_BIT(DATA_BOUND_COMPLETE_STS,     1)
REGDEF_BIT(ADDR_BOUND_COMPLETE_STS,     1)
REGDEF_BIT(,                    14)
REGDEF_BIT(DATA_BOUND_COMPLETE_RESULT,  1)
REGDEF_BIT(ADDR_BOUND_COMPLETE_RESULT,  1)
REGDEF_BIT(,                    14)
REGDEF_END(DMA_BOUNDING_STS_REG)    /* declare register cache type "DMA_BOUNDING_STS_REG" end */

#define DMA_BOUNDING_REPORT_REG_OFS (0x20C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_BOUNDING_REPORT_REG)   /* declare register cache type "DMA_BOUNDING_REPORT_REG" begin */
REGDEF_BIT(DATA_BOUND_FAIL_PAD,     4)
REGDEF_BIT(,                4)
REGDEF_BIT(ADDR_BOUND_FAIL_PAD,     4)
REGDEF_BIT(,                20)
REGDEF_END(DMA_BOUNDING_REPORT_REG)     /* declare register cache type "DMA_BOUNDING_REPORT_REG" end */

#define DMA_QOS_CONTROL_REG_OFS     (0x400 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_CONTROL_REG)       /* declare register cache type "DMA_QOS_CONTROL_REG" begin */
REGDEF_BIT(QOS_CA9_EN,             1)
REGDEF_BIT(,                       1)
REGDEF_BIT(,                       1)
REGDEF_BIT(,                       1)
REGDEF_BIT(,                       1)
REGDEF_BIT(,                       1)
REGDEF_BIT(,                       1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_GRPH_EN,             1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_GRPH2_EN,            1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_ETH_EN,              1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_USB_EN,              1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_SDIO_EN,             1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_SDIO2_EN,            1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_SDIO3_EN,            1)
REGDEF_BIT(,                       1)
REGDEF_BIT(QOS_CRYPTO_EN,           1)
REGDEF_BIT(,                       9)
REGDEF_END(DMA_QOS_CONTROL_REG)         /* declare register cache type "DMA_QOS_CONTROL_REG" end */

#define DMA_QOS_TIMEOUT0_REG_OFS        (0x410 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_TIMEOUT0_REG)      /* declare register cache type "DMA_QOS_TIMEOUT0_REG" begin */
REGDEF_BIT(QOS_CA9_TIMEOUT,       14)
REGDEF_BIT(,                       2)
REGDEF_BIT(,                      14)
REGDEF_BIT(,                       2)
REGDEF_END(DMA_QOS_TIMEOUT0_REG)        /* declare register cache type "DMA_QOS_TIMEOUT0_REG" end */

#define DMA_QOS_TIMEOUT1_REG_OFS        (0x414 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_TIMEOUT1_REG)      /* declare register cache type "DMA_QOS_TIMEOUT1_REG" begin */
REGDEF_BIT(,                      14)
REGDEF_BIT(,                       2)
REGDEF_BIT(,                      14)
REGDEF_BIT(,                       2)
REGDEF_END(DMA_QOS_TIMEOUT1_REG)        /* declare register cache type "DMA_QOS_TIMEOUT1_REG" end */

#define DMA_QOS_TIMEOUT2_REG_OFS        (0x418 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_TIMEOUT2_REG)      /* declare register cache type "DMA_QOS_TIMEOUT2_REG" begin */
REGDEF_BIT(QOS_GRPH_TIMEOUT,        14)
REGDEF_BIT(,                       2)
REGDEF_BIT(QOS_GRPH2_TIMEOUT,       14)
REGDEF_BIT(,                       2)
REGDEF_END(DMA_QOS_TIMEOUT2_REG)        /* declare register cache type "DMA_QOS_TIMEOUT2_REG" end */

#define DMA_QOS_TIMEOUT3_REG_OFS        (0x41C + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_TIMEOUT3_REG)      /* declare register cache type "DMA_QOS_TIMEOUT3_REG" begin */
REGDEF_BIT(QOS_ETH_TIMEOUT,         14)
REGDEF_BIT(,                       2)
REGDEF_BIT(QOS_USB_TIMEOUT,         14)
REGDEF_BIT(,                       2)
REGDEF_END(DMA_QOS_TIMEOUT3_REG)        /* declare register cache type "DMA_QOS_TIMEOUT3_REG" end */

#define DMA_QOS_TIMEOUT4_REG_OFS        (0x420 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_TIMEOUT4_REG)      /* declare register cache type "DMA_QOS_TIMEOUT4_REG" begin */
REGDEF_BIT(QOS_SDIO_TIMEOUT,        14)
REGDEF_BIT(,                       2)
REGDEF_BIT(QOS_SDIO2_TIMEOUT,       14)
REGDEF_BIT(,                       2)
REGDEF_END(DMA_QOS_TIMEOUT4_REG)        /* declare register cache type "DMA_QOS_TIMEOUT4_REG" end */

#define DMA_QOS_TIMEOUT5_REG_OFS        (0x424 + ARBIT_OFFSET)
REGDEF_BEGIN(DMA_QOS_TIMEOUT5_REG)      /* declare register cache type "DMA_QOS_TIMEOUT5_REG" begin */
REGDEF_BIT(QOS_SDIO3_TIMEOUT,       14)
REGDEF_BIT(,                       2)
REGDEF_BIT(QOS_CRYPTOR_TIMEOUT,     14)
REGDEF_BIT(,                       2)
REGDEF_END(DMA_QOS_TIMEOUT5_REG)        /* declare register cache type "DMA_QOS_TIMEOUT5_REG" end */


#define MONITOR_OFFSET              0x9000

#define DMA_RECORD_PERIOD_REG_OFS       (0x00 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD_PERIOD_REG)     /* declare register cache type "DMA_RECORD_PERIOD_REG" begin */
REGDEF_BIT(MONITOR_PERIOD,          26)
REGDEF_BIT(,               6)
REGDEF_END(DMA_RECORD_PERIOD_REG)       /* declare register cache type "DMA_RECORD_PERIOD_REG" end */

#define DMA_BANDWIDTH_SIZE_REG_OFS      (0x04 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_BANDWIDTH_SIZE_REG)    /* declare register cache type "DMA_BANDWIDTH_SIZE_REG" begin */
REGDEF_BIT(TOTAL_ACC_SIZE, 28)
REGDEF_BIT(, 3)
REGDEF_BIT(TOTAL_ACC_OVF, 1)
REGDEF_END(DMA_BANDWIDTH_SIZE_REG)      /* declare register cache type "DMA_BANDWIDTH_SIZE_REG" end */

#define DMA_BANDWIDTH_REQ_REG_OFS       (0x08 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_BANDWIDTH_REQ_REG)     /* declare register cache type "DMA_BANDWIDTH_REQ_REG" begin */
REGDEF_BIT(TOTAL_ACC_REQ, 28)
REGDEF_BIT(, 3)
REGDEF_BIT(TOTAL_REQ_OVF, 1)
REGDEF_END(DMA_BANDWIDTH_REQ_REG)       /* declare register cache type "DMA_BANDWIDTH_REQ_REG" end */


#define DMA_RECORD_CONFIG0_REG_OFS      (0x10 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD_CONFIG0_REG)    /* declare register cache type "DMA_RECORD_CONFIG0_REG" begin */
REGDEF_BIT(MONITOR0_DMA_CH,         8)
REGDEF_BIT(MONITOR0_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_BIT(MONITOR1_DMA_CH,         8)
REGDEF_BIT(MONITOR1_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_END(DMA_RECORD_CONFIG0_REG)      /* declare register cache type "DMA_RECORD_CONFIG0_REG" end */

#define DMA_RECORD_CONFIG1_REG_OFS      (0x14 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD_CONFIG1_REG)    /* declare register cache type "DMA_RECORD_CONFIG1_REG" begin */
REGDEF_BIT(MONITOR2_DMA_CH,         8)
REGDEF_BIT(MONITOR2_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_BIT(MONITOR3_DMA_CH,         8)
REGDEF_BIT(MONITOR3_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_END(DMA_RECORD_CONFIG1_REG)      /* declare register cache type "DMA_RECORD_CONFIG1_REG" end */

#define DMA_RECORD_CONFIG2_REG_OFS      (0x18 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD_CONFIG2_REG)    /* declare register cache type "DMA_RECORD_CONFIG2_REG" begin */
REGDEF_BIT(MONITOR4_DMA_CH,         8)
REGDEF_BIT(MONITOR4_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_BIT(MONITOR5_DMA_CH,         8)
REGDEF_BIT(MONITOR5_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_END(DMA_RECORD_CONFIG2_REG)      /* declare register cache type "DMA_RECORD_CONFIG2_REG" end */

#define DMA_RECORD_CONFIG3_REG_OFS      (0x1C + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD_CONFIG3_REG)    /* declare register cache type "DMA_RECORD_CONFIG3_REG" begin */
REGDEF_BIT(MONITOR6_DMA_CH,         8)
REGDEF_BIT(MONITOR6_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_BIT(MONITOR7_DMA_CH,         8)
REGDEF_BIT(MONITOR7_DMA_DIR,        2)
REGDEF_BIT(,              6)
REGDEF_END(DMA_RECORD_CONFIG3_REG)      /* declare register cache type "DMA_RECORD_CONFIG3_REG" end */

#define DMA_RECORD0_SIZE_REG_OFS        (0x30 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD0_SIZE_REG)      /* declare register cache type "DMA_RECORD0_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD0_SIZE_REG)        /* declare register cache type "DMA_RECORD0_SIZE_REG" end */

#define DMA_RECORD0_COUNT_REG_OFS       (0x34 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD0_COUNT_REG)     /* declare register cache type "DMA_RECORD0_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD0_COUNT_REG)       /* declare register cache type "DMA_RECORD0_COUNT_REG" end */

#define DMA_RECORD1_SIZE_REG_OFS        (0x38 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD1_SIZE_REG)      /* declare register cache type "DMA_RECORD1_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD1_SIZE_REG)        /* declare register cache type "DMA_RECORD1_SIZE_REG" end */

#define DMA_RECORD1_COUNT_REG_OFS       (0x3C + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD1_COUNT_REG)     /* declare register cache type "DMA_RECORD1_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD1_COUNT_REG)       /* declare register cache type "DMA_RECORD1_COUNT_REG" end */

#define DMA_RECORD2_SIZE_REG_OFS        (0x40 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD2_SIZE_REG)      /* declare register cache type "DMA_RECORD2_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD2_SIZE_REG)        /* declare register cache type "DMA_RECORD2_SIZE_REG" end */

#define DMA_RECORD2_COUNT_REG_OFS       (0x44 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD2_COUNT_REG)     /* declare register cache type "DMA_RECORD2_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD2_COUNT_REG)       /* declare register cache type "DMA_RECORD2_COUNT_REG" end */

#define DMA_RECORD3_SIZE_REG_OFS        (0x48 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD3_SIZE_REG)      /* declare register cache type "DMA_RECORD3_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD3_SIZE_REG)        /* declare register cache type "DMA_RECORD3_SIZE_REG" end */

#define DMA_RECORD3_COUNT_REG_OFS       (0x4C + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD3_COUNT_REG)     /* declare register cache type "DMA_RECORD3_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD3_COUNT_REG)       /* declare register cache type "DMA_RECORD3_COUNT_REG" end */

#define DMA_RECORD4_SIZE_REG_OFS        (0x50 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD4_SIZE_REG)      /* declare register cache type "DMA_RECORD4_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD4_SIZE_REG)        /* declare register cache type "DMA_RECORD4_SIZE_REG" end */

#define DMA_RECORD4_COUNT_REG_OFS       (0x54 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD4_COUNT_REG)     /* declare register cache type "DMA_RECORD4_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD4_COUNT_REG)       /* declare register cache type "DMA_RECORD4_COUNT_REG" end */

#define DMA_RECORD5_SIZE_REG_OFS        (0x58 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD5_SIZE_REG)      /* declare register cache type "DMA_RECORD5_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD5_SIZE_REG)        /* declare register cache type "DMA_RECORD5_SIZE_REG" end */

#define DMA_RECORD5_COUNT_REG_OFS       (0x5C + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD5_COUNT_REG)     /* declare register cache type "DMA_RECORD5_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD5_COUNT_REG)       /* declare register cache type "DMA_RECORD5_COUNT_REG" end */

#define DMA_RECORD6_SIZE_REG_OFS        (0x60 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD6_SIZE_REG)      /* declare register cache type "DMA_RECORD6_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD6_SIZE_REG)        /* declare register cache type "DMA_RECORD6_SIZE_REG" end */

#define DMA_RECORD6_COUNT_REG_OFS       (0x64 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD6_COUNT_REG)     /* declare register cache type "DMA_RECORD6_COUNT_REG" begin */
REGDEF_BIT(MONITOR_ACC_REQ,         28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_REQ_OVF,         1)
REGDEF_END(DMA_RECORD6_COUNT_REG)       /* declare register cache type "DMA_RECORD6_COUNT_REG" end */

#define DMA_RECORD7_SIZE_REG_OFS        (0x68 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_RECORD7_SIZE_REG)      /* declare register cache type "DMA_RECORD7_SIZE_REG" begin */
REGDEF_BIT(MONITOR_ACC_SIZE,        28)
REGDEF_BIT(,              3)
REGDEF_BIT(MONITOR_SIZE_OVF,        1)
REGDEF_END(DMA_RECORD7_SIZE_REG)        /* declare register cache type "DMA_RECORD7_SIZE_REG" end */

#define DMA_CHANNEL_OUTSTANDING_REG_OFS       (0x100 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_CHANNEL_OUTSTANDING_REG)     /* declare register cache type "DMA_RECORD7_COUNT_REG" begin */
REGDEF_BIT(CNN_OUTSTANDING,         6)
REGDEF_BIT(CNN2_OUTSTANDING,        6)
REGDEF_BIT(NUE_OUTSTANDING,         3)
REGDEF_BIT(,              17)
REGDEF_END(DMA_CHANNEL_OUTSTANDING_REG)       /* declare register cache type "DMA_RECORD7_COUNT_REG" end */

#if 0
#define DMA_MIPS1_BANDWIDTH_REG_OFS     (0xB0 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_MIPS1_BANDWIDTH_REG)   /* declare register cache type "DMA_MIPS1_BANDWIDTH_REG" begin */
REGDEF_BIT(REQ_NUM, 16)
REGDEF_BIT(reserved, 16)
REGDEF_END(DMA_MIPS1_BANDWIDTH_REG)     /* declare register cache type "DMA_MIPS1_BANDWIDTH_REG" end */

#define DMA_MIPS2_BANDWIDTH_REG_OFS     (0xB4 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_MIPS2_BANDWIDTH_REG)   /* declare register cache type "DMA_MIPS2_BANDWIDTH_REG" begin */
REGDEF_BIT(REQ_NUM, 16)
REGDEF_BIT(reserved, 16)
REGDEF_END(DMA_MIPS2_BANDWIDTH_REG)     /* declare register cache type "DMA_MIPS2_BANDWIDTH_REG" end */

#define DMA_DSP1_BANDWIDTH_REG_OFS      (0xB8 + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_DSP1_BANDWIDTH_REG)    /* declare register cache type "DMA_DSP1_BANDWIDTH_REG" begin */
REGDEF_BIT(REQ_NUM, 16)
REGDEF_BIT(reserved, 16)
REGDEF_END(DMA_DSP1_BANDWIDTH_REG)      /* declare register cache type "DMA_DSP1_BANDWIDTH_REG" end */

#define DMA_DSP2_BANDWIDTH_REG_OFS      (0xBC + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_DSP2_BANDWIDTH_REG)    /* declare register cache type "DMA_DSP2_BANDWIDTH_REG" begin */
REGDEF_BIT(REQ_NUM, 16)
REGDEF_BIT(reserved, 16)
REGDEF_END(DMA_DSP2_BANDWIDTH_REG)      /* declare register cache type "DMA_DSP2_BANDWIDTH_REG" end */
#endif

#define DMA_DSP2_BANDWIDTH_REG_OFS      (0xBC + MONITOR_OFFSET)
REGDEF_BEGIN(DMA_DSP2_BANDWIDTH_REG)    /* declare register cache type "DMA_DSP2_BANDWIDTH_REG" begin */
REGDEF_BIT(REQ_NUM, 16)
REGDEF_BIT(reserved, 16)
REGDEF_END(DMA_DSP2_BANDWIDTH_REG)      /* declare register cache type "DMA_DSP2_BANDWIDTH_REG" end */


//
// 0x1000~0x7FFC: DDR PHY region (provided by IP team)
//
#define DMA_PHY_CONTROL_REG_OFS    0x1000

#define DMA_PHY_00_REG_OFS      (0x0*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_00_REG)
REGDEF_BIT(ADJ_SEL_CK, 4)
REGDEF_BIT(ADJ_TYPE_CK, 1)
REGDEF_BIT(ADJ_EN_CK, 1)
REGDEF_BIT(, 26)
REGDEF_END(DMA_PHY_00_REG)

#define DMA_PHY_07_REG_OFS      (0x7*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_07_REG)
REGDEF_BIT(ADJ_SEL_DQS, 4)
REGDEF_BIT(ADJ_TYPE_DQS, 1)
REGDEF_BIT(ADJ_EN_DQS, 1)
REGDEF_BIT(, 26)
REGDEF_END(DMA_PHY_07_REG)

#define DMA_PHY_09_REG_OFS      (0x9*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_09_REG)
REGDEF_BIT(, 5)
REGDEF_BIT(MCLK_POL, 1)
REGDEF_BIT(D0_WEN_EN, 1)
REGDEF_BIT(D1_WEN_EN, 1)
REGDEF_BIT(, 24)
REGDEF_END(DMA_PHY_09_REG)

#define DMA_PHY_0A_REG_OFS      (0xA*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_0A_REG)
REGDEF_BIT(REN0_SEL, 4)
REGDEF_BIT(REN1_SEL, 4)
REGDEF_BIT(, 24)
REGDEF_END(DMA_PHY_0A_REG)

#define DMA_PHY_0B_REG_OFS      (0xB*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_0B_REG)
REGDEF_BIT(REN2_SEL, 4)
REGDEF_BIT(REN3_SEL, 4)
REGDEF_BIT(, 24)
REGDEF_END(DMA_PHY_0B_REG)

#define DMA_PHY_38_REG_OFS    (0x38*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_38_REG)
REGDEF_BIT(PLEG_CA, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_38_REG)

#define DMA_PHY_39_REG_OFS    (0x39*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_39_REG)
REGDEF_BIT(NLEG_CA, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_39_REG)

#define DMA_PHY_40_REG_OFS    (0x40*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_40_REG)
REGDEF_BIT(PLEG_MCLK, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_40_REG)

#define DMA_PHY_41_REG_OFS    (0x41*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_41_REG)
REGDEF_BIT(NLEG_MCLK, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_41_REG)

#define DMA_PHY_42_REG_OFS    (0x42*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_42_REG)
REGDEF_BIT(PLEG_0_LB, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_42_REG)

#define DMA_PHY_43_REG_OFS    (0x43*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_43_REG)
REGDEF_BIT(NLEG_0_LB, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_43_REG)

#define DMA_PHY_44_REG_OFS    (0x44*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_44_REG)
REGDEF_BIT(PLEG_0_HB, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_44_REG)

#define DMA_PHY_45_REG_OFS    (0x45*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_45_REG)
REGDEF_BIT(NLEG_0_HB, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_45_REG)

#define DMA_PHY_46_REG_OFS    (0x46*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_46_REG)
REGDEF_BIT(PLEG_0_DQS, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_46_REG)

#define DMA_PHY_47_REG_OFS    (0x47*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_47_REG)
REGDEF_BIT(NLEG_0_DQS, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_47_REG)

#define DMA_PHY_53_REG_OFS    (0x53*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_53_REG)
REGDEF_BIT(NLEG_CA_MRST, 5)
REGDEF_BIT(ODTEN_CA_MCS, 1)
REGDEF_BIT(ODTEN_CA_MRST, 1)
REGDEF_BIT(Reserved, 24)
REGDEF_END(DMA_PHY_53_REG)

#define DMA_PHY_54_REG_OFS    (0x54*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_54_REG)
REGDEF_BIT(NLEG_CA_MCS, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_54_REG)

#define DMA_PHY_57_REG_OFS    (0x57*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_57_REG)
REGDEF_BIT(PLEG_CA_MCS, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_57_REG)

#define DMA_PHY_58_REG_OFS    (0x58*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_58_REG)
REGDEF_BIT(PLEG_CA_MRST, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_58_REG)

#define DMA_PHY_85_REG_OFS    (0x85*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_85_REG)
REGDEF_BIT(ODT_MODE_CK, 3)
REGDEF_BIT(ODT_EN_CK, 1)
REGDEF_BIT(ODT_MODE_CMD, 3)
REGDEF_BIT(ODT_EN_CMD, 1)
REGDEF_BIT(Reserved, 24)
REGDEF_END(DMA_PHY_85_REG)

#define DMA_PHY_86_REG_OFS    (0x86*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_86_REG)
REGDEF_BIT(ODT_MODE_DQS0, 3)
REGDEF_BIT(ODT_EN_DQS0, 1)
REGDEF_BIT(ODT_MODE_D0, 3)
REGDEF_BIT(ODT_EN_D0, 1)
REGDEF_BIT(Reserved, 24)
REGDEF_END(DMA_PHY_86_REG)

#define DMA_PHY_AE_REG_OFS    (0xAE*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_AE_REG)
REGDEF_BIT(REG_WR_DQ1, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_AE_REG)

#define DMA_PHY_AF_REG_OFS    (0xAF*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_AF_REG)
REGDEF_BIT(REG_WR_DQS1, 5)
REGDEF_BIT(Reserved, 27)
REGDEF_END(DMA_PHY_AF_REG)

#define DMA_PHY_B0_REG_OFS    (0xB0*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B0_REG)
REGDEF_BIT(REG_CMD, 5)
REGDEF_BIT(, 1)
REGDEF_BIT(DATA0_REG_EN12, 1)
REGDEF_BIT(REG_RESET, 1)
REGDEF_BIT(Reserved, 24)
REGDEF_END(DMA_PHY_B0_REG)

#define DMA_PHY_B1_REG_OFS    (0xB1*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B1_REG)
REGDEF_BIT(REG_CLK, 5)
REGDEF_BIT(REG_VREF_2, 2)
    REGDEF_BIT(Reserved                 ,25)
REGDEF_END(DMA_PHY_B1_REG)

#define DMA_PHY_B2_REG_OFS    (0xB2*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B2_REG)
    REGDEF_BIT(REG_WR_DQ0               , 5)
    REGDEF_BIT(Reserved                 ,27)
REGDEF_END(DMA_PHY_B2_REG)

#define DMA_PHY_B3_REG_OFS    (0xB3*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B3_REG)
    REGDEF_BIT(REG_WR_DQS0              , 5)
    REGDEF_BIT(Reserved                 ,27)
REGDEF_END(DMA_PHY_B3_REG)

#define DMA_PHY_B4_REG_OFS    (0xB4*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B4_REG)
    REGDEF_BIT(REG_WR_DQ2               , 5)
    REGDEF_BIT(Reserved                 ,27)
REGDEF_END(DMA_PHY_B4_REG)

#define DMA_PHY_B5_REG_OFS    (0xB5*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B5_REG)
    REGDEF_BIT(REG_WR_DQS2              , 5)
    REGDEF_BIT(Reserved                 ,27)
REGDEF_END(DMA_PHY_B5_REG)

#define DMA_PHY_B7_REG_OFS    (0xB7*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B7_REG)
    REGDEF_BIT(REG_WR_ADDR              , 5)    // CMD and ADDR
    REGDEF_BIT(                         , 1)
    REGDEF_BIT(FDLOCK1                  , 1)
    REGDEF_BIT(FDLOCK0                  , 1)
    REGDEF_BIT(Reserved                 ,24)
REGDEF_END(DMA_PHY_B7_REG)

#define DMA_PHY_B8_REG_OFS    (0xB8*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B8_REG)
    REGDEF_BIT(REG_DQS1_90              , 4)
    REGDEF_BIT(REG_DQS0_90              , 4)
    REGDEF_BIT(Reserved                 ,24)
REGDEF_END(DMA_PHY_B8_REG)

#define DMA_PHY_B9_REG_OFS    (0xB9*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_B9_REG)
    REGDEF_BIT(REG_DQS3_90              , 4)
    REGDEF_BIT(REG_DQS2_90              , 4)
    REGDEF_BIT(Reserved                 ,24)
REGDEF_END(DMA_PHY_B9_REG)

#define DMA_PHY_BC_REG_OFS    (0xBC*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_BC_REG)
    REGDEF_BIT(REG_WR_DQ3               , 5)
    REGDEF_BIT(Reserved                 ,27)
REGDEF_END(DMA_PHY_BC_REG)

#define DMA_PHY_BD_REG_OFS    (0xBD*4 + DMA_PHY_CONTROL_REG_OFS)
REGDEF_BEGIN(DMA_PHY_BD_REG)
    REGDEF_BIT(REG_WR_DQS3              , 5)
    REGDEF_BIT(Reserved                 ,27)
REGDEF_END(DMA_PHY_BD_REG)

#endif /* _ASM_ARMV8_GIC_H_ */
