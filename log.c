#include "log.h"

#include <stdio.h>

int log_timer(const struct Timer* ts)
{
	FILE *f;
	if ((f = fopen("timer_log.log", "a")) == NULL) {
		fputs("Failed to create log file.", stderr);
		return -1;
	}

	int m = (ts->time_elapsed_sec / 60) % 60;
	int s = ts->time_elapsed_sec % 60;
	int h = ts->time_elapsed_sec / 3600;

	char buff[100];
	sprintf(buff, "%s%d:%d:%d\n", asctime(localtime(&ts->start)), h, m, s);

	fputs(buff, f);

	fclose(f);

	return 0;
}
