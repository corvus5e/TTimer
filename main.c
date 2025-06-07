/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdlib.h>

#include "timer.h"
#include "render.h"

int main(int argc, char *argv[]){
	int secs = 0;
	if(argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;
	int countDown = 1;

	render_init();

	struct Timer t = create_timer(secs, 1000000);

	start_timer(&t, render); // Blocks until finishes

	render_dispose();

	return 0;
}

