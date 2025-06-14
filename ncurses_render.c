#include "render.h"

#include <ncurses.h>

#define BUF_LEN 10

extern const struct Texture _textures[];

int max_row, max_col;

void get_offset(int* y, int *x, int w, int h);

void render_init(){
	initscr();
	noecho();
	curs_set(0);
	getmaxyx(stdscr,max_row, max_col);
}

void render(int n){
	erase();

	char buf[BUF_LEN];
	int len = 0;

	if((len = snprintf(&buf[0], BUF_LEN, "%d", n)) < 0)
		return;

	int w = _textures[0].width; // assume size of textures are same
	int h = _textures[0].heigh;
	int column_shift = 0;

	int y_offset, x_offset;
	get_offset(&y_offset, &x_offset, len * w, h);

	for(char *s = buf; *s; ++s){
		const struct Texture *t = &_textures[*s - '0'];
		const char *d = t->data;

		for(int i = 0; i < h; ++i){
			for(int j = 0; j < w; ++j){
				mvaddch(i + y_offset, j + column_shift + x_offset, *(d + i*w + j));
			}
		}
		column_shift += w;
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

