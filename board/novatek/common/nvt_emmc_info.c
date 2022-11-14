#include <common.h>
#include <asm-generic/errno.h>
#include <mmc.h>
#include <nvt_emmc_info.h>
#include <asm/arch/hardware.h>
extern u8 saved_ext_csd[MMC_MAX_BLOCK_LEN];

char* get_mmc_info(u32 ext_idx, u8 ext_value)
{
	int i;
	//		printf("ext_idx =%d ext_value = %d\n",ext_idx ,ext_value );

	for (i = 0 ; i <sizeof( mmc_ext_cxt)/sizeof(mmc_ext_cxt[0]) ; i++) {

		if(mmc_ext_cxt[i].ext_idx == ext_idx)
			return mmc_ext_cxt[i].ext_info[ext_value];
	}
			return NULL;
}

char* get_mmc_mim_perf(u32 ext_idx, u8 ext_value)
{
	int i, mmc_cxt_idx , mmc_ext_value_idx;
	//		printf("ext_idx =%d ext_value = %d\n",ext_idx ,ext_value );

	for (i = 0 ; i <sizeof( mmc_ext_cxt)/sizeof(mmc_ext_cxt[0]) ; i++) {

		if (mmc_ext_cxt[i].ext_idx == ext_idx) {
			mmc_cxt_idx= i;
			break;
		}
	}
	switch (ext_value)
	{
		case 0x00 :
			mmc_ext_value_idx = 0;
			break;
		case 0x08 :
			mmc_ext_value_idx = 1;
			break;
		case 0x0A :
			mmc_ext_value_idx = 2;
			break;
		case 0x0F :
			mmc_ext_value_idx = 3;
			break;
		case 0x14 :
			mmc_ext_value_idx = 4;
			break;
		case 0x1E :
			mmc_ext_value_idx = 5;
			break;
		case 0x28 :
			mmc_ext_value_idx = 6;
			break;
		case 0x32 :
			mmc_ext_value_idx = 7;
			break;
		case 0x3C :
			mmc_ext_value_idx = 8;
			break;
		case 0x46 :
			mmc_ext_value_idx = 9;
			break;
		case 0x50 :
			mmc_ext_value_idx = 10;
			break;
		case 0x64 :
			mmc_ext_value_idx = 11;
			break;
		case 0x78 :
			mmc_ext_value_idx = 12;
			break;
		case 0x8C :
			mmc_ext_value_idx = 13;
			break;
		case 0xA0 :
			mmc_ext_value_idx = 14;
			break;
		default:
				printf("Get ext_csd fail!\n");
				return NULL;
			break;
	}
			return mmc_ext_cxt[mmc_cxt_idx].ext_info[mmc_ext_value_idx];
}

void print_mmc_write_reliability(void)
{
	u8 ext_value = saved_ext_csd[EXT_CSD_WR_REL_SET] ;
	printf("=============================================\n");
	printf("[High-density erase group definition(175)]:\n");
	printf("%s\n", (saved_ext_csd[EXT_CSD_ERASE_GROUP_DEF])?"Enable":"Disable");
	printf("=============================================\n");
	printf("[User area write protection(167)]:\n");
	if (ext_value & 0x01)
		printf("%s\n", get_mmc_info(167, 1));
	else
		printf("%s\n", get_mmc_info(167, 0));

	if (ext_value & 0x02)
		printf("%s\n", get_mmc_info(167, 3));
	else
		printf("%s\n", get_mmc_info(167, 2));

	if (ext_value & 0x04)
		printf("%s\n", get_mmc_info(167, 5));
	else
		printf("%s\n", get_mmc_info(167, 4));

	if (ext_value & 0x08)
		printf("%s\n", get_mmc_info(167, 7));
	else
		printf("%s\n", get_mmc_info(167, 6));

	if (ext_value & 0x10)
		printf("%s\n", get_mmc_info(167, 9));
	else
		printf("%s\n", get_mmc_info(167, 8));

	printf("=============================================\n");
	ext_value = saved_ext_csd[EXT_CSD_WR_REL_PARAM];
	printf("[Write reliability parameter(166)]:\n");
	if (ext_value & 0x01)
		printf("%s\n", get_mmc_info(166, 1));
	else
		printf("%s\n", get_mmc_info(166, 0));

	if (ext_value & 0x04)
		printf("%s\n", get_mmc_info(166, 3));
	else
		printf("%s\n", get_mmc_info(166, 2));

	if (ext_value & 0x10)
		printf("%s\n", get_mmc_info(166, 5));
	else
		printf("%s\n", get_mmc_info(166, 4));
		
	printf("=============================================\n");
	printf("[Partitioning Setting(155)]:\n");
	if (saved_ext_csd[EXT_CSD_PARTITION_SETTING])
		printf("Device partition setting complete\n");
	else
		printf("Device partition setting NOT complete\n");


	printf("=============================================\n");


}

