#include "timer.h"

#include <unistd.h>

struct Timer create_timer(int ticks, int freq)
{
	struct Timer t = {ticks, 0, freq};
	return t;
}

int start_timer(struct Timer * const t, onTick update)
{
	for(int i = 0; i < t->ticks; ++i){
		update(i);
		usleep(t->freq);

	}

	update(t->ticks);

	return 0;
}

