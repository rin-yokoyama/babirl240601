#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bbterm.h>

int get_terminfo(int *row, int *col){
  char termtype[256],term_buffer[2048];
  int ret,val;

  if(setupterm(NULL, fileno(stdout), (int *)0) == ERR){
    printf("bi-term: error setupterm\n");
    exit(1);
  }

  strcpy(termtype, getenv("TERM"));

  if(termtype[0] == 0){
    printf("Can't get terminal type -> use vt100.\n");
    strcpy(termtype,"vt100");
    ret = 0;
  }else{
    ret = 1;
  }

  tgetent(term_buffer,termtype);
  *row = tgetnum("li");
  *col = tgetnum("co");

  val = tgetnum("li");
  printf("li : %d  ",val);
  val = tgetnum("co");
  printf("co : %d  ",val);
  val = tgetnum("it");
  printf("it : %d  ",val);
  val = tgetnum("ih");
  printf("ih : %d  ",val);
  val = tgetnum("ws");
  printf("ws : %d  ",val);

  return ret;
}
