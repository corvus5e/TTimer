#include <db/db.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sqlite3/sqlite3.h"
#include <timer/timer.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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

	_time_ranges.data[_time_ranges.size].start = (time_t)atoll(argv[0]);
	_time_ranges.data[_time_ranges.size].end = (time_t)atoll(argv[1]);
	_time_ranges.size++;

	return 0;
}

static int settings_callback(void *arg, int argc, char **argv, char **azColName)
{
	if(argc != 5) {
		fprintf(stderr, "Error in db: settings_callback - Unexpected number of columns\n");
		return -1;
	}

	struct AppSettings * settings = (struct AppSettings *)arg;

	settings->stopped_on_app_start = atoi(argv[0]) > 0;
	settings->stop_after_min = atoi(argv[1]);
	settings->min_seconds_to_save = atoi(argv[2]);
	settings->save_on_term_signal = atoi(argv[3]);
	settings->idle_pause_time = atoi(argv[4]);

	return 0;
}

static int get_db_path(char *out_path, size_t max_len);

int db_init()
{
	char db_path[PATH_MAX];
	if (get_db_path(db_path, sizeof(db_path)) != 0) {
		fprintf(stderr, "Failed to determine database path\n");
		return 1;
	}

	int status = sqlite3_open(db_path, &_db);

	if (status) {
		fprintf(stderr, "Failed to open a database %s\n", db_path);
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
				     min_seconds_to_save int, \
				     save_on_term int, \
				     idle_pause_time int);";

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
			(%lld, %lld);", (long long)ti.start, (long long)ti.end);

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
			WHERE end > %lld AND start < %lld;",
			(long long)start, (long long)end);

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
	struct TimeInterval *trimmed_range = _time_ranges.data;
	struct TimeInterval *guard = trimmed_range + _time_ranges.size;
	for(; trimmed_range < guard; ++trimmed_range) {
		if(trimmed_range->start < start)
			trimmed_range->start = start;
		if(trimmed_range->end > end)
			trimmed_range->end = end;
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
				     min_seconds_to_save, save_on_term, idle_pause_time) \
					values (%d, %d, %d, %d, %d);",
	    s.stopped_on_app_start, s.stop_after_min, s.min_seconds_to_save, s.save_on_term_signal, s.idle_pause_time);


	status = sqlite3_exec(_db, insert_query, NULL, NULL, &errmsg);

	sqlite3_free(insert_query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int get_db_path(char *out_path, size_t max_len) {
	const char *xdg_state = getenv("XDG_STATE_HOME");
	if (xdg_state && xdg_state[0] != '\0') {
		if (snprintf(out_path, max_len, "%s/TTimer/time_db.db", xdg_state) >= (int)max_len) {
			return -1;
		}
		return 0;
	}

	const char *home = getenv("HOME");
	if (home && home[0] != '\0') {
		if (snprintf(out_path, max_len, "%s/.local/state/TTimer/time_db.db", home) >= (int)max_len) {
			return -1;
		}
		return 0;
	}

	// Fallback to current directory if neither XDG_STATE_HOME nor HOME is available
	if (snprintf(out_path, max_len, "data/time_db.db") >= (int)max_len) {
		return -1;
	}
	return 0;
}

