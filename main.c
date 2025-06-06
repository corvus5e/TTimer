/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "render.h"

int main(int argc, char *argv[]){
	int secs = 0;
	if(argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;

	render_init();

	for(int i = 0; i < secs; ++i){
		render(i);
		fflush(stdout);
		usleep(1000000);
	}

	render(secs);

	render_dispose();

	return 0;
}

