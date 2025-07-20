#ifndef _TTIMER_LOG_H_
#define _TTIMER_LOG_H_

#include "timer.h"

int db_init();

int db_save_time(const struct TimeInterval);

int db_get_time(struct TimeInterval **time_ranges, size_t *size, int day_from_today);

int db_dispose();

#endif
