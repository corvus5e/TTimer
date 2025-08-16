#include <db/db.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sqlite3/sqlite3.h"
#include <timer/timer.h>

#define DB_NAME "data/time_db.db"
#define TBL_NAKE "tbl1"

#define SECS_DAY 86400

#define MAX_RANGES_PER_DAY 100 // TODO: Make dynamic later

static sqlite3 *_db;

struct TimeRangeArray {
	struct TimeInterval data[MAX_RANGES_PER_DAY];
	size_t size;
};

static struct TimeRangeArray _time_ranges;

static int time_ranges_callback(void *arg, int argc, char **argv, char **azColName)
{
	if(argc != 2) {
		fprintf(stderr, "Error in db: time_ranges_callback - Unexpected number of columns\n");
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

static int settings_callback(void *arg, int argc, char **argv, char **azColName)
{
	if(argc != 3) {
		fprintf(stderr, "Error in db: settings_callback - Unexpected number of columns\n");
		return -1;
	}

	struct AppSettings * settings = (struct AppSettings *)arg;

	settings->stopped_on_app_start = atoi(argv[0]) > 0;
	settings->stop_after_min = atoi(argv[1]);
	settings->min_seconds_to_save = atoi(argv[2]);

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

	const char *tbl1_sql = "create table if not exists tbl1 (start int, end int);";
	char *err_msg;

	status = sqlite3_exec(_db, tbl1_sql, NULL, NULL, &err_msg);
	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return 1;
	}

	const char *settings_sql = "create table if not exists settings \
				    (stopped_on_app_start int, \
				     stop_after_min int, \
				     min_seconds_to_save int);";

	status = sqlite3_exec(_db, settings_sql, NULL, NULL, &err_msg);
	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
		return 1;
	}

	return 0;
}

int db_dispose()
{
	sqlite3_close(_db);
	return 0;
}

int db_save_time(const struct TimeInterval ti)
{
	if(!_db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}


	char *insert_query = sqlite3_mprintf("insert into tbl1 (start, end) \
			values\
			(%lu, %lu);", ti.start, ti.end);

	char *errmsg;

	int status = sqlite3_exec(_db, insert_query, NULL, NULL, &errmsg);

	sqlite3_free(insert_query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int db_get_time(struct TimeInterval interval, struct TimeInterval **time_ranges, size_t *size)
{
	if(!_db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	_time_ranges.size = 0;

	time_t start = interval.start;
	time_t end = interval.end;

	char *query = sqlite3_mprintf("select start, end from tbl1 \
			WHERE start >= %lu AND start < %lu OR end >= %lu AND end < %lu;",
			start, end, start, end);

	char *errmsg;
	int status = sqlite3_exec(_db, query, time_ranges_callback, NULL, &errmsg);

	sqlite3_free(query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	*time_ranges = &_time_ranges.data[0];
	*size = _time_ranges.size;

	// Trim do fit in one day
	struct TimeInterval *d = _time_ranges.data;
	for(int i = 0; i < _time_ranges.size; ++i) {
		if(d->start < start)
			d->start = start;
		if(d->end > end)
			d->end = end;
	}

	return 0;
}

int db_get_settings(struct AppSettings * s)
{
	if(!_db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	char *select_query = sqlite3_mprintf("select * from settings limit 1");

	char *errmsg;

	int status = sqlite3_exec(_db, select_query, settings_callback, s, &errmsg);

	sqlite3_free(select_query);

	if (status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int db_save_settings(struct AppSettings s)
{
	if(!_db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	char *errmsg;

	/* Delete previous setings */
	char *delete_query = sqlite3_mprintf("delete from settings");

	int status = sqlite3_exec(_db, delete_query, NULL, NULL, &errmsg);

	sqlite3_free(delete_query);

	if (status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}
	
	/* Insert new settings */
	char *insert_query = sqlite3_mprintf(
	    "insert into settings (stopped_on_app_start, stop_after_min, \
				     min_seconds_to_save) \
					values (%d, %d, %d);",
	    s.stopped_on_app_start, s.stop_after_min, s.min_seconds_to_save);


	status = sqlite3_exec(_db, insert_query, NULL, NULL, &errmsg);

	sqlite3_free(insert_query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

