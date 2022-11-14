/**
    NVT OPTees utilities for command customization

    @file       nvt_tzpc_utils.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __NVT_TZPC_UTILS_H__
#define __NVT_TZPC_UTILS_H__

typedef enum {
	CPU_TZPC_SCE_CTRL       = 0x62,
	CPU_TZPC_EFUSE_CTRL     = 0x66,
	CPU_TZPC_TRNG_CTRL      = 0x68,
	CPU_TZPC_RSA_CTRL       = 0x6a,

	CPU_TZPC_HASH_CTRL      = 0x70,
	CPU_TZPC_TZPC_CTRL      = 0xFF,

	CPU_TZPC_MAX,

} CPU_TZPC_MEMORY_MAP;

typedef struct {
	CPU_TZPC_MEMORY_MAP     base_address;
	char                   *module_name;

} TZPC_NAME, *pTZPC_NAME;


#if 0
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
	UINT32 bReserved: 28;
	// ch 160
	UINT32 bCPU: 1;
	UINT32 bReserved2: 15;

	UINT32 bCPU2: 1;                           // for compatible
	UINT32 bDSP_0: 1;
	UINT32 bDSP_1: 1;
	UINT32 bDSP2_0: 1;
	UINT32 bDSP2_1: 1;

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
#endif
#endif /* __NVT_TZPC_UTILS_H__ */