void print_mmc_partition_cfg(void)
{
	u8 ext_value = saved_ext_csd[EXT_CSD_PART_CONF] ;
	printf("=============================================\n");
	printf("Partition Config:\n");
	//			printf("ext_idx =%d ext_value = %d\n",ext_idx ,ext_value );
	printf("%s\n", get_mmc_info(EXT_CSD_PART_CONF, ((ext_value & EXT_CSD_BOOT_CFG_EN)>>3) + 8));
	printf("%s\n", get_mmc_info(EXT_CSD_PART_CONF, (ext_value & EXT_CSD_BOOT_CFG_ACC)));
	printf("%s\n", get_mmc_info(EXT_CSD_PART_CONF, ((ext_value & EXT_CSD_BOOT_CFG_ACK)>>6) + 16));
	printf("=============================================\n");

}
void print_mmc_life(void)
{
	u8 ext_eol = saved_ext_csd[EXT_CSD_PRE_EOL_INFO];
	u8 life_timeA = saved_ext_csd[EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_A];
	u8 life_timeB = saved_ext_csd[EXT_CSD_DEVICE_LIFE_TIME_EST_TYP_B];
	printf("=============================================\n");

	printf("Pre EOL info = %s\n", (ext_eol > 0x4)?pre_eol_info[0x4] : pre_eol_info[ext_eol]);
	printf("Life time typeA = %s\n", (life_timeA > 0x4)?life_time_typeAorB[0xC] : life_time_typeAorB[life_timeA]);
	printf("Life time typeB = %s\n", (life_timeB > 0x4)?life_time_typeAorB[0xC] : life_time_typeAorB[life_timeB]);
	printf("=============================================\n");

}

