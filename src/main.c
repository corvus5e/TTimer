/*
 * TTimer - timer, which prints time to terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <HomeTUI/home_tui.h>
#include <app_context.h>
#include <timer/timer.h>

#include <db/db.h>

#include <ui/settings_view.h>
#include <ui/graph_view.h>
#include <ui/timer_view.h>
#include <ui/help_view.h>

#define EXIT_APP 2
#define IDLE_INPUT -1 // TODO: Put into HomeTUI

int handle_input_global(struct AppContext *context, int *altered_input);

/* App actions */
int save_settings(struct AppContext *ctx, struct AppSettings new_settings);

int timer_update_callback(struct AppContext *ctx);

int pause_resume(struct AppContext *ctx);

int get_time_intervals(struct TimeInterval time_period, struct TimeInterval **intervals, size_t *size);

/* Saves time if last active interval is valid (timer is paused or stopped)
 * And interval is not zero */
int save_active_inteval_time(struct Timer *, int min_seconds_to_save);

void termination_signal_handler(int);

/* Global pointer to current app context*/
struct AppContext *s_ctx_ptr = NULL;

int main(void)
{
	struct sigaction sa;
	sa.sa_handler = termination_signal_handler;
	if(sigaction(SIGINT, &sa, NULL) < 0){
		fprintf(stderr, "Failed to setup SIGTERM handler\n");
	}

	render_init(1000); /* 1 second */

	if (db_init() != 0) {
		fprintf(stderr, "Failed to initialize db\n");
		return 1;
	}

	struct Timer timer;
	timer_reset(&timer);

	struct AppContext ctx;
	s_ctx_ptr = &ctx;
	ctx.timer = &timer;
	ctx.view = TIMER_VIEW;
	ctx.day_shift = 0;

	if (db_get_settings(&ctx.settings) != 0) {
		fprintf(stderr, "Failed to read settings. Using defaults\n");
		ctx.settings.stopped_on_app_start = 1;
		ctx.settings.stop_after_min = -1; // Do not stop at all
		ctx.settings.min_seconds_to_save = 0;
		ctx.settings.save_on_term_signal = 0;
	}

	struct settings_view *sv = create_settings_view(&ctx, save_settings, db_get_settings);
	if (!sv) {
		fprintf(stderr, "Failed to create settings view\n");
		return 1;
	}

	struct graph_view *gv = create_graph_view(&ctx, get_time_intervals);
	if (!gv) {
		fprintf(stderr, "Failed to create graph view\n");
		return 1;
	}

	struct timer_view *tv = create_timer_view(&ctx, pause_resume, timer_update_callback);
	if (!tv) {
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
				render_timer_view(tv);
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

		//TODO: Think on the return value from this handler
		status = handle_input_global(&ctx, &input);

		if (status == EXIT_APP)
			break;

		if(status)
			continue;

		/* Handle input */
		switch (ctx.view) {
		case TIMER_VIEW:
			status = handle_input_timer_view(tv, input);
			break;
		case HELP_VIEW: /* Nothing to handle here */
			break;
		case GRAPH_VIEW:
			status = handle_input_graph_view(gv, input);
			break;
		case SETTINGS_VIEW:
			status = handle_input_settings_view(sv, input);
			break;
		}
	}

	/* Print exit message and release resources */
	int cols, lines;
	get_window_size(&cols, &lines);

	render_text(2, lines - 2, "Press any key to exit ...\n");
	set_input_timeout(-1);
	get_keyboard_input();

	render_dispose();
	db_dispose();

	return 0;
}

int handle_input_global(struct AppContext *ctx, int *input_key)
{
	if (*input_key == ESC) {
		ctx->view = TIMER_VIEW;
		*input_key = IDLE_INPUT;
		return 0;
	}

	if (*input_key == 'q') {
		timer_stop(ctx->timer);
		save_active_inteval_time(ctx->timer, ctx->settings.min_seconds_to_save);
		return EXIT_APP;
	}

	if(ctx->view != TIMER_VIEW) /* Navigation to other views only from TIMER_VIEW */
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
int save_settings(struct AppContext *ctx, struct AppSettings new_settings)
{
	db_save_settings(new_settings);
	ctx->settings = new_settings;
	ctx->view = TIMER_VIEW;
	timer_update(ctx->timer);
	return 0;
};

int timer_update_callback(struct AppContext *ctx)
{	//TODO: Is this callback needed, pass timer_update directly ?
	timer_update(ctx->timer);
	return 1;
}

int pause_resume(struct AppContext *ctx)
{
	if (ctx->timer->stopped) {
		timer_start(ctx->timer);
	} else {
		timer_pause(ctx->timer);
		save_active_inteval_time(ctx->timer, ctx->settings.min_seconds_to_save);
	}

	return 1;
}

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

void termination_signal_handler(int sig_num) {
	if(s_ctx_ptr && s_ctx_ptr->settings.save_on_term_signal ) {
		if(!s_ctx_ptr->timer) {
			fprintf(stderr, "termination_signal_handler: timer ptr is NULL\n");
		}
		timer_stop(s_ctx_ptr->timer);
		save_active_inteval_time(s_ctx_ptr->timer, s_ctx_ptr->settings.min_seconds_to_save);
	}

	render_dispose();
	db_dispose();
	exit(0);
}

