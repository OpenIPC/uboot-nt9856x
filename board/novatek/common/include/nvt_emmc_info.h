
#ifndef __NVT_MMC_INFO_H
#define __NVT_MMC_INFO_H
#include <asm/types.h>
/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_FFU_INSTALL		(0x01)
#define EXT_CSD_FFU_MODE		(0x01)
#define EXT_CSD_NORMAL_MODE		(0x00)
#define EXT_CSD_FFU			(1<<0)
#define EXT_CSD_UPDATE_DISABLE		(1<<0)
#define EXT_CSD_HPI_SUPP		(1<<0)
#define EXT_CSD_HPI_IMPL		(1<<1)
//#define EXT_CSD_CMD_SET_NORMAL		(1<<0)
#define EXT_CSD_BOOT_WP_B_PWR_WP_DIS	(0x40)
#define EXT_CSD_BOOT_WP_B_PERM_WP_DIS	(0x10)
#define EXT_CSD_BOOT_WP_B_PERM_WP_EN	(0x04)
#define EXT_CSD_BOOT_WP_B_PWR_WP_EN	(0x01)
#define EXT_CSD_BOOT_INFO_HS_MODE	(1<<2)
#define EXT_CSD_BOOT_INFO_DDR_DDR	(1<<1)
#define EXT_CSD_BOOT_INFO_ALT		(1<<0)
#define EXT_CSD_BOOT_CFG_ACK		(1<<6)
#define EXT_CSD_BOOT_CFG_EN		(0x38)
#define EXT_CSD_BOOT_CFG_ACC		(0x07)
#define EXT_CSD_RST_N_EN_MASK		(0x03)
#define EXT_CSD_HW_RESET_EN		(0x01)
#define EXT_CSD_HW_RESET_DIS		(0x02)
#define EXT_CSD_PART_CONFIG_ACC_MASK	  (0x7)
#define EXT_CSD_PART_CONFIG_ACC_NONE	  (0x0)
#define EXT_CSD_PART_CONFIG_ACC_BOOT0	  (0x1)
#define EXT_CSD_PART_CONFIG_ACC_BOOT1	  (0x2)
#define EXT_CSD_PART_CONFIG_ACC_USER_AREA (0x7)
#define EXT_CSD_PART_CONFIG_ACC_ACK	  (0x40)
#define EXT_CSD_PARTITIONING_EN		(1<<0)
#define EXT_CSD_ENH_ATTRIBUTE_EN	(1<<1)
#define EXT_CSD_ENH_4			(1<<4)
#define EXT_CSD_ENH_3			(1<<3)
#define EXT_CSD_ENH_2			(1<<2)
#define EXT_CSD_ENH_1			(1<<1)
//#define EXT_CSD_ENH_USR			(1<<0)
#define EXT_CSD_REV_V5_1		8
#define EXT_CSD_REV_V5_0		7
#define EXT_CSD_REV_V4_5		6
#define EXT_CSD_REV_V4_4_1		5
#define EXT_CSD_REV_V4_3		3
#define EXT_CSD_REV_V4_2		2
#define EXT_CSD_REV_V4_1		1
#define EXT_CSD_REV_V4_0		0

