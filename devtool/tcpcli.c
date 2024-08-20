/** devtool/tcpcli.c
 *  last modified : 
 *  TCP Client for babinfo
 *
 *  Hidetada Baba (RIKEN)
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>


int sfd = 0, sock = 0;
//char buff[EB_BUFF_SIZE]; // < malloc!!!!
char *buff;

void quit(void){

  if(sock) close(sock);
  if(sfd) close(sfd);

  exit(0);
}


void show_buff(void){
  RIDFHD hd;
  RIDFRHD rhd;
  int idx = 0, size;

  memcpy((char *)&hd, buff+idx, sizeof(hd));
  size = ridf_sz(hd) * 2;

  rhd = ridf_dechd(hd);
  printf("ly=%d, cid=%d, size=%d, efn=%d\n",
  rhd.layer, rhd.classid, rhd.blksize, rhd.efn);

  idx = sizeof(hd);
  while(idx < size){
    memcpy((char *)&hd, buff+idx, sizeof(hd));
    idx += ridf_sz(hd) * 2;

    rhd = ridf_dechd(hd);
    //if(rhd.classid == RIDF_STATUS){
      printf("ly=%d, cid=%d, size=%d, efn=%d\n",
	     rhd.layer, rhd.classid, rhd.blksize, rhd.efn);
      //}
  }    

}

int recvmain(void){
  struct sockaddr_in caddr;
  int clen, rsize, tsz, len;
  RIDFHD hd;

  clen = sizeof(caddr);
  if((sock = accept(sfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    return 0;
  }

  while(1){
    tsz = recv(sock, (char *)&hd, sizeof(hd), MSG_WAITALL);
    if(tsz < 0){
      close(sock);
      return 0;
    }
    memcpy(buff, (char *)&hd, sizeof(hd));
    len = ridf_sz(hd) * 2;
    rsize = tsz;
    //printf("len=0x%08x\n", hd.hd1);
    //printf("len=0%x08\n", hd.hd2);
    while(rsize < len){
      tsz = recv(sock, buff+rsize, len - rsize, MSG_WAITALL);
      if(tsz < 0){
	close(sock);
	return 0;
      }
      rsize += tsz;
    }
    show_buff();
  }

  return 0;
}

int main(int argc, char *argv[]){
  struct sttcpclinfo thisinfo;
  fd_set fdset;
  int tsock, ret;
  
  if(argc < 3){
    printf("tcpcli BABINFOHOST THISPORT [THISHOST]\n");
    exit(0);
  }

  buff = malloc(EB_BUFF_SIZE);
  memset(buff, 0, EB_BUFF_SIZE);

  thisinfo.port = strtol(argv[2], NULL, 0);
  if(argc > 3){
    strncpy(thisinfo.host, argv[3], sizeof(thisinfo.host)-1);
  }else{
    gethostname(thisinfo.host, sizeof(thisinfo.host));
  }

  signal(SIGINT, (void *)quit);

  printf("thishost=%s / port=%d\n", thisinfo.host, thisinfo.port);

  /* Make command and oneshot port */
  if(!(sfd = mktcpsock(thisinfo.port))){
    printf("Can't make socket on port = %d\n", thisinfo.port);
    quit();
  }

  if(!(tsock = mktcpsend(argv[1], INFCOMPORT))){
    printf("Can't connecto to babinfo in %s\n", argv[1]);
    quit();
  }
  ret = eb_set(tsock, INF_MK_TCPCLIENT, (char *)&thisinfo, sizeof(thisinfo));
  close(tsock);

  if(!ret){
    printf("Exceed the maximum number of tcpclient\n");
    quit();
  }


  printf("goto main\n");
  /* Main loop for command and one short */
  while(1){
    FD_ZERO(&fdset);
    FD_SET(sfd, &fdset);
    if(select(sfd+1, &fdset,NULL,NULL,NULL) != 0){
      recvmain();
    }
  }


  return 0;
}
