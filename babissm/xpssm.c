/*
 * last modified : 11/06/30 18:42:44 
 * babirl/babissm/xpssm.c
 * 
 * front-end for xport mssm module
 *
 * xpssm HOSTNAME on/off
 *
 * the setting of XPORT+MSSM should be 
 *  Configurable Pin 2 =
 *   Function     = General Purpose I/O 
 *   Direction    = Output
 *   Active Level = Low
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libbabirl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char *argv[]){
  int sock, cnt;
  char sbuff[9], rbuff[5], of = 0;

  if(argc != 3){
    printf("xpssm HOSTNAME on/off\n");
    exit(0);
  }

  if(!strcmp(argv[2], "off")){
    of = 0x02;
  }

  if(!(sock = mktcpsend(argv[1], 0x77f0))){
    printf("Can't connect %s\n", argv[1]);
    exit(1);
  }

  memset(sbuff, 0, sizeof(sbuff));
  memset(rbuff, 0, sizeof(rbuff));

  sbuff[0] = 0x1b;
  sbuff[1] = 0x02;
  sbuff[5] = of;
  printf("of = %02x\n", of);
  for(cnt=0;cnt<5;cnt++){
  printf("%d %02x\n", sock, sbuff[cnt]);
  }

  send(sock, sbuff, sizeof(sbuff), 0);
  cnt = recv(sock, rbuff, sizeof(rbuff), MSG_WAITALL);

  close(sock);

  printf("cnt=%d\n", cnt);
  printf("of = %02x\n", of);
  for(sock=0;sock<5;sock++){
  printf("%d %02x\n", sock, rbuff[sock]);
  }

  if(rbuff[1] & 0x02){
    printf("off\n");
  }else{
    printf("on\n");
  }


  return 0;
}