char *secure_remova_type[] = {
	"information removed by an erase of the physical memory",
	"information removed by an overwriting the addressed locations with a character"
	"followed by an erase",
	"information removed by an overwriting the addressed locations with a character, its"
	"complement, then a random character",
	"information removed using a vendor defined"
};
char *mode_config[] = {
	"Normal Mode",
	"FFU Mode",
	"Vendor Specific Mode",
	"Reserved"
};
char *power_off_notification[] = {
	"NO_POWER_NOTIFICATION",
	"POWERED_ON",
	"POWER_OFF_SHORT",
	"POWER_OFF_LONG",
	"SLEEP_NOTIFICATION"
};
char *context_conf[] = {
	"Context is closed and is no longer active",
	"Context is configured and activated as a write-only context and according to the rest of the "
	"bits in this configuration register",
	"Context is configured and activated as a read-only context and according to the rest of the "
	"bits in this configuration register",
	"Context is configured and activated as a read/write context and according to the rest of the"
	"bits in this configuration register",
	"Context is not following Large Unit rules",
	"Context follows Large Unit rules",
	"MODE0 (normal)",
	"MODE1 (non-Large Unit, reliable mode or Large Unit unit-by-unit mode)",
	"MODE2 (Large Unit, one-unit-tail mode)",
	"Reserved"
};
char *ext_partition_attribute[] = {
	"Default (no extended attribute)",
	"System code",
	"Non-persistent",
	"Reserved"
};
char *exception_events_status[] = {
	"URGENT_BKOPS",
	"DYNCAP_NEEDED",
	"SYSPOOL_EXHAUSTED",
	"PACKED_FAILURE",
	"EXTENDED_SECURITY_FAILURE",
	"Reserved"
};
char *exception_events_ctrl[] = {
	"Reserved",
	"DYNCAP_EVENT_EN",
	"SYSPOOL_EVENT_EN",
	"PACKED_EVENT_EN",
	"EXTENDED_SECURITY_EN",
	"Reserved"
};
char *periodic_weakup[] = {
	"infinity (no wakeups).",
	"months",
	"weeks",
	"days",
	"hours",
	"minutes",
	"Reserved",
	"Reserved"
};
char *partition_attribute[] = {
	"Default.",
	"Set Enhanced attribute in User Data Area",
	"Default.",
	"Set Enhanced attribute in General Purpose partition 1",
	"Default.",
	"Set Enhanced attribute in General Purpose partition 2",
	"Default.",
	"Set Enhanced attribute in General Purpose partition 3",
	"Default.",
	"Set Enhanced attribute in General Purpose partition 4",
	"Reserved"
};
char *partition_support[] = {
	"obsolete.",
	"Device supports partitioning features",
	"obsolete",
	"Device can have enhanced technological features in partitions and user data area",
	"n/a.",
	"Device can have extended partitions attribute",
	"Reserved"
};
char *hpi_mgmt[] = {
	"HPI mechanism not activated by the host (default)",
	"HPI mechanism activated by the host",
	"Reserved"
};
char *rst_n_function[] = {
	"RST_n signal is temporarily disabled (default)",
	"RST_n signal is permanently enabled",
	"RST_n signal is permanently disabled",
	"Reserved"
};
char *bkops_en[] = {
	"Host does not support background operations handling and is not expected to write to "
	"BKOPS_START field.",
	"Host is indicating that it shall periodically write to BKOPS_START field to manually start"
	"background operations.",
	"Device shall not perform background operations while not servicing the host.",
	"Device may perform background operations while not servicing the host.",
	"Reserved"
};
char *wr_rel_param[] = {
	"obsolete",
	"All the WR_DATA_REL parameters in the WR_REL_SET registers are R/W.",
	"obsolete",
	"The device supports the enhanced definition of reliable write",
	"RPMB transfer size is either 256B (single 512B frame) or 512B (two 512B frame).",
	"RPMB transfer size is either 256B (single 512B frame), 512B (two 512B frame), or 8KB (Thirty two 512B frames).",
	"Reserved"
};
char *wr_rel_set[] = {
	"In the main user area, write operations have been optimized for performance and existing"
	"data could be at risk if a power failure occurs.",
	"In the main user area, the device protects previously written data if power failure occurs.",
	"In general purpose partition 1, the write operation has been optimized for performance and"
	"existing data in the partition could be at risk if a power failure occurs.",
	"In general purpose partition 1, the device protects previously written data if power failure occurs",
	"In general purpose partition 2, the write operation has been optimized for performance and"
	"existing data in the partition could be at risk if a power failure occurs.",
	"In general purpose partition 2, the device protects previously written data if power failure occurs.",
	"In general purpose partition 3, the write operation has been optimized for performance and"
	"existing data in the partition could be at risk if a power failure occurs.",
	"In general purpose partition 3, the device protects previously written data if power failure occurs",
	"In general purpose partition 4, the write operation has been optimized for performance and"
	"existing data in the partition could be at risk if a power failure occurs",
	"In general purpose partition 4, the device protects previously written data if power failure occurs",
	"Reserved Bit"
};
char *user_wp_status[] = {
	"Power-on write protection is not applied when CMD28 is issued",
	"Apply Power-On Period protection to the protection group indicated by CMD28.",
	"Permanent write protection is not applied when CMD28 is issued.",
	"Apply permanent write protection to the protection group indicated by CMD28.",
	"Power-on write protection can be applied to write protection groups.",
	"Disable the use of power-on period write protection for write protection groups",
	"Permanent write protection can be applied to write protection groups.",
	"Permanently disable the use of permanent write protection for write protection groups",
	"Host is permitted to set PERM_WRITE_PROTECT (CSD[13]).",
	"Disable the use of PERM_WRITE_PROTECT (CSD[13]).",
	"Password protection features are enabled.",
	"Password protection features (ERASE (Forcing erase), LOCK, UNLOCK, CLR_PWD,SET_PWD) are disabled permanently."
};
char *boot_wp_status[] = {
	"Boot Area 1 is not protected",
	"Boot Area 1 is Power on protected",
	"Boot Area 1 is Permanently Protected",
	"Reserved",
	"Boot Area 2 is not protected",
	"Boot Area 2 is Power on protected",
	"Boot Area 2 is Permanently Protected",
	"Reserved"
	"Reserved"
};
char *boot_bus_conditions[] = {
	"x1 (sdr) or x4 (ddr) bus width in boot operation mode (default)",
	"x4 (sdr/ddr) bus width in boot operation mode",
	"x8 (sdr/ddr) bus width in boot operation mode",
	"Reserved",
	"Reset bus width to x1, single data rate and backward compatible timings after boot operation (default)",
	"Retain BOOT_BUS_WIDTH and BOOT_MODE values after boot operation.",
	"Use single data rate + backward compatible timings in boot operation (default)",
	"Use single data rate + High Speed timings in boot operation mode",
	"Use dual data rate in boot operation",
	"Reserved"
};
char *boot_config_prot[] = {
	"PWR_BOOT_CONFIG_PROT is not enabled (default)",
	"Disable the change of boot configuration register bits relating to boot mode operation",
	"Reserved",
	"PERM_BOOT_CONFIG_PROT is not enabled (default)",
	"Permanently disable the change of boot configuration register bits relating boot mode operation",
	"Reserved"
};
char *partition_config[] = {
	"No access to boot partition (default)",
	"R/W boot partition 1",
	"R/W boot partition 2",
	"R/W Replay Protected Memory Block (RPMB)",
	"Access to General Purpose partition 1",
	"Access to General Purpose partition 2",
	"Access to General Purpose partition 3",
	"Access to General Purpose partition 4",
	"Device not boot enabled (default)",
	"Boot partition 1 enabled for boot",
	"Boot partition 2 enabled for boot",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"User area enabled for boot",
	"No boot acknowledge sent (default)",
	"Boot acknowledge sent during boot operation Bit",
	"Reserved"
};

