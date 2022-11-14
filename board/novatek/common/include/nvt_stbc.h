/*
 *  board/novatek/evb670b/nvt_stbc.h
 *
 *  Author:	Alvin lin
 *  Created:	April 29, 2013
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __NVT_STBC_H__
#define __NVT_STBC_H__
void nvt_stbc_unlock_reg(void);
void nvt_stbc_set_keypass(int enable);
void nvt_stbc_stay_in_sram(void);
void nvt_stbc_exit_idle(void);
void nvt_stbc_reset(void);
#define STBCDAT_SIGN     "SDAT"

//Power on mode store in partition 7(important data), which use EXT4 filesystem
#define POWERON_MODE_PARTITON 7
#define POWERON_MODE_NAME "power_mode.bin"
#define USR_MODE_PARTITON 8
#define USR_MODE_NAME "usr_stbc.bin"

#define MAC_FILE_NAME	"mac.bin"

typedef struct _stbcdat {
	unsigned char sign[4];//'S','D','A','T'
	unsigned int uboot_load_id;//0 or 1
	unsigned char reserved[504];
} stbcdat;

#define STBCDATA_SIGN		"STBC"

typedef enum {
	POWERON_MODE_ON,
	POWERON_MODE_STB,	
	POWERON_MODE_LAST,
	POWERON_MODE_NUMS
}FAC_POWERON_MODE_TYPE;

typedef enum _stbc_pwr_sts {
	STBC_PWR_ON,
	STBC_PWR_STANDBY	
}FAC_POWERON_LAST_MODE;

typedef enum _backlight_flag {
	BACKLIGHT_ON,
	BACKLIGHT_OFF	
}BACKLIGHT_FLAG_E;

typedef enum _keypad_lock_flag { 
	KEYPAD_UNLOCK,
	KEYPAD_LOCK	
}KEYPAD_LOCK_FLAG;

typedef enum _cec_auto_power_on_mode{
	AUTO_POWER_OFF,
	AUTO_POWER_ON
}CEC_AUTO_POWER_ON_MODE;

/*typedef struct _stbc_data {
	u8  Sign[4];//signature 'S','T','B','C'
	FAC_POWERON_MODE_TYPE PowerOnMode;
	FAC_POWERON_LAST_MODE PowerStatus;
	BACKLIGHT_FLAG_E BacklightOnFlag;
	KEYPAD_LOCK_FLAG KeypadLockFlag;  
	CEC_AUTO_POWER_ON_MODE CecAutoPowerOnMode;
} FAC_STBC_DATA;*/

typedef struct _stbc_data{
	char Sign[4];//signature 'S','T','B','C'
	FAC_POWERON_MODE_TYPE PowerOnMode;
}FAC_STBC_DATA;

typedef struct _usr_stbc_data{
	char Sign[4];//signature 'S','T','B','C'
	FAC_POWERON_LAST_MODE PowerStatus;
	BACKLIGHT_FLAG_E BacklightOnFlag;
	KEYPAD_LOCK_FLAG KeypadLockFlag;  
	CEC_AUTO_POWER_ON_MODE CecAutoPowerOnMode;
}USR_STBC_DATA;

