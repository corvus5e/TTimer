#ifndef _TTIMER_H_
#define _TTIMER_H_

#include <time.h>

struct Timer {
	int stopped;
	int paused;
	time_t start;
	int time_elapsed_sec;
	time_t last_resume_start;
	int total_paused_time_sec;
};

typedef int *TimeGraph;

void timer_init(struct Timer *ts);

void timer_update(struct Timer *ts);

void timer_start(struct Timer *ts);

void timer_stop(struct Timer *ts);

void timer_pause(struct Timer *ts);

#endif
