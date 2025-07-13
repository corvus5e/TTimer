#ifndef _TTIMER_RENDER_H_
#define _TTIMER_RENDER_H_

#include "timer.h"

struct Texture {
	char *data;
	int width;
	int heigh;
};

enum UserInput {
	STOP_TIMER_INPUT,
	PAUSE_RESUME_TIMER_INPUT,
	HELP_INPUT,
	BACK_INPUT,
	GRAPH_INPUT,
	IDLE_INPUT,
};

void render_init();

void render_timer(const struct Timer *);

void render_help();

void render_graph();

void render_dispose();

enum UserInput get_user_input();

#endif
