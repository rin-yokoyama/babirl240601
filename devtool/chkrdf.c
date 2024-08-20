/* babirl/devtool/chkrdf.c
 * 
 * last modified : 06/12/26 16:24:03 
 * 
 * RDF checker
 *
 * Hidetada Baba
 * baba@ribf.riken.jp
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rdf.h>

#define BLKSIZE 8192

struct rdf_blkinfost info;
unsigned short buff[BLKSIZE];

int main(int argc, char *argv[]){
  FILE *fd;
  int evtn;
  int i, j, k;

  if(argc != 2){
    printf("chkrdf FILENAME\n");
    exit(0);
  }

  if((fd = fopen(argv[1], "r")) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }

  while(fread(buff, 2, BLKSIZE, fd) == BLKSIZE){
    memset((char *)&info, 0, sizeof(info));
    evtn = rdf_scanblk(buff, &info, BLKSIZE);
    printf("evtn = %d\n", evtn);

    for(i=0;i<evtn;i++){
      printf("evtn=%d, size=%d, ptr=%p\n", i, info.evtsize[i], info.evtptr[i]);
      for(k=0;k<info.evtsize[i];k++){
	printf("%04x ", info.evtptr[i][k]);
      }
      printf("\n");
      for(j=0;j<RDF_MAXSEGID;j++){
	if(info.segptr[i][j]){
	  printf("segid=%d, size=%d, ptr=%p\n",
		 info.segid[i][j], info.segsize[i][j], info.segptr[i][j]);
	  for(k=0;k<info.segsize[i][j];k++){
	    printf("%04x ", info.segptr[i][j][k]);
	  }
	  printf("\n");
	  getchar();
	}
      }
    }

    //if(evtn) break;
  }

  fclose(fd);

  return 0;
}
