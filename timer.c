#include "timer.h"

#include <unistd.h>

void timer_init(struct Timer *ts)
{
	ts->stopped = 0;
	ts->paused = 0;
	ts->start = 0;
	ts->time_elapsed_sec = 0;
	ts->total_paused_time_sec = 0;
	ts->last_pause_start = 0;
}

void timer_update(struct Timer *ts)
{
	if (!ts->stopped && !ts->paused) {
		ts->time_elapsed_sec = (int)difftime(time(NULL), ts->start) -
				       ts->total_paused_time_sec;
	}
}

void timer_start(struct Timer *ts)
{
	ts->start = time(NULL);
	ts->time_elapsed_sec = 0;
}

void timer_stop(struct Timer *ts)
{
	ts->stopped = 1;
}

void timer_pause(struct Timer *ts)
{
	if (!ts->paused) {
		ts->last_pause_start = time(NULL);
		ts->paused = 1;
	} else {
		ts->total_paused_time_sec +=
		    difftime(time(NULL), ts->last_pause_start);
		ts->paused = 0;
	}
}
