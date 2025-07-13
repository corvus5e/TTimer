#include "io.h"

#include <ncurses.h>

#include "timer.h"

#define GETCH_TIMEOUT_MS 1000

#define BUF_LEN 10
#define ESC 27

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

void render_graph()
{
	const int col_w = 3;

	// TODO: place holder for graph render
	erase();
	for (int min = 0; min < 12; ++min)
		mvprintw(LINES - 3 - min, 1, "%d", 5 * (min+1));

	for (int h = 0; h < 24; ++h) {
		mvprintw(LINES - 1, 5 + col_w * h + 1, "%d", h);
	}

	// TODO: input data minutes timer was working for each hour
	int min[24] = {10, 0, 0, 0, 0, 0, 0, 0, 30, 60, 40, 0,
		       10, 0, 0, 0, 0, 0, 0, 0, 0,  30, 60, 40};

	for (int i = 0; i < 24; ++i) {
		int blocks = min[i] / 5;
		for (int k = 0; k < col_w; ++k)
			for (int j = 1; j <= blocks; ++j) {
				mvaddch(LINES - 2 - j, 5 + col_w * i + k, '#');
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

