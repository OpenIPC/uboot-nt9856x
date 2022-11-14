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

static TZPC_NAME tz_pc_NA51055[] = {
	{CPU_TZPC_SCE_CTRL,         "   SCE controller"},
	{CPU_TZPC_EFUSE_CTRL,       " EFUSE controller"},
	{CPU_TZPC_TRNG_CTRL,        "  TRNG controller"},
	{CPU_TZPC_RSA_CTRL,         "   RSA controller"},
	{CPU_TZPC_HASH_CTRL,        "  HASH controller"},
	{CPU_TZPC_TZPC_CTRL,        "  TZPC controller"},
};

static TZPC_NAME tz_pc_NA51084[] = {
	{CPU_TZPC_SCE_CTRL,         "   SCE controller"},
	{CPU_TZPC_EFUSE_CTRL,       " EFUSE controller"},
	{CPU_TZPC_TRNG_CTRL,        "  TRNG controller"},
	{CPU_TZPC_RSA_CTRL,         "   RSA controller"},
	{CPU_TZPC_HASH_CTRL,        "  HASH controller"},
	{CPU_TZPC_TZPC_CTRL,        "  TZPC controller"},
};

#define TERMNULL                            0           // NULL
#define BACKSPACE                           8           // back space
#define NEWLINE                             10          // new line
#define UART_ESC                            27
#define UART_ENTER                          13

#define NA51055                             0x48210000
#define NA51084								0x50210000
#define EMU_COMMON_CMD_SIZE    20
static char emuCommonBuf[EMU_COMMON_CMD_SIZE];

static UINT32 chip_id = 0x0;

#define INREG32(x)          (*((volatile UINT32*)(x)))
#define OUTREG32(x, y)      (*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

/**
* Call with struct optee_msg_arg as argument
 *
 * Call register usage:
 * a0   SMC Function ID, OPTEE_SMC*CALL_WITH_ARG
 * a1   Upper 32 bits of a 64-bit physical pointer to a struct optee_msg_arg
 * a2   Lower 32 bits of a 64-bit physical pointer to a struct optee_msg_arg
 * a3   Cache settings, not used if physical pointer is in a predefined shared
 *      memory area else per OPTEE_SMC_SHM_*
 * a4-6 Not used
 * a7   Hypervisor Client ID register
 *
 * Normal return register usage:
 * a0   Return value, OPTEE_SMC_RETURN_*
 * a1-3 Not used
 * a4-7 Preserved
**/

static int atoi(const char *str)
{
	return (int)simple_strtoul(str, '\0', 10);
}

/**
    Get a string from UART

    Get a string from UART.
    This api would be returned only if the ENTER key is pressed or the length of the buffer is reached in normal operation.
    Besides, the uart_abortGetChar() can also abort this api.

    @param[out] pcString        The incoming string.
    @param[in,out] pcBufferLen  For input, it's the maximum length of the buffer. And it will also return length of the input string.

    @return
     - @b E_PAR: Parameter error. pcString or pcBufferLen is NULL or buffer length is 0.
     - @b E_OK:  Get string successfully
     - @b E_OBJ: Input string longer than buffer
*/
INT32 uart_get_string(char *pcString, UINT32 *pcBufferLen)
{
	UINT8   uiBuferLen;
	int     ch;

	// check parameters
	if (pcString == NULL ||         // Parameter is NULL
		pcBufferLen == NULL ||      // Parameter is NULL
		*pcBufferLen == 0) {        // Buffer length is 0
		return -1;
	}

	uiBuferLen = *pcBufferLen;

	*pcBufferLen = 0;

	while (*pcBufferLen < uiBuferLen) {
		ch = serial_getc();
		*pcString = ch;
		if (*pcString == BACKSPACE) {
			if (*pcBufferLen) {
				(*pcBufferLen)--;
				pcString--;
			}
		} else if ((*pcString == NEWLINE) || (*pcString == UART_ENTER)) {

			*pcString = TERMNULL;
			return 0;
		} else {
			(*pcBufferLen)++;
			pcString++;
		}
	}
	return -1;
}

