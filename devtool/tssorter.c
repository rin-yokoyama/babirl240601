/* babirl : devtool/tssorter.c
 * last modified : 09/12/05 01:04:31 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Time stamp sorting program
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>

/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

struct tsbst{
  unsigned long long int ts;
  unsigned long long int fp;
};

int rfd, wfd;
struct tsbst *tsb;


void quit(void);

void quit(void){
  if(rfd) close(rfd);
  if(wfd) close(wfd);
  if(tsb) free(tsb);

  exit(0);
}

int comparets(const void *a, const void *b){
  long long int ats=0, bts=0;

  memcpy((char *)&ats, (char *)a, 7);
  memcpy((char *)&bts, (char *)b, 7);

  if(ats > bts){
    return 1;
  }else if(ats < bts){
    return -1;
  }else{
    return 0;
  }

}


int main(int argc, char *argv[]){
  //RIDFHD hd;
  //RIDFHDBLKN blknhd;
  struct stat fst;

  printf("tssorter\n");
  
  if((rfd = open(argv[1], O_RDONLY)) < 0){
    printf("Can't open time stamp table %s\n", argv[1]);
    quit();
  }

  if((wfd = open(argv[2], O_WRONLY|O_CREAT, 0644)) < 0){
    printf("Can't open output table %s\n", argv[2]);
    quit();
  }

  fstat(rfd, &fst);
  tsb = malloc(fst.st_size);
  
  if(tsb == NULL){
    printf("Can't malloc memory (size=%llu)\n", (unsigned long long int )fst.st_size);
    quit();
  }
  
  read(rfd, tsb, fst.st_size);

  qsort(tsb, fst.st_size/sizeof(struct tsbst), sizeof(struct tsbst), comparets);

  write(wfd, tsb, fst.st_size);


  quit();

  return 0;
}

