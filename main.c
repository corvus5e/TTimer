/*
 * TTimer - timer, which prints time to terminal
*/

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "io.h"
#include "log.h"
#include "timer_state.h"

int run_timer(void *arg);
int handle_user_input(void *arg);

int main(int argc, char *argv[])
{
	int secs = 0;
	if (argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;
	int countUp = 0;

	render_init();

	struct TimerState ts;
	ts.stopped = 0;
	ts.paused = 0;

	thrd_t timer_thread;
	if (thrd_create(&timer_thread, run_timer, &ts) != thrd_success) {
		fputs("Failed to create timer thread\n", stderr);
		exit(1);
	}

	handle_user_input(&ts); // Blocking call

	render_dispose();

	thrd_join(timer_thread, NULL);

	return 0;
}

int run_timer(void *arg)
{
	struct TimerState *ts = (struct TimerState*)arg;

	ts->start = time(NULL);
	ts->time_elapsed_sec = 0;

	struct timespec delay;
	delay.tv_nsec = 100000000;

	while (!ts->stopped) {
		timer_update(ts);
		render(ts);
		nanosleep(&delay, NULL); // 100 ms
	}

	return 0;
}

int handle_user_input(void *arg)
{
	struct TimerState *ts = (struct TimerState*)arg;

	while (!ts->stopped) {
		switch (get_user_input()) {
		case STOP_TIMER:
			timer_stop(ts);
			log_timer(ts);
			break;
		case PAUSE_RESUME_TIMER:
			timer_pause(ts);
			break;
		}
	}

	return 0;
}