char *strobe_support[] = {
	"No support of Enhanced Strobe mode ",
	"device supports Enhanced Strobe mode "
};
char *hs_timing[] = {
	"Selecting backwards compatibility interface timing",
	"High Speed",
	"HS200",
	"HS400",
	"50 ohm.(x1).    Default Driver Type. Supports up to 200 MHz operation.",
	"33 ohm.(x1.5).  Supports up to 200 MHz operation.",
	"66 ohm.(x0.75). The weakest driver that supports up to 200 MHz operation.",
	"100 ohm.(x0.5).  For low noise and low EMI systems."
	"Maximal operating frequency is decided by Host design.",
	"40 ohm.(x1.2)."
};
char *cmd_set_rev[] = {
	"v4.0",
	"Reserved"
};
char *ext_csd_rev[] = {
	"4.0",
	"4.1",
	"4.2",
	"4.3",
	"4.41",
	"4.5",
	"5.0",
	"5.1"
};

char *s_cmd_set[] = {
	"Standard MMC",
	"Allocated by MMCA",
	"Allocated by MMCA",
	"Allocated by MMCA",
	"Allocated by MMCA",
	"Reserved"
};
char *hpi_features[] = {
	"Obsolete",
	"HPI mechanism supported  (default)",
	"HPI mechanism implementation based on CMD13",
	"HPI mechanism implementation based on CMD12 "
};

