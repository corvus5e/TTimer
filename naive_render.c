#include "render.h"

#include <stdio.h>
#include <stdlib.h>

#define BUF_LEN 10

extern const struct Texture _textures[];

void render_init(){ } // No need to init, do nothing

void render_dispose(){
	printf("\n\a");
}

void render(int n){

	system("clear");
	char buf[BUF_LEN];

	if(snprintf(&buf[0], BUF_LEN, "%d", n) < 0)
		return;

	int w = _textures[0].width; // assume size of textures are same
	int h = _textures[0].heigh;

	for(int i = 0; i < h; ++i){
		for(char *s = buf; *s; ++s){
			const struct Texture *t = &_textures[*s - '0'];
			const char *d = t->data;
			for(int j = 0; j < w; ++j)
				putchar(*(d + i*w + j));
		}
		putchar('\n');
	}
}

