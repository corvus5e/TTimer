#ifndef _TTIMER_TIMER_H_
#define _TTIMER_TIMER_H_

struct Timer {
	int ticks; /* The amount of ticks to do*/
	int curr;  /* Current value of counter */
	int freq;  /* Frequency of counter update, in microseconds */
	short direction; /* 0 - count down, non zero - count up */
};

typedef void (*onTick)(int curr);

struct Timer create_timer(int ticks, int freq, int direction);
         int start_timer(struct Timer * const t, onTick update);

#endif
