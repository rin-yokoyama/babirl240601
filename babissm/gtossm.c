/*
 * last modified : 13/07/24 08:54:57 
 * babirl/babissm/gtossm.c
 * 
 * front-end for xport mssm module
 *
 * gtossm HOSTNAME on/off
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libbabirl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

const int MAXTRY = 200;

const char *ofstr[] = {"off\0", "on\0"};

int chkupper(char c){
  if(isupper(c)){
    return 1;
  }else{
    return 0;
  }
}

int main(int argc, char *argv[]){
  int sock = 0, try = 0, size = 4, readf=0, i=0, latchf=0;
  int daq = 0;
  char of[64];

  if(argc < 3){
    printf("gtossm HOSTNAME on/off/com/noop/read/latch\n");
    exit(0);
  }

  if(!strcmp(argv[2], "on")){
    sprintf(of, "!!0Z");
    daq = 1;
  }else if(!strcmp(argv[2], "off")){
    sprintf(of, "!!0z");
    daq = 1;
  }else if(!strcmp(argv[2], "com")){
    size = snprintf(of, sizeof(of), "!!%s", argv[3]);
  }else if(!strcmp(argv[2], "read")){
    size = snprintf(of, sizeof(of), "!!@@");
    readf = 1;
  }else if(!strcmp(argv[2], "latch")){
    size = snprintf(of, sizeof(of), "!!@0");
    readf = 1;
    latchf = 1;
  }else{
    // noop
    size = snprintf(of, sizeof(of), "!!");
  }

  while(!sock){
    sock = mktcpsend(argv[1], 10001);
    try++;
    if(try > MAXTRY) break;
    usleep(50000); // 50ms * 200 = 10s
  }

  if(!sock){
    printf("Can't connect %s\n", argv[1]);
    exit(1);
  }
  send(sock, of, size, 0);
  if(readf){
    size = 0;
    while(size<32){
      size += recv(sock, of, 32, MSG_WAITALL);
    }
  }
  if(daq){
    // do noop
    size = snprintf(of, sizeof(of), "!!");
    send(sock, of, size, 0);
  }
  close(sock);

  if(readf){
    for(i=0;i<4;i++){
      printf("%c", of[i]);
    }
    printf(" : Version\n");
    if(!latchf){
      for(i=6;i<14;i++){
	printf("%c : Trigger %d (%s)\n", of[i], i-5, ofstr[chkupper(of[i])]);
      }
    }
    for(i=14;i<30;i++){
      printf("%c : Busy %2d (%s)\n", of[i], i-13, ofstr[chkupper(of[i])]);
    }
    printf("%c : Test LED (%s)\n", of[30], ofstr[chkupper(of[30])]);
    printf("%c : DAQ Start (%s)\n", of[31], ofstr[chkupper(of[31])]);
  }

  return 0;
}
