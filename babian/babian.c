/* babirl : babian/babian.c
 * last modified : 14/04/17 12:15:55 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Data analysis server
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

/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

int sock[MAXANPT], listlen, shmid[MAXANPT], semid[MAXANPT], clen[MAXANPT];
struct sockaddr_in saddr[MAXANPT], caddr[MAXANPT];
char *shmp[MAXANPT], *fshmp[MAXANPT];
struct sembuf semb[MAXANPT];
union semun semunion[MAXANPT];

/* Quit */
void quit(void){
  int i;

  for(i=0;i<MAXANPT;i++){
    if(sock[i]){
      close(sock[i]);
    }

    if(shmid[i]){
      delshm(shmid[i], shmp[i]);
    }
    if(semid[i]){
      delsem(semid[i], &semunion[i]);
    }
  }

  rmpid("babian");

  exit(0);
}

int main(int argc, char *argv[]){
  unsigned int blocknum[MAXANPT];
  //char data[MAXANPT][EB_EFBLOCK_BUFFSIZE * WORDSIZE];
  char data[MAXANPT][BABIAN_SHM_BUFF_SIZE]; // Shared memory size = 512kB fix
  RIDFHD hd;
  RIDFRHD rhd;
  int i, maxsockid, rid;
  fd_set fdset;

  for(i=0;i<MAXANPT;i++){
    blocknum[i] = 0;
    semb[i].sem_num = 0;
    semb[i].sem_flg = SEM_UNDO;
    sock[i] = 0;
  }

  /* PID file */
  if(chkpid("babian")){
    printf("babian: Error, another babian may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babian\n");
    exit(0);
  }

  /* Signal */
  signal(SIGINT, (void *)quit);

#ifndef DEBUG
  daemon(1, 0);
#endif
  if(!mkpid("babian")){
    printf("babian: Error, another babian may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babian\n");
    exit(0);
  }


  /* Shared Memory */
  for(i=0;i<MAXANPT;i++){
    if(!(shmid[i] = initshm(ANSHMKEY+i*2, sizeof(data[i]) + sizeof(blocknum[i]), &shmp[i]))){
      printf("Can't create shared memory %d\n", i);
      exit(0);
    }
    fshmp[i] = shmp[i] + sizeof(data[i]);
    
    /* Semaphore */
    if(!(semid[i] = initsem(ANSEMKEY+i*2, &semunion[i]))){
      printf("Can't create semaphore %d\n", i);
      quit();
    }
  }

  maxsockid = 0;
  for(i=0;i<MAXANPT;i++){
    memset(data[i], 0, sizeof(data[i]));
    memset(shmp[i], 0, sizeof(data[i]) + sizeof(blocknum[i]));

    if(!(sock[i] = mkudpsock(ANRCVPORT+i, &saddr[i]))){
      quit();
    }
    clen[i] = sizeof(caddr[i]);
    if(sock[i] > maxsockid){
      maxsockid = sock[i];
    }
  }
  

  /* List Data Socket */
  while(1){
    FD_ZERO(&fdset);
    for(i=0;i<MAXANPT;i++){
      FD_SET(sock[i], &fdset);
    }
    rid = 0;

    if(select(maxsockid+1, &fdset, NULL, NULL, NULL) != 0){
      for(i=0;i<MAXANPT;i++){
	if(FD_ISSET(sock[i], &fdset)) rid = i;
      }
      DB(printf("recvfrom rid = %d\n", rid));

      recvfrom(sock[rid], data[rid], sizeof(data[rid]), 0,
	       (struct sockaddr *)&caddr[rid], (socklen_t*)&clen[rid]);
      
      memcpy((char *)&hd, data[rid], sizeof(hd));
      rhd = ridf_dechd(hd);

      switch(rhd.classid){
      case RIDF_EF_BLOCK:
      case RIDF_EA_BLOCK:
      case RIDF_EAEF_BLOCK:
	sem_p(semid[rid], &semb[rid]);
	memcpy(shmp[rid], data[rid], rhd.blksize * WORDSIZE);
	if(blocknum[rid] == 0xffffff00){
	  blocknum[rid] = 0;
	}else if(data[rid][0] == 0x0a){
	  blocknum[rid] = 0xffffffff;
	}else{
	  blocknum[rid]++;
	}
	memcpy(fshmp[rid], (char *)&blocknum[rid], sizeof(blocknum[rid]));
	sem_v(semid[rid], &semb[rid]);
	break;
      default:
	printf("\n ****** Error header ******\n");
	printf("layer = %d, classid = %d\n", rhd.layer, rhd.classid);
	printf("blksize = %d, efn = %d\n", rhd.blksize, rhd.efn);
	printf("\n");
	break;
      }
    }
  }

  return 0;
}

