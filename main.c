/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "io.h"

static int stopped = 0;
static int paused = 0;
static time_t pause_start = 0;
static int total_paused_time = 0;

int run_timer(void *arg);

int main(int argc, char *argv[])
{
	int secs = 0;
	if (argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;
	int countUp = 0;

	render_init();

	thrd_t timer_thread;

	if (thrd_create(&timer_thread, run_timer, NULL) != thrd_success) {
		fputs("Failed to create timer thread\n", stderr);
		exit(1);
	}

	while (!stopped) {
		switch (getchar()) {
		case 'q':
			stopped = 1;
			break;
		case 32 /*space*/:
			if (!paused) {
				pause_start = time(NULL);
				paused = 1;
			} else {
				total_paused_time +=
				    difftime(time(NULL), pause_start);
				paused = 0;
			}
			break;
		}
	}

	render_dispose();

	thrd_join(timer_thread, NULL);

	return 0;
}

int run_timer(void *arg)
{
	time_t start = time(NULL);
	int time_elapsed = 0;

	while (!stopped) {
		if (!paused)
			time_elapsed = (int)difftime(time(NULL), start) -
				       total_paused_time;

		render(time_elapsed);
		usleep(100000); // 100 ms
	}

	return 0;
}
