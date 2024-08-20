/* babirl : devtool/curblk.c
 * last modified : 08/03/13 11:49:23 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Get current one block from babinfo
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <bi-config.h>
#include <bi-common.h>

int infcon(char *host){
  int infsock;

  /* Connect to babild */
  if(!(infsock = mktcpsend(host, INFCOMPORT))){
    printf("curblk: Can't connet to babinfo.\n");
    exit(0);
    return 0;
  }
  
  return infsock;
}


int main(int argc, char *argv[]){
  int sock, len, i,k ;
  char buff[EB_EFBLOCK_BUFFSIZE];

  if(argc != 2){
    printf("curblk HOSTNAME\n");
    exit(0);
  }

  sock = infcon(argv[1]);

  len = eb_get(sock, INF_GET_RAWDATA, buff);
  close(sock);

  printf("length %d\n", len);

  for(i=0;i<10;i++){
    for(k=0;k<4;k++){
      printf("%08x ", *(unsigned int *)(buff+k*4+i*16));
    }
    printf("\n");
  }


  return 0;
}
