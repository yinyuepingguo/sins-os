#ifndef _ASM_TIME_H
#define _ASM_TIME_H

#include <asm/io.h>

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static inline void arch_read_time(struct tm *time)
{
	do {
		time->sec = CMOS_READ(0);
		time->min = CMOS_READ(2);
		time->hour = CMOS_READ(4);
		time->mday = CMOS_READ(7);
		time->mon = CMOS_READ(8) - 1;
		time->year = CMOS_READ(9);
	} while (time->sec != CMOS_READ(0));
	BCD_TO_BIN(time->sec);
	BCD_TO_BIN(time->min);
	BCD_TO_BIN(time->hour);
	BCD_TO_BIN(time->mday);
	BCD_TO_BIN(time->mon);
	BCD_TO_BIN(time->year);

	time->year += 2000;	
}

#endif
