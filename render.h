#ifndef _TTIMER_RENDER_H_
#define _TTIMER_RENDER_H_

struct Texture {
	char *data;
	int width;
	int heigh;
};

void render_init();

void render(int n);

void render_dispose();

#endif
