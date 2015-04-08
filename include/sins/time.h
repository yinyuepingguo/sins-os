#ifndef _SINS_TIME_H
#define _SINS_TIME_H

#include <types.h>

typedef long time_t;

struct tm {
	int sec;
	int min;
	int hour;
	int mday;
	int mon;
	int year;
};

extern void time_init();
extern volatile time_t startup_time;
extern time_t mktime(struct tm *tm);
extern time_t current_time();

#include <asm/time.h>

#define read_time(tm)	arch_read_time(tm)

#endif
