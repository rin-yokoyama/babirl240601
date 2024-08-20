/* babissm/babissm.c
 * last modified : 10/12/01 12:08:05 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Start Stop Manager
 *
 */

/* Version */
#define VERSION "22.Mar.2012"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <bi-config.h>
#include <bi-common.h>
#include "babissm.h"

void quit(void);

void quit(void){
  if(comfd) close(comfd);
  rmpid("babissm");

  exit(0);
}

/************************* commain ****************************/
int commain(void){
  struct sockaddr_in caddr;
  int sock, clen;
  int com, len, ret = 1;
  int idx=0;
  char tline[512], rline[4096];
  FILE *fp = NULL;

  clen = sizeof(caddr);
  if((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("babinfo: Error in accept commain\n");
    return 0;
  }

  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, combuff, len, MSG_WAITALL);
  memcpy((char *)&com, combuff, sizeof(com));
  
  switch(com){
  case SSM_SET_SSMINFO:
    DB(printf("babissm: set ssminfo\n"));
    memcpy((char *)&ssminfo, combuff+sizeof(com), sizeof(ssminfo));
    DB(printf("ex=%d, of=%d, host=%s, sta=%s, sto=%s\n", ssminfo.ex, ssminfo.of, ssminfo.host, ssminfo.start, ssminfo.stop));
    break;
  case SSM_START:
    DB(printf("babissm: start\n"));
    if(ssminfo.of){
      system(ssminfo.start);
    }
    break;
  case SSM_STOP:
    DB(printf("babissm: stop\n"));
    if(ssminfo.of){
      memset(rline, 0, sizeof(rline));
      if((fp = popen(ssminfo.stop, "r")) == NULL){
	/* noop */
      }else{
	memset(tline, 0, sizeof(tline));
	while(fgets(tline, sizeof(tline), fp)){
	  if(!strncmp(tline, "GTOERROR", 8)){
	    DB(printf("popen %s\n", tline));
	    strcpy(rline+idx, tline);
	    idx += strlen(tline);
	  }
	}
	pclose(fp);
      }
      //system(ssminfo.stop);
    }
    break;
  default:
    break;
  }

  if(idx == 0){
    send(sock, (char *)&ret, sizeof(ret), 0);
  }else{
    send(sock, (char *)&idx, sizeof(idx), 0);
    send(sock, rline, idx, 0);
  }
  close(sock);

  return 1;
}

/*************************** Main *****************************/
int main(int argc, char *argv[]){
  fd_set fdset;
  //int opt, ret, l;

  /* Change working directory */
  if(getenv("BABIRLDIR")){
    chdir(getenv("BABIRLDIR"));
  }else{
    chdir(BABIRLDIR);
  }

  /* PID file */
  if(chkpid("babissm")){
    printf("babissm: Error, another babissm may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babissm\n");
    exit(0);
  }

#ifndef DEBUG
  if(!(int)getuid()) nice(-15);
  daemon(1, 0);
#endif

  if(!mkpid("babissm")){
    printf("babissm: Error, another babissm may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babissm\n");
    exit(0);
  }


  comfd = 0;
  memset((char *)&ssminfo, 0, sizeof(ssminfo));
  
  signal(SIGINT, (void *)quit);
  signal(SIGPIPE, SIG_IGN);

  /* Make command and oneshot port */
  if((comfd = mktcpsock(SSMCOMPORT)) == -1){
    printf("Can't make command and oneshot port\n");
    exit(0);
  }

  /*
  l = sizeof(opt);
  ret = getsockopt(comfd, SOL_SOCKET, SO_RCVTIMEO,
	     &opt, &l);
  printf("%d %d\n", opt, ret);
  ret = getsockopt(comfd, SOL_SOCKET, SO_SNDTIMEO,
	     &opt, &l);
  printf("%d %d\n", opt, ret);
  ret = getsockopt(comfd, SOL_SOCKET, SO_REUSEADDR,
	     &opt, &l);
  printf("%d %d\n", opt, ret);
  */
  /* Main loop for command and one short */
  while(1){
    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    if(select(comfd+1, &fdset,NULL,NULL,NULL) != 0){
      commain();
    }
  }

  return 0;
}
