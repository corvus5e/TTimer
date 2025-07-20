#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sqlite3/sqlite3.h"
#include "timer.h"

#define DB_NAME "time_db.db"
#define TBL_NAKE "tbl1"

#define SECS_DAY 86400

#define MAX_RANGES_PER_DAY 100 // TODO: Make dynamic later

static sqlite3 *_db;

struct TimeRangeArray {
	struct TimeRange data[MAX_RANGES_PER_DAY];
	size_t size;
};

static struct TimeRangeArray _time_ranges;

static int callback(void *arg, int argc, char **argv, char **azColName)
{
	if(argc != 2) {
		fprintf(stderr, "Error in db: Unexpected number of columns\n");
		return -1;
	}

	if(_time_ranges.size >= MAX_RANGES_PER_DAY) {
		fprintf(stderr, "Error in db: Query result exceeded TimeRangeArray size\n");
		return -1;
	}

	_time_ranges.data[_time_ranges.size].start = (time_t)atoi(argv[0]);
	_time_ranges.data[_time_ranges.size].end = (time_t)atoi(argv[1]);
	_time_ranges.size++;

	return 0;
}

int db_init()
{
	int status = sqlite3_open(DB_NAME, &_db);

	if (status) {
		fprintf(stderr, "Failed to open a database %s\n", DB_NAME);
		sqlite3_close(_db);
		return 1;
	}

	const char *sql = "create table if not exists tbl1 (start int, end int);";
	char *errmsg;

	status = sqlite3_exec(_db, sql, NULL, NULL, &errmsg);
	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int db_dispose()
{
	sqlite3_close(_db);
	return 0;
}

int db_save_time(const struct Timer *timer)
{
	if(!_db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	char *query = sqlite3_mprintf("insert into tbl1 (start, end) \
			values\
			(%lu, %lu);", timer->start,
			timer->start + timer->time_elapsed_sec - timer->total_paused_time_sec);

	char *errmsg;

	int status = sqlite3_exec(_db, query, NULL, NULL, &errmsg);

	sqlite3_free(query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int db_get_time(struct TimeRange **time_ranges, size_t *size, int day_shift)
{
	if(!_db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	_time_ranges.size = 0;

	time_t now = time(NULL);
	struct tm *local_time = localtime(&now);

	// Clear todays time
	local_time->tm_sec = 0;
	local_time->tm_min = 0;
	local_time->tm_hour = 0;
	local_time->tm_mday += day_shift;

	time_t start = mktime(local_time);

	local_time->tm_mday += 1;
	time_t end = mktime(local_time);

	char *query = sqlite3_mprintf("select start, end from tbl1 \
			WHERE start >= %lu AND start < %lu OR end >= %lu AND end < %lu;",
			start, end, start, end);

	char *errmsg;
	int status = sqlite3_exec(_db, query, callback, NULL, &errmsg);

	sqlite3_free(query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	*time_ranges = &_time_ranges.data[0];
	*size = _time_ranges.size;

	// Trim do fit in one day
	struct TimeRange *d = _time_ranges.data;
	for(int i = 0; i < _time_ranges.size; ++i) {
		if(d->start < start)
			d->start = start;
		if(d->end > end)
			d->end = end;
	}

	return 0;
}
