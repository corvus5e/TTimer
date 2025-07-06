#include "log.h"

#include <stdio.h>

int log_timer(const struct TimerState* ts)
{
	FILE *f;
	if ((f = fopen("timer_log.log", "a")) == NULL) {
		fputs("Failed to create log file.", stderr);
		return -1;
	}

	int min = ts->time_elapsed_sec / 60;
	int sec = ts->time_elapsed_sec % 60;

	char buff[20];
	sprintf(buff, "%s%d:%d:%d\n", asctime(localtime(&ts->start)), 0, min, sec);

	fputs(buff, f);

	fclose(f);

	return 0;
}
