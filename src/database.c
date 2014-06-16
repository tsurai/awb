#include "database.h"
#include <stdlib.h>
#include <stdbool.h>

bool db_init(sqlite3 *sqldb) {
  char err[256];
  
  /* create tables if not exists */
  return sqlite3_exec(sqldb, "CREATE TABLE IF NOT EXISTS history ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "protocol VARCHAR(10),"
    "baseurl TEXT,"
    "request TEXT,"
    "hits INTEGER,"
    "title TEXT,"
    "lasthit TIMESTAMP DEFAULT CURRENT_TIMESTAMP)", NULL, NULL, (char **)&err) == SQLITE_OK;
}

sqlite3 *db_open(const char *filename) {
  sqlite3 *sqldb;

  /* open database */
  if(sqlite3_open_v2(filename, &sqldb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
    return NULL;
  }

  if(!db_init(sqldb))
    return NULL;

  return sqldb;
}

void db_close(sqlite3 *sqldb) {
  if(sqldb != NULL)
    sqlite3_close(sqldb);
}
/*
void db_query(sqlite3 *sqldb, const char *query, int n, ...) {
  va_list vl;
  va_start(vl, n);
  
  for (i=0;i<n;i++)
  {
    val=va_arg(vl, double);
    printf("", );
  }

  va_end(vl);
}*/