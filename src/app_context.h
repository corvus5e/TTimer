#ifndef _TTIMER_APP_CONTEXT_H_
#define _TTIMER_APP_CONTEXT_H_

enum AppView { TIMER_VIEW, HELP_VIEW, GRAPH_VIEW, SETTINGS_VIEW};

struct AppSettings {
	int stopped_on_app_start;
	int stop_after_min;
	int min_seconds_to_save;
	int save_on_term_signal;
};

struct AppContext {
	struct Timer *timer;
	enum AppView view;
	struct AppSettings settings;
	int idle_paused;
	int day_shift; //TODO: Move to graph_view ?
	const struct TextureAtlas *textures;
	int w, h, n; //TODO: Replace with Texture struct
};

typedef int (*AppAction)(struct AppContext *);

#endif
