#ifndef _TTIMER_GRAPH_VIEW_H_
#define _TTIMER_GRAPH_VIEW_H_

#include <timer/timer.h>
#include <ui/view.h>

struct AppContext;

typedef int (*GetTimeIngervals)(struct TimeInterval time_period, struct TimeInterval **intervals, size_t *size);

struct view *create_graph_view(struct AppContext *ctx, GetTimeIngervals);

#endif
