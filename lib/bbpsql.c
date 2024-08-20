/* bbpsql.c
 * last modified : 16/12/28 16:03:34 
 *
 * Library for PostgreSQL access
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bbpsql.h>

/* macros */
#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

static char conninfo[1024];
static PGconn *conn;
static PGresult *lres;

int bbpsql_connect(char *host, char *dbname, char *user, char *passwd){
  memset(conninfo, 0, sizeof(conninfo));
  sprintf(conninfo, "host=%s dbname=%s user=%s password=%s",
	  host, dbname, user, passwd);

  conn = PQconnectdb(conninfo);
  DB(printf("Status : %d\n", PQstatus(conn)));
  if(PQstatus(conn) == CONNECTION_BAD){
    DB(printf("DB connection is bad\n"));
    return 0;
  }
  
  return 1;
}

int bbpsql_end(void){
  PQfinish(conn);

  return 1;
}


int bbpsql_sql(char *sql, PGresult **res){
  *res = PQexec(conn, sql);

  if(!res || PQresultStatus(*res) == PGRES_FATAL_ERROR){
    //DB(printf("bbpsql: Error from PSQL %s\n", PQerrorMessage(conn)));
    printf("bbpsql: Error from PSQL %s\n", PQerrorMessage(conn));
    //sprintf(sql, "%s", PQerrorMessage(conn));
    return 0;
  }

  return 1;
}

int bbpsql_sql_noresult(char *sql){
  lres = PQexec(conn, sql);
  if(PQresultStatus(lres) != PGRES_COMMAND_OK){
    //DB(printf("bbpsql: Error from PSQL %s\n", PQerrorMessage(conn)));
    printf("bbpsql: Error from PSQL %s\n", PQerrorMessage(conn));
    //sprintf(sql, "%s", PQerrorMessage(conn));
  }
  PQclear(lres);

  return 1;
}

int bbpsql_clear_result(PGresult *res){
  PQclear(res);

  return 1;
}

int bbpsql_get_oneresult(PGresult *res, char *ret){

  if(PQnfields(res) && PQntuples(res)){
    sprintf(ret, "%s", PQgetvalue(res, 0, 0));
  }else{
    sprintf(ret, "No Result");
    return 0;
  }

  return 1;
}

int bbpsql_get_idxresult(PGresult *res, char *ret, int x){

  if((PQnfields(res) > x) && PQntuples(res)){
    sprintf(ret, "%s", PQgetvalue(res, 0, x));
  }else{
    ret[0] = 0;
    return 0;
  }

  return 1;
}

int bbpsql_get_values(PGresult *res, char *ret){
  int i, j, f, t, idx = 0;
  f = PQnfields(res);
  t = PQntuples(res);
  if(f && t){
    for(i=0;i<t;i++){
      for(j=0;j<f;j++){
	idx += sprintf(ret+idx, "%s,", PQgetvalue(res, i, j));
      }
      ret[idx-1] = 0;
      idx--;
      idx += sprintf(ret+idx, "\n");
    }
    ret[idx-1] = 0;
    idx--;
  }else{
    idx = sprintf(ret, "No Result");
  }
  return idx;
}

int bbpsql_get_values_cm(PGresult *res, char *ret, int *cm){
  int i, j, f, t, idx = 0;
  f = PQnfields(res);
  t = PQntuples(res);
  if(f && t){
    for(i=0;i<t;i++){
      for(j=0;j<f;j++){
	if(cm[j]){
	  idx += sprintf(ret+idx, "\"%s\",", PQgetvalue(res, i, j));
	}else{
	  idx += sprintf(ret+idx, "%s,", PQgetvalue(res, i, j));
	}
      }
      ret[idx-1] = 0;
      idx--;
      idx += sprintf(ret+idx, "\n");
    }
    ret[idx-1] = 0;
    idx--;
  }else{
    idx = sprintf(ret, "No Result");
  }
  return idx;
}

int bbpsql_get_values_csv(PGresult *res, char *ret, int x){
  int i, f, t, idx = 0;
  f = PQnfields(res);
  t = PQntuples(res);
  if(f && t){
    for(i=0;i<t;i++){
      idx += sprintf(ret+idx, "%s,", PQgetvalue(res, i, x));
    }
  }
  ret[idx-1] = 0;
  idx--;

  return idx;
}


int bbpsql_get_values_csv_cal(PGresult *res, char *ret,
			      int a, int b, unsigned long long int c){
  int i, f, t, idx = 0;
  unsigned long long int sa, sb, sc;
  
  f = PQnfields(res);
  t = PQntuples(res);
  if(f && t){
    for(i=0;i<t;i++){
      sa = strtoll(PQgetvalue(res, i, a), NULL, 0);
      sb = strtoll(PQgetvalue(res, i, b), NULL, 0);
      sc = sa + sb * c;
      idx += sprintf(ret+idx, "%llu,", sc);
    }
  }
  ret[idx-1] = 0;
  idx--;

  return idx;
}

int bbpsql_get_values_csv_cm(PGresult *res, char *ret, int x, int *cm){
  int i, f, t, idx = 0;
  f = PQnfields(res);
  t = PQntuples(res);
  if(f && t){
    for(i=0;i<t;i++){
      if(cm[i]){
	idx += sprintf(ret+idx, "\"%s\",", PQgetvalue(res, i, x));
      }else{
	idx += sprintf(ret+idx, "%s,", PQgetvalue(res, i, x));
      }
    }
  }
  ret[idx-1] = 0;
  idx--;

  return idx;
}


int bbpsql_get_fields(PGresult *res, char *ret){
  int i, idx = 0;
  if(PQnfields(res)){
    for(i=0;i<PQnfields(res);i++){
      idx += sprintf(ret+idx, "%s,", PQfname(res, i));
    }
    ret[idx-1] = 0;
    idx--;
  }else{
    idx = sprintf(ret, "No Fields");
  }

  return idx;
}

int bbpsql_get_fields_values(PGresult *res, char *ret){
  int idx;
  
  idx = bbpsql_get_fields(res, ret);
  idx += sprintf(ret+idx, "\n");
  idx += bbpsql_get_values(res, ret+idx);

  return idx;
}

int bbpsql_get_fields_values_cm(PGresult *res, char *ret, int *cm){
  int idx;
  
  idx = bbpsql_get_fields(res, ret);
  idx += sprintf(ret+idx, "\n");
  idx += bbpsql_get_values_cm(res, ret+idx, cm);

  return idx;
}

void bbpsql_safetext(char *test, int len){
  int i;

  for(i=0;i<len;i++){
    if(test[i] == '\"' || test[i] == '\''
       || test[i] == '\n' || test[i] == '\r'){
      test[i] = ' ';
    }else if(test[i] == '\\'){
      test[i] = '/';
    }
  }
}
