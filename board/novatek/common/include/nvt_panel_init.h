/*
 *  nvt_panel_init.h
 *
 *  Created:	Mar 22, 2016
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_PANEL_INIT_H__
#define __NVT_PANEL_INIT_H__

#define TRUE				1
#define FALSE				0

#define BLEND_OSD_MODE			0
#define SEPARATE_OSD_MODE		1

#define _ENABLE_		1
#define _DISABLE_		0

#define _DEFAULT_		0

#define PANEL_OUTPUT_LVDS_MODE            ( 0x80 )
#define PANEL_OUTPUT_VX1_MODE             ( 0x55 )

#define SYS_MPLL_PAGE_EN	*((volatile unsigned long *)(SYS_CLK_REG_BASE + 0xBC))
#define SYS_CLK_REG_BASE                (0xfd020000)
#define _SYS_MPLL_PAGE_B_EN	0x00000001
#define _SYS_MPLL_PAGE_0_EN	0x00000002

#define LCDSEL_SHIFTBITS		2
#define FHDSEL_SHIFTBITS		1
#define JEIDASEL_SHIFTBITS		0


#define HAL_READ_UINT32( _register_, _value_ ) \
        ((_value_) = *((volatile unsigned int *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ ) \
        (*((volatile unsigned int *)(_register_)) = (_value_))

#define _MPLL_EnablePage0() \
    SYS_MPLL_PAGE_EN = _SYS_MPLL_PAGE_0_EN;


#define GPE_DIR_OUTPUT(index)           ((*(volatile unsigned long*)REG_GPE_DIR) |= (1<<(index)))
#define GPE_OUTPUT_SET(index)           ((*(volatile unsigned long*)REG_GPE_SET) |= (1<<(index)))
#define REG_GPE_BASE                    (0xFD100000)

#define REG_GPE_SET                     (REG_GPE_BASE+0x04)
#define REG_GPE_DIR                     (REG_GPE_BASE+0x08)

#define GPB_DIR_OUTPUT(index)           ((*(volatile unsigned long*)REG_GPB_DIR) |= (1<<(index)))
#define GPB_OUTPUT_SET(index)           ((*(volatile unsigned long*)REG_GPB_SET) |= (1<<(index)))
#define GPB_OUTPUT_CLEAR(index)       ((*(volatile unsigned long*)REG_GPB_CLEAR) = (0x00|(1<<(index))))
#define GPB_FUN_SET(index)                ((*(volatile unsigned long*)REG_GPB_FUN) |= (1<<(index)))

#define REG_GPB_BASE                    (0xFD0F0000)

#define REG_GPB_SET                     (REG_GPB_BASE+0x44)
#define REG_GPB_DIR                     (REG_GPB_BASE+0x48)
#define REG_GPB_CLEAR                 (REG_GPB_BASE+0x40)
#define REG_GPB_FUN                     (REG_GPB_BASE+0x64)

#define REG_GPC_BASE                    (0xFD100000)
#define REG_GPC_CLEAR                   (REG_GPC_BASE+0x00)
#define REG_GPC_SET                     (REG_GPC_BASE+0x04)
#define REG_GPC_DIR                     (REG_GPC_BASE+0x08)


#define GPC_DIR_OUTPUT(index)            ((*(volatile unsigned long*)REG_GPC_DIR) |= (1<<(index)))
#define GPC_OUTPUT_SET(index)           ((*(volatile unsigned long*)REG_GPC_SET) |= (1<<(index)) )
#define GPC_OUTPUT_CLR(index)           ((*(volatile unsigned long*)REG_GPC_CLEAR) |= (1<<(index)) )


#define P1C0B0X	( 0xFC0B0000 ) // T-Con
#define P1C0D00 ( 0xFC0D0000 ) // OSD...
#define P1D0600	( 0xFD060000 )
#define P1D1001	( 0xFD100100 ) // LVDS
#define P1D100C	( 0xFD100C00 )
#define P1D1010	( 0xFD101000 )
#define P0F		( 0xFE000F00 )
#define P34		( 0xFE003400 )
#define P7F		( 0xFE007F00 )
#define P1D6A00	( 0xFD6A0000 )
#define P1D0200	( 0xFD020000 ) 
#define P1D6800  (0xFD680000)
#define P1C1E00  (0xFC1E0000)
#define P1C1E01  (0xFC1E0100)

#define P1D0600R04 ( P1D0600 + 0x04 )
#define P1D1001R04 ( P1D1001 + 0x04 )
#define P1D1001R08 ( P1D1001 + 0x08 )
#define P1D1001R0C ( P1D1001 + 0x0C )
#define P1D1001R10 ( P1D1001 + 0x10 )
#define P1D1001R14 ( P1D1001 + 0x14 )
#define P1D100CR00 ( P1D100C + 0x00 )
#define P1D100CR04 ( P1D100C + 0x04 )
#define P1D100CR08 ( P1D100C + 0x08 )
#define P1D100CR0C ( P1D100C + 0x0C )
#define P1D100CR10 ( P1D100C + 0x10 )
#define P1D1010R1C ( P1D1010 + 0x1C )
#define P1D100CR18 ( P1D100C + 0x18 )
#define P1D0200RFC ( P1D0200 + 0xFC )

#define P0FR00 ( P0F + 0x00 )
#define P0FR04 ( P0F + 0x04 )
#define P0FR08 ( P0F + 0x08 )
#define P0FR0C ( P0F + 0x0C )
#define P0FR10 ( P0F + 0x10 )
#define P0FR14 ( P0F + 0x14 )
#define P0FR18 ( P0F + 0x18 )
#define P0FR1C ( P0F + 0x1C )
#define P0FR20 ( P0F + 0x20 )
#define P0FR28 ( P0F + 0x28 )
#define P0FR2C ( P0F + 0x2C )
#define P0FR30 ( P0F + 0x30 )
#define P0FR34 ( P0F + 0x34 )
#define P0FR38 ( P0F + 0x38 )
#define P0FR3C ( P0F + 0x3C )
#define P0FR40 ( P0F + 0x40 )
#define P0FR44 ( P0F + 0x44 )
#define P0FR48 ( P0F + 0x48 )
#define P0FR5C ( P0F + 0x5C )
#define P0FR60 ( P0F + 0x60 )
#define P0FREC ( P0F + 0xEC )
#define P0FRFC ( P0F + 0xFC )




#define P1C1E00R00 ( P1C1E00 + 0x00 )
#define P1C1E01R0C ( P1C1E01 + 0x0C )

typedef enum _PANEL_SEL
{
	LED_1366x768,
	LED_1920x1080,
	LED_3840x2160,
	PDP_1920x1080,
	PDP_1024x768,
	PDP_852x480
}PANEL_SEL;


typedef enum _EN_DRV_SCLR_VIDEO_OUTPUT_DITHER
{
	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_SCALER_DITHER_10BITS,
	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_SCALER_DITHER_8BITS,
	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_SCALER_DITHER_6BITS,

	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_DP_DITHER_10BITS,
	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_DP_DITHER_8BITS,
	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_DP_DITHER_6BITS,

	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER_TOTAL
} EN_DRV_SCLR_VIDEO_OUTPUT_DITHER;


typedef enum _EN_DRV_SCLR_LVDS_OUT
{
	EN_DRV_SCLR_LVDS_OUT_SINGLE,
	EN_DRV_SCLR_LVDS_OUT_DUAL,
	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_1PORT_3PAIR_6X,
	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_1PORT_6PAIR_6X,
	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_2PORT_3PAIR_6X,

	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_1PORT_3PAIR_8X,
	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_1PORT_6PAIR_8X,
	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_2PORT_3PAIR_8X,

	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_2PORT_6PAIR_6X,
	EN_DRV_SCLR_LVDS_OUT_MINI_LVDS_2PORT_6PAIR_8X,

	EN_DRV_SCLR_VX1_OUT_VIDEO_8LAN_OSD_4LAN, // Video 4K2k 60 OSD 4k2k30
	EN_DRV_SCLR_VX1_OUT_VIDEO_8LAN_OSD_2LAN, // Video 4K2k 60 OSD FULL HD
	EN_DRV_SCLR_VX1_OUT_VIDEO_4LAN_OSD_4LAN, // Video 4K2k 30 OSD 4k2k30
	EN_DRV_SCLR_VX1_OUT_VIDEO_4LAN_OSD_2LAN, // Video 4K2k 30 OSD FULL HD
	EN_DRV_SCLR_VX1_OUT_VIDEO_2LAN_OSD_2LAN, // Video FULL HD OSD FULL HD
	EN_DRV_SCLR_LVDS_OUT_TOTAL
} EN_DRV_SCLR_LVDS_OUT;

typedef enum _EN_DRV_SCLR_VX1_COMB
{
	EN_DRV_SCLR_VX1_COMB_1LAN_NOOSD 		= 0x00,  // 74.25MHz Video
	EN_DRV_SCLR_VX1_COMB_1LAN_1LANOSD		= 0x01,  // 74.25MHz Video / OSD
	EN_DRV_SCLR_VX1_COMB_2LAN_NOOSD			= 0x02,  // 148.5MHz Video
	EN_DRV_SCLR_VX1_COMB_2LAN_1LANOSD		= 0x03,  // 148.5MHz Video; 74.25MHz OSD
	EN_DRV_SCLR_VX1_COMB_2LAN_2LANOSD		= 0x04,  // 148.5MHz Video; 148.5MHz OSD
	EN_DRV_SCLR_VX1_COMB_4LAN_NOOSD			= 0x10,  // 4K2K30 Video 
	EN_DRV_SCLR_VX1_COMB_4LAN_1LANOSD		= 0x11,  // 4K2K30 Video / 74.25MHz OSD,
	EN_DRV_SCLR_VX1_COMB_4LAN_2LANOSD		= 0x12,  // 4K2K30 Video / FULL HD OSD,
	EN_DRV_SCLR_VX1_COMB_4LAN_4LANOSD		= 0x15,  // 4K2K30 Video / 4K2K30 OSD,
	EN_DRV_SCLR_VX1_COMB_8LAN_NOOSD			= 0x16,  // 4K2K60 Video 
	EN_DRV_SCLR_VX1_COMB_8LAN_1LANOSD		= 0x17,  // 4K2K60 Video / 74.25MHz OSD,
	EN_DRV_SCLR_VX1_COMB_8LAN_2LANOSD		= 0x18,  // 4K2K60 Video / FULL HD OSD,
	EN_DRV_SCLR_VX1_COMB_8LAN_4LANOSD		= 0x1B,  // 4K2K60 Video / 4K2K30 OSD,

	EN_DRV_SCLR_VX1_COMB_TOTAL
	
} EN_DRV_SCLR_VX1COMB;


typedef enum _EN_DRV_SCLR_LVDS_FMT
{
#if defined(CONFIG_OLD_PANEL_PARAM)
    EN_DRV_SCLR_LVDS_FMT_10BITS_NS,
    EN_DRV_SCLR_LVDS_FMT_10BITS_JEIDA,
    EN_DRV_SCLR_LVDS_FMT_10BITS_VESA,
    EN_DRV_SCLR_LVDS_FMT_8BITS_VESA,
    EN_DRV_SCLR_LVDS_FMT_8BITS_JEIDA,
#else
    EN_DRV_SCLR_LVDS_FMT_8BITS_JEIDA,
    EN_DRV_SCLR_LVDS_FMT_8BITS_VESA,
    EN_DRV_SCLR_LVDS_FMT_10BITS_JEIDA,
    EN_DRV_SCLR_LVDS_FMT_10BITS_VESA,
    EN_DRV_SCLR_LVDS_FMT_10BITS_NS,
#endif
	EN_DRV_SCLR_LVDS_FMT_TOTAL
} EN_DRV_SCLR_LVDS_FMT;

typedef enum _EN_DRV_SCLR_PANEL_RBSWAP
{
	EN_DRV_SCLR_PANEL_RBSWAP_NORMAL,
	EN_DRV_SCLR_PANEL_RBSWAP_SWAP,

	EN_DRV_SCLR_PANEL_RBSWAP_TOTAL
} EN_DRV_SCLR_PANEL_RBSWAP;

typedef enum _EN_DRV_SCLR_LVDS_ABSWAP
{
	EN_DRV_SCLR_LVDS_ABSWAP_NORMAL,
	EN_DRV_SCLR_LVDS_ABSWAP_SWAP,

	EN_DRV_SCLR_LVDS_ABSWAP_TOTAL
} EN_DRV_SCLR_LVDS_ABSWAP;

typedef enum _EN_DRV_SCLR_LVDS_SINGLE_PORT_SWAP
{
	EN_DRV_SCLR_LVDS_SINGLE_PORT_SWAP_NORMAL,
	EN_DRV_SCLR_LVDS_SINGLE_PORT_SWAP_SWAP,

	EN_DRV_SCLR_LVDS_SINGLE_PORT_SWAP_TOTAL
} EN_DRV_SCLR_LVDS_SINGLE_PORT_SWAP;


struct ST_KER_VID_PANEL_SETTING
{
    u8 		VX1;
	u8 		FRC;
	u8 		SEP;
	u16     u16Width;
	u16     u16Height;
	u16     u16TypHTotal;
	u8      u8MinHSyncWidth;
	u16     u16MinHSyncBackPorch; // Reserved for future use
	u16     u16HSyncStart;
	u16     u16TypVTotal;
	u16     u16MaxVTotal;
	u8      u8MinVSyncWidth;
	u16     u16MinVSyncBackPorch; // Reserved for future use
	u16     u16VSyncStart;
	u8      u8Phase;
	bool    b8DualPixelOutput;
	bool    b8ABSwap;
	bool    b8ChannelSwap;        // Reserved for future use
	bool    b8FrameSyncEnable;
	u8      u8FrameSyncMode;
	u8      u8MISC;
	bool    b8Mode10Bits;
	bool    b8JIEDA;
	bool    b8RBSwap;
	u32     u32PLL;               // 0.01MHz
	EN_DRV_SCLR_VIDEO_OUTPUT_DITHER      eDitheringBit;
	u16     u16LVDSPhase;
	bool 	DPLL;
	u8 		enLvdsFormat;
	u16 	PanelPowerOnDelay1;
	u8		u8Tcon;
      u16 	u16PanelPowerGPIO; 
      u16 	u16PanelBackLightGPIO;
	u8		u8PanelType;
	u16		u16VX1SwingG1;
	u16		u16VX1PreEmphasisG1;
	u8		u8LocalDIMControl;
	u8		u8PanelI2cPortNum;
	u8		u8PanelI2cSlaveAddr;
	u16		PanelPowerOnDelay4;
       u8         PanelIOMode;
};

struct PST_OSD_DISP_FORMAT
{
	u32  	ulVertical_Start;
	u32  	ulHorizontal_Start ;
	u32 	ulSrcWidth;
	u32 	ulSrcHeight;
	u32 	ulDestWidth;
	u32 	ulDestHeight;
	u32 	ulLineOffset;
	u8		ucColorFormat;
	u8 		ucPlaneID;
	u8 		ucVFlip_Enable;
	u32* 	pulBimpAddr;
};

typedef enum _EN_DRV_SCLR_GPIO_GROUP
{
    EN_DRV_SCLR_GPIO_GROUP_A = 0,  
    EN_DRV_SCLR_GPIO_GROUP_B = 1,  
    EN_DRV_SCLR_GPIO_GROUP_C = 2,  
    EN_DRV_SCLR_GPIO_GROUP_D = 3,  
    EN_DRV_SCLR_GPIO_GROUP_E = 4,  
    EN_DRV_SCLR_GPIO_GROUP_F = 5,  
    EN_DRV_SCLR_GPIO_GROUP_G = 6,
    EN_DRV_SCLR_GPIO_GROUP_H = 7,
    EN_DRV_SCLR_GPIO_GROUP_I = 8,  
} EN_DRV_SCLR_GPIO_GROUP;


u32 UTL_SHL_GetValue( u32 u32Val, u32 u32Msb, u32 u32Lsb );
u32 UTL_SHL_GetValue( u32 u32Val, u32 u32Msb, u32 u32Lsb );
void UTL_SHL_WriteClockGenRegMask( u32 u32Addr, u32 u32Msb, u32 u32Lsb, u32 u32Val );
void _Out_P_SetVx1On(bool bLvdsOn);
void _Vx1PhyInitSetting( EN_DRV_SCLR_VX1COMB enOutput );
bool _VID_PowerON_Sequence(void);
void panel_init_late(void);

#endif
