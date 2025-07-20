#ifndef _TTIMER_RENDER_H_
#define _TTIMER_RENDER_H_

#include "timer.h"
#include "settings.h"

struct Texture {
	char *data;
	int width;
	int heigh;
};

enum UserInput {
	Q_KEY,
	SPACE_KEY,
	R_KEY,
	H_KEY,
	J_KEY,
	K_KEY,
	L_KEY,
	ESC_KEY,
	G_KEY,
	S_KEY,
	UPDATE_INPUT,
	IDLE_INPUT,
};

void render_init();

void render_timer(const struct Timer *);

void render_help();

void render_graph(struct TimeInterval *, size_t);

void render_settings(const struct AppSettings *);

void render_dispose();

enum UserInput get_user_input();

#endif
