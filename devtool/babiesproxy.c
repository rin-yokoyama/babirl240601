#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libbabirl.h>

static pthread_t prothread;
static int prosock = 0;

void promain(void){
  struct sockaddr_in caddr;
  int clen;
  int srvsock = 0;

  /* Make proxy port */
  if(!(srvsock = mktcpsock(ESCOMPORTPROXY + efrc.efid))){
    printf("Cannot make tcp socket\n");
    return 0;
  }

  while(1){
    clen = sizeof(caddr);
    if((prosock = accept(srvsock, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
      perror("Error in accept commain\n");
    }
    while(1){
      sleep(1);
    }
  }
}

void commain(void){
  struct sockaddr_in caddr;
  char buff[BABIRL_COM_SIZE], rbuff[BABIRL_COM_SIZE];
  char tdir[80]={0}, f[80]={0};
  int sock, clen, len, rlen;
  int com, ret, arg;
  
  clen = sizeof(caddr);
  if((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("Error in accept commain\n");
    return 0;
  }
  
  memset(buff, 0, sizeof(buff));
  memset(rbuff, 0, sizeof(rbuff));
  rlen = 0;
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);

  /* babies command */
  memcpy((char *)&com, buff, sizeof(com));

  if(prosock){
    send(prosock, (char *)&len, sizeof(len), 0);
    send(prosock, buff, len, 0);
  }
  
  switch(com){
  case ES_SET_CONFIG:
    rlen = 4;
    if(prosock){
      recv(prosock, (char *)&rlen, sizeof(rlen), MSG_WAITALL);
      recv(prosock, rbuff, rlen, MSG_WAITALL);
    }
    send(sock, (char *)&rlen, sizeof(rlen),0);
    send(sock, rbuff, rlen, 0);

    break;
  case ES_GET_CONFIG:
    rlen = sizeof(efrc);
    if(prosock){
      recv(prosock, (char *)&rlen, sizeof(rlen), MSG_WAITALL);
      recv(prosock, rbuff, rlen, MSG_WAITALL);
    }
    send(sock, (char *)&rlen, sizeof(rlen),0);
    send(sock, rbuff, rlen, 0);
    break;
  case ES_RELOAD_DRV:
    break;
  case ES_RUN_START:
    break;
  case ES_RUN_NSSTA:
    // run start with no save mode
    DB(printf("libbabies: ES_NSRUN_START\n"));
    memcpy((char *)&arg, buff+sizeof(com), sizeof(arg));
    if(arg == ES_EF_OFF && !fstart){
      DB(printf("libbabies: Run started, but not joining event build\n"));
      break;
    }

    // event number = 0
    evtn = 0;

    // User start function
    if(nssta){
      nssta();
    }else{
      if(start) start();
    }

    // when start ready, return start ack
    ret = 1;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);

    pthread_mutex_lock(&waitmutex);
    sflag = STAT_RUN_NSSTA;
    pthread_mutex_unlock(&waitmutex);

    creat_runstfile();

    break;
  case ES_RUN_STOP:
    DB(printf("libbabies: ES_RUN_STOP\n"));
    // return command ack
    ret = 1;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    sflag = STAT_RUN_WAITSTOP;

    unlink_runstfile();
    
    break;
  case ES_GET_EVTN:
    // return the event number
    ret = evtn;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    break;
  case ES_GET_RUNSTAT:
    if(isbkfile()){
      ret = -1;
    }else{
      ret = sflag;
    }
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    break;
  case WHOAREYOU:
    DB(printf("libbabies : WHOAREYOU\n"));
    memset(f, 0, sizeof(f));
    len = sprintf(f, name);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, f, len, 0);
    break;
  case ES_QUIT:
    DB(printf("libbabies: ES_QUIT\n"));
    if(!sflag) wrap_quit();
    break;
  case ES_CON_EFR:
    DB(printf("libbabies: ES_CON_EFR\n"));
    if(!efr_connect()){
      DB(printf("libbabies: EFR connection faild\n"));
    }
    break;
  case ES_DIS_EFR:
    DB(printf("libbabies: ES_DIS_EFR\n"));
    if(!efr_disconnect()){
      DB(printf("libbabies: EFR disconnection faild\n"));
    }
  }

  close(sock);
  
}


int main(int argc, char *argv){
  int comfd, profd;
  int sn;
  fd_set fdset;

  /* Make command port */
  if(!(comfd = mktcpsock(ESCOMPORT + efrc.efid))){
    printf("Cannot make tcp socket\n");
    return 0;
  }

  pthread_create(&prothread, NULL, (void *)promain, NULL);

  // main loop for babies
  while(1){
    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    if(select(comfd+1, &fdset, NULL, NULL, NULL) != 0){
      if(FD_ISSET(comfd, &fdset)){
	commain();
      }else{
	usleep(1);
      }
    }
  }

  return 0;
}
