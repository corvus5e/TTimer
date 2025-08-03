/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "HomeTUI/home_tui.h"
#include "draw.h"
#include "db.h"
#include "timer.h"
#include "settings.h"

#define EXIT_APP 1
#define UPDATE_INPUT 512
#define IDLE_INPUT -1 //TODO: Put into HomeTUI

enum AppView { TIMER_VIEW, HELP_VIEW, GRAPH_VIEW, SETTINGS_VIEW};

struct AppSettings _settings;

struct AppContext {
	struct Timer *timer;
	enum AppView view;
	int day_shift;
};

int handle_input_graph_view(struct AppContext *context, int input_key);
int handle_input_timer_view(struct AppContext *context, int input_key);
int handle_input_help_view(struct AppContext *context, int input_key);
int handle_input_settings_view (struct AppContext *context, int input_key);

/* Saves time if last active interval is valid (timer is paused or stopped)
 * And interval is not zero */
int save_active_inteval_time(struct Timer*);

int main(void)
{
	_settings.stopped_on_app_start = 1;
	_settings.min_seconds_to_save = 30;

	render_init(1000);

	if(db_init() != 0) {
		fprintf(stderr, "Failed to initialize db\n");
		return 1;
	}

	struct Timer timer;
	timer_reset(&timer);

	struct AppContext cxt;
	cxt.timer = &timer;
	cxt.view = TIMER_VIEW;
	cxt.day_shift = 0;

	if(!_settings.stopped_on_app_start)
		timer_start(&timer);

	int input = UPDATE_INPUT;

	for (int status;;) {
		switch (cxt.view) {
		case TIMER_VIEW:
			status = handle_input_timer_view(&cxt, input);
			break;
		case HELP_VIEW:
			status = handle_input_help_view(&cxt, input);
			break;
		case GRAPH_VIEW:
			status = handle_input_graph_view(&cxt, input);
			break;
		case SETTINGS_VIEW:
			status = handle_input_settings_view(&cxt, input);
			break;
		}

		if (status == EXIT_APP)
			break;

		input = get_keyboard_input();
		//TODO: handle resize
	}

	render_dispose();

	db_dispose();

	return 0;
}

int handle_input_graph_view(struct AppContext *ctx, int input) {
	switch (input) {
	case ESC:
		ctx->view = TIMER_VIEW;
		return handle_input_timer_view(ctx, UPDATE_INPUT);
	case 'q':
		ctx->view = TIMER_VIEW;
		return handle_input_timer_view(ctx, input);
	case 'r':
		ctx->day_shift = 0;
		input = UPDATE_INPUT;
		break;
	case 'h':
		ctx->day_shift -= 1;
		input = UPDATE_INPUT;
		break;
	case 'l':
		ctx->day_shift += 1;
		input = UPDATE_INPUT;
		break;
	default:
		break;
	}

	if(input == UPDATE_INPUT) {
		struct TimeInterval *recorder_intervals;
		size_t n = 0;
		struct TimeInterval day_interval = get_day_interval(time(NULL), ctx->day_shift);
		if (db_get_time(day_interval, &recorder_intervals, &n))
			fprintf(stderr, "Error: Failed to get data from db\n");
		render_graph(day_interval, recorder_intervals, n);
	}

	return 0;
}

int handle_input_timer_view(struct AppContext *ctx, int input) {
	switch (input) {
	case 'h':
		ctx->view = HELP_VIEW;
		return handle_input_help_view(ctx, UPDATE_INPUT);
	case 'g':
		ctx->view = GRAPH_VIEW;
		return handle_input_graph_view(ctx, UPDATE_INPUT);
	case 's':
		ctx->view = SETTINGS_VIEW;
		return handle_input_settings_view(ctx, UPDATE_INPUT);
	case ' ':
		if (ctx->timer->stopped) {
			timer_start(ctx->timer);
		} else {
			timer_pause(ctx->timer);
			save_active_inteval_time(ctx->timer);
		}
		break;
	case 'q':
		timer_stop(ctx->timer);
		save_active_inteval_time(ctx->timer);
		return EXIT_APP;
	case UPDATE_INPUT:
	case IDLE_INPUT:
		timer_update(ctx->timer);
		break;
	default:
		break;
	}

	int w, h;
	get_window_size(&w, &h);
	render_timer(ctx->timer, (w - 48) / 2, (h - 7) / 2); //TODO: Do something with it :)

	return 0;
}

int handle_input_help_view(struct AppContext *ctx, int input) {
	switch (input) {
	case 'h':
		ctx->day_shift -= 1;
		break;
	case 'l':
		ctx->day_shift += 1;
		break;
	case 'g':
		ctx->view = GRAPH_VIEW;
		return handle_input_graph_view(ctx, UPDATE_INPUT);
	case ESC:
		ctx->view = TIMER_VIEW;
		return handle_input_timer_view(ctx, IDLE_INPUT);
	case 'q':
		ctx->view = TIMER_VIEW;
		return handle_input_timer_view(ctx, input);
	case UPDATE_INPUT:
		render_help();
		break;
	default:
		break;
	}

	return 0;
}

int handle_input_settings_view (struct AppContext *ctx, int input) {
	switch (input) {
	case 'q':
		ctx->view = TIMER_VIEW;
		return handle_input_timer_view(ctx, input);
	case UPDATE_INPUT:
		render_settings(&_settings);
	case ESC:
		ctx->view = TIMER_VIEW;
		return handle_input_timer_view(ctx, IDLE_INPUT);
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

