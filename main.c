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
#include "db.h"
#include "timer.h"

enum AppView { TIMER_VIEW, HELP_VIEW, GRAPH_VIEW};

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
	if(db_init() != 0) {
		fprintf(stderr, "Failed to initialize db\n");
		return 1;
	}

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

	pthread_join(thread, NULL);

	render_dispose();
	db_dispose();

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
			if(db_get_time(NULL, NULL))
				fprintf(stderr, "Error: Failed to get data from db\n");
			*context->view = GRAPH_VIEW;
			break;
		case STOP_TIMER_INPUT:
			timer_stop(timer);
			if (db_save_time(timer)) //TODO: Consider move it from input listen loop?
				fprintf(stderr, "Error: Failed to store data into db\n");
			break;
		case PAUSE_RESUME_TIMER_INPUT:
			timer_pause(timer);
			break;
		}
	}

	return 0;
}
