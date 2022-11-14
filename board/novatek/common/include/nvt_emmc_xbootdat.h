/*
 *  board/novatek/nt72670btk/nt72670b_emmc_xbootdat.h
 *
 *  Author:	Alvin lin
 *  Created:	Jun 5, 2015
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NT72670B_EMMC_XBOOT_DAT_H__
#define __NT72670B_EMMC_XBOOT_DAT_H__

#include <asm/types.h>

#define PANEL_PARAM_SZ 		512
#if defined(CONFIG_OLD_PANEL_PARAM)
#define PANEL_PARAM_VER 	1
#else
#define PANEL_PARAM_VER 	3
#endif
#define PANEL_PARM_NUM 		100
#define PANEL_INDEX_SIGN 	"PIDX"
#define PANEL_PARAM_SIGN 	"PANL"

typedef enum {
	XDAT_HELP_DUMP,
	XDAT_HELP_SAVE,
	XDAT_HELP_ERASE,
	XDAT_HELP_MAX
} NVT_EMMC_XBOOTDAT_HELP_TYPE;

typedef struct _nvt_emmc_xbootdat_info {
	char *xdat_name;
	int (*xdathelp)(struct _nvt_emmc_xbootdat_info *this, NVT_EMMC_XBOOTDAT_HELP_TYPE help_type);
	int (*xdatdump)(struct _nvt_emmc_xbootdat_info *this, int argc, char* const argv[]);
	int (*xdatsave)(struct _nvt_emmc_xbootdat_info *this, int argc, char* const argv[]);
	int (*xdaterase)(struct _nvt_emmc_xbootdat_info *this, int argc, char* const argv[]);
	unsigned int start_sector;//count in sector
	unsigned int size;//count in sector
} nvt_emmc_xbootdat_info;

typedef struct _stMode {
	u16 u16Width;	
	u16 u16Height;
	u16 u16TypHTotal;
	u8 u8HSyncWidth;
	u16 u16HSyncBackPorch;
	u16 u16HSyncStart;
	u16 u16TypVTotal;
	u8 u8VSyncWidth;
	u16 u16VSyncBackPorch;
	u16 u16VSyncStart;
	u16 u16PLL;
} stMode;

#if defined(CONFIG_OLD_PANEL_PARAM)
typedef struct _panel_param {
	u8 sign[4];
	u32 version;
	u8 u8PanelID;
	u8 b8Mode50Hz;
	u8 u8Panel;
	u8 au8Name[16];
	u8 b8DualPixelOutput;
	u8 b8ABSwap;
	u8 b8ChannelSwap;
	u8 b8FrameSyncEnable;
	u8 u8FrameSyncMode;
	u8 u8MISC;
	u8 enLvdsFormat;
	u8 b8RBSwap;
	u8 u8DitheringBit;
	u16 u16LVDSPhase;
	u16 u16LVDSPhase_PortA;
	u16 u16LVDSPhase_PortB;
	u16 u16VFreqDiff_Min;
	u16 u16VFreqDiff_Max;
	u16 u16VTotalDiff_Min;
	u16 u16VTotalDiff_Max;
	stMode stMode50Hz;
	stMode stMode60Hz;
	u16 u16PanelPowerOnDelay1;
	u16 u16PanelPowerOnDelay2;
	u16 u16PanelPowerOFFDelay1;
	u16 u16PanelPowerOFFDelay2;
	u16 u16BackLightLevel;
	u8 b8PWMDutyInv;
	u8 b8HFlip;
	u8 b8VFlip;
	u8 b8OutputPanelFHD;
	u8 VX1;
	u8 FRC;
	u8 SEP;
	u8 b8Blinking;
	u8 u8PanelType;
	u8 b8ForcePanelTiming2D;
	u8 b8ForcePanelTiming2DSport;
	u8 b8ForcePanelTiming3D;
	u16 u16PanelPowerGPIO;
	u16 u16PanelBackLightGPIO;
	u16 u16PanelTiming3D;
	u16 u16PWM2DSync;
	u16 u16PWM3DSync;
	u16 u16PDIMFreq2D50Hz;
	u16 u16PDIMFreq2D60Hz;
	u16 u16PDIMFreq2DSport50Hz;
	u16 u16PDIMFreq2DSport60Hz;
	u16 u16PDIMFreq3D50Hz;
	u16 u16PDIMFreq3D60Hz;
	u16 u16PDIMDelay2D50Hz;
	u16 u16PDIMDelay2D60Hz;
	u16 u16PDIMDelay2DSport50Hz;
	u16 u16PDIMDelay2DSport60Hz;
	u16 u16PDIMDelay3D50Hz;
	u16 u16PDIMDelay3D60Hz;
	u16 u16ADIMFreq2D50Hz;
	u16 u16ADIMFreq2D60Hz;
	u16 u16ADIMFreq2DSport50Hz;
	u16 u16ADIMFreq2DSport60Hz;
	u16 u16ADIMFreq3D50Hz;
	u16 u16ADIMFreq3D60Hz;
	u8 b8LocalDIMControl;
	u16 u16LRDelay3D;
	u16 u16BTDelay3D;
	u8 b8PDIMConByPanel3D;
	u8 u8Tcon;
	u8 u8TconType;
	u16 u16VX1PreEmphasisG1;
	u16 u16VX1PreEmphasisG2;
	u16 u16VX1PreEmphasisG3;
	u16 u16VX1SwingG1;
	u16 u16VX1SwingG2;
	u16 u16VX1SwingG3;
	u8 b8PanelPowerInvert;
	u8 b8BacklightPowerInver;
	u16 u16BacklightFreq;
}panel_param;
#else//CONFIG_OLD_PANEL_PARAM
typedef struct _panel_param {
	u8 sign[4];
	u32 version;
	u8 u8PanelID;
	u8 b8Mode50Hz;
	u8 u8Panel;
	u8 au8Name[100];
	u8 b8DualPixelOutput;
	u8 b8ABSwap;
	u8 b8ChannelSwap;
	u8 b8FrameSyncEnable;
	u8 u8FrameSyncMode;
	u8 u8MISC;
	u8 enLvdsFormat;
	u8 b8RBSwap;
	u8 u8DitheringBit;
	u16 u16LVDSPhase;
	u16 u16LVDSPhase_PortA;
	u16 u16LVDSPhase_PortB;
	u16 u16VFreqDiff_Min;
	u16 u16VFreqDiff_Max;
	u16 u16VTotalDiff_Min;
	u16 u16VTotalDiff_Max;
	stMode stMode50Hz;
	stMode stMode60Hz;
	u16 u16PanelPowerOnDelay1;
	u16 u16PanelPowerOnDelay2;
	u16 u16PanelPowerOnDelay3;
	u16 u16PanelPowerOnDelay4;
	u16 u16PanelPowerOFFDelay1;
	u16 u16PanelPowerOFFDelay2;
	u16 u16PanelPowerOFFDelay3;
	u16 u16PanelPowerGPIO;
	u16 u16PanelBackLightGPIO;
	u16 u16BackLightLevel;
	u16 u16BacklightFreq;
	u8 b8PWMDutyInv;
	u8 b8PanelPowerInvert;
	u8 b8BacklightPowerInvert;
	u8 b8HFlip;
	u8 b8VFlip;
	u8 VX1;
	u8 FRC;
	u8 SEP;
	u8 u8PanelType;
	u8 u8Tcon;
	u8 u8TconType;
	u8 u8LocalDIMControl;
	u8 u8LocalDIM_Zone_X;
	u8 u8LocalDIM_Zone_Y;
	u16 u16VX1PreEmphasisG1;
	u16 u16VX1PreEmphasisG2;
	u16 u16VX1PreEmphasisG3;
	u16 u16VX1SwingG1;
	u16 u16VX1SwingG2;
	u16 u16VX1SwingG3;
	u8 u8PanelIOMode;
	u8 u8PanelPwmMaxDuty;
	u8 u8PanelPwmMinDuty;
	u8 u8PanelCustomerID;
	u8 u8Reserved0;
	u8 u8Reserved1;
	u8 u8Reserved2;
	u8 u8Reserved3;
	u8 u8Reserved4;
	u8 u8Reserved5;
	u8 u8Reserved6;
	u8 u8Reserved7;
	u8 u8Reserved8;
	u8 u8Reserved9;
	u8 u8PanelI2cPortNum;
	u8 u8PanelI2cSlaveAddr;
	u8 u8PanelI2cWorkLog0;
	u8 u8PanelI2cWorkLog1;
	u8 u8PanelI2cWorkLog2;
	u8 u8PanelI2cWorkLog3;
	u8 u8PanelI2cWorkLog4;
	u8 u8PanelI2cWorkLog5;
	u8 u8PanelI2cWorkLog6;
	u8 u8PanelI2cWorkLog7;
	u8 u8PanelI2cWorkLog8;
	u8 u8PanelI2cWorkLog9;
	u8 u8PanelI2cWorkLog10;
	u8 u8PanelI2cWorkLog11;
	u8 u8PanelI2cWorkLog12;
	u8 u8PanelI2cWorkLog13;
	u8 u8PanelI2cWorkLog14;
	u8 u8PanelI2cWorkLog15;
	u8 u8PanelI2cWorkLog16;
	u8 u8PanelI2cWorkLog17;
	u8 u8PanelI2cWorkLog18;
	u8 u8PanelI2cWorkLog19;
	u8 u8PanelI2cWorkLog20;
	u8 u8PanelI2cWorkLog21;
	u8 u8PanelI2cWorkLog22;
	u8 u8PanelI2cWorkLog23;
	u8 u8PanelI2cWorkLog24;
	u8 u8PanelI2cWorkLog25;
	u8 u8PanelI2cWorkLog26;
	u8 u8PanelI2cWorkLog27;
	u8 u8PanelI2cWorkLog28;
	u8 u8PanelI2cWorkLog29;
	u8 u8PanelI2cWorkLog30;
	u8 u8PanelI2cWorkLog31;
	u8 u8PanelI2cWorkLog32;
	u8 u8PanelI2cWorkLog33;
	u8 u8PanelI2cWorkLog34;
	u8 u8PanelI2cWorkLog35;
	u8 u8PanelI2cWorkLog36;
	u8 u8PanelI2cWorkLog37;
	u8 u8PanelI2cWorkLog38;
	u8 u8PanelI2cWorkLog39;
	u8 u8PanelI2cWorkLog40;
	u8 u8PanelI2cWorkLog41;
	u8 u8PanelI2cWorkLog42;
	u8 u8PanelI2cWorkLog43;
	u8 u8PanelI2cWorkLog44;
	u8 u8PanelI2cWorkLog45;
	u8 u8PanelI2cWorkLog46;
	u8 u8PanelI2cWorkLog47;
	u8 u8PanelI2cWorkLog48;
	u8 u8PanelI2cWorkLog49;
	u8 u8PanelI2cWorkLog50;
	u8 u8PanelI2cWorkLog51;
	u8 u8PanelI2cWorkLog52;
	u8 u8PanelI2cWorkLog53;
	u8 u8PanelI2cWorkLog54;
	u8 u8PanelI2cWorkLog55;
	u8 u8PanelI2cWorkLog56;
	u8 u8PanelI2cWorkLog57;
	u8 u8PanelI2cWorkLog58;
	u8 u8PanelI2cWorkLog59;
	u8 u8PanelI2cWorkLog60;
	u8 u8PanelI2cWorkLog61;
	u8 u8PanelI2cWorkLog62;
	u8 u8PanelI2cWorkLog63;
	u8 u8PanelI2cWorkLog64;
	u8 u8PanelI2cWorkLog65;
	u8 u8PanelI2cWorkLog66;
	u8 u8PanelI2cWorkLog67;
	u8 u8PanelI2cWorkLog68;
	u8 u8PanelI2cWorkLog69;
	u8 u8PanelI2cWorkLog70;
	u8 u8PanelI2cWorkLog71;
	u8 u8PanelI2cWorkLog72;
	u8 u8PanelI2cWorkLog73;
	u8 u8PanelI2cWorkLog74;
	u8 u8PanelI2cWorkLog75;
	u8 u8PanelI2cWorkLog76;
	u8 u8PanelI2cWorkLog77;
	u8 u8PanelI2cWorkLog78;
	u8 u8PanelI2cWorkLog79;
	u8 u8PanelI2cWorkLog80;
	u8 u8PanelI2cWorkLog81;
	u8 u8PanelI2cWorkLog82;
	u8 u8PanelI2cWorkLog83;
	u8 u8PanelI2cWorkLog84;
	u8 u8PanelI2cWorkLog85;
	u8 u8PanelI2cWorkLog86;
	u8 u8PanelI2cWorkLog87;
	u8 u8PanelI2cWorkLog88;
	u8 u8PanelI2cWorkLog89;
	u8 u8PanelI2cWorkLog90;
	u8 u8PanelI2cWorkLog91;
	u8 u8PanelI2cWorkLog92;
	u8 u8PanelI2cWorkLog93;
	u8 u8PanelI2cWorkLog94;
	u8 u8PanelI2cWorkLog95;
	u8 u8PanelI2cWorkLog96;
	u8 u8PanelI2cWorkLog97;
	u8 u8PanelI2cWorkLog98;
	u8 u8PanelI2cWorkLog99;
}panel_param;

#endif//CONFIG_OLD_PANEL_PARAM

typedef struct {
	panel_param p;
	u8 padding[PANEL_PARAM_SZ - sizeof(panel_param)];
} panel_param_blk;

typedef struct {
	u8 sign[4];
	u32 version;
	u32 index;
	u32 param_cnt;
} panel_index;

typedef struct {
	panel_index idx;
	u8 padding[PANEL_PARAM_SZ - sizeof(panel_index)];
} panel_index_blk;


void xbootdat_init(unsigned int xbootdat_start_offset);
nvt_emmc_xbootdat_info* nvt_emmc_get_xdat_by_name(char *name);
int nvt_prepare_kercmd(void);
int nvt_prepare_rcvcmd(void);
#endif
