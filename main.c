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
#include "timer.h"

enum AppView { TIMER_VIEW, HELP_VIEW, GRAPH_VIEW };

struct AppContext {
	struct Timer *timer;
	enum AppView *view;
};

void* run_state_loop(void *arg);
int handle_user_input(void *arg);

int main(int argc, char *argv[])
{
	int secs = 0;
	if (argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;

	render_init();

	struct Timer timer;
	enum AppView view = TIMER_VIEW;

	timer_init(&timer);

	struct AppContext context;
	context.timer = &timer;
	context.view = &view;

	pthread_t thread;
	if (pthread_create(&thread, NULL, run_state_loop, &context) != 0) {
		fputs("Failed to create timer thread\n", stderr);
		exit(1);
	}

	handle_user_input(&context); // Blocking call

	render_dispose();

	pthread_join(thread, NULL);

	return 0;
}

void *run_state_loop(void *arg)
{
	struct AppContext *context = (struct AppContext*)arg;
	struct Timer *timer = context->timer;
	enum AppView *view = context->view;

	timer_start(timer);

	struct timespec delay;
	delay.tv_nsec = 100000000; // 100 ms

	while (!timer->stopped) {
		switch (*view) {
		case TIMER_VIEW:
			render_timer(timer);
			break;
		case HELP_VIEW:
			render_help();
			break;
		case GRAPH_VIEW:
			render_graph();
			break;
		}
		timer_update(timer);
		nanosleep(&delay, NULL);
	}

	return NULL;
}

int handle_user_input(void *arg)
{
	struct AppContext *context = (struct AppContext*)arg;
	struct Timer *timer = context->timer;

	while (!timer->stopped) {
		switch (get_user_input()) {
		case HELP_INPUT:
			*context->view = HELP_VIEW;
			break;
		case BACK_INPUT:
			*context->view = TIMER_VIEW;
			break;
		case GRAPH_INPUT:
			*context->view = GRAPH_VIEW;
			break;
		case STOP_TIMER_INPUT:
			timer_stop(timer);
			log_timer(timer);
			break;
		case PAUSE_RESUME_TIMER_INPUT:
			timer_pause(timer);
			break;
		}
	}

	return 0;
}
