/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"
#include "db.h"
#include "timer.h"

enum AppView { TIMER_VIEW, HELP_VIEW, GRAPH_VIEW};

struct AppContext {
	struct Timer *timer;
	enum AppView view;
	int day_shift;
};

int handle_input_graph_view(struct AppContext *context, enum UserInput);
int handle_input_timer_view(struct AppContext *context, enum UserInput);
int handle_input_help_view(struct AppContext *context, enum UserInput);

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

	timer_init(&timer);

	struct AppContext context;
	context.timer = &timer;
	context.view = TIMER_VIEW;
	context.day_shift = 0;

	timer_start(&timer);

	// Render first to appear immediately
	render_timer(&timer);

	while (!timer.stopped) {
		enum UserInput input = get_user_input();
		switch (context.view) {
		case TIMER_VIEW:
			handle_input_timer_view(&context, input);
			break;
		case HELP_VIEW:
			handle_input_help_view(&context, input);
			break;
		case GRAPH_VIEW:
			handle_input_graph_view(&context, input);
			break;
		}
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
		struct TimeRange *tr;
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
	case SPACE_KEY:
		timer_pause(context->timer);
		break;
	case Q_KEY:
		timer_stop(context->timer);
		if (db_save_time(context->timer)) // TODO: Consider move it from listen loop?
			fprintf(stderr, "Error: Failed to store data into db\n");
		break;
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

