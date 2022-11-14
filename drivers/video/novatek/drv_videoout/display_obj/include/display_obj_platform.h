#ifndef __IDE_PLATFORM_H_
#define __IDE_PLATFORM_H__

#include "display_int.h"
#if (defined __UITRON || defined __ECOS)
#include "NvtVerInfo.h"
#include "kernel.h"
#include "Debug.h"
#include "nvtDrvProtected.h"
#include "pll.h"
#include "pll_protected.h"
#include "hdmitx.h"
#include "top.h"
#include "mi.h"
#include "dsi.h"


// Default debug level
#ifndef __DBGLVL__
#define __DBGLVL__  1       // Output all message by default. __DBGLVL__ will be set to 1 via make parameter when release code.
#endif

// Default debug filter
#ifndef __DBGFLT__
#define __DBGFLT__  "*"     // Display everything when debug level is 2
#endif

#include "DebugModule.h"
#elif defined __FREERTOS
#include <stdlib.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "dma_protected.h"
#include "dispdev_ioctrl.h"
#include "ide_protected.h"
#include "display_obj_dbg.h"

#else
#ifdef CONFIG_TARGET_NA51055
	#include "../.././nvt-na51055/include/ide_protected.h"
#endif
//#include <plat-na51055/top.h>
//#include "./include/dispdev_ioctrl.h"
//#include "./include/display_obj_dbg.h"
#include <common.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/dispctrl.h>
#include <asm/arch/displyr.h>
#include <asm/arch/display.h>
#include <asm/arch/dispdev_ioctrl.h>
#include <asm/nvt-common/rcw_macro_bit.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/arch/dispdevctrl.h>
//#include "kwrap/type.h"
//#include <mach/fmem.h>
//#include <mach/rcw_macro.h>
//#include <linux/fb.h>
//#include "tv.h"

#define _FPGA_EMULATION_ 0

#endif


extern void display_obj_platform_do_div(UINT64 *dividend, UINT32 divisor);
extern ER display_obj_platform_pll_en(DISPCTRL_SRCCLK clk_src);
extern ER display_obj_platform_set_clk_src(IDE_ID id, DISPCTRL_SRCCLK clk_src);
extern UINT32 display_obj_platform_va2pa(UINT32 addr);
#endif
