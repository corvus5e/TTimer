#ifndef _TTIMER_LOG_H_
#define _TTIMER_LOG_H_

#include "timer.h"

int db_init();

int db_save_time(const struct Timer*);

int db_get_time(TimeGraph out);

int db_dispose();

#endif
