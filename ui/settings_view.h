#ifndef _TTIMER_SETTINGS_VIEW_H_
#define _TTIMER_SETTINGS_VIEW_H_

#include "app_context.h"

struct settings_view *create_settings_view(struct AppContext *ctx, AppAction save_settings);

int handle_input_settings_view(struct settings_view *, int input_key);

void render_settings_view(struct settings_view *);

void dispose_settings_view(struct settings_view *);

#endif
