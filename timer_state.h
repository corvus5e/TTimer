#ifndef _TTIMER_H_
#define _TTIMER_H_

#include <time.h>

struct TimerState {
	int stopped;
	int paused;
	time_t start;
	int time_elapsed_sec;
	time_t last_pause_start;
	int total_paused_time_sec;
};

void timer_init(struct TimerState *ts);

void timer_update(struct TimerState *ts);

void timer_stop(struct TimerState *ts);

void timer_pause(struct TimerState *ts);

#endif
