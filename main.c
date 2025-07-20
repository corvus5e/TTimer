/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"
#include "db.h"
#include "timer.h"
#include "settings.h"

#define EXIT_APP 1

enum AppView { TIMER_VIEW, HELP_VIEW, GRAPH_VIEW, SETTINGS_VIEW};

struct AppSettings _settings;

struct AppContext {
	struct Timer *timer;
	enum AppView view;
	int day_shift;
};

int handle_input_graph_view(struct AppContext *context, enum UserInput);
int handle_input_timer_view(struct AppContext *context, enum UserInput);
int handle_input_help_view(struct AppContext *context, enum UserInput);
int handle_input_settings_view (struct AppContext *context, enum UserInput);

/* Saves time if last active interval is valid (timer is paused or stopped)
 * And interval is not zero */
int save_active_inteval_time(struct Timer*);

int main(void)
{
	_settings.stopped_on_app_start = 1;
	_settings.min_seconds_to_save = 30;

	render_init();

	if(db_init() != 0) {
		fprintf(stderr, "Failed to initialize db\n");
		return 1;
	}

	struct Timer timer;
	timer_reset(&timer);

	struct AppContext context;
	context.timer = &timer;
	context.view = TIMER_VIEW;
	context.day_shift = 0;

	if(!_settings.stopped_on_app_start)
		timer_start(&timer);

	enum UserInput input = UPDATE_INPUT;

	for (int status;;) {
		switch (context.view) {
		case TIMER_VIEW:
			status = handle_input_timer_view(&context, input);
			break;
		case HELP_VIEW:
			status = handle_input_help_view(&context, input);
			break;
		case GRAPH_VIEW:
			status = handle_input_graph_view(&context, input);
			break;
		case SETTINGS_VIEW:
			status = handle_input_settings_view(&context, input);
			break;
		}

		if (status == EXIT_APP)
			break;

		input = get_user_input();
	}

	render_dispose();

	db_dispose();

	return 0;
}

int handle_input_graph_view(struct AppContext *context, enum UserInput input) {
	switch (input) {
	case ESC_KEY:
		context->view = TIMER_VIEW;
		return handle_input_timer_view(context, UPDATE_INPUT);
	case Q_KEY:
		context->view = TIMER_VIEW;
		return handle_input_timer_view(context, Q_KEY);
	case R_KEY:
		context->day_shift = 0;
		input = UPDATE_INPUT;
		break;
	case H_KEY:
		context->day_shift -= 1;
		input = UPDATE_INPUT;
		break;
	case L_KEY:
		context->day_shift += 1;
		input = UPDATE_INPUT;
		break;
	default:
		break;
	}
	switch(input) {
	case UPDATE_INPUT:
		struct TimeInterval *tr;
		size_t n = 0;
		if (db_get_time(&tr, &n, context->day_shift))
			fprintf(stderr, "Error: Failed to get data from db\n");
		render_graph(tr, n);
	default:
		break;
	}

	return 0;
}

int handle_input_timer_view(struct AppContext *context, enum UserInput input) {
	switch (input) {
	case H_KEY:
		context->view = HELP_VIEW;
		return handle_input_help_view(context, UPDATE_INPUT);
	case G_KEY:
		context->view = GRAPH_VIEW;
		return handle_input_graph_view(context, UPDATE_INPUT);
	case S_KEY:
		context->view = SETTINGS_VIEW;
		return handle_input_settings_view(context, UPDATE_INPUT);
	case SPACE_KEY:
		if (context->timer->stopped) {
			timer_start(context->timer);
		} else {
			timer_pause(context->timer);
			save_active_inteval_time(context->timer);
		}
		break;
	case Q_KEY:
		timer_stop(context->timer);
		save_active_inteval_time(context->timer);
		return EXIT_APP;
	case UPDATE_INPUT:
	case IDLE_INPUT:
		timer_update(context->timer);
		break;
	default:
		break;
	}

	render_timer(context->timer);

	return 0;
}

int handle_input_help_view(struct AppContext *context, enum UserInput input) {
	switch (input) {
	case H_KEY:
		context->day_shift -= 1;
		break;
	case L_KEY:
		context->day_shift += 1;
		break;
	case G_KEY:
		context->view = GRAPH_VIEW;
		return handle_input_graph_view(context, UPDATE_INPUT);
	case ESC_KEY:
		context->view = TIMER_VIEW;
		return handle_input_timer_view(context, IDLE_INPUT);
	case Q_KEY:
		context->view = TIMER_VIEW;
		return handle_input_timer_view(context, Q_KEY);
	case UPDATE_INPUT:
		render_help();
		break;
	default:
		break;
	}

	return 0;
}

int handle_input_settings_view (struct AppContext *context, enum UserInput input) {
	switch (input) {
	case Q_KEY:
		context->view = TIMER_VIEW;
		return handle_input_timer_view(context, Q_KEY);
	case ESC_KEY:
		context->view = TIMER_VIEW;
		return handle_input_timer_view(context, IDLE_INPUT);
	case UPDATE_INPUT:
		render_settings(&_settings);
		break;
	default:
		break;
	}

	return 0;
}

int save_active_inteval_time(struct Timer* timer) {
	if(timer->start == 0) /* Timer has not been even started */
		return 1;

	/* If both paused and stopped true, then it was stopped from paused state, and 
	   last active interval already saved
	   If both paused and stopped false, then active interval is not finished yet */
	if(timer->paused == timer->stopped) 
		return 1;

	struct TimeInterval ti = timer->last_active_interval;

	if(difftime(ti.end, ti.start) <= _settings.min_seconds_to_save) /* Active time interval incorrect or too small */
		return 1;

	return db_save_time(timer->last_active_interval);
}

