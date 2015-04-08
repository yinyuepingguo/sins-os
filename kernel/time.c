#include <sins/time.h>
#include <sins/timer.h>
#include <div64.h>

#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

volatile time_t startup_time = 0;

/* interestingly, we assume leap-years */
static int month[12] = {
	0,
	DAY*(31),
	DAY*(31+29),
	DAY*(31+29+31),
	DAY*(31+29+31+30),
	DAY*(31+29+31+30+31),
	DAY*(31+29+31+30+31+30),
	DAY*(31+29+31+30+31+30+31),
	DAY*(31+29+31+30+31+30+31+31),
	DAY*(31+29+31+30+31+30+31+31+30),
	DAY*(31+29+31+30+31+30+31+31+30+31),
	DAY*(31+29+31+30+31+30+31+31+30+31+30)
};

time_t mktime(struct tm * tm)
{
	time_t res;
	int year;

	year = tm->year - 1970;
/* magic offsets (y+1) needed to get leapyears right.*/
	res = YEAR*year + DAY*((year+1)/4);
	res += month[tm->mon];
/* and (y+2) here. If it wasn't a leap-year, we have to adjust */
	if (tm->mon>1 && ((year+2)%4))
		res -= DAY;
	res += DAY*(tm->mday-1);
	res += HOUR*tm->hour;
	res += MINUTE*tm->min;
	res += tm->sec;
	return res;
}

/* timer and time are different. But time init is easy.We put them together. */
void time_init()
{
	struct tm now;

	read_time(&now);
	startup_time = mktime(&now);
}

time_t current_time()
{
	u64 tmp_jiffies = jiffies;
	
	do_div64(tmp_jiffies, HZ);
	return startup_time + (u32)tmp_jiffies;
}

asmlinkage time_t sys_time(struct tm *tm)
{
	if (tm != NULL)
		read_time(tm);
	return current_time();
}
