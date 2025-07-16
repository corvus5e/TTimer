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
	enum AppView *view;
};

int hande_event(struct AppContext *context, enum UserInput);

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

	timer_start(&timer);

	// Render first to appear immediately
	render_timer(&timer);

	while (!timer.stopped && hande_event(&context, get_user_input()) == 0)
		;

	render_dispose();

	db_dispose();

	return 0;
}

int hande_event(struct AppContext *context, enum UserInput input)
{
	switch (input) {
	case HELP_INPUT:
		*context->view = HELP_VIEW;
		render_help();
		break;
	case GRAPH_INPUT: {
		struct TimeRange *tr;
		size_t n = 0;
		if (db_get_time(&tr, &n))
			fprintf(stderr, "Error: Failed to get data from db\n");
		*context->view = GRAPH_VIEW;
		render_graph(tr, n);
		}
		break;
	case STOP_TIMER_INPUT:
		timer_stop(context->timer);
		 if (db_save_time(context->timer)) // TODO: Consider move it from listen loop?
		 fprintf(stderr,
		 	"Error: Failed to store data into db\n");
		break;
	case PAUSE_RESUME_TIMER_INPUT:
		timer_pause(context->timer);
	case BACK_INPUT:
		*context->view = TIMER_VIEW;
	case IDLE_INPUT:
		if (*context->view == TIMER_VIEW) {
			timer_update(context->timer);
			render_timer(context->timer);
		}
		break;
	}

	return 0;
}