static UINT32 emu_get_number(UINT32 uiStart, UINT32 uiEnd)
{
	UINT32  cLen;
	UINT32  result;

	while (1) {
		if (uiEnd < 10) {
			cLen = 1;
		} else if (uiEnd < 100) {
			cLen = 2;
		} else if (uiEnd < 1000) {
			cLen = 3;
		} else if (uiEnd < 10000) {
			cLen = 4;
		} else if (uiEnd < 100000) {
			cLen = 5;
		} else if (uiEnd < 1000000) {
			cLen = 6;
		} else {
			cLen = 7;
		}
		memset((void *)emuCommonBuf, 0x0, EMU_COMMON_CMD_SIZE);
		printf(">> Input > %d ~ %d\r\n", uiStart, uiEnd);
		uart_get_string(emuCommonBuf, &cLen);
		result = atoi(emuCommonBuf);

		if ((result <= uiEnd) && (result >= uiStart)) {
			return result;
		}
	}
}

void debug_dump_addr(UINT32 addr, UINT32 size)
{
	UINT32 i, j;

	for (j = addr; j < (addr + size); j += 0x10) {
		printf("0x%08x:", j);
		for (i = 0; i < 4; i++) {
			printf("%08x", *(UINT32 *)(j + 4 * i));
			printf("  ");
		}
		printf("\r\n");
	}
}

