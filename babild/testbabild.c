#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bi-config.h>
#include <bi-common.h>

#include "ebstrcom.h"

struct stdaqinfo daqinfo;
struct struninfo runinfo;
struct stssminfo ssminfo;
struct stclinfo  clinfo[MAXCLI];

unsigned long long int totsize = 100000;
unsigned int gloevtn = 10;

int main(int argc, char *argv[]){
  char *ret;
  char com[1024];
  char cflag[4];
  int flag, asccom;

  cflag[0] = 'A';
  cflag[1] = '\n';
  cflag[2] = 'C';
  cflag[3] = '\n';

  memcpy((char *)&flag, cflag, sizeof(flag));

  memset((char *)&daqinfo, 0 , sizeof(daqinfo));


  sprintf(com, "set\ndaqinfo\nrunname\ntestrun desuyo\nrunnumber\n120\nebsize\n8123\nef\n5\nbabildes\n0");
  printf("chkset %d\n", chksetcom(com));
  ret = parsetextcom(com, &asccom);

  sprintf(com, "set\nruninfo\nheader\nstart of run\nender\nend of run");
  printf("chkset %d\n", chksetcom(com));
  ret = parsetextcom(com, &asccom);

  sprintf(com, "set\neflist\nid\n33\nhost\ntesthost\nname\ntesthostname\nid\n49\nof\n1\nname\n49name\nhost\n49host\nid\n22\nof\n3\nname\naaaa\nhost\nbbbbb");
  printf("chkset %d\n", chksetcom(com));
  ret = parsetextcom(com, &asccom);

  sprintf(com, "set\neflist\nid\n49\ndel\n\nid\n150\nname\ncccc\nhost\ncc");
  printf("chkset %d\n", chksetcom(com));
  ret = parsetextcom(com, &asccom);


  sprintf(com, "get\ndaqinfo\nget\nruninfo\nget\neflist");
  printf("chkset %d\n", chksetcom(com));
  ret = parsetextcom(com, &asccom);


  printf("\n------------------------\n");
  printf("%s\n", ret);
  printf("------------------------\n");


  sprintf(com, "get\nebinfo");
  printf("chkset %d\n", chksetcom(com));
  ret = parsetextcom(com, &asccom);

  printf("\n------------------------\n");
  printf("%s\n", ret);
  printf("------------------------\n");

  return 0;
}
