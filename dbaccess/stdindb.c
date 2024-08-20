/* dbaccess/stdindb.c
 * last modified : 11/09/29 16:47:47 
 * Hidetada Baba (RIKEN)
 *
 * DB access 
 *  commands from stdin
 * 
 * stdindb HOSTName DBName User Password
 *
 * 
 *  BEGIN\n
 *   SQL ...
 *  COMMIT\n
 *  QUIT\n (quit for this program)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <bbpsql.h>

#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif
          
int main(int argc, char *argv[]){
  char buff[10*1024], text[1024];
  PGresult *res = NULL;

  if(argc != 5){/* macros */

    printf("stdindb HOSTName DBName User Password\n");
    exit(0);
  }

  if(!bbpsql_connect(argv[1], argv[2], argv[3], argv[4])){
    printf("Can't connect DB host=%s, db=%s, user=%s, pass=%s\n",
	   argv[1], argv[2], argv[3], argv[4]);
  }


  while(1){
    fgets(buff, sizeof(buff), stdin);
    buff[strlen(buff)-1] = 0;
    
    if(!strcmp(buff, "QUIT")) break;

    DB(printf("SQL = %s\n", buff));
    if(!bbpsql_sql(buff, &res)){
      printf("Error SQL\n");
    }else{
      bbpsql_get_oneresult(res, text);
      printf("%s\n", text);
    }
    bbpsql_clear_result(res);

  }


  bbpsql_end();

  return 0;
}

