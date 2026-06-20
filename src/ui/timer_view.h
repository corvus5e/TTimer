#ifndef _TTIMER_TIMER_VIEW_H_
#define _TTIMER_TIMER_VIEW_H_

#include <app_context.h>
#include <ui/view.h>

//TODO: Move it to a separate file ?
struct Texture {
	char *data;
	int width;
	int heigh;
};

struct view **create_timer_view(struct AppContext *ctx, AppAction on_pause_resume, AppAction timer_update);

#endif
