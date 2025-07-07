#include "io.h"

#include <ncurses.h>

#include "timer_state.h"

#define BUF_LEN 10

extern const struct Texture _textures[];

int max_row, max_col;

void get_offset(int* y, int *x, int w, int h);

void render_init(){
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	getmaxyx(stdscr,max_row, max_col);
}

void render(const struct TimerState *ts)
{
	erase();

	char buf[BUF_LEN];
	int len = 0;

	int seconds = ts->time_elapsed_sec % 60;
	int minutes = ts->time_elapsed_sec / 60;
	int hours = minutes / 60;

	if ((len = snprintf(&buf[0], BUF_LEN, "%02d:%02d:%02d", hours, minutes, seconds)) < 0)
		return;

	int y_offset, x_offset;
	get_offset(&y_offset, &x_offset, 48/*width of clock*/, 7/*height of clock*/);

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
		mvprintw(max_row - 1, 0, "Paused");
	}

	refresh();
}

void render_dispose(){
	mvaddstr(max_row - 1, 0, "Press any key to exit ...");
	refresh();
	getch();
	endwin();
}

void get_offset(int* y, int *x, int w, int h){
	//TODO: Add out of bound checks
	*x = (max_col - 1 - w) / 2;
	*y = (max_row - 1  - h) / 2;
}

enum UserInput get_user_input()
{
	switch (getch()) {
	case 'q':
		return STOP_TIMER;
	case ' ':
		return PAUSE_RESUME_TIMER;
	}
	return 0;
}

