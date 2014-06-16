#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <stdarg.h>

sqlite3 *db_open(const char *filename);
/*void db_query(sqlite3 *sqldb, const char *query, ...);*/

#endif