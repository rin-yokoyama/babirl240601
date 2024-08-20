/* babirl : devtool/monan.c
 * last modified : 07/01/18 14:37:31 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Monitor babian shared memory
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>


/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  struct sembuf semb;
  union semun semunion;
  char *shmp, *fshmp;
  unsigned int blocknum, tblocknum;
  int shmid, semid, i;
  unsigned short sval;

  blocknum = 0;

  /* Shared Memory */
  //  if(!(shmid = initshm(ANSHMKEY,
  //		       EB_EFBLOCK_BUFFSIZE * WORDSIZE + sizeof(blocknum),
  //		       &shmp))){
  if(!(shmid = initshm(ANSHMKEY,
		       BABIAN_SHM_BUFF_SIZE + sizeof(blocknum),
		       &shmp))){
    printf("Can't create shared memory\n");
    exit(0);
  }
  fshmp = shmp + BABIAN_SHM_BUFF_SIZE;

  //fshmp = shmp + EB_EFBLOCK_BUFFSIZE * WORDSIZE;

  /* Semaphore */
  if(!(semid = initsem(ANSEMKEY, &semunion))){
    printf("Can't create semaphore\n");
    exit(0);
  }

  while(1){
    sem_p(semid, &semb);     // Lock shared memory
    memcpy((char *)&tblocknum, fshmp, sizeof(blocknum));
    if(blocknum != tblocknum){
      blocknum = tblocknum;
      memcpy((char *)&hd, shmp, sizeof(hd));
      rhd = ridf_dechd(hd);
      printf("ly=%d, cid=%d, size=%d, efn=%d\n",
	     rhd.layer, rhd.classid, rhd.blksize, rhd.efn);         
      for(i=0;i<64;i+=2){
	if(i%16 == 0){
	  printf("\n");
	}
	memcpy((char *)&sval, shmp+i, sizeof(sval));
	printf("%04x ", sval);
      }
      printf("\n");
    }
    sem_v(semid, &semb);     // Unlock shared memory

    sleep(1);
  }

  return 0;
}
