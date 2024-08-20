#include <stdio.h>
#include <time.h>

char ret[64];

char *now_time(void){
  struct tm *t_st;
  time_t now;

  time(&now);
  t_st = localtime(&now);

  sprintf(ret,"%02d/%02d/%02d %02d:%02d:%02d",
	  t_st->tm_year-100, t_st->tm_mon+1, t_st->tm_mday,
	  t_st->tm_hour, t_st->tm_min, t_st->tm_sec);

  return ret;
}
