#include "io.h"

#include <ncurses.h>
#include <string.h>
#include <time.h>

#include "timer.h"

#define GETCH_TIMEOUT_MS 1000

#define BUF_LEN 10
#define ESC 27

#define SECS_DAY 86400

#define MINUTES_BLOCK 5
#define GRAPH_ROWS 12 /* The hours is devided on MINUTES_BLOCK */
#define GRAPH_COLS 24 /* Display all 24 hours*/

#define COLS_WIDTH 3

extern const struct Texture _textures[];

static int y_offset, x_offset;

void get_offset(int* y, int *x, int w, int h);

void render_init(){
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	get_offset(&y_offset, &x_offset, 48/*width of clock*/, 7/*height of clock*/);
	timeout(GETCH_TIMEOUT_MS);
}

void render_timer(const struct Timer*ts)
{
	erase();

	char buf[BUF_LEN];
	int len = 0;

	int seconds = ts->time_elapsed_sec % 60;
	int minutes = (ts->time_elapsed_sec / 60) % 60;
	int hours = ts->time_elapsed_sec / 3600;

	if ((len = snprintf(&buf[0], BUF_LEN, "%02d:%02d:%02d", hours, minutes, seconds)) < 0)
		return;

	int column_shift = 0;

	for (char *s = buf; *s; ++s) {

		const struct Texture *t = &_textures[*s == ':' ? 10 : *s - '0'];

		const char *d = t->data;

		for (int i = 0; i < t->heigh; ++i) {
			for (int j = 0; j < t->width; ++j) {
				mvaddch(i + y_offset,
					j + column_shift + x_offset,
					*(d + i * t->width + j));
			}
		}

		column_shift += t->width;
	}

	if (ts->paused) {
		mvprintw(LINES - 1, 0, "Paused");
	}

	refresh();
}

void render_help()
{
	erase();
	mvprintw(1, 5, "TTimer");
	mvprintw(3, 5, "'Space' - pause/resume timer");
	mvprintw(3, 5, "'q'     - quit timer and save your time");
	mvprintw(4, 5, "'Esc'   - back to timer");
	mvprintw(5, 5, "'g'     - show time graph");
	mvprintw(6, 5, "'h'     - show this help");
	mvprintw(7, 5, "Window size: %d %d", LINES, COLS);

	refresh();
}

void render_graph(struct TimeRange *tr, size_t n)
{
	erase();

	char table[GRAPH_COLS][GRAPH_ROWS] = {0};

	for (int min = 0; min < GRAPH_ROWS; ++min)
		mvprintw(LINES - 3 - min, 1, "%d", 5 * (min + 1));

	for (int h = 0; h < GRAPH_COLS; ++h) {
		mvprintw(LINES - 1, 5 + COLS_WIDTH * h + 1, "%d", h);
	}

	struct tm *buf;
	struct tm total = {0};

	for (int i = 0; i < n; ++i) {
		struct TimeRange *c = tr + i;

		// TODO: Trim start/end of day correctly

		buf = localtime(&c->start);

		total.tm_mday -= buf->tm_mday;
		total.tm_hour -= buf->tm_hour;
		total.tm_min -= buf->tm_min;
		total.tm_sec -= buf->tm_sec;
		int start_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;

		buf = localtime(&c->end);

		total.tm_mday += buf->tm_mday;
		total.tm_hour += buf->tm_hour;
		total.tm_min += buf->tm_min;
		total.tm_sec += buf->tm_sec;

		mktime(&total);

		int end_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;

		for (int k = start_offset; k <= end_offset; ++k)
			table[k / GRAPH_ROWS][k % GRAPH_ROWS] = 1;
	}

	time_t now = time(NULL);
	buf = localtime(&now);
	mvprintw(1, 1, "%d.%d.%d",buf->tm_mday,buf->tm_mon, buf->tm_year);
	mvprintw(2, 1, "Total time worked: %dh %dm %ds",  total.tm_hour, total.tm_min,
		 total.tm_sec);

	for (int i = 0; i < GRAPH_COLS; ++i) {
		for (int j = 0; j < GRAPH_ROWS; ++j) {
			if (table[i][j] > 0)
				mvaddch(LINES - 3 - j, 5 + COLS_WIDTH * i + 1, '#');
		}
	}

	refresh();
}

void render_dispose()
{
	timeout(-1);
	mvaddstr(LINES - 1, 0, "Press any key to exit ...");
	refresh();
	getch();
	endwin();
}

void get_offset(int *y, int *x, int w, int h)
{
	// TODO: Add out of bound checks
	*x = (COLS - 1 - w) / 2;
	*y = (LINES - 1 - h) / 2;
}

enum UserInput get_user_input()
{
	switch (getch()) {
	case 'q':
		return STOP_TIMER_INPUT;
	case 'h':
		return HELP_INPUT;
	case 'g':
		return GRAPH_INPUT;
	case ESC:
		return BACK_INPUT;
	case ' ':
		return PAUSE_RESUME_TIMER_INPUT;
	case KEY_RESIZE:
		return HELP_INPUT; // TODO: handle resize
	}

	return IDLE_INPUT; // could be treated as use timer "tick"
}

