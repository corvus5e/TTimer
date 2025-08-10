#ifndef _TTIMER_GRAPH_VIEW_H_
#define _TTIMER_GRAPH_VIEW_H_

#include "app_context.h"

#include "timer.h"

typedef int (*GetTimeIngervals)(struct TimeInterval time_period, struct TimeInterval **intervals, size_t *size);

struct graph_view *create_graph_view(struct AppContext *ctx, GetTimeIngervals);

int handle_input_graph_view(struct graph_view *, int input_key);

void render_graph_view(struct graph_view *);

void dispose_graph_view(struct graph_view *);

#endif
