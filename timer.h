#ifndef _TTIMER_H_
#define _TTIMER_H_

#include <time.h>

struct TimeInterval {
	time_t start;
	time_t end;
};

struct Timer {
	int stopped;
	int paused;
	time_t start;
	int active_elapsed_time;
	int total_paused_time_sec;
	struct TimeInterval last_active_interval;
};

void timer_reset(struct Timer *ts);

void timer_update(struct Timer *ts);

void timer_start(struct Timer *ts);

void timer_stop(struct Timer *ts);

void timer_pause(struct Timer *ts);

#endif