static int do_tzpc(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	UINT32  selectionBit = 0;
	UINT32  element;
	UINT32  address, address_ofs, address_bit, tzpc_address;
	UINT32  reg;
	UINT32	index;

	chip_id = INREG32(0xF00100F0);
	printf("do_tzpc argc = %d\r\n", argc);
	if (argc < 2) {
		printf("argc %d error\n", argc);
		return -1;
	}

	printf("cmd = %s\r\n", argv[1]);

	printf("chip id = 0x%08x\r\n", chip_id);

	reg = *(UINT32 *)0xF0020078;

	*(UINT32 *)0xF0020078 = (reg | (0xF << 25));

    //APB timeout enabled
	//reg = *(UINT32 *)0xF0FF0020;
	//*(UINT32 *)0xF0FF0020 = (reg | (1 << 31));
	//debug_dump_addr(0xF0FF0000, 0x40);
	//printf("Enable APB timeout\r\n");

	if (strncmp(argv[1], "pct", 3) == 0) {
		if (chip_id == NA51055) {
			element = sizeof(tz_pc_NA51055) / sizeof(TZPC_NAME);
		} else {
			element = sizeof(tz_pc_NA51084) / sizeof(TZPC_NAME);
		}
		printf("=>Select test module 0 ~ %d\r\n", element);
		selectionBit = emu_get_number(0, element-1);

		if (chip_id == NA51055) {
			address = tz_pc_NA51055[selectionBit].base_address;
		} else {
			address = tz_pc_NA51084[selectionBit].base_address;
		}
		address_ofs = address;
		address_bit = address_ofs % 32;
		address_ofs = (address_ofs / 32) * 4;

		tzpc_address = 0xF0FF0000 + address_ofs;
		address = (0xF0000000 | (address << 16));
//		if (chip_id == NA51055) {
//			printf(("TZPC test base address = 0x%08x -> %s\r\n", address, tz_pc_NA51055[selectionBit].module_name));
//		} else {
//			printf(("TZPC test base address = 0x%08x -> %s\r\n", address, tz_pc_NA51084[selectionBit].module_name));
//		}

		if (chip_id == NA51055) {
			printf("[51055][%s]\r\n", tz_pc_NA51055[selectionBit].module_name);
		} else {
			printf("[51084][%s]\r\n", tz_pc_NA51084[selectionBit].module_name);
		}
		printf(" -> TZPC base address = 0x%08x bit[%d]\r\n", tzpc_address, address_bit);

		if (address == 0xf0700000) {
			debug_dump_addr(address, 128);
		} else if (address == 0xff600000) {
			debug_dump_addr(address, 0x70);
		} else {
			debug_dump_addr(address, 256);
		}

		if(tzpc_address == 0xFF600000) {
			*(volatile UINT32 *)0xF0FF0020 |= (1 << 2);
		} else {
			*(volatile UINT32 *)tzpc_address |= (1 << address_bit);
		}

		printf("switch to secure mode allow only\r\n");

		debug_dump_addr(address, 256);
	} else if (strncmp(argv[1], "slave_clk", 9) == 0) {
		if (chip_id == NA51055) {
			element = sizeof(tz_pc_NA51055) / sizeof(TZPC_NAME);
		} else {
			element = sizeof(tz_pc_NA51084) / sizeof(TZPC_NAME);
		}
		address_ofs = 0x24;
		tzpc_address = 0xF0FF0000 + address_ofs;
		address_bit = 0;

		printf(" -> TZPC base address = 0x%08x bit[%02d]\r\n", (int)tzpc_address, address_bit);

		reg = *(volatile UINT32 *)0xF0020080;
		printf("   RSTN ori [0xF0020080] crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

		reg = *(volatile UINT32 *)0xF0020084;
		printf("   RSTN ori [0xF0020084]    RTC->16[%d], WDT->17[%d],TMR->18[%d], TOP->27[%d],efuse->28[%d]\r\n", ((reg >> 16) & 0x1), ((reg >> 17) & 0x1), ((reg >> 18) & 0x1), ((reg >> 27) & 0x1), ((reg >> 28) & 0x1));

		reg = *(volatile UINT32 *)0xF0020088;
		printf("   RSTN ori [0xF0020084]   DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));


		reg = *(volatile UINT32 *)0xF002001C;
		printf(" clksel ori [0xF002001C] Crypto->[21..20][0x%02x], RSA->[23..22][0x%02x]\r\n", ((reg >> 20) & 0x3), ((reg >> 22) & 0x3));

		reg = *(volatile UINT32 *)0xF0020024;
		printf(" clksel ori [0xF0020024]   HASH->[17..16][0x%02x],TRNG->[18][%d], TRNG_RO->[19][%d], DRTC->[20][%d]\r\n", ((reg >> 16) & 0x3), ((reg >> 18) & 0x1), ((reg >> 19) & 0x1), ((reg >> 20) & 0x1));

		reg = *(volatile UINT32 *)0xF0020070;
		printf(" clk_en ori [0xF0020070] Crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

		*(volatile UINT32 *)0xF0020074 |= ((1 << 17) | (1 << 18) | (1 << 27));
		reg = *(volatile UINT32 *)0xF0020074;
		printf(" clk_en ori [0xF0020074]    WDT->17[%d],TMR->18[%d], TOP->27[%d]\r\n", ((reg >> 17) & 0x1), ((reg >> 18) & 0x1), ((reg >> 27) & 0x1));

		*(volatile UINT32 *)0xF0020078 |= (1 << 22);
		reg = *(volatile UINT32 *)0xF0020078;
		printf(" clk_en ori [0xF0020078]   DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));

		printf(" -> TZPC base address = 0x%08x bit[%02d] => Enable CKG slave \r\n", (int)(tzpc_address), address_bit);

		*(volatile UINT32 *)(tzpc_address) |= 0x1;

		*(volatile UINT32 *)0xF0020080 &= ~(1 << 23);
		reg = *(volatile UINT32 *)0xF0020080;
		printf("   RSTN clr [0xF0020080] crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

		*(volatile UINT32 *)0xF0020084 &= ~((1 << 16) | (1 << 17) | (1 << 18) | (1 << 27) | (1 << 28));
		reg = *(volatile UINT32 *)0xF0020084;
		printf("   RSTN clr [0xF0020084]   RTC->16[%d],WDT->17[%d],TMR->18[%d],TOP->27[%d],efuse->28[%d]\r\n", ((reg >> 16) & 0x1), ((reg >> 17) & 0x1), ((reg >> 18) & 0x1), ((reg >> 27) & 0x1), ((reg >> 28) & 0x1));

		*(volatile UINT32 *)0xF0020088 &= ~((1 << 22) | (1 << 25) | (1 << 26) | (1 << 27));
		reg = *(volatile UINT32 *)0xF0020088;
		printf("   RSTN clr [0xF0020084]  DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));

		*(volatile UINT32 *)0xF002001C |= ((0x3 << 20) | (0x3 << 22));
		reg = *(volatile UINT32 *)0xF002001C;
		printf(" clksel set [0xF002001C] Crypto->[21..20][0x%02x], RSA->[23..22][0x%02x]\r\n", ((reg >> 20) & 0x3), ((reg >> 22) & 0x3));

		*(volatile UINT32 *)0xF0020024 |= ((0x3 << 16) | (0x1 << 18) | (0x1 << 19) | (0x1 << 20));
		reg = *(volatile UINT32 *)0xF0020024;
		printf(" clksel set [0xF0020024]   HASH->[17..16][0x%02x],TRNG->[18][%d], TRNG_RO->[19][%d], DRTC->[20][%d]\r\n", ((reg >> 16) & 0x3), ((reg >> 18) & 0x1), ((reg >> 19) & 0x1), ((reg >> 20) & 0x1));


		*(volatile UINT32 *)0xF0020070 |= ((0x1 << 23));
		reg = *(volatile UINT32 *)0xF0020070;
		printf(" clk_en set [0xF0020070] Crypto->23[%d]\r\n", ((reg >> 23) & 0x1));

		*(volatile UINT32 *)0xF0020074 &= ~((0x1 << 17) | (0x1 << 18) | (0x1 << 27));
		reg = *(volatile UINT32 *)0xF0020074;
		printf(" clk_en clr [0xF0020074]  WDT->17[%d],TMR->18[%d],TOP->27[%d]\r\n", ((reg >> 17) & 0x1), ((reg >> 18) & 0x1), ((reg >> 27) & 0x1));

		*(volatile UINT32 *)0xF0020078 &= ~((0x1 << 22) | (0x1 << 25) | (0x1 << 26) | (0x1 << 27));
		reg = *(volatile UINT32 *)0xF0020078;
		printf(" clk_en clr [0xF0020078] DRTC->22[%d],TRNG->25[%d],RSA->26[%d],HASH->27[%d]\r\n", ((reg >> 22) & 0x1), ((reg >> 25) & 0x1), ((reg >> 26) & 0x1), ((reg >> 27) & 0x1));


	} else if (strncmp(argv[1], "slave_trng", 10) == 0) {
		address_ofs = 0x00;
		tzpc_address = 0xF0680000 + address_ofs;
		address_bit = 0;

		printf(" -> TRNG base address = 0x%08x bit[%02d]\r\n", (int)tzpc_address, address_bit);
		reg = *(volatile UINT32 *)tzpc_address;
		printf(" ctrl reg [0x%08x] = 0x%08x\r\n", (int)tzpc_address, (int)reg);


		*(volatile UINT32 *)(0xF0FF0024) |= 0x2;
		*(volatile UINT32 *)tzpc_address = 0x0;
		reg = *(volatile UINT32 *)tzpc_address;
		printf(" ctrl reg [0x%08x] = 0x%08x\r\n", (int)tzpc_address, (int)reg);


	} else if (strncmp(argv[1], "slave_top", 9) == 0) {
		address_ofs = 0x24;
		tzpc_address = 0xF0FF0000 + address_ofs;
		address_bit = 0;

		printf(" -> TZPC base address = 0x%08x bit[%02d]\r\n", (int)tzpc_address, address_bit);

		reg = *(volatile UINT32 *)0xF0011000;
		printf(" RSA SRAM ori [0x%08x] = bit25[%d]\r\n", (int)reg, (int)((reg >> 25) & 0x1));

		*(volatile UINT32 *)(0xF0FF0024) |= 0x4;

		*(volatile UINT32 *)0xF0011000 &= ~(1 << 25);
		reg = *(volatile UINT32 *)0xF0011000;
		printf(" RSA SRAM clr [0x%08x] = bit25[%d]\r\n", (int)reg, (int)((reg >> 25) & 0x1));

	} else if(strncmp(argv[1], "tzpc_dump", 9) == 0) {
		if (chip_id == NA51055) {
			element = sizeof(tz_pc_NA51055) / sizeof(TZPC_NAME);
		} else {
			element = sizeof(tz_pc_NA51084) / sizeof(TZPC_NAME);
		}
		for(index = 0; index < element; index ++) {
			if (chip_id == NA51055) {
				address = tz_pc_NA51055[index].base_address;
			} else {
				address = tz_pc_NA51084[index].base_address;
			}
			address_ofs = address;
			address_bit = address_ofs % 32;
			address_ofs = (address_ofs / 32) * 4;
			tzpc_address = 0xF0FF0000 + address_ofs;
			address = (0xF0000000 | (address << 16));

			if (chip_id == NA51055) {
				printf("TZPC[0x%08x]bit[%2d]->[0x%08x]", tzpc_address, address_bit, address/*, tz_pc_NA51055[index].module_name*/);
				if((*(volatile UINT32 *)tzpc_address & (1<<address_bit)) == (1<<address_bit)) {
					printf("[SEC only]=>[%s]\r\n", tz_pc_NA51055[index].module_name);
				} else {
					printf("[BOTH]=>[%s]\r\n", tz_pc_NA51055[index].module_name);
				}
			} else {
				printf("TZPC[0x%08x]bit[%2d] -> [0x%08x] => %s\r\n", tzpc_address, address_bit, address, tz_pc_NA51084[index].module_name);
				if((*(volatile UINT32 *)tzpc_address & (1<<address_bit)) == (1<<address_bit)) {
					printf("[SEC only]=>[%s]\r\n", tz_pc_NA51084[index].module_name);
				} else {
					printf("[BOTH]=>[%s]\r\n", tz_pc_NA51084[index].module_name);
				}
			}

		}
	}
#if 0
	else if (strncmp(argv[1], "mem_prot_in_1", 13) == 0) {
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


		printf(" Memory address = 0x%08x size = 0x4000\r\n", (int)uiTestAddrStart);

		for (uiSet = 0; uiSet < 6; uiSet++) {
			for (protect_mode = 0; protect_mode < 4; protect_mode++) {
				printf("====================set[%d] mode[%d]====================\r\n", uiSet, protect_mode);
                ProtectAttr.chEnMask.bCPU_NS = TRUE;
				ProtectAttr.uiProtectlel = protect_mode;
				ProtectAttr.uiStartingAddr = uiTestAddrStart;
				ProtectAttr.uiSize = uiTestSize * 2;

				//fLib_PutSerialStr("1. CPU write protect test start, write original pattern, test set %d, test mode %d\r\n", uiSet, protect_mode);

				printf("address[0x%08x] size[0x%08x]\r\n", (int)uiTestAddrStart, (int)uiTestSize);

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
							printf("cpu read fail = 0x%08x\r\n", *(volatile UINT32 *)(uiTestAddrStart + i));
						} else {
							printf("cpu read = 0x%08x\r\n", *(volatile UINT32 *)(uiTestAddrStart + i));
							if(*(volatile UINT32 *)(uiTestAddrStart + i) == 0x55aa55aa)
								printf(" --> corrected");
							else
								printf(" -->     error");
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
							printf("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						printf("CPU write protect test success\r\n");
					}
				} else if (protect_mode == 1) {
					bResult = TRUE;

					for (i = 0; i < 8; i += 4) {
						if (*(volatile UINT32 *)(uiTestAddrStart + i) != 0x69696969) {
							bResult = FALSE;
							printf("CPU write protect test fail\r\n");
							//while(1);
							break;
						}
					}
					if (bResult) {
						printf("CPU write protect test success\r\n");
					}
				}

				dma_disableWPFunc(uiSet);
			}
		}
	}
#endif
	else if (strncmp(argv[1], "apb_tout", 8) == 0) {
		if (chip_id == NA51055) {
			printf("=>NT98528 support only\r\n");
		} else {
			printf("=>Enable / disable APB timeout enabled\r\n");
			reg = *(UINT32 *)0xF0FF0020;
			*(UINT32 *)0xF0FF0020 = (reg | (1 << 31));
			debug_dump_addr(0xF0FF0000, 128);
		}
	} else if (strncmp(argv[1], "nv_counter", 10) == 0) {
		if (chip_id == NA51055) {
			printf("=>NT98525 NV counter over flow @ 49\r\n");
			int i;
			printf("=>NT98528 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT32 *)0xF0FF0030 = 1;
				reg = *(UINT32 *)0xF0FF0030;
				printf("	=>[%2d] read back [%2d]\r\n", i, reg);
			}
		} else {
			int i;
			printf("=>NT98528 NV counter over flow @ 64\r\n");
			for (i = 0; i < 70; i++) {
				*(UINT32 *)0xF0FF0030 = 1;
				reg = *(UINT32 *)0xF0FF0030;
				printf("	=>[%2d] read back [%2d][0x%08x] overflow==>[%d]\r\n", i, (reg & 0xFFFF), reg, ((reg & 0x10000) >> 16));

			}
		}
	}
	return 0;
}


U_BOOT_CMD(nvt_tzpc, 3, 0, do_tzpc,
		   "tzpc emulation cmd:",
		   "[Option] \n"
		   "                 [pct] : primary protected control test\n"
		   "            [apb_tout] : enable APB bus timeout\n"
		   "          [nv_counter] : nv_counter\n"
		   "           [tzpc_dump] : dump register info\n"
		   "           [slave_clk] : slave  ckg protected control test\n"
		   "          [slave_trng] : slave trng protected control test\n"
		   "           [slave_top] : slave  top protected control test\n"
		   "       [mem_prot_in_1] : mem protect in @ dram1 @ non secure world\n"
		   "       [mem_prot_in_2] : mem protect in @ dram2 @ non secure world\n"
		   "      [mem_prot_out_1] : mem protect out @ dram1 @ non secure world\n"
		   "      [mem_prot_out_1] : mem protect out @ dram2 @ non secure world\n"
		  );
