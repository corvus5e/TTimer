#ifndef _TTIMER_TIMER_VIEW_H_
#define _TTIMER_TIMER_VIEW_H_

#include "app_context.h"

//TODO: Move it to a separate file ?
struct Texture {
	char *data;
	int width;
	int heigh;
};

struct timer_view *create_timer_view(struct AppContext *ctx, AppAction on_pause_resume, AppAction timer_update);

int handle_input_timer_view(struct timer_view *, int input_key);

void render_timer_view(struct timer_view *);

void dispose_timer_view(struct timer_view *);

#endif
