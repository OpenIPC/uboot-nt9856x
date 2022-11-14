#include <common.h>
#include <command.h>
#include <dm.h>
#include <rtc.h>
#include <asm/io.h>
#include <compiler.h>
#include <stdlib.h>
#include <asm/armv7.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include "rtc_reg.h"


#define RTC_INT_KEY     0xA
#define RTC_INT_OSC_ANALOG_CFG 0x9
#define YEAR_OFFSET 1900

#define RTC_GETREG(ofs)        INW(IOADDR_RTC_REG_BASE+(ofs))
#define RTC_SETREG(ofs, value) OUTW(IOADDR_RTC_REG_BASE +(ofs),(value))

static const unsigned short rtc_ydays[2][13] = {
        /* Normal years */
        { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
        /* Leap years */
        { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

void rtc_trigger_cset(void)
{
        union RTC_STATUS_REG status_reg;
        union RTC_CTRL_REG ctrl_reg;

        /*Wait for RTC SRST done*/
        do {
                status_reg.reg = RTC_GETREG(RTC_STATUS_REG_OFS);
        } while (status_reg.bit.srst_sts == 1);

        /*Wait for RTC is ready for next CSET*/
        do {
                ctrl_reg.reg = RTC_GETREG(RTC_CTRL_REG_OFS);
        } while (ctrl_reg.bit.cset == 1);

        /*Trigger CSET*/
        ctrl_reg.reg = RTC_GETREG(RTC_CTRL_REG_OFS);
        ctrl_reg.bit.cset = 1;
        ctrl_reg.bit.cset_inten = 1;
        RTC_SETREG(RTC_CTRL_REG_OFS, ctrl_reg.reg);

}

int rtc_get(struct rtc_time *tm)
{
	uint32_t days, months, years, month_days;
        union RTC_TIMER_REG timer_reg;
        union RTC_DAYKEY_REG daykey_reg;


        timer_reg.reg = RTC_GETREG(RTC_TIMER_REG_OFS);
        daykey_reg.reg = RTC_GETREG(RTC_DAYKEY_REG_OFS);
        days = daykey_reg.bit.day;



        for (years = 0; days >= rtc_ydays[is_leap_year(years + 1900)][12]; years++) {
                days -= rtc_ydays[is_leap_year(years + 1900)][12];
        }

        for (months = 1; months < 13; months++) {
                if (days <= rtc_ydays[is_leap_year(years + 1900)][months]) {
                        days -= rtc_ydays[is_leap_year(years + 1900)][months-1];
                        months--;
                        break;
                }
        }

        month_days =  rtc_ydays[is_leap_year(years + 1900)][months+1] - rtc_ydays[is_leap_year(years + 1900)][months];

        if (days == month_days) {
                months++;
                days = 1;
        } else
                days++; /*Align linux time format*/



        tm->tm_sec  = timer_reg.bit.sec;
        tm->tm_min  = timer_reg.bit.min;
        tm->tm_hour = timer_reg.bit.hour;
        tm->tm_mday = days;
        tm->tm_mon  = months;
        tm->tm_year = years + YEAR_OFFSET;
		rtc_calc_weekday(tm);

        debug("after read time: sec = %d, min = %d, hour = %d, mday = %d," \
        "mon = %d, year = %d, wday = %d, yday = %d," \
        "\n", tm->tm_sec, tm->tm_min, tm->tm_hour, tm->tm_mday, \
        tm->tm_mon, tm->tm_year, tm->tm_wday, tm->tm_yday);


	return 0;
}

int rtc_set(struct rtc_time *tm)
{

	int year_looper ;
	uint32_t days = 0;
	union RTC_TIMER_REG timer_reg;
	union RTC_DAYKEY_REG daykey_reg;
	union RTC_CTRL_REG ctrl_reg;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	tm->tm_year=tm->tm_year - YEAR_OFFSET;

	for (year_looper = 0; year_looper < tm->tm_year; year_looper++)
			days += rtc_ydays[is_leap_year(year_looper + 1900)][12];

	days += rtc_ydays[is_leap_year(year_looper + 1900)][tm->tm_mon];
	tm->tm_mday--; /*subtract the day which is not ended*/
	days += tm->tm_mday;


	ctrl_reg.reg = RTC_GETREG(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.day_sel = 1;
	ctrl_reg.bit.time_sel = 1;
	RTC_SETREG(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	timer_reg.reg = 0;
	timer_reg.bit.sec = tm->tm_sec;
	timer_reg.bit.min = tm->tm_min;
	timer_reg.bit.hour = tm->tm_hour;
	RTC_SETREG(RTC_TIMER_REG_OFS, timer_reg.reg);

	daykey_reg.reg = RTC_GETREG(RTC_DAYKEY_REG_OFS);
	daykey_reg.bit.day = days;
	RTC_SETREG(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

	rtc_trigger_cset();

	return 0;

}

void rtc_reset(void)
{
	//u32 reg_data;


	union RTC_OSCAN_REG oscan_reg;
	union RTC_STATUS_REG status_reg;
	union RTC_DAYKEY_REG daykey_reg;
	union RTC_CTRL_REG ctrl_reg;

	// Wait for previous SRST done
	do {
			status_reg.reg = RTC_GETREG(RTC_STATUS_REG_OFS);
	} while (status_reg.bit.srst_sts);

	// Wait for RTC is ready for next CSET
	do {
			ctrl_reg.reg = RTC_GETREG(RTC_CTRL_REG_OFS);
	} while (ctrl_reg.bit.cset);

	// Do software reset
	ctrl_reg.reg = 0;
	ctrl_reg.bit.srst = 1;
	RTC_SETREG(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	// Wait for RTC is ready for next CSET
	do {
			ctrl_reg.reg = RTC_GETREG(RTC_CTRL_REG_OFS);
	} while (ctrl_reg.bit.srst);


	ctrl_reg.reg = RTC_GETREG(RTC_CTRL_REG_OFS);
	ctrl_reg.bit.cset = 0;
	ctrl_reg.bit.key_sel = 1;
	ctrl_reg.bit.pwralarmday_sel = 1;
	ctrl_reg.bit.day_sel = 1;
	RTC_SETREG(RTC_CTRL_REG_OFS, ctrl_reg.reg);

	daykey_reg.reg = RTC_GETREG(RTC_DAYKEY_REG_OFS);
	daykey_reg.bit.key = RTC_INT_KEY;
	RTC_SETREG(RTC_DAYKEY_REG_OFS, daykey_reg.reg);

	/*Set OSC analog parameter*/
	oscan_reg.reg = RTC_GETREG(RTC_OSCAN_REG_OFS);
	oscan_reg.bit.osc_analogcfg = RTC_INT_OSC_ANALOG_CFG;
	RTC_SETREG(RTC_OSCAN_REG_OFS, oscan_reg.reg);

	rtc_trigger_cset();

}
