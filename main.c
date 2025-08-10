/*
 * TTimer - timer, which prints time to terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "HomeTUI/home_tui.h"
#include "app_context.h"
#include "db.h"
#include "draw.h"
#include "timer.h"
#include "ui/settings_view.h"
#include "ui/graph_view.h"

#define EXIT_APP 2
#define IDLE_INPUT -1 // TODO: Put into HomeTUI

int handle_input_timer_view(struct AppContext *context, int input_key);
int handle_input_global(struct AppContext *context, int *altered_input);

/* App actions */
int on_save_settings(struct AppContext *ctx);

int get_time_intervals(struct TimeInterval time_period, struct TimeInterval **intervals, size_t *size);

/* Saves time if last active interval is valid (timer is paused or stopped)
 * And interval is not zero */
int save_active_inteval_time(struct Timer *, int min_seconds_to_save);

int main(void)
{
	render_init(1000); /* 1 second */

	if (db_init() != 0) {
		fprintf(stderr, "Failed to initialize db\n");
		return 1;
	}

	struct Timer timer;
	timer_reset(&timer);

	struct AppContext ctx;
	ctx.timer = &timer;
	ctx.view = TIMER_VIEW;
	ctx.day_shift = 0;

	ctx.settings.stopped_on_app_start = 1;
	ctx.settings.stop_after_min = -1; // Do not stop at all
	ctx.settings.min_seconds_to_save = 0;

	struct settings_view *sv = create_settings_view(&ctx, on_save_settings);
	if (!sv) {
		fprintf(stderr, "Failed to create settings view\n");
		return 1;
	}

	struct graph_view *gv = create_graph_view(&ctx, get_time_intervals);
	if (!gv) {
		fprintf(stderr, "Failed to create graph view\n");
		return 1;
	}


	if (!ctx.settings.stopped_on_app_start)
		timer_start(&timer);

	int input = IDLE_INPUT;

	for (int status = 1;;) {
		/* Render views */
		if (status) {
			switch (ctx.view) {
			case TIMER_VIEW:;
				int w, h;
				get_window_size(&w, &h);
				timer_update(ctx.timer);
				render_timer(ctx.timer, (w - 48) / 2, (h - 7) / 2); // TODO: Do something with it :)
				break;
			case HELP_VIEW:
				render_help();
				break;
			case GRAPH_VIEW:
				render_graph_view(gv);
				break;
			case SETTINGS_VIEW:
				render_settings_view(sv);
				break;
			}
		}

		input = get_keyboard_input();

		status = handle_input_global(&ctx, &input);

		if (status == EXIT_APP)
			break;

		if(status)
			continue;

		/* Handle input */
		switch (ctx.view) {
		case TIMER_VIEW:
			status = handle_input_timer_view(&ctx, input);
			break;
		case HELP_VIEW:
			break;
		case GRAPH_VIEW:
			status = handle_input_graph_view(gv, input);
			break;
		case SETTINGS_VIEW:
			status = handle_input_settings_view(sv, input);
			break;
		}
	}

	render_dispose();

	db_dispose();

	return 0;
}


int handle_input_timer_view(struct AppContext *ctx, int input)
{
	if (input == ' ') {
		if (ctx->timer->stopped) {
			timer_start(ctx->timer);
		} else {
			timer_pause(ctx->timer);
			save_active_inteval_time(ctx->timer, ctx->settings.min_seconds_to_save);
		}
		return 1;
	}

	return input == IDLE_INPUT;
}

int handle_input_global(struct AppContext *ctx, int *input_key)
{
	if (*input_key == ESC) {
		ctx->view = TIMER_VIEW;
		*input_key = IDLE_INPUT;
		return 1;
	}

	if (*input_key == 'q') {
		timer_stop(ctx->timer);
		save_active_inteval_time(ctx->timer, ctx->settings.min_seconds_to_save);
		handle_input_timer_view(ctx, IDLE_INPUT);
		return EXIT_APP;
	}

	if(ctx->view != TIMER_VIEW) /* Navidation to other views only from TIMER_VIEW */
		return 0;

	switch (*input_key) {
	case 'g':
		ctx->view = GRAPH_VIEW;
		*input_key = IDLE_INPUT;
		return 1;
	case 'h':
		ctx->view = HELP_VIEW;
		*input_key = IDLE_INPUT;
		return 1;
	case 's':
		ctx->view = SETTINGS_VIEW;
		*input_key = IDLE_INPUT;
		return 1;
	}

	return 0;
}

/* View callbacks */
int on_save_settings(struct AppContext *ctx)
{
	ctx->view = TIMER_VIEW;
	return 0;
};

int get_time_intervals(struct TimeInterval time_period, struct TimeInterval **intervals, size_t *size)
{
	if (db_get_time(time_period, intervals, size)) {
		fprintf(stderr, "Error: Failed to get data from db\n");
		return 0;
	}

	return 1;
}

int save_active_inteval_time(struct Timer *timer, int min_seconds_to_save)
{
	if (timer->start == 0) /* Timer has not been even started */
		return 1;

	/* If both paused and stopped true, then it was stopped from paused
	   state, and last active interval already saved If both paused and
	   stopped false, then active interval is not finished yet */
	if (timer->paused == timer->stopped)
		return 1;

	struct TimeInterval ti = timer->last_active_interval;

	if (difftime(ti.end, ti.start) <= min_seconds_to_save) /* Active time interval incorrect or too small */
		return 1;

	return db_save_time(timer->last_active_interval);
}
