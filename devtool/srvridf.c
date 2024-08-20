/* devtool/srvridf.c
 * 
 * last modified : 08/10/14 21:19:10 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Data transfer to babian from RIDF file
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  //char buff[EB_EFBLOCK_BUFFSIZE * WORDSIZE];
  char *buff;
  FILE *fd;
  int intr = 1, sock, sid;
  char hostname[] = "localhost\0";
  struct sockaddr_in caddr;

  buff = (char *)malloc(EB_EFBLOCK_BUFFSIZE * WORDSIZE);

  sid = 0;
  printf("%d\n", __LINE__);

  if(argc < 2){
    printf("srvridf FILENAME [INTERVAL]\n");
    printf("  default INTERVAL = 1 s\n");
    exit(0);
  }
  printf("%d\n", __LINE__);

  if((fd = fopen(argv[1], "r")) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }
  printf("%d\n", __LINE__);

  if(argc == 3){
    if((intr = strtol(argv[2], NULL, 0)) < 1){
      intr = 1;
    }
  }

  memset(buff, 0, EB_EFBLOCK_BUFFSIZE * WORDSIZE);

  if(!(sock = mkudpsend(ANRCVPORT+sid, &caddr, hostname))){
    printf("Can't make socket\n");
    exit(0);
  }


  printf("%d\n", __LINE__);

  while(fread((char *)&hd, sizeof(hd), 1, fd) == 1){
    rhd = ridf_dechd(hd);

    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      memcpy(buff, (char *)&hd, sizeof(hd));
      fread(buff+sizeof(hd), 2, rhd.blksize - sizeof(hd)/WORDSIZE, fd);
      break;
    default:
      printf("Error in RIDF file\n");
      fclose(fd);
      return 0;
      break;
    }
    
    printf("SENDTO\n");
    sendto(sock, buff, rhd.blksize * WORDSIZE, 0,
	   (struct sockaddr *)&caddr, sizeof(caddr));

    sleep(intr);
  }

  close(sock);
  fclose(fd);

  return 0;
}
