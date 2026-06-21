#include "timer_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <timer/timer.h>
#include <HomeTUI/home_tui.h>
#include <ui/view.h>
#include <ui/char_codes.h>

#define BUF_LEN 10

#define SECS_DAY 86400

struct timer_view {
	struct view *view;
	struct AppContext *ctx;
	AppAction on_pause_resume;
	AppAction update_timer;
};

int handle_input_timer_view(struct view *, int input_key);

void render_timer_view(const struct view *);

void dispose_timer_view(struct view *);

struct view *create_timer_view(struct AppContext *ctx, AppAction on_pause_resume, AppAction update_timer)
{
	struct timer_view *timer_view = (struct timer_view *)malloc(sizeof(struct timer_view));
	if(!timer_view)
		return NULL;

	timer_view->view = view_create(render_timer_view, handle_input_timer_view, dispose_timer_view, timer_view);

	if(!timer_view->view) {
		return NULL;
	}

	if(!ctx || !on_pause_resume || !update_timer)
		return NULL;

	timer_view->ctx = ctx;
	timer_view->on_pause_resume = on_pause_resume;
	timer_view->update_timer = update_timer;

	return timer_view->view;
}

int handle_input_timer_view(struct view *v, int input)
{
	struct timer_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return IGNORED;
	}

	if (input == ' ') {
		view->on_pause_resume(view->ctx);
		return PROCESSED;
	}
	else {
		view->update_timer(view->ctx);
	}

	return PROCESSED;
}

#define BIG_TIMER_WIDTH 48
#define BIG_TIME_HEIGH 7

void render_timer_view(const struct view *v) {
	const struct timer_view *view = get_owner(v);
	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return;
	}

	int cols = 0;
	int lines = 0;
	get_window_size(&cols, &lines);
	int x = (cols - view->ctx->w * 8/*"%02d:%02d:%02d"*/) / 2;
	int y = (lines - view->ctx->h) / 2;

	render_clear();

	char buf[BUF_LEN];
	int len = 0;

	int seconds = view->ctx->timer.active_elapsed_time % 60;
	int minutes = (view->ctx->timer.active_elapsed_time / 60) % 60;
	int hours = view->ctx->timer.active_elapsed_time / 3600;

	if ((len = snprintf(&buf[0], BUF_LEN, "%02d:%02d:%02d", hours, minutes, seconds)) < 0)
		return;

	if(!view->ctx->textures){
		render_text(x, y, buf); //TODO: Center nicely
	}
	else {
		int i = 0;
		for (const char *s = buf; *s; ++s) {
			UI_CHAR *data = get_texture(view->ctx->textures, *s == ':' ? 10 : *s - '0');
			render_block(data, x + i*view->ctx->w, y, view->ctx->w, view->ctx->h);
			++i;
		}
	}

	// Print current time
	time_t now = time(NULL);
	const char *time_str = ctime(&now);
	int time_len = strlen(time_str);
	x = (cols - time_len) / 2;
	y += BIG_TIME_HEIGH + 2;
	render_ftext(x, y, "%s", time_str);

	if (view->ctx->timer.paused) {
		for (int i = 4; i < cols - 4; ++i) {
			render_cell(i, 2, TIMER_H_BORDER);
			render_cell(i, lines - 3, TIMER_H_BORDER);
		}

		 for (int i = 3; i < lines - 3; ++i) {
		 	render_cell(3, i, TIMER_V_BORDER);
		 	render_cell(cols - 4, i, TIMER_V_BORDER);
		 }

		 render_cell(3, 2, TIMER_UL_CORNER);
		 render_cell(cols - 4, 2, TIMER_UR_CORNER);
		 render_cell(3, lines - 3, TIMER_LL_CORNER);
		 render_cell(cols - 4, lines - 3, TIMER_LR_CORNER);

		render_text(4, lines - 4, "Paused");
	} else if (view->ctx->timer.stopped && view->ctx->timer.active_elapsed_time == 0 /*Initial start*/) {
		render_text(4, lines - 4, "Press space to start");
	}

	render_update();
}

void dispose_timer_view(struct view *v)
{
	struct timer_view *view = get_owner(v);
	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return;
	}

	free(view->view);
	free(view);
}

