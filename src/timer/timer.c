#include "timer.h"

#include <unistd.h>

void timer_reset(struct Timer *ts)
{
	ts->stopped = 1;
	ts->paused = 0;
	ts->start = 0;
	ts->active_elapsed_time = 0;
	ts->previous_eplapsed_time = 0;
	ts->last_active_interval.start = 0;
	ts->last_active_interval.end = 0;
}

void timer_update(struct Timer *ts)
{
	if (!ts->stopped && !ts->paused) {
		ts->last_active_interval.end = time(NULL);
		ts->active_elapsed_time = ts->previous_eplapsed_time + (int)difftime(ts->last_active_interval.end, ts->last_active_interval.start);
	}
}

void timer_start(struct Timer *ts)
{
	if(!ts->stopped)
		return;

	timer_reset(ts);
	ts->start = time(NULL);
	ts->last_active_interval.start = ts->last_active_interval.end = ts->start;
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
		ts->previous_eplapsed_time += (int)difftime(ts->last_active_interval.end, ts->last_active_interval.start);
	} else {          /* Resume */
		ts->last_active_interval.start = ts->last_active_interval.end = time(NULL);
		ts->paused = 0;
	}
}

int timer_active_elapsed_time(const struct Timer *ts)
{
	return ts->active_elapsed_time;
}

struct TimeInterval get_day_interval(time_t time, int day_shift)
{
	struct tm *local_time = localtime(&time);

	// Clear todays time
	local_time->tm_sec = 0;
	local_time->tm_min = 0;
	local_time->tm_hour = 0;
	local_time->tm_mday += day_shift;
	time_t start = mktime(local_time);

	// Whole current day
	local_time->tm_mday += 1;
	time_t end = mktime(local_time);

	struct TimeInterval day_interval;
	day_interval.start = start;
	day_interval.end = end;

	return day_interval;
}
