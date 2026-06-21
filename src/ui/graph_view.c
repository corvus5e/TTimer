#include "graph_view.h"

#include <stdio.h>
#include <stdlib.h>
#include "HomeTUI/home_tui.h"
#include "char_codes.h"
#include "app_context.h"

#define SECS_DAY 86400

#define MINUTES_BLOCK 5
#define GRAPH_ROWS 12 /* The hours is devided on MINUTES_BLOCK */
#define GRAPH_COLS 24 /* Display all 24 hours*/
#define COLS_WIDTH 3

struct graph_view {
	struct view *view;
	struct AppContext *ctx;
	GetTimeIngervals get_intervals;
	int day_shift;
};

int handle_input_graph_view(struct view *, int input_key);

void render_graph_view(const struct view *);

void dispose_graph_view(struct view *);


struct view *create_graph_view(struct AppContext *ctx, GetTimeIngervals get_intervals)
{
	struct graph_view *graph_view = (struct graph_view *)malloc(sizeof(struct graph_view));

	if(!graph_view)
		return NULL;

	graph_view->view = view_create(render_graph_view, handle_input_graph_view, dispose_graph_view, graph_view);

	if(!graph_view->view) {
		return NULL;
	}

	if(!ctx || !get_intervals )
		return NULL;

	graph_view->ctx = ctx;
	graph_view->day_shift = 0;
	graph_view->get_intervals = get_intervals;

	return graph_view->view;
}

int handle_input_graph_view(struct view *v, int input)
{
	struct graph_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return IGNORED;
	}

	switch (input) {
	case 'r':
		view->day_shift = 0;
		return PROCESSED;
	case 'h':
		view->day_shift -= 1;
		return PROCESSED;
	case 'l':
		view->day_shift += 1;
		return PROCESSED;
	}

	return IGNORED;
}

void render_graph_view(const struct view * v)
{
	struct graph_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return;
	}

	struct TimeInterval *time_intervals;
	size_t n;
	struct TimeInterval day_interval = get_day_interval(time(NULL), view->day_shift);

	if(!view->get_intervals(day_interval, &time_intervals, &n)) {
		fprintf(stderr, "graph_view: no data retrieved to render\n");
		return;
	}

	int cols, lines;
	get_window_size(&cols, &lines);

	render_clear();

	for (int min = 0; min < GRAPH_ROWS; ++min)
		render_ftext(1, lines - 3 - min, "%d", 5 * (min + 1));

	for (int h = 0; h < GRAPH_COLS; ++h) {
		render_ftext(5 + COLS_WIDTH * h + 1, lines - 1, "%d", h);
	}

	struct tm *buf;
	long total = 0;

	for (int i = 0; i < n; ++i) {
		struct TimeInterval *c = time_intervals + i;

		buf = localtime(&c->start);
		int start_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;
		int start_day = buf->tm_mday;

		buf = localtime(&c->end);
		if(start_day < buf->tm_mday){
			buf->tm_hour = 23;
			buf->tm_min = 59;
			buf->tm_mday -= 1;
		}
		int end_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;

		long curr_diff = difftime(c->end, c->start);
		total += curr_diff;

		//TODO: Do better grouping
		short fg = curr_diff > 5*60 ? COLOR_MEDIUM_GREEN : COLOR_DARK_GREEN;
		set_color(fg, -1);

		for (int k = start_offset; k <= end_offset; ++k)
			render_cell(5 + COLS_WIDTH * (k / GRAPH_ROWS) + 1, lines - 3 - (k % GRAPH_ROWS), GRAPH_SHORT_CELL);

		reset_colors();
	}


	buf = localtime(&day_interval.start); // Get date
	render_ftext(1, 1, "%02d.%02d.%d",buf->tm_mday, buf->tm_mon + 1, 1900 + buf->tm_year);


    	long hours = total/ 3600;
    	long minutes = (total % 3600) / 60;
    	long seconds = total % 60;

	render_ftext(1, 2, "Total time worked: %luh %lum %lus",  hours, minutes, seconds);

	render_update();
}


void dispose_graph_view(struct view *v)
{
	struct graph_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return;
	}

	free(view);
}
