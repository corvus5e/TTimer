#ifndef _TTIMER_DRAW_H_
#define _TTIMER_DRAW_H_

#include "timer.h"
#include "app_context.h"

struct Texture {
	char *data;
	int width;
	int heigh;
};

void render_timer(const struct Timer *, int x, int y);

void render_help();

void render_graph(struct TimeInterval scope, struct TimeInterval *, size_t);

void render_settings(const struct AppSettings *);

#endif
