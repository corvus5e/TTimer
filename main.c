/*
 * TTimer - timer, which prints time to terminal
*/

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <threads.h>

#include "io.h"

static int stopped = 0;

int run_timer(void *arg);

int main(int argc, char *argv[]){
	int secs = 0;
	if (argc < 2 || (secs = atoi(argv[1])) <= 0)
		secs = 60;
	int countUp = 0;

	render_init();

	thrd_t timer_thread;

	if(thrd_create(&timer_thread, run_timer, NULL) != thrd_success){
		fputs("Failed to create timer thread\n", stderr);
		exit(1);
	}

	while(!(stopped = (getchar() == 'q')))
		usleep(100000); // 100 ms

	render_dispose();

	thrd_join(timer_thread, NULL);

	return 0;
}

int run_timer(void *arg)
{
	time_t start = time(NULL);

	while (!stopped) {
		render((int)difftime(time(NULL), start));
		usleep(100000); // 100 ms
	}

	return 0;
}
