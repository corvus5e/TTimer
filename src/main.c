/*
 * TTimer - timer, which prints time to terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <HomeTUI/home_tui.h>
#include <os_utils/os_utils.h>
#include <app_context.h>
#include <timer/timer.h>

#include <db/db.h>

#include <ui/view.h>
#include <ui/settings_view.h>
#include <ui/graph_view.h>
#include <ui/timer_view.h>
#include <ui/help_view.h>

#define EXIT_APP 2
#define VIEW_UPDATED 3

int handle_input_global(struct AppContext *context, int *altered_input);

/* App actions */
int save_settings(struct AppContext *ctx, struct AppSettings new_settings);

int timer_update_callback(struct AppContext *ctx);

int pause_resume(struct AppContext *ctx);

int get_time_intervals(struct TimeInterval time_period, struct TimeInterval **intervals, size_t *size);

/* Saves time if last active interval is valid (timer is paused or stopped)
 * And interval is not zero */
int save_active_inteval_time(struct Timer *, int min_seconds_to_save);

int handle_idle_time(struct AppContext *ctx);

void termination_signal_handler(int);

/* Global pointer to current app context*/
struct AppContext *s_ctx_ptr = NULL;
struct view *help_view = NULL;
struct view *settings_view = NULL;
struct view *graph_view = NULL;
struct view *timer_view = NULL;

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

	struct AppContext ctx = create_app_context();
	s_ctx_ptr = &ctx;

	const struct TextureAtlas *textures = load_figlet_texture("src/HomeTUI/assets/mono12.txt");
	if (!textures) {
		fprintf(stderr, "Failed to load fonts\n");
	}
	else {
		get_texture_dims(textures, &ctx.n, &ctx.w, &ctx.h);
		ctx.textures = textures;
	}


	if (db_get_settings(&ctx.settings) != 0) {
		fprintf(stderr, "Failed to read settings. Using defaults\n");
		ctx.settings.stopped_on_app_start = 1;
		ctx.settings.stop_after_min = -1; // Do not stop at all
		ctx.settings.min_seconds_to_save = 0;
		ctx.settings.save_on_term_signal = 0;
	}

	help_view = create_help_view();
	if (!help_view) {
		fprintf(stderr, "Failed to create help view\n");
		return 1;
	}

	settings_view = create_settings_view(&ctx, save_settings, db_get_settings);
	if (!settings_view) {
		fprintf(stderr, "Failed to create settings view\n");
		return 1;
	}

	graph_view = create_graph_view(&ctx, get_time_intervals);
	if (!graph_view) {
		fprintf(stderr, "Failed to create graph view\n");
		return 1;
	}

	timer_view = create_timer_view(&ctx, pause_resume, timer_update_callback);
	if (!timer_view) {
		fprintf(stderr, "Failed to create graph view\n");
		return 1;
	}

	if (!ctx.settings.stopped_on_app_start)
		timer_start(&ctx.timer);

	ctx.current_view = timer_view;

	int input = IDLE_INPUT;

	for (int event = VIEW_UPDATED;;) {
		/* Render views */
		if (event) {
			view_render(ctx.current_view);
		}

		input = get_keyboard_input();

		//TODO: Think on the return value from this handler
		event = handle_input_global(&ctx, &input);

		if (event == EXIT_APP)
			break;

		if (event == VIEW_UPDATED)
			continue;

		event = view_process_input(ctx.current_view, input);
	}

	/* Print exit message and release resources */
	int cols, lines;
	get_window_size(&cols, &lines);

	render_text(2, lines - 2, "Press any key to exit ...\n");
	set_input_timeout(-1);
	get_keyboard_input();

	render_dispose();
	db_dispose();
	dispose_os_resources();

	return 0;
}

int handle_input_global(struct AppContext *ctx, int *input_key)
{
	if (*input_key == ESC) {
		ctx->current_view = timer_view;
		*input_key = IDLE_INPUT;
		return VIEW_UPDATED;
	}

	if (*input_key == 'q') {
		timer_stop(&ctx->timer);
		save_active_inteval_time(&ctx->timer, ctx->settings.min_seconds_to_save);
		return EXIT_APP;
	}

	if(ctx->current_view != timer_view) /* Navigation to other views only from TIMER_VIEW */
		return 0;

	switch (*input_key) {
	case 'g':
		ctx->current_view = graph_view;
		*input_key = IDLE_INPUT;
		return VIEW_UPDATED;
	case 'h':
		ctx->current_view = help_view;
		*input_key = IDLE_INPUT;
		return VIEW_UPDATED;
	case 's':
		ctx->current_view = settings_view;
		*input_key = IDLE_INPUT;
		return VIEW_UPDATED;
	}

	return handle_idle_time(ctx);
}

/* View callbacks */
int save_settings(struct AppContext *ctx, struct AppSettings new_settings)
{
	db_save_settings(new_settings);
	ctx->settings = new_settings;
	ctx->current_view = timer_view;
	timer_update(&ctx->timer);
	return 0;
};

int timer_update_callback(struct AppContext *ctx)
{	//TODO: Is this callback needed, pass timer_update directly ?
	timer_update(&ctx->timer);
	return 1;
}

int pause_resume(struct AppContext *ctx)
{
	ctx->idle_paused = 0; // Reset idle-paused state on manual interaction
	if (ctx->timer.stopped) {
		timer_start(&ctx->timer);
	} else {
		timer_pause(&ctx->timer);
		save_active_inteval_time(&ctx->timer, ctx->settings.min_seconds_to_save);
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

int handle_idle_time(struct AppContext *ctx) {
	if (ctx->timer.stopped) {
		return 0;
	}

	const float limit = 5.0f;
	float it = idle_time();
	if(!ctx->idle_paused) {
		if (!ctx->timer.paused && it > limit) {
			pause_resume(ctx);
			ctx->idle_paused = 1;
			return VIEW_UPDATED;
		}
	}
	else {
		if (ctx->timer.paused && it < limit) {
			pause_resume(ctx);
			ctx->idle_paused = 0;
			return VIEW_UPDATED;
		}
	}

	return 0;
}

void termination_signal_handler(int sig_num) {
	if(s_ctx_ptr && s_ctx_ptr->settings.save_on_term_signal ) {
		timer_stop(&s_ctx_ptr->timer);
		save_active_inteval_time(&s_ctx_ptr->timer, s_ctx_ptr->settings.min_seconds_to_save);
	}

	render_dispose();
	db_dispose();
	exit(0);
}