char *data_tag_support[] = {
	"System Data Tag mechanism not supported (default)",
	"System Data Tag mechanism supported"
};
char *bkops_support[] = {
	"Obsolete",
	"Background operations are supported"
};
char *csd_structure[] = {
	"CSD version No. 1.0  (Allocated by MMCA)",
	"CSD version No. 1.1  (Allocated by MMCA)",
	"CSD version No. 1.2 (Version 4.1-4.2 4.3-4.41-4.5-4.51-5.0-5.01-5.1)",
	"Reserved for future use"
};
char *device_type[] = {
	"HS eMMC @26MHz - at rated device voltage(s)",
	"HS eMMC @52MHz - at rated device voltage(s)",
	"HS Dual Data Rate eMMC @52MHz 1.8V or 3V I/O",
	"HS Dual Data Rate eMMC @52MHz 1.2V I/O",
	"HS200 Single Data Rate eMMC @200MHz 1.8V I/O",
	"HS200 Single Data Rate eMMC @200MHz 1.2V I/O",
	"HS400 Dual Data Rate e‧MMC at 200 MHz 1.8V I/O",
	"HS400 Dual Data Rate e‧MMC at 200 MHz 1.2V I/O"
};
char *driver_strength_type[] = {
	"50 ohm.(x1). Default Driver Type. Supports up to 200 MHz operation.",
	"33 ohm.(x1.5). Supports up to 200 MHz operation.",
	"66 ohm.(x0.75).The weakest driver that supports up to 200 MHz operation.",
	"100 ohm.(x0.5). For low noise and low EMI systems."
	"Maximal operating frequency is decided by Host design.",
	"40 ohm.(x1.2)."
};
char *boot_info[] = {
	"Device does not support alternative boot method (obsolete)",
	"Device supports alternative boot method.",
	"Device does not support dual data rate during boot.",
	"Device supports dual data rate during boot.",
	"Device does not support high speed timing during boot.",
	"Device supports high speed timing during boot."
};
char *sec_feature_support[] = {
	"Secure purge operations are not supported on the device",
	"Secure purge operations are supported.",
	"Device does not support the automatic erase operation"
	"on retired defective portions of the array.",
	"Device supports the automatic erase operation on retired"
	"defective portions of the array.",
	"Device does not support the secure and insecure trim operations",
	"Device supports the secure and insecure trim operations.",
	"Device does not support the sanitize operation.",
	"Device supports the sanitize operation."
};
char *min_sdr_perf[] = {
	"For Devices not reaching the 2.4MB/s value",
	"Class A: 2.4MB/s and is the next allowed value (16x150kB/s)",
	"Class B: 3.0MB/s and is the next allowed value (20x150kB/s)",
	"Class C: 4.5MB/s and is the next allowed value (30x150kB/s)",
	"Class D: 6.0MB/s and is the next allowed value (40x150kB/s)",
	"Class E: 9.0MB/s and is the next allowed value (60x150kB/s)",
	"Class E: 9.0MB/s and is the next allowed value (60x150kB/s)",
	"Class F: Equals 12.0MB/s and is the next allowed value (80x150kB/s)",
	"Class G: Equals 15.0MB/s and is the next allowed value (100x150kB/s)",
	"Class H: Equals 18.0MB/s and is the next allowed value (120x150kB/s)",
	"Class J: Equals 21.0MB/s and is the next allowed value (140x150kB/s) ",
	"Class K: Equals 24.0MB/s and is the next allowed value (160x150kB/s)",
	"Class M: Equals 30.0MB/s and is the next allowed value (200x150kB/s)",
	"Class O: Equals 36.0MB/s and is the next allowed value (240x150kB/s)",
	"Class R: Equals 42.0MB/s and is the next allowed value (280x150kB/s)",
	"Class T: Equals 48.0MB/s and is the last defined value (320x150kB/s)"
};

char *min_ddr_perf[] = {
	"For Devices not reaching the 4.8MB/s value",
	"Class A: Equals 4.8MB/s and is the next allowed value (16x300kB/s)",
	"Class B: Equals 6.0MB/s and is the next allowed value (20x300kB/s)",
	"Class C: Equals 9.0MB/s and is the next allowed value (30x300kB/s)",
	"Class D: Equals 12.0MB/s and is the next allowed value (40x300kB/s)",
	"Class E: Equals 18.0MB/s and is the last defined value (60x300kB/s)",
	"Class F: Equals 24.0MB/s and is the next allowed value (80x300kB/s)",
	"Class G: Equals 30.0MB/s and is the next allowed value (100x300kB/s)",
	"Class H: Equals 36.0MB/s and is the next allowed value (120x300kB/s)",
	"Class J: Equals 42.0MB/s and is the last defined value (140x300kB/s)",
	"Class K: Equals 48.0MB/s and is the next allowed value (160x300kB/s)",
	"Class M: Equals 60.0MB/s and is the next allowed value (200x300kB/s)",
	"Class O: Equals 72.0MB/s and is the next allowed value (24 0x300kB/s)",
	"Class R: Equals 84.0MB/s and is the next allowed value (280x300kB/s)",
	"Class T: Equals 96.0MB/s and is the last defined value (320x300kB/s)"
};
char *pwr_cl_ddr52_200_360[] = {
	"100mA : 200mA",
	"120mA : 220mA",
	"150mA : 250mA",
	"180mA : 280mA",
	"200mA : 300mA",
	"220mA : 320mA",
	"250mA : 350mA",
	"300mA : 400mA",
	"350mA : 450mA",
	"400mA : 500mA",
	"450mA : 550mA",
	"500mA : 600mA",
	"600mA : 700mA",
	"700mA : 800mA",
	"800mA : 900mA",
	">800mA : >900mA"
};

