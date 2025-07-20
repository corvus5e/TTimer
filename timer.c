#include "timer.h"

#include <unistd.h>

void timer_reset(struct Timer *ts)
{
	ts->stopped = 1;
	ts->paused = 0;
	ts->start = 0;
	ts->active_elapsed_time = 0;
	ts->total_paused_time_sec = 0;
	ts->last_active_interval.start = 0;
	ts->last_active_interval.end = 0;
}

void timer_update(struct Timer *ts)
{
	if (!ts->stopped && !ts->paused) {
		ts->active_elapsed_time = (int)difftime(time(NULL), ts->start) -
				       ts->total_paused_time_sec;
	}
}

void timer_start(struct Timer *ts)
{
	if(!ts->stopped)
		return;

	timer_reset(ts);
	ts->start = time(NULL);
	ts->last_active_interval.start = ts->start;
	ts->stopped = 0;
}

void timer_stop(struct Timer *ts)
{
	ts->stopped = 1;

	if(!ts->paused)
		ts->last_active_interval.end = time(NULL);
}

void timer_pause(struct Timer *ts)
{
	if(ts->stopped)
		return;

	if (!ts->paused) { /* Pause */
		ts->paused = 1;
		ts->last_active_interval.end = time(NULL);
	} else {          /* Resume */
		time_t previouse_active_end = ts->last_active_interval.end;
		ts->last_active_interval.start = time(NULL);
		ts->total_paused_time_sec +=
		    difftime(ts->last_active_interval.start, previouse_active_end);
		ts->paused = 0;
	}
}
