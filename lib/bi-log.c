/* lib/bi-log.c
 * 
 * Online log buffer
 *
 * Hidetada Baba
 *
 */

#include <string.h>

#define MAXLOGLINE 5
#define MAXLOGCHAR 256
static int idx = 0;
static char log[MAXLOGLINE][MAXLOGCHAR]; // 1kbytes buffer
static char retlog[MAXLOGLINE][MAXLOGCHAR];

void initlog(void){
  idx = 0;
  memset(log, 0, sizeof(log));
}

char *getlog(void){
  int i, ri = 0;
  for(i=idx+1;i<MAXLOGLINE;i++){
    if(ri < MAXLOGLINE && ri >= 0){
      strncpy(retlog[ri], log[i], sizeof(retlog[ri]));
    }
    ri++;
  }
  for(i=0;i<idx+1;i++){
    if(ri < MAXLOGLINE && ri >= 0){
      strncpy(retlog[ri], log[i], sizeof(retlog[ri]));
    }
    ri++;
  }
  return retlog[0];
}

int getlogsize(void){
  return (int)sizeof(retlog);
}

int storelog(char *s){
  strncpy(log[idx], s, sizeof(log[idx]));
  idx++;
  if(idx == MAXLOGLINE) idx = 0;

  return idx;
}
