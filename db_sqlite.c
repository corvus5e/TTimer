#include "db.h"

#include "stdio.h"
#include "sqlite3/sqlite3.h"

#define DB_NAME "time_db.db"
#define TBL_NAKE "tbl1"

sqlite3 *db;

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	for (int i = 0; i < argc; i++) {
		//fprintf(stderr, "%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

	return 0;
}

int db_init()
{
	int status = sqlite3_open(DB_NAME, &db);

	if (status) {
		fprintf(stderr, "Failed to open a database %s\n", DB_NAME);
		sqlite3_close(db);
		return 1;
	}

	const char *sql = "create table if not exists tbl1 (start int, end int);";
	char *errmsg;

	status = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int db_dispose()
{
	sqlite3_close(db);
	return 0;
}

int db_save_time(const struct Timer *timer)
{
	if(!db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	char *query = sqlite3_mprintf("insert into tbl1 (start, end) \
			values\
			(%lu, %lu);", timer->start,
			timer->start + timer->time_elapsed_sec - timer->total_paused_time_sec);

	char *errmsg;

	int status = sqlite3_exec(db, query, NULL, NULL, &errmsg);

	sqlite3_free(query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}

int db_get_time(time_t *time, int *n)
{
	if(!db) {
		fprintf(stderr, "Error: db is NULL\n");
		return -1;
	}

	//NOTE: Query today sessions
	char *query = sqlite3_mprintf("select start, end from tbl1 \
			WHERE \
			start >= unixepoch('now', 'start of day') AND \
			start < unixepoch('now', 'start of day', '+1 day') \
			OR \
			end >= unixepoch('now', 'start of day') AND \
			end < unixepoch('now', 'start of day', '+1 day');");

	char *errmsg;

	int status = sqlite3_exec(db, query, callback, NULL, &errmsg);

	sqlite3_free(query);

	if(status != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 1;
	}

	return 0;
}