char *pwr_cl_ddr52_200_195[] = {
	"65mA : 130mA",
	"70mA : 140mA",
	"80mA : 160mA",
	"90mA : 180mA",
	"100mA : 200mA",
	"120mA : 220mA",
	"140mA : 240mA",
	"160mA : 260mA",
	"180mA : 280mA",
	"200mA : 300mA",
	"250mA : 350mA",
	"300mA : 400mA",
	"350mA : 450mA",
	"400mA : 500mA",
	"500mA : 600mA",
	">500mA : >600mA"
};

char *bkops_status[] = {
	"No operations required",
	"Operations outstanding (non critical)",
	"Operations outstanding (performance being impacted)",
	"Operations outstanding (critical)",
};
char *pre_eol_info[] = {
	"Not Defined",
	"Normal",
	"Warning",
	"Urgent",
	"Reserved"
};
char *life_time_typeAorB[] = {
	"Not defined ",
	"0% - 10% device life time used",
	"10% - 20% device life time used ",
	"20% - 30% device life time used",
	"30% - 40% device life time used",
	"40% - 50% device life time used",
	"50% - 60% device life time used",
	"60% - 70% device life time used",
	"70% - 80% device life time used",
	"80% - 90% device life time used",
	"90% - 100% device life time used",
	"Exceeded its maximum estimated device life time",
	"Reserved"
};

typedef struct _mmc_ext_info {
	char **ext_info;
	u32 ext_idx;
}mmc_ext_info_t;

mmc_ext_info_t mmc_ext_cxt[] = {
	{secure_remova_type, 16},
	{mode_config, 30},
	{power_off_notification, 34},
	{context_conf, 51},
	{ext_partition_attribute, 52},
	{ext_partition_attribute, 53},
	{exception_events_status, 54},
	{exception_events_ctrl, 56},
	{periodic_weakup, 131},
	{partition_attribute, 156},
	{partition_support, 160},
	{hpi_mgmt, 161},
	{rst_n_function, 162},
	{bkops_en, 163},
	{wr_rel_param, 166},
	{wr_rel_set, 167},
	{user_wp_status, 171},
	{boot_wp_status, 174},
	{boot_bus_conditions, 177},
	{boot_config_prot, 178},
	{partition_config, 179},
	{strobe_support, 184},
	{hs_timing, 185},
	{pwr_cl_ddr52_200_195, 187},
	{cmd_set_rev, 189},
	{ext_csd_rev, 192},
	{csd_structure, 194},
	{device_type, 196},
	{driver_strength_type, 197},
	{pwr_cl_ddr52_200_195, 200},
	{pwr_cl_ddr52_200_195, 201},
	{pwr_cl_ddr52_200_360, 202},
	{pwr_cl_ddr52_200_360, 203},
	{min_ddr_perf, 205},
	{min_ddr_perf, 206},
	{min_ddr_perf, 207},
	{min_ddr_perf, 208},
	{min_ddr_perf, 209},
	{min_ddr_perf, 210},
	{boot_info, 228},
	{sec_feature_support, 231},
	{min_ddr_perf,234 },
	{min_ddr_perf,235 },
	{pwr_cl_ddr52_200_195, 236},
	{pwr_cl_ddr52_200_360, 237},
	{pwr_cl_ddr52_200_195, 238},
	{pwr_cl_ddr52_200_360, 239},
	{bkops_status, 246},
	{pre_eol_info, 267},
	{life_time_typeAorB, 268},
	{life_time_typeAorB, 269},
	{data_tag_support, 499},
	{bkops_support, 502},
	{hpi_features, 503},
	{s_cmd_set, 504}
};
char *mmc_ext_cmd[] = {
	"mmc ext all - display all extcsd info of the current eMMC device",
	"mmc ext life - display life time of the current eMMC device",
	"mmc ext boot - display the boot configuration of the current eMMC device",
	"mmc ext wr - display the write reliability parameter of the current eMMC device",
};

#endif
