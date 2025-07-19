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
		for (int i = 4; i < COLS-4; ++i) {
			mvaddch(2, i, '-');
			mvaddch(LINES - 3, i, '-');
		}

		 for (int i = 3; i < LINES - 3; ++i) {
		 	mvaddch(i, 3, '|');
		 	mvaddch(i, COLS - 4, '|');
		 }

		 mvaddch(2, 3, '*');
		 mvaddch(2, COLS - 4, '*');
		 mvaddch(LINES - 3 , 3, '*');
		 mvaddch(LINES - 3, COLS - 4, '*');

		mvprintw(LINES - 4, 4, "Paused");
	}

	refresh();
}

void render_help()
{
	erase();
	mvprintw(1, 5, "TTimer");
	mvprintw(2, 5, "------");
	mvprintw(3, 5, "'Space' - pause/resume timer");
	mvprintw(3, 5, "'q'     - quit timer and save your time");
	mvprintw(4, 5, "'Esc'   - back to timer");
	mvprintw(5, 5, "'g'     - show time graph");
	mvprintw(6, 5, "'h'     - show this help");
	mvprintw(8, 5, "In graph view");
	mvprintw(9, 5, "-------------");
	mvprintw(10,5, "'h'     - switch 1 day backwards");
	mvprintw(11,5, "'l'     - switch 1 day forward");
	mvprintw(12,5, "'r'     - reset to today");
	mvprintw(13,5, "Window size: %d %d", LINES, COLS);

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
	long total = 0;

	for (int i = 0; i < n; ++i) {
		struct TimeRange *c = tr + i;

		// TODO: Trim start/end of day correctly

		buf = localtime(&c->start);
		int start_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;

		buf = localtime(&c->end);
		int end_offset = buf->tm_hour * GRAPH_ROWS + buf->tm_min / MINUTES_BLOCK;

		total += difftime(c->end, c->start);

		for (int k = start_offset; k <= end_offset; ++k)
			table[k / GRAPH_ROWS][k % GRAPH_ROWS] = 1;
	}


	if(tr && n > 0) {
		buf = localtime(&tr->end);
		mvprintw(1, 1, "%02d.%02d.%d",buf->tm_mday, buf->tm_mon, 1900 + buf->tm_year);
	}
	else
		mvprintw(1, 1, "--.--.----");


    	long hours = total/ 3600;
    	long minutes = (total % 3600) / 60;
    	long seconds = total % 60;

	mvprintw(2, 1, "Total time worked: %luh %lum %lus",  hours, minutes, seconds);

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
		return Q_KEY;
	case 'h':
		return H_KEY;
	case 'r':
		return R_KEY;
	case 'j':
		return J_KEY;
	case 'k':
		return K_KEY;
	case 'l':
		return L_KEY;
	case 'g':
		return G_KEY;
	case ESC:
		return ESC_KEY;
	case ' ':
		return SPACE_KEY;
	case KEY_RESIZE:
		return H_KEY; // TODO: handle resize
	}

	return IDLE_INPUT; // could be treated as use timer "tick"
}

