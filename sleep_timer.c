#include "timer.h"

#include <unistd.h>

struct Timer create_timer(int ticks, int freq, int direction)
{
	struct Timer t = {ticks, 0, freq, direction};
	return t;
}

int start_timer(struct Timer * const t, onTick update)
{
	int start, end, inc;

	if(t->direction == 0){ /* count down */
		start = t->ticks;
		end = 0;
		inc = -1;
	}
	else { /* count up */
		start = 0;
		end = t->ticks;
		inc = 1;
	}

	for(int i = start; i != end; i += inc){
		update(i);
		usleep(t->freq);
	}

	update(end);

	return 0;
}

