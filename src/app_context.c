#include "app_context.h"

struct AppContext create_app_context() {
	struct AppContext ctx;
	ctx.current_view = NULL;
	ctx.current_view_focused = 0;
	ctx.day_shift = 0;
	ctx.textures = NULL;
	ctx.idle_paused = 0;

	timer_reset(&ctx.timer);

	return ctx;
}
