#ifndef _TTIMER_SETTINGS_VIEW_H_
#define _TTIMER_SETTINGS_VIEW_H_

#include "app_context.h"

typedef int (*SaveSettings)(struct AppContext *, struct AppSettings new_settings);
typedef int (*GetSettings)(struct AppSettings * settings);

struct view *create_settings_view(struct AppContext *, SaveSettings, GetSettings);

#endif
