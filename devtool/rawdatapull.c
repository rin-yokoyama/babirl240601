/* babirl : devtool/rawdatapull
 * last modified : 13/11/02 23:39:52 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Data analysis client
 * an example pull raw data for online analysis
 * without external libs
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef INFCOMPORT
#define INFCOMPORT  17516   ///< babinfo communication port
#endif

#ifndef INF_GET_RAWDATA

#ifndef EB_EFBLOCK_SIZE
#ifdef LARGEBUFF
#define EB_EFBLOCK_SIZE 0x200000   ///< Usual max size of block data = 4MB (written in short word)
#else
#define EB_EFBLOCK_SIZE 0x20000   ///< Usual max size of block data = 256kB (written in short word)
#endif 
#endif
#ifndef EB_EFBLOCK_BUFFSIZE
#define EB_EFBLOCK_BUFFSIZE EB_EFBLOCK_SIZE * 2  ///< Usual size of block data 
#endif
#endif

#ifndef INF_GET_RAWDATA
#define INF_GET_RAWDATA    10
#endif

#ifndef INF_GET_BLOCKNUM
#define INF_GET_BLOCKNUM   11
#endif

int sock;
struct sockaddr_in saddr, caddr;
char *data;

int mktcpsend(char *host, unsigned short port){
  int sock = 0;
  struct hostent *hp;
  struct sockaddr_in saddr;

  if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){
    perror("bi-tcp.mktcpsend: Can't make socket.\n");
    return 0;
  }

  memset((char *)&saddr,0,sizeof(saddr));

  if((hp = gethostbyname(host)) == NULL){
    printf("bi-tcp.mktcpsend : No such host (%s)\n", host);
    return 0;
  }

  memcpy(&saddr.sin_addr,hp->h_addr,hp->h_length);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);

  if(connect(sock,(struct sockaddr *)&saddr,sizeof(saddr)) < 0){
    perror("bi-tcp.mktcpsend: Error in tcp connect.\n");
    close(sock);
    return 0;
  }

  return sock;
}

int eb_get(int sock, int com, char *dest){
  int len;

  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, len, 0);

  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, dest, len, MSG_WAITALL);

  return len;
}

int infcon(char *host){
  int infsock;

  /* Connect to babild */
  if(!(infsock = mktcpsend(host, INFCOMPORT))){
    printf("babicon: Can't connet to babinfo.\n");
    exit(0);
    return 0;
  }
  
  return infsock;
}

/* Quit */
void quit(void){

  exit(0);
}

int main(int argc, char *argv[]){
  //unsigned int blocknum;
  unsigned int orgnum = 0xffffffff, num = 0;
  char ebhostname[128];
  
  if((data = malloc(EB_EFBLOCK_BUFFSIZE * 2)) == NULL){
    printf("too large buffer \n");
    quit();
  }

  //blocknum = 0;
  sock = 0;

  if(argc != 2){
    printf("rawdatapull SERVERNAME\n");
    exit(0);
  }else{
    strncpy(ebhostname, argv[1], sizeof(ebhostname)-1);
  }
    
  signal(SIGINT, (void *)quit);

  /* List Data Socket */
  while(1){
    usleep(100000);
    if((sock = infcon(ebhostname))){
      eb_get(sock, INF_GET_BLOCKNUM, (char *)&num);
      close(sock);
    }else{
      printf("Can not connect %s\n", ebhostname);
      continue;
    }
    if(num != orgnum){
      orgnum = num;
      if((sock = infcon(ebhostname))){
	eb_get(sock, INF_GET_RAWDATA, data);
	close(sock);
	printf("New data get\n");
      }else{
	printf("Can not connect %s\n", ebhostname);
	continue;
      }
    }else{
      continue;
    }

  }
  
  return 0;
}

