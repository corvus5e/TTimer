#include "graph_view.h"

#include <stdio.h>
#include <stdlib.h>
#include "HomeTUI/home_tui.h"

#define SECS_DAY 86400

#define MINUTES_BLOCK 5
#define GRAPH_ROWS 12 /* The hours is devided on MINUTES_BLOCK */
#define GRAPH_COLS 24 /* Display all 24 hours*/
#define COLS_WIDTH 3

struct graph_view {
	struct AppContext *ctx;
	GetTimeIngervals get_intervals;
	int day_shift;
};

struct graph_view *create_graph_view(struct AppContext *ctx, GetTimeIngervals get_intervals)
{
	struct graph_view *view = (struct graph_view *)malloc(sizeof(struct graph_view));

	if(!view)
		return NULL;

	if(!ctx || !get_intervals )
		return NULL;

	view->ctx = ctx;
	view->day_shift = 0;
	view->get_intervals = get_intervals;

	return view;
}

int handle_input_graph_view(struct graph_view *view, int input)
{
	switch (input) {
	case 'r':
		view->day_shift = 0;
		return 1;
	case 'h':
		view->day_shift -= 1;
		return 1;
	case 'l':
		view->day_shift += 1;
		return 1;
	}

	return 0;
}

void render_graph_view(struct graph_view * view)
{
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

	char table[GRAPH_COLS][GRAPH_ROWS] = {0};

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

		total += difftime(c->end, c->start);

		for (int k = start_offset; k <= end_offset; ++k)
			table[k / GRAPH_ROWS][k % GRAPH_ROWS] = 1;
	}


	buf = localtime(&day_interval.start); // Get date
	render_ftext(1, 1, "%02d.%02d.%d",buf->tm_mday, buf->tm_mon + 1, 1900 + buf->tm_year);


    	long hours = total/ 3600;
    	long minutes = (total % 3600) / 60;
    	long seconds = total % 60;

	render_ftext(1, 2, "Total time worked: %luh %lum %lus",  hours, minutes, seconds);

	for (int i = 0; i < GRAPH_COLS; ++i) {
		for (int j = 0; j < GRAPH_ROWS; ++j) {
			if (table[i][j] > 0)
				render_cell(5 + COLS_WIDTH * i + 1, lines - 3 - j, '#');
		}
	}

	render_update();
}


void dispose_graph_view(struct graph_view *view)
{
	free(view);
}
