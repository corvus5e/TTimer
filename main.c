/*
 * TTimer - timer, which prints time to terminal
*/

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "io.h"
#include "log.h"
#include "timer_state.h"

void* run_timer(void *arg);
int handle_user_input(void *arg);

int main(int argc, char *argv[])
{
	int secs = 0;
	if (argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;

	render_init();

	struct TimerState ts;

	timer_init(&ts);

	pthread_t thread;
	if (pthread_create(&thread, NULL, run_timer, &ts) != 0) {
		fputs("Failed to create timer thread\n", stderr);
		exit(1);
	}

	handle_user_input(&ts); // Blocking call

	render_dispose();

	pthread_join(thread, NULL);

	return 0;
}

void *run_timer(void *arg)
{
	struct TimerState *ts = (struct TimerState*)arg;

	ts->start = time(NULL);
	ts->time_elapsed_sec = 0;

	struct timespec delay;
	delay.tv_nsec = 100000000; // 100 ms

	while (!ts->stopped) {
		timer_update(ts);
		render(ts);
		nanosleep(&delay, NULL);
	}

	return NULL;
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
