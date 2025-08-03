#include "draw.h"

#include "HomeTUI/home_tui.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "timer.h"

#define GETCH_TIMEOUT_MS 1000

#define BUF_LEN 10
#define ESC 27
#define KEY_RESIZE 0632

#define SECS_DAY 86400

#define MINUTES_BLOCK 5
#define GRAPH_ROWS 12 /* The hours is devided on MINUTES_BLOCK */
#define GRAPH_COLS 24 /* Display all 24 hours*/

#define COLS_WIDTH 3

extern const struct Texture _textures[];

void render_timer(const struct Timer *ts, int x, int y)
{
	render_clear();

	char buf[BUF_LEN];
	int len = 0;

	int seconds = ts->active_elapsed_time % 60;
	int minutes = (ts->active_elapsed_time / 60) % 60;
	int hours = ts->active_elapsed_time / 3600;

	if ((len = snprintf(&buf[0], BUF_LEN, "%02d:%02d:%02d", hours, minutes, seconds)) < 0)
		return;

	int column_shift = 0;

	for (char *s = buf; *s; ++s) {

		const struct Texture *t = &_textures[*s == ':' ? 10 : *s - '0'];

		const char *d = t->data;

		for (int i = 0; i < t->heigh; ++i) {
			for (int j = 0; j < t->width; ++j) {
				render_cell(j + column_shift + x, i + y,
					*(d + i * t->width + j));
			}
		}

		column_shift += t->width;
	}

	int cols, lines;
	get_window_size(&cols, &lines);

	if (ts->paused) {
		for (int i = 4; i < cols - 4; ++i) {
			render_cell(i, 2, '-');
			render_cell(i, lines - 3, '-');
		}

		 for (int i = 3; i < lines - 3; ++i) {
		 	render_cell(3, i, '|');
		 	render_cell(cols - 4, i, '|');
		 }

		 render_cell(3, 2, '*');
		 render_cell(cols - 4, 2, '*');
		 render_cell(3, lines - 3, '*');
		 render_cell(cols - 4, lines - 3, '*');

		render_text(4, lines - 4, "Paused");
	} else if (ts->stopped && ts->active_elapsed_time == 0 /*Initial start*/) {
		render_text(4, lines - 4, "Press space to start");
	}

	render_update();
}

void render_help()
{
	int cols, lines;
	get_window_size(&cols, &lines);

	render_clear();

	render_text(5, 1, "TTimer");
	render_text(5, 2, "------");
	render_text(5, 3, "'Space' - pause/resume timer");
	render_text(5, 3, "'q'     - quit timer and save your time");
	render_text(5, 4, "'Esc'   - back to timer");
	render_text(5, 5, "'g'     - show time graph");
	render_text(5, 6, "'s'     - show settings");
	render_text(5, 7, "'h'     - show this help");
	render_text(5, 9, "In graph view");
	render_text(5, 10, "-------------");
	render_text(5, 11, "'h'     - switch 1 day backwards");
	render_text(5, 12, "'l'     - switch 1 day forward");
	render_text(5, 13, "'r'     - reset to today");
	render_ftext(5, 14, "Window size: %d %d", lines, cols);

	render_update();
}

void render_graph(struct TimeInterval day_interval, struct TimeInterval *tr, size_t n)
{
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
		struct TimeInterval *c = tr + i;

		buf = localtime(&c->start);
		int start_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;

		buf = localtime(&c->end);
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

void render_settings(const struct AppSettings *s)
{
	render_clear();
	render_text(3, 1, "Settings");
	render_text(3, 2, "--------");
	render_ftext(3, 3, "Stopped on app start: %s\n", s->stopped_on_app_start ? "ON" : "OFF");
	render_ftext(3, 4, "Minimus seconds to save: %d\n", s->min_seconds_to_save);
	render_update();
}

