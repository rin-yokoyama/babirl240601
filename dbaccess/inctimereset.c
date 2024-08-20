/* dbaccess/inctimereset.c
 * last modified : 11/10/19 17:10:36 
 * Hidetada Baba (RIKEN)
 *
 * DB access 
 *  to increment Time Reset Counter
 * 
 * inctimereset HOSTName DBName User Password
 *
 * 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <bbpsql.h>

#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif
          
int main(int argc, char *argv[]){
  char buff[10*1024], text[1024];
  char tdate[1024];
  PGresult *res = NULL;
  int cnt, idx;
  struct tm *date;
  time_t tt;

  if(argc != 5){/* macros */

    printf("stdindb HOSTName DBName User Password\n");
    exit(0);
  }

  if(!bbpsql_connect(argv[1], argv[2], argv[3], argv[4])){
    printf("Can't connect DB host=%s, db=%s, user=%s, pass=%s\n",
	   argv[1], argv[2], argv[3], argv[4]);
  }


  sprintf(buff, "SELECT max(\"Counter\") from \"Date\"");
  if(!bbpsql_sql(buff, &res)){
    printf("Error SQL\n");
  }else{
    if(bbpsql_get_oneresult(res, text)){
      cnt = strtol(text, NULL, 0);
      cnt = cnt + 1;
    }else{
      cnt = 1;
    }

    time(&tt);
    date = localtime(&tt);
    sprintf(tdate, "%04d-%02d-%02d %02d:%02d:%02d",
	    date->tm_year+1900, date->tm_mon+1, date->tm_mday,
	    date->tm_hour, date->tm_min, date->tm_sec);

    memset(buff, 0, sizeof(buff));

    idx = sprintf(buff, "INSERT INTO \"Date\"(\"Counter\", \"Date\") ");
    idx += sprintf(buff+idx, "VALUES('%d', '%s')", cnt, tdate);

    bbpsql_sql_noresult(buff);
  }
  bbpsql_clear_result(res);

  bbpsql_end();

  return 0;
}

