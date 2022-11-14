#include <common.h>
#include <asm/io.h>
#include <compiler.h>
#include <stdlib.h>
#include <asm/armv7.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>



#define RTC_TIMER_REG_OFS 0x00
#define RTC_CTRL_REG_OFS 0x10
#define RTC_PWBC_REG_OFS 0x18
#define RTC_PWRALM_REG_OFS 0x20

#define RTC_GETREG(ofs)        INW(IOADDR_RTC_REG_BASE+(ofs))
#define RTC_SETREG(ofs, value) OUTW(IOADDR_RTC_REG_BASE +(ofs),(value))

int do_nvt_poweroff(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{

		u32 reg_data;

		reg_data = RTC_GETREG(RTC_PWBC_REG_OFS) | 0x10;
		RTC_SETREG(RTC_PWBC_REG_OFS,reg_data);

		mdelay(250);

		reg_data = RTC_GETREG(RTC_PWBC_REG_OFS) | 0x1;
		RTC_SETREG(RTC_PWBC_REG_OFS,reg_data);

		udelay(250);
		reg_data = RTC_GETREG(RTC_PWBC_REG_OFS) | 0x2;
		RTC_SETREG(RTC_PWBC_REG_OFS,reg_data);
		return 0;
}


U_BOOT_CMD(
	nvt_poweroff,   1,  0,  do_nvt_poweroff,
	"To do nvt platform poweroff.", "\n"
);