typedef enum _EN_DRV_STBC_POWER_ON_EVENT
{
	EN_DRV_STBC_POWER_ON_EVENT_LAST_POWER_IS_ON = 1,
	EN_DRV_STBC_POWER_ON_EVENT_AC_POWER_TO_ON,

	//remote-controller wake up.
    EN_DRV_STBC_POWER_ON_EVENT_RC_PWR_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N0_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N1_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N2_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N3_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N4_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N5_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N6_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N7_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N8_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_N9_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_PRG_UP_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_PRG_DOWN_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_IN_SRC_KEY,//16

    EN_DRV_STBC_POWER_ON_EVENT_RC_KEY_START = EN_DRV_STBC_POWER_ON_EVENT_RC_PWR_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_RC_KEY_END = EN_DRV_STBC_POWER_ON_EVENT_RC_IN_SRC_KEY,

	//front-panel wake up.
	EN_DRV_STBC_POWER_ON_EVENT_FP_PWR_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_FP_PRG_UP_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_FP_PRG_DOWN_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_FP_VOL_UP_KEY, 		//touchpad
    EN_DRV_STBC_POWER_ON_EVENT_FP_VOL_DOWN_KEY, 		//touchpad
    EN_DRV_STBC_POWER_ON_EVENT_FP_TVAV_KEY, 			//touchpad
    EN_DRV_STBC_POWER_ON_EVENT_FP_IN_SRC_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_FP_MENU_KEY,

    EN_DRV_STBC_POWER_ON_EVENT_FP_KEY_START = EN_DRV_STBC_POWER_ON_EVENT_FP_PWR_KEY,
    EN_DRV_STBC_POWER_ON_EVENT_FP_KEY_END = EN_DRV_STBC_POWER_ON_EVENT_FP_MENU_KEY,


	EN_DRV_STBC_POWER_ON_EVENT_VALID_SYNC_TO_ON, 
	EN_DRV_STBC_POWER_ON_EVENT_WAKE_UP_TIME_MATCH,
	EN_DRV_STBC_POWER_ON_EVENT_CEC_POWER_ON,
    EN_DRV_STBC_POWER_ON_EVENT_SCART_PIN8_EXT1,
    EN_DRV_STBC_POWER_ON_EVENT_SCART_PIN8_EXT2,
    EN_DRV_STBC_POWER_ON_EVENT_CUSTOMER1,
    EN_DRV_STBC_POWER_ON_EVENT_CUSTOMER2,
    EN_DRV_STBC_POWER_ON_EVENT_CUSTOMER3,

	EN_DRV_STBC_POWER_ON_EVENT_RESET, 				//SLSL 2010-06-28 add system reset event
	EN_DRV_STBC_POWER_ON_EVENT_UPGRADE,
	EN_DRV_STBC_POWER_ON_EVENT_WDT_RESET,
	EN_DRV_STBC_POWER_ON_EVENT_DBG_MENU,
	
	EN_DRV_STBC_POWER_ON_EVENT_LONG_PRESS_IR_RESET,
	EN_DRV_STBC_POWER_ON_EVENT_LONG_PRESS_KEYPAD_RESET,
	EN_POWER_ON_EVENT_AC_POWER_TO_ON_THEN_STANDBY = 0x2E,  //0X2E , for BT RC, standby after firmware load in u-boot
	
	EN_DRV_STBC_POWER_ON_EVENT_NUM=(EN_DRV_STBC_POWER_ON_EVENT_LONG_PRESS_KEYPAD_RESET+1),

	EN_DRV_STBC_POWER_ON_EVENT_RC_KEY_NUM = (EN_DRV_STBC_POWER_ON_EVENT_RC_KEY_END - EN_DRV_STBC_POWER_ON_EVENT_RC_KEY_START + 1),
	EN_DRV_STBC_POWER_ON_EVENT_FP_KEY_NUM = (EN_DRV_STBC_POWER_ON_EVENT_FP_KEY_END - EN_DRV_STBC_POWER_ON_EVENT_FP_KEY_START + 1),

	EN_DRV_STBC_POWER_ON_EVENT_INVALID = 0xFF

} EN_DRV_STBC_POWER_ON_EVENT;

typedef enum
{
	EN_CLOCK_DIVIDER_12M = 0,
	EN_CLOCK_DIVIDER_6M,
	EN_CLOCK_DIVIDER_3M
}EN_CLOCK_DIVIDER;

int nvt_stbc_boot_check(void);
void nvt_stbc_enable_wdog(unsigned short sec);
void nvt_stbc_disable_wdog(void);
unsigned int nvt_stbc_read_power_on_event(void);
unsigned int nvt_stbc_get_clock_divider(void);

typedef enum {
	NVT_SAR_ADC0,
	NVT_SAR_ADC1,
	NVT_SAR_ADC2,
	NVT_SAR_ADC3,
	NVT_SAR_ADC4,
	NVT_SAR_ADC5,
	NVT_SAR_ADC_MAX,
} NVT_SAR_ADC_ID;

typedef enum _EN_GPIO_CTRL
{
	EN_GPIO_CTRL_FUNCTION,
	EN_GPIO_CTRL_GPIO_INPUT,
	EN_GPIO_CTRL_GPIO_OUTPUT_HIGH,
	EN_GPIO_CTRL_GPIO_OUTPUT_LOW,	
	EN_GPIO_CTRL_TOTAL
}EN_GPIO_CTRL;

void nvt_stbc_sar_adc_init(void);
int nvt_stbc_sar_adc_pressed(NVT_SAR_ADC_ID id);
void nvt_stbc_set_gpio(u8 pin, EN_GPIO_CTRL enCtrl);


#endif
