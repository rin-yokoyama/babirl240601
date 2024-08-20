/* bbpsql.h
 * last modified : 13/04/15 14:55:00 
 *
 * Header for PostgreSQL access
 *
 */

#include <libpq-fe.h>

int bbpsql_connect(char *host, char *dbname, char *user, char *passwd);
int bbpsql_end(void);
int bbpsql_sql(char *sql, PGresult **res);
int bbpsql_sql_noresult(char *sql);
int bbpsql_clear_result(PGresult *res);
int bbpsql_get_oneresult(PGresult *res, char *ret);
int bbpsql_get_idxresult(PGresult *res, char *ret, int x);
int bbpsql_get_values(PGresult *res, char *ret);
int bbpsql_get_values_csv(PGresult *res, char *ret, int x);
int bbpsql_get_values_cm(PGresult *res, char *ret, int *cm);
int bbpsql_get_values_csv_cm(PGresult *res, char *ret, int x, int *cm);
int bbpsql_get_fields(PGresult *res, char *ret);
int bbpsql_get_fields_values(PGresult *res, char *ret);
int bbpsql_get_fields_values_cm(PGresult *res, char *ret, int *cm);
void bbpsql_safetext(char *test, int len);
int bbpsql_get_values_csv_cal(PGresult *res, char *ret,
			      int a, int b, unsigned long long int c);
