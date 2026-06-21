#ifndef _TTIMER_APP_CONTEXT_H_
#define _TTIMER_APP_CONTEXT_H_

#include "timer/timer.h"

struct AppSettings {
	int stopped_on_app_start;
	int stop_after_min;
	int min_seconds_to_save;
	int save_on_term_signal;
	int idle_pause_time;
};

struct view;

struct AppContext {
	struct Timer timer;
	struct view *current_view;
	int current_view_focused;
	struct AppSettings settings;
	int idle_paused;
	int day_shift; //TODO: Move to graph_view ?
	const struct TextureAtlas *textures;
	int w, h, n; //TODO: Replace with Texture struct
};

typedef int (*AppAction)(struct AppContext *);

struct AppContext create_app_context();

#endif
