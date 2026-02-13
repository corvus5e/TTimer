#include "timer_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <timer/timer.h>
#include <HomeTUI/home_tui.h>

#define BUF_LEN 10

#define SECS_DAY 86400

#define IDLE_INPUT -1 // TODO: Put into HomeTUI


extern const struct Texture _textures[];

struct timer_view {
	struct AppContext *ctx;
	AppAction on_pause_resume;
	AppAction update_timer;
};

struct timer_view *create_timer_view(struct AppContext *ctx, AppAction on_pause_resume, AppAction update_timer)
{
	struct timer_view *view = (struct timer_view *)malloc(sizeof(struct timer_view));

	if(!view)
		return NULL;

	if(!ctx || !on_pause_resume || !update_timer)
		return NULL;

	view->ctx = ctx;
	view->on_pause_resume = on_pause_resume;
	view->update_timer = update_timer;

	return view;
}

int handle_input_timer_view(struct timer_view *view, int input)
{
	if (input == ' ') {
		view->on_pause_resume(view->ctx);
		return 1;
	}

	if(input == IDLE_INPUT) {
		view->update_timer(view->ctx);
		return 1;
	}

	return 0;
}

#define BIG_TIMER_WIDTH 48
#define BIG_TIME_HEIGH 7

void render_timer_view(struct timer_view *view)
{
	int cols = 0;
	int lines = 0;
	get_window_size(&cols, &lines);
	int x = (cols - BIG_TIMER_WIDTH) / 2;
	int y = (lines - BIG_TIME_HEIGH) / 2;

	render_clear();

	struct Timer *ts = view->ctx->timer;

	char buf[BUF_LEN];
	int len = 0;

	int seconds = ts->active_elapsed_time % 60;
	int minutes = (ts->active_elapsed_time / 60) % 60;
	int hours = ts->active_elapsed_time / 3600;

	if ((len = snprintf(&buf[0], BUF_LEN, "%02d:%02d:%02d", hours, minutes, seconds)) < 0)
		return;

	int column_shift = 0;

	for (char *s = buf; *s; ++s) {

		const struct Texture *t = &_textures[*s == ':' ? 10 : *s - '0'];

		const char *d = t->data;

		for (int i = 0; i < t->heigh; ++i) {
			for (int j = 0; j < t->width; ++j) {
				render_cell(j + column_shift + x, i + y,
					*(d + i * t->width + j));
			}
		}

		column_shift += t->width;
	}

	// Print current time
	time_t now = time(NULL);
	const char *time_str = ctime(&now);
	int time_len = strlen(time_str);
	x = (cols - time_len) / 2;
	y += BIG_TIME_HEIGH + 2;
	render_ftext(x, y, "%s", time_str);

	if (ts->paused) {
		for (int i = 4; i < cols - 4; ++i) {
			render_cell(i, 2, '-');
			render_cell(i, lines - 3, '-');
		}

		 for (int i = 3; i < lines - 3; ++i) {
		 	render_cell(3, i, '|');
		 	render_cell(cols - 4, i, '|');
		 }

		 render_cell(3, 2, '*');
		 render_cell(cols - 4, 2, '*');
		 render_cell(3, lines - 3, '*');
		 render_cell(cols - 4, lines - 3, '*');

		render_text(4, lines - 4, "Paused");
	} else if (ts->stopped && ts->active_elapsed_time == 0 /*Initial start*/) {
		render_text(4, lines - 4, "Press space to start");
	}

	render_update();
}

void dispose_timer_view(struct timer_view *view)
{
	free(view);
}
