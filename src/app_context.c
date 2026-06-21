#include "app_context.h"

struct AppContext create_app_context() {
	struct AppContext ctx;
	ctx.current_view = NULL;
	ctx.current_view_focused = 0;
	ctx.day_shift = 0;
	ctx.textures = NULL;
	ctx.idle_paused = 0;

	// Defualt settings
	ctx.settings.stopped_on_app_start = 1;
	ctx.settings.stop_after_min = -1; // Do not stop at all
	ctx.settings.min_seconds_to_save = 0;
	ctx.settings.save_on_term_signal = 0;
	ctx.settings.idle_pause_time = -1; // Do not pause on idle time

	timer_reset(&ctx.timer);

	return ctx;
}
