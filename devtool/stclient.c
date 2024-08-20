/* babirl/devtool/stclient.c
 * last modified : 10/12/09 18:46:37 
 *
 * Hidetada Baba  baba@ribf.riken.jp
 * RIKEN 
 *
 * Receive streaming data from stserver
 *  this program is written without any babirl library
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// globals
int sock = 0;



// from include/isoba.h
/****** Definition ******/
#define ISOBA_STCOMPORT         10080        // Server port
#define ISOBA_MAX_CONNECTION    50           // Maximum connection in program
#define ISOBA_MAX_BUFFER        100          // Maximum number of buffer

#define ISOBA_CONNECTION_OK     0x00000000
#define ISOBA_CONNECTION_EXCEED 0x00000001

#define ISOBA_COM_FROMFIRST     0x00000001
#define ISOBA_COM_FROMLAST      0x00000000


// from ridf.h
#define RIDF_SZ(x)  (x & 0x003fffff)        // Block Size


// from lib/bi-tcp.c
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
    perror("bi-tcp.mktcpsend: No such host");
    return 0;
  }

  memcpy(&saddr.sin_addr,hp->h_addr,hp->h_length);
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);

  //DB(printf("host : %s  port : %d\n",host,port);)

  if(connect(sock,(struct sockaddr *)&saddr,sizeof(saddr)) < 0){
    perror("bi-tcp.mktcpsend: Error in tcp connect.\n");
    close(sock);
    return 0;
  }

  return sock;
}

void quit(void){
  if(sock) close(sock);
  exit(1);
}

int main(int argc, char *argv[]){
  int com, hd, sz, ret;
  char buff[256*1024];

  if(argc < 2){
    printf("stclient HOSTNAME [hoge]\n");
    printf("   without argument: get latest data only\n");
    printf("   with    argument: get all buffered data and latest data\n\n");
    exit(0);
  }
    
  if(argc >= 3){
    printf("From First data\n");
    com = ISOBA_COM_FROMFIRST;
  }else{
    printf("Latest data only\n");
    com = 0;
  }

  if(!(sock = mktcpsend(argv[1], ISOBA_STCOMPORT))){
    printf("Can't connect to %s\n", argv[1]);
    exit(0);
  }

  signal(SIGINT, (void *)quit);

  send(sock, (char *)&com, sizeof(com), 0);
  recv(sock, (char *)&ret, sizeof(ret), MSG_WAITALL);

  if(ret){
    printf("Error from server code=%d\n", ret);
  }else{
    printf("Connected\n");
  }

  while(1){
    // Receive header which includes the size information
    if(recv(sock, &hd, sizeof(hd), MSG_WAITALL) <= 0) break;
    printf("%08x\n", hd);
    // Calc. receive data size
    sz = RIDF_SZ(hd) * 2 - sizeof(hd);
    // copy header to buff
    memcpy(buff, (char *)&hd, sizeof(hd));
    // receive data
    if(recv(sock, buff+sizeof(hd), sz, MSG_WAITALL) <= 0) break;

    printf("Received size = %d\n", (sz + (int)sizeof(hd))/2);
  }

  printf("Connection lost\n");
  quit();

  return 0;
}
