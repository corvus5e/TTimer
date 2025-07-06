#ifndef _TTIMER_RENDER_H_
#define _TTIMER_RENDER_H_

#include "timer_state.h"

struct Texture {
	char *data;
	int width;
	int heigh;
};

enum UserInput {
	STOP_TIMER = 1,
	PAUSE_RESUME_TIMER = 2,
};

void render_init();

void render(const struct TimerState *);

void render_dispose();

enum UserInput get_user_input();

#endif