void print_mmc_ext_csd(void)
{
	int i;
	char *ext_info;
	u8 ext_value = 0, ext_csd_rev = 0;
	ext_csd_rev = saved_ext_csd[EXT_CSD_REV] ;
	u32 value = 0;

	ext_info = get_mmc_info(EXT_CSD_REV, ext_csd_rev);
	printf("=============================================\n");
	printf("  Extended CSD rev 1.%d (MMC %s)\n", ext_csd_rev, ext_info);
	printf("=============================================\n\n");
	if (ext_csd_rev < 3) {
		printf("No ext_csd\n");
		return;
	}

	/* Parse the Extended CSD registers.
	 * Reserved bit should be read as "0" in case of spec older
	 * than A441.
	 */
	ext_info = get_mmc_info(EXT_CSD_S_CMD_SET, saved_ext_csd[EXT_CSD_S_CMD_SET]);
	printf("=============================================\n");
	printf("[S_CMD_SET]:\n");
	printf("%s\n", ext_info);
	printf("=============================================\n");


	if (ext_value & EXT_CSD_HPI_SUPP) {
		printf("=============================================\n");
		printf("[EXT_CSD_HPI_FEATURE] =%s\n",get_mmc_info(EXT_CSD_HPI_FEATURE,
					saved_ext_csd[EXT_CSD_HPI_FEATURE] & EXT_CSD_HPI_SUPP));
		printf("%s\n",get_mmc_info(EXT_CSD_HPI_FEATURE,
					(saved_ext_csd[EXT_CSD_HPI_FEATURE] & EXT_CSD_HPI_SUPP) & EXT_CSD_HPI_IMPL)+2);
		printf("=============================================\n");
	}


	printf("=============================================\n");
	printf("[BKOPS_SUPPORT]=%s\n", get_mmc_info(EXT_CSD_BKOPS_SUPPORT,
				saved_ext_csd[EXT_CSD_BKOPS_SUPPORT]));

	printf("=============================================\n");

	if (ext_csd_rev >= 6) {
		printf("=============================================\n");
		printf("[Max Packet Read Cmd]:\n");
		printf("0x%02x\n",saved_ext_csd[501]);
		printf("=============================================\n");

		printf("[Max Packet Write Cmd [MAX_PACKED_WRITES]:\n");
		printf("0x%02x\n",saved_ext_csd[500]);

		printf("=============================================\n");
		printf("[DATA_TAG_SUPPORT]:\n");
		printf("%s\n", get_mmc_info(499, saved_ext_csd[499]));

		printf("=============================================\n");
		printf("[Data TAG Unit Size]:\n");
		printf("0x%02x\n", saved_ext_csd[498]);

		printf("=============================================\n");
		printf("[Tag Resources Size]:\n");
		printf("0x%02x\n",saved_ext_csd[497]);

		printf("=============================================\n");
		printf("[Context Management Capabilities]:\n");
		printf("0x%02x\n", saved_ext_csd[496]);

		printf("=============================================\n");
		printf("[Large Unit Size]:\n");
		printf("0x%02x]\n", saved_ext_csd[495]);

		printf("=============================================\n");
		printf("[Extended partition attribute support]:\n");
		printf("0x%02x\n", saved_ext_csd[494]);

		printf("=============================================\n");
		printf("[Generic CMD6 Timer]:\n");
		printf("0x%02x\n", saved_ext_csd[248]);

		printf("=============================================\n");
		printf("[Power off notification]:\n");
		printf("0x%02x\n", saved_ext_csd[247]);
		printf("=============================================\n");
		printf("[Cache Size]:\n");
		printf("%d KiB\n",saved_ext_csd[249] << 0 | (saved_ext_csd[250] << 8) |
				(saved_ext_csd[251] << 16) | (saved_ext_csd[252] << 24));
		printf("=============================================\n");
	}

	/* A441: Reserved [501:247]
A43: reserved [246:229] */
	if (ext_csd_rev >= 5) {
		printf("=============================================\n");
		printf("[BKOPS_STATUS]:\n");
		printf("%s\n", get_mmc_info(246, saved_ext_csd[246]));
		printf("=============================================\n");

		/* CORRECTLY_PRG_SECTORS_NUM [245:242] TODO */
		printf("=============================================\n");
		printf("[CORRECTLY_PRG_SECTORS_NUM_0]:\n");
		printf("0x%02x\n", saved_ext_csd[242]);
		printf("[CORRECTLY_PRG_SECTORS_NUM_1]:\n");
		printf("0x%02x\n", saved_ext_csd[243]);
		printf("[CORRECTLY_PRG_SECTORS_NUM_2]:\n");
		printf("0x%02x\n", saved_ext_csd[244]);
		printf("[CORRECTLY_PRG_SECTORS_NUM_3]:\n");
		printf("0x%02x\n", saved_ext_csd[245]);
		printf("=============================================\n");

		printf("[1st Initialisation Time after programmed sector]:\n");
		printf("0x%02x\n", saved_ext_csd[241]);
		printf("=============================================\n");

		/* A441: reserved [237-236] */

		/* A441: reserved [240] */
		printf("[Power class for 52MHz, DDR at 3.6V (4 bit bus)]:\n");
		printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(239,
					saved_ext_csd[239]&0xf));
		printf("[Power class for 52MHz, DDR at 3.6V (8 bit bus)]:\n");
		printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(239,
					(saved_ext_csd[239]>>4)&0xf));
		printf("[Power class for 52MHz, DDR at 1.95V (4 bit bus)]:\n");
		printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(238,
					saved_ext_csd[238]&0xf));
		printf("[Power class for 52MHz, DDR at 1.95V (8 bit bus)]:\n");
		printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(238,
					(saved_ext_csd[238]>>4)&0xf));

		printf("=============================================\n");
		if (ext_csd_rev >= 6) {
			printf("[Power class for 200MHz at 3.6V (4 bit bus)]:\n");
			printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(237,
						saved_ext_csd[237]&0xf));
			printf("[Power class for 200MHz at 3.6V (8 bit bus)]:\n");
			printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(237,
						(saved_ext_csd[237]>>4)&0xf));
			printf("[Power class for 200MHz, at 1.95V (4 bit bus)]:\n");
			printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(236,
						saved_ext_csd[236]&0xf));
			printf("[Power class for 200MHz, at 1.95V (8 bit bus)]:\n");
			printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(236,
						(saved_ext_csd[236]>>4)&0xf));
			printf("=============================================\n");
		}


		printf("[Minimum Performance for 8bit at 52MHz in DDR mode]:\n");
		printf("[MIN_PERF_DDR_W_8_52]=%s\n", get_mmc_mim_perf(235,
					saved_ext_csd[235]));
		printf("[MIN_PERF_DDR_R_8_52]=%s\n", get_mmc_mim_perf(234,
					saved_ext_csd[234]));
		printf("=============================================\n");
		/* A441: reserved [233] */
		printf("[TRIM Multiplier]:\n");
		printf("0x%02x\n", saved_ext_csd[232]);
		printf("=============================================\n");
		printf("[SEC_FEATURE_SUPPORT]:\n");
		if (saved_ext_csd[23l]&0x1) {
			printf("%s\n", get_mmc_info(231,1));
			if (saved_ext_csd[23l]&0x4) 
				printf("%s\n", get_mmc_info(231,3));
			else
				printf("%s\n", get_mmc_info(231,2));

			if (saved_ext_csd[23l]&0x10) 
				printf("%s\n", get_mmc_info(231,5));
			else
				printf("%s\n", get_mmc_info(231,4));

			if (saved_ext_csd[23l]&0x40) 
				printf("%s\n", get_mmc_info(231,7));
			else
				printf("%s\n", get_mmc_info(231,6));
		} else
			printf("%s\n", get_mmc_info(231,0));
		printf("=============================================\n");
	}
	if (ext_csd_rev == 5) { /* Obsolete in 4.5 */
		printf("[Secure Erase Multiplier]:\n");
		printf("0x%02x\n",saved_ext_csd[230]);
		printf("[Secure TRIM Multiplier]:\n");
		printf("0x%02x]\n",saved_ext_csd[229]);
		printf("=============================================\n");
	}
	ext_value = saved_ext_csd[EXT_CSD_BOOT_INFO];
	printf("[Boot Information]:\n");
	if (ext_value & EXT_CSD_BOOT_INFO_ALT)
		printf("%s\n", get_mmc_info(EXT_CSD_BOOT_INFO,1));
	else
		printf("%s\n", get_mmc_info(EXT_CSD_BOOT_INFO,0));
	if (ext_value & EXT_CSD_BOOT_INFO_DDR_DDR)
		printf("%s\n", get_mmc_info(EXT_CSD_BOOT_INFO,3));
	else
		printf("%s\n", get_mmc_info(EXT_CSD_BOOT_INFO,2));
	if (ext_value & EXT_CSD_BOOT_INFO_HS_MODE)
		printf("%s\n", get_mmc_info(EXT_CSD_BOOT_INFO,5));
	else
		printf("%s\n", get_mmc_info(EXT_CSD_BOOT_INFO,4));

	printf("=============================================\n");
	/* A441/A43: reserved [227] */
	printf("[Boot partition size]:\n");
	printf("0x%02x\n", saved_ext_csd[226]);
	printf("[Access size]:\n");
	printf("0x%02x\n", saved_ext_csd[225]);

	printf("=============================================\n");
	ext_value = saved_ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
	printf("[High-capacity erase unit size]:\n");
	printf("0x%02x\n", saved_ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]);
	printf(" i.e. %u KiB\n", 512 * ext_value);
	printf("=============================================\n");

	printf("[High-capacity erase timeout]:\n");
	printf("0x%02x]\n",saved_ext_csd[223]);
	printf("=============================================\n");
	printf("[Reliable write sector count]:\n");
	printf("0x%02x\n",saved_ext_csd[222]);

	printf("=============================================\n");
	ext_value = saved_ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
	printf("[High-capacity W protect group size]:\n");
	printf("0x%02x\n",ext_value);
	printf("i.e. %lu KiB\n", 512l * saved_ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * ext_value);

	printf("=============================================\n");
	printf("[Sleep current]:\n");
	printf("0x%02x\n", saved_ext_csd[220]);
	printf("[Sleep current]:\n");
	printf("0x%02x\n", saved_ext_csd[219]);
	/* A441/A43: reserved [218] */
	printf("[Sleep/awake timeout]:\n");
	printf("0x%02x\n", saved_ext_csd[217]);
	/* A441/A43: reserved [216] */

	printf("=============================================\n");
	unsigned int sectors =
		(saved_ext_csd[EXT_CSD_SEC_COUNT_3] << 24) | (saved_ext_csd[EXT_CSD_SEC_COUNT_2] << 16) |
		(saved_ext_csd[EXT_CSD_SEC_COUNT_2] << 8) | saved_ext_csd[EXT_CSD_SEC_COUNT_2];
	printf("[Sector Count]:\n");
	printf("0x%08x\n", sectors);

	/* over 2GiB devices are block-addressed */
	if (sectors > (2u * 1024 * 1024 * 1024) / 512)
		printf(" Device is block-addressed\n");
	else
		printf(" Device is NOT block-addressed\n");

	printf("=============================================\n");
	/* A441/A43: reserved [211] */
	printf("[Minimum Write Performance for 8bit]:\n");
	printf("[MIN_PERF_W_8_52]= %s\n", get_mmc_mim_perf(210,
				saved_ext_csd[210]));
	printf("[MIN_PERF_R_8_52]= %s\n", get_mmc_mim_perf(209,
				saved_ext_csd[209]));
	printf("[MIN_PERF_W_8_26_4_52]= %s\n", get_mmc_mim_perf(208,
				saved_ext_csd[208]));
	printf("[MIN_PERF_R_8_26_4_52]= %s\n", get_mmc_mim_perf(207,
				saved_ext_csd[207]));
	printf("Minimum Write Performance for 4bit:\n");
	printf("[MIN_PERF_W_4_26]= %s\n", get_mmc_mim_perf(206,
				saved_ext_csd[206]));
	printf("[MIN_PERF_R_4_26]= %s\n", get_mmc_mim_perf(205,
				saved_ext_csd[205]));
	printf("=============================================\n");
	/* A441/A43: reserved [204] */
	printf("[Power classes registers]:\n");
	printf(" [PWR_CL_26_360: 0x%02x]\n", saved_ext_csd[203]);
	printf("Power class for 26MHz at 3.6V (4 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(203,
				saved_ext_csd[203]&0xf));
	printf("Power class for 26MHz at 3.6V (8 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(203,
				(saved_ext_csd[203]>>4)&0xf));
	printf("Power class for 52MHz at 3.6V (4 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]=%s\n", get_mmc_info(202,
				saved_ext_csd[202]&0xf));
	printf("Power class for 52MHz at 3.6V (8 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]= %s\n", get_mmc_info(202,
				(saved_ext_csd[202]>>4)&0xf));
	printf("Power class for 26MHz, at 1.95V (4 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]= %s\n", get_mmc_info(201,
				saved_ext_csd[201]&0xf));
	printf("Power class for 26MHz, at 1.95V (8 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]= %s\n", get_mmc_info(201,
				(saved_ext_csd[201]>>4)&0xf));
	printf("Power class for 52MHz, at 1.95V (4 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]= %s\n", get_mmc_info(200,
				saved_ext_csd[200]&0xf));
	printf("Power class for 52MHz, at 1.95V (8 bit bus)\n");
	printf("[Max RMS Current:Max Peak Current]= %s\n", get_mmc_info(200,
				(saved_ext_csd[200]>>4)&0xf));

	printf("=============================================\n");
	/* A43: reserved [199:198] */
	if (ext_csd_rev >= 5) {
		printf("[Partition switching timing]:\n");
		printf("%d:ms\n", saved_ext_csd[199]*10);
		printf("[Out-of-interrupt busy timing]:\n");
		printf("%d:ms\n", saved_ext_csd[198]*10);
		printf("=============================================\n");
	}

	/* A441/A43: reserved	[197] [195] [193] [190] [188]
	 * [186] [184] [182] [180] [176] */

	if (ext_csd_rev >= 6) {
		printf("[I/O Driver Strength]:\n");
		printf("%s\n", get_mmc_info(197, saved_ext_csd[197]));
		printf("=============================================\n");
	}

	/* DEVICE_TYPE in A45, CARD_TYPE in A441 */
	ext_value = saved_ext_csd[196];
	printf("[Card Type]:\n");
	if (ext_value & 0x01) printf("%s\n", get_mmc_info(196, 0));
	if (ext_value & 0x02) printf("%s\n", get_mmc_info(196, 1));
	if (ext_value & 0x04) printf("%s\n", get_mmc_info(196, 2));
	if (ext_value & 0x08) printf("%s\n", get_mmc_info(196, 3));
	if (ext_value & 0x10) printf("%s\n", get_mmc_info(196, 4));
	if (ext_value & 0x20) printf("%s\n", get_mmc_info(196, 5));
	if (ext_value & 0x40) printf("%s\n", get_mmc_info(196, 6));
	if (ext_value & 0x80) printf("%s\n", get_mmc_info(196, 7));

	printf("=============================================\n");
	printf("[CSD structure version]:\n");
	printf("%s\n", get_mmc_info(194, saved_ext_csd[194]));
	/* ext_csd_rev = ext_csd[EXT_CSD_REV] (already done!!!) */
	printf("=============================================\n");
	printf("[Command set]:\n");
	printf("0x%02x\n", saved_ext_csd[191]);
	printf("=============================================\n");
	printf("[Command set revision]:\n");
	printf("%s\n", get_mmc_info(189, saved_ext_csd[189]));
	printf("=============================================\n");
	printf("[Power class]:\n");
	printf("[Max RMS Current:Max Peak Current]= %s\n", get_mmc_info(187,
				saved_ext_csd[187]&0xf));

	printf("=============================================\n");
	printf("[High-speed interface timing]:\n");
	ext_value = saved_ext_csd[185];
	printf("%s\n", get_mmc_info(185, ext_value & 0xF));
	printf("%s\n", get_mmc_info(185, ((ext_value >>4) & 0xF)+4));

	printf("=============================================\n");
	printf("[Strobe Support]:\n");
	printf("%s\n", get_mmc_info(184,saved_ext_csd[184]));
	/* bus_width: ext_csd[183] not readable */

	printf("=============================================\n");
	printf("[Erased memory content]:\n");
	printf("0x%02x\n", saved_ext_csd[181]);

	print_mmc_partition_cfg();

	ext_value = saved_ext_csd[178];
	printf("[Boot config protection]:\n");
	if (ext_value & 0x01)
		printf("%s\n", get_mmc_info(178, 1));
	else
		printf("%s\n", get_mmc_info(178, 0));
	if (ext_value & 0x0E) printf("%s\n", get_mmc_info(178, 2));

	if (ext_value & 0x10)
		printf("%s\n", get_mmc_info(178, 4));
	else
		printf("%s\n", get_mmc_info(178, 3));
	if (ext_value & 0xE0) printf("%s\n", get_mmc_info(178, 5));

	printf("=============================================\n");
	printf("[Boot bus Conditions]:\n");
	printf("%s\n", get_mmc_info(177, saved_ext_csd[EXT_CSD_BOOT_BUS_WIDTH] & 0x3));
	printf("%s\n", get_mmc_info(177, ((saved_ext_csd[EXT_CSD_BOOT_BUS_WIDTH] >> 2 ) & 0x1)+4));
	printf("%s\n", get_mmc_info(177, ((saved_ext_csd[EXT_CSD_BOOT_BUS_WIDTH] >> 3) & 0x3)+6));
	if (saved_ext_csd[177] & 0xE) printf("%s\n", get_mmc_info(177, 9));

	printf("=============================================\n");
	printf("[High-density erase group definition]:\n");
	printf("%s\n", (saved_ext_csd[EXT_CSD_ERASE_GROUP_DEF])?"Enable":"Disable");
	printf("=============================================\n");
	printf("[Boot write protection status]:\n");
	printf("%s\n", get_mmc_info(174, saved_ext_csd[EXT_CSD_BOOT_WP_STATUS] & 0x3));
	printf("%s\n", get_mmc_info(174, ((saved_ext_csd[EXT_CSD_BOOT_WP_STATUS] >> 2) & 0x3) +4));
	if (saved_ext_csd[174] & 0xF8) printf("%s\n", get_mmc_info(174, 8));

	if (ext_csd_rev >= 5) {
		printf("=============================================\n");
		/* A441]: reserved [172] */
		ext_value = saved_ext_csd[171];
		printf("[User area write protection]:");
		if (ext_value & 0x01)
			printf("%s\n", get_mmc_info(171, 1));
		else
			printf("%s\n", get_mmc_info(171, 0));

		if (ext_value & 0x04)
			printf("%s\n", get_mmc_info(171, 3));
		else
			printf("%s\n", get_mmc_info(171, 2));

		if (ext_value & 0x08)
			printf("%s\n", get_mmc_info(171, 5));
		else
			printf("%s\n", get_mmc_info(171, 4));

		if (ext_value & 0x10)
			printf("%s\n", get_mmc_info(171, 7));
		else
			printf("%s\n", get_mmc_info(171, 6));

		if (ext_value & 0x40)
			printf("%s\n", get_mmc_info(171, 9));
		else
			printf("%s\n", get_mmc_info(171, 8));

		if (ext_value & 0x80)
			printf("%s\n", get_mmc_info(171, 11));
		else
			printf("%s\n", get_mmc_info(171, 10));

		/* A441]: reserved [170] */
		printf("=============================================\n");
		printf("[FW configuration]:\n");
		printf("%s\n", (saved_ext_csd[169]&0x1)?"FW updates enabled":"FW update disabled permanently");
		printf("=============================================\n");
		printf("[RPMB Size]:\n");
		if(saved_ext_csd[168])
			printf("%d x 128Kbytes\n", (saved_ext_csd[168]));
		else
			printf("No RPMB partiiton available\n");

		print_mmc_write_reliability();
		/* sanitize_start ext_csd[165]]: not readable
		 * bkops_start ext_csd[164]]: only writable */
		ext_value = saved_ext_csd[163];
		printf("[Enable background operations handshake]:\n");
		if (ext_value & 0x01)
			printf("%s\n", get_mmc_info(163, 1));
		else
			printf("%s\n", get_mmc_info(163, 0));
		if (ext_value & 0x02)
			printf("%s\n", get_mmc_info(163, 3));
		else
			printf("%s\n", get_mmc_info(163, 2));

		printf("=============================================\n");
		printf("[H/W reset function]:\n");
		printf("%s\n", get_mmc_info(162, saved_ext_csd[162]&0x3));

		printf("=============================================\n");
		printf("HPI management]:\n");
		printf("%s\n", get_mmc_info(161, saved_ext_csd[161]&0x1));


		printf("=============================================\n");
		ext_value = saved_ext_csd[160];
		printf("[Partitioning Support]:\n");
		if (ext_value & 0x01)
			printf("%s\n", get_mmc_info(160, 1));
		else
			printf("%s\n", get_mmc_info(160, 0));

		if (ext_value & 0x02)
			printf("%s\n", get_mmc_info(160, 3));
		else
			printf("%s\n", get_mmc_info(160, 2));

		if (ext_value & 0x04)
			printf("%s\n", get_mmc_info(160, 5));
		else
			printf("%s\n", get_mmc_info(160, 4));

		printf("=============================================\n");
		ext_value = (saved_ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_2] << 16) |
			(saved_ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_1] << 8) |
			saved_ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT];

		printf("[Max Enhanced Area Size]:\n");
		printf(" i.e. %lu KiB\n", 512l * ext_value * saved_ext_csd[221] * saved_ext_csd[224]);

		printf("=============================================\n");
		printf("[Partitions attribute]:\n");
		if (ext_value & 0x01)
			printf("%s\n", get_mmc_info(156, 1));
		else
			printf("%s\n", get_mmc_info(156, 0));

		if (ext_value & 0x02)
			printf("%s\n", get_mmc_info(156, 3));
		else
			printf("%s\n", get_mmc_info(156, 2));

		if (ext_value & 0x04)
			printf("%s\n", get_mmc_info(156, 5));
		else
			printf("%s\n", get_mmc_info(156, 4));

		if (ext_value & 0x08)
			printf("%s\n", get_mmc_info(156, 7));
		else
			printf("%s\n", get_mmc_info(156, 6));

		if (ext_value & 0x10)
			printf("%s\n", get_mmc_info(156, 9));
		else
			printf("%s\n", get_mmc_info(156, 8));

		printf("[Partitioning Setting]:\n");
		if (saved_ext_csd[EXT_CSD_PARTITION_SETTING])
			printf("Device partition setting complete\n");
		else
			printf("Device partition setting NOT complete\n");

		printf("=============================================\n");
		printf("[General Purpose Partition Size]:\n");
		printf("[GP_SIZE_MULT_4]: 0x%06x\n", (saved_ext_csd[154] << 16) |
				(saved_ext_csd[153] << 8) | saved_ext_csd[152]);
		printf("[GP_SIZE_MULT_3]: 0x%06x\n", (saved_ext_csd[151] << 16) |
				(saved_ext_csd[150] << 8) | saved_ext_csd[149]);
		printf("[GP_SIZE_MULT_2]: 0x%06x\n", (saved_ext_csd[148] << 16) |
				(saved_ext_csd[147] << 8) | saved_ext_csd[146]);
		printf("[GP_SIZE_MULT_1]: 0x%06x\n", (saved_ext_csd[145] << 16) |
				(saved_ext_csd[144] << 8) | saved_ext_csd[143]);

		value =	(saved_ext_csd[EXT_CSD_ENH_SIZE_MULT_2] << 16) |
			(saved_ext_csd[EXT_CSD_ENH_SIZE_MULT_1] << 8) |
			saved_ext_csd[EXT_CSD_ENH_SIZE_MULT];
		printf("=============================================\n");
		printf("[Enhanced User Data Area Size]:\n");
		printf("[ENH_SIZE_MULT]: 0x%06x\n", value);
		printf("i.e. %lu KiB\n", 512l * value *saved_ext_csd[221] * saved_ext_csd[224]);

		value =	(saved_ext_csd[EXT_CSD_ENH_START_ADDR_3] << 24) |
			(saved_ext_csd[EXT_CSD_ENH_START_ADDR_2] << 16) |
			(saved_ext_csd[EXT_CSD_ENH_START_ADDR_1] << 8) |
			saved_ext_csd[EXT_CSD_ENH_START_ADDR];
		printf("=============================================\n");
		printf("[Enhanced User Data Start Address]:\n");
		printf(" i.e. %lu bytes offset\n", ((sectors > (2u * 1024 * 1024 * 1024) / 512) ?
					512l : 1l) * value);

		printf("=============================================\n");
		/* A441]: reserved [135] */
		printf("[Bad Block Management mode]:\n");
		if (saved_ext_csd[134])
			printf("All data must be erased from defective "
					"memory array regions before they are retired from use.\n");
		else
			printf("(Default) Feature disabled \n");
		/* A441: reserved [133:0] */
	}
	printf("=============================================\n");
	/* B45 */
	if (ext_csd_rev >= 6) {
		/* tcase_support ext_csd[132] not readable */
		printf("[Periodic Wake-up}:\n");
		printf("%d %s\n", saved_ext_csd[131]&0x1F, get_mmc_info(131, (saved_ext_csd[131]>>5)&0x7));
		printf("=============================================\n");
		printf("[Program CID/CSD in DDR mode support]:\n");
		if (saved_ext_csd[130])
			printf("CMD26 and CMD27 are considered legal in both single data rate and dual data rate mode.\n");
		else
			printf("(Default) CMD26 and CMD27 must be used in single data rate mode.\n");

		printf("=============================================\n");
		printf("[Vendor Specific Fields]:\n");
		for (i = 127; i >= 64; i--)
			printf("[VENDOR_SPECIFIC_FIELD[%d]]: 0x%02x\n",
					i, saved_ext_csd[i]);
		printf("=============================================\n");

		printf("[Native sector size]:\n");
		if (saved_ext_csd[63])
			printf("Native sector size is 4KB\n");
		else
			printf("Native sector size is 512B\n");

		printf("=============================================\n");
		printf("[Sector size emulation]:\n");
		if (saved_ext_csd[62])
			printf(":Device is using the larger than 512B native sector size\n");
		else
			printf("Device is emulating a 512B sector size or uses a native 512B sector size\n");

		printf("=============================================\n");
		printf("[Sector size]:\n");
		if (saved_ext_csd[61])
			printf("Data sector size is 4KB\n");
		else
			printf("Data sector size is 512B\n");

		printf("=============================================\n");
		printf("1st initialization after disabling sector size emulation\n");
		printf("[INI_TIMEOUT_EMU]:\n");
		printf("%d ms\n",saved_ext_csd[60]*100);

		printf("=============================================\n");
		printf("[Class 6 commands control]:\n");
		if (saved_ext_csd[59])
			printf("Dynamic Capacity\n");
		else
			printf("Write Protect (Default)\n");

		printf("=============================================\n");
		printf("[Number of addressed group to be Released]:\n");
		printf("%d\n",saved_ext_csd[58]);

		printf("=============================================\n");
		ext_value =	saved_ext_csd[56];
		printf("[Exception events control]:\n");
		if (ext_value & 0x02) printf("%s\n", get_mmc_info(56, 1));
		if (ext_value & 0x04) printf("%s\n", get_mmc_info(56, 2));
		if (ext_value & 0x08) printf("%s\n", get_mmc_info(56, 3));
		if (ext_value & 0x10) printf("%s\n", get_mmc_info(56, 4));

		printf("=============================================\n");
		ext_value =	saved_ext_csd[56];
		printf("[Exception events status]:\n");
		if (ext_value & 0x01) printf("%s\n", get_mmc_info(54, 0));
		if (ext_value & 0x02) printf("%s\n", get_mmc_info(54, 1));
		if (ext_value & 0x04) printf("%s\n", get_mmc_info(54, 2));
		if (ext_value & 0x08) printf("%s\n", get_mmc_info(54, 3));
		if (ext_value & 0x10) printf("%s\n", get_mmc_info(54, 4));


		printf("=============================================\n");
		printf("[Extended Partitions Attribute]:\n");
		printf("EXT1:%s\n",get_mmc_info(52,saved_ext_csd[52]&0x3));
		printf("EXT2:%s\n",get_mmc_info(52,(saved_ext_csd[52] >> 4)&0x3));
		printf("EXT3:%s\n",get_mmc_info(53,saved_ext_csd[53]&0x3));
		printf("EXT4:%s\n",get_mmc_info(53,(saved_ext_csd[53] >> 4)&0x3));
		printf("=============================================\n");

		printf("[Context configuration]:\n");
		for (i = 37; i <= 51; i++) {
			printf("=============================================\n");
			printf("[ID#%d]:\n",i - 36 ,get_mmc_info(51,saved_ext_csd[i]&0x3));
			printf("Activation and direction selection:\n%s\n", get_mmc_info(51,saved_ext_csd[i]&0x3));
			printf("Large Unit context:%s\n", get_mmc_info(51,saved_ext_csd[i]&0x4));
			printf("Large Unit multiplier:%d\n",(saved_ext_csd[i]>>3)&0x7);
			printf("Reliability mode:%s\n", get_mmc_info(51,(saved_ext_csd[i]>>6)&0x3));
			printf("=============================================\n");

		}

		printf("[Packed command status]:\n");
		if (saved_ext_csd[36] & 0x01) printf("Error\n");
		if (saved_ext_csd[36] & 0x02) printf("Indexed Error\n");

		printf("=============================================\n");
		printf("[Packed command failure index]:\n");
		printf("%d\n",saved_ext_csd[35]);

		printf("=============================================\n");
		printf("[Power Off Notification]:\n");
		printf("%s\n", get_mmc_info(34,saved_ext_csd[34]));

		printf("=============================================\n");
		printf("[Control to turn the Cache ON/OFF]:\n");
		printf("CACHE_EN:%s\n", (saved_ext_csd[33])?"ON":"OFF");
		/* flush_cache ext_csd[32] not readable */
		/*Reserved [31:0] */
	}
	printf("=============================================\n");
	printf("[Barrier feature control]:\n");
	printf("BARRIER_EN:%s\n", (saved_ext_csd[31])?"ON":"OFF");

	printf("=============================================\n");
	printf("[Mode config]:\n");
	printf("%s\n", get_mmc_info(30, saved_ext_csd[30]&0x3));

	printf("=============================================\n");
	printf("[Mode operation codes]:\n");
	if (saved_ext_csd[29] & 0x01) printf("FFU_INSTALL\n");
	if (saved_ext_csd[29] & 0x02) printf("FFU_ABORT\n");

	printf("=============================================\n");
	printf("[FFU status]:\n");
	if (!saved_ext_csd[26]) printf("Success\n");
	if (saved_ext_csd[26] == 0x10) printf("General error\n");
	if (saved_ext_csd[26] == 0x11) printf("Firmware install error\n");
	if (saved_ext_csd[26] == 0x12) printf("Error in downloading firmware\n");

	value =	(saved_ext_csd[25] << 24) |
		(saved_ext_csd[24] << 16) |
		(saved_ext_csd[23] << 8) | saved_ext_csd[22];
	printf("=============================================\n");
	printf("[Preload data size]:\n");
	printf("%d*%s\n", value,((saved_ext_csd[61])?"4KB":"512B"));

	value =	(saved_ext_csd[21] << 24) |
		(saved_ext_csd[20] << 16) |
		(saved_ext_csd[19] << 8) | saved_ext_csd[18];
	printf("=============================================\n");
	printf("[Max preload data size]:\n");
	printf("%d*%s\n", value,((saved_ext_csd[61])?"4KB":"512B"));

	printf("=============================================\n");
	printf("[Product state awareness enablement]:\n");
	ext_value =	saved_ext_csd[17];
	if (ext_value & 0x01) printf("Manual mode Supported\n");
	if (ext_value & 0x02) printf("Auto mode Supported\n");

	if (ext_value & 0x10)
		printf("Production State Awareness is enabled.\n");
	else
		printf("Production State Awareness is disabled\n");

	if (ext_value & 0x20)
		printf("Auto mode is enabled.\n");
	else
		printf("Manual mode is enabled.\n");

	printf("=============================================\n");
	printf("[Secure removal tyupe]:\n");
	ext_value =	saved_ext_csd[16];
	if (ext_value & 0x01)
		printf("information removed by an erase of the physical memory \n");
	if (ext_value & 0x02)
		printf("information removed by an overwriting the addressed locations with a character followed\n"
				"by an erase\n");
	if (ext_value & 0x04)
		printf("information removed by an overwriting the addressed locations with a character, its\n"
				"complement, then a random character\n");
	if (ext_value & 0x08)
		printf("information removed using a vendor defined.\n");
	printf("%s\n", get_mmc_info(16,(saved_ext_csd[16]>>3)&0x3));
	printf("=============================================\n");

	if (ext_csd_rev >= 7) {
		printf("[eMMC Firmware Version]:\n");
		printf("%d\n",saved_ext_csd[254]);
		print_mmc_life();
	}

	if (ext_csd_rev >= 8) {
		printf("[Command Queue Support]:\n");
		printf(" 0x%02x\n",saved_ext_csd[EXT_CSD_CMDQ_SUPPORT]);
		printf("=============================================\n");
		printf("[Command Queue Depth]:\n");
		printf("%u\n",(saved_ext_csd[EXT_CSD_CMDQ_DEPTH] & 0x1f) + 1);
		printf("=============================================\n");
		printf("[Command Enabled]:\n");
		printf("%s\n",(saved_ext_csd[EXT_CSD_CMDQ_MODE_EN])?"ON":"OFF");
		printf("=============================================\n");
	}
}
void print_mmc_ext_info_help(void)
{
	int i = 0;

	for (i= 0; i < sizeof(mmc_ext_cmd)/sizeof(mmc_ext_cmd[0]); i++) {
		printf("%s\n",mmc_ext_cmd[i]);

	}


}

