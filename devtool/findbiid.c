#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>

const int rport = BABIMOPORT;
int sock, rsock;
struct sockaddr_in saddr, caddr, raddr;

void udprecv(void){
  unsigned char data[256];
  int magic;
  int clen;
  struct sockaddr_in caddr;
  struct hostent *host;

  while(1){
    clen = sizeof(caddr);
    recvfrom(rsock, (char *)data, sizeof(data), 0,
	     (struct sockaddr *)&caddr, (socklen_t*)&clen);
    memcpy((char *)&magic, data, 4);
    if(magic == 0xf7000000){
      printf("------------------\n");
      printf("BIID = %s\n", data+16);
      printf("IP   = %u.%u.%u.%u\n", data[64],data[65],data[66],data[67]);
      printf("MAC  = %02X:%02X:%02X:%02X:%02X:%02X\n",
	     data[72],data[73],data[74],data[75],
	     data[76],data[77]);
      
      host = gethostbyaddr(&caddr.sin_addr.s_addr,
			   sizeof(caddr.sin_addr.s_addr), AF_INET);
      if(host != NULL){
	printf("Host = %s\n", host->h_name);
      }

      if(data[96]){
	printf("Ver  = %s\n", data+96);
      }
      printf("------------------\n\n");
    }
    
  }
}


int main(int argc, char *argv[]){
  pthread_t udpthread;
  char com[4];
  unsigned int t = 1;
  short port, mag=0xf600;

  port = BABIMOPORT + 1;

  rsock = mkudpsock(port, &raddr);
  pthread_create(&udpthread, NULL, (void *)udprecv, NULL);
  pthread_detach(udpthread);


  sock = mkbroadsend(rport, &saddr);
  memcpy(com, (char *)&port, 2);
  memcpy(com+2, (char *)&mag, 2);
  sendto(sock, com, 4, 0, (struct sockaddr *)&saddr, sizeof(saddr));


  if(argc > 1){
    t = strtoul(argv[1], NULL, 0);
    printf("Wait %u seconds\n", t);
  }

  sleep(t);


  return 0;
}

