/* babirl : babian/babian.c
 * last modified : 14/05/02 10:09:07 
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

int sock, listlen, shmid, semid, clen;
struct sockaddr_in saddr, caddr;
char *shmp, *fshmp;
struct sembuf semb;
union semun semunion;

int infcon(char *host){
  int infsock;

  /* Connect to babild */
  if(!(infsock = mktcpsend(host, INFCOMPORT))){
    printf("babicon: Can't connet to babinfo.\n");
    //exit(0);
    return 0;
  }
  
  return infsock;
}

char *data;

/* Quit */
void quit(void){

  if(sock){
    close(sock);
  }

  if(shmid){
    delshm(shmid, shmp);
  }
  if(semid){
    delsem(semid, &semunion);
  }
  
  if(data) free(data);


  rmpid("babianpull");

  exit(0);
}

int main(int argc, char *argv[]){
  unsigned int blocknum;
  RIDFHD hd;
  RIDFRHD rhd;
  unsigned int orgnum = 0xffffffff, num = 0;
  char ebhostname[128];
  int buffsize;

  //buffsize = EB_EFBLOCK_BUFFSIZE*2;
  //if(buffsize > 8 * 1024 * 1024){
  buffsize = 8 * 1024 * 1024; //8MB fix
  //}
  
  if((data = malloc(buffsize)) == NULL){
    printf("too large buffer %d\n", buffsize);
    quit();
  }
  

  blocknum = 0;
  semb.sem_num = 0;
  semb.sem_flg = SEM_UNDO;
  sock = 0;

   if(argc != 2){
    printf("babianpull SERVERNAME\n");
    printf(" for the moment, data copied into shmid=6\n");
    printf(" for the moment, take data each 100ms\n");
    exit(0);
  }else{
    strncpy(ebhostname, argv[1], sizeof(ebhostname)-1);
  }
    
  /* PID file */
  if(chkpid("babianpull")){
    printf("babianpull: Error, another babianpull may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babian/pull\n");
    exit(0);
  }

  /* Signal */
  signal(SIGINT, (void *)quit);

#ifndef DEBUG
  daemon(1, 0);
#endif
  if(!mkpid("babianpull")){
    printf("babianpull: Error, another babian may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babianpull\n");
    exit(0);
  }


  /* Shared Memory */
  if(!(shmid = initshm(ANSHMKEY+MAXANPT*2, buffsize + sizeof(blocknum), &shmp))){
    printf("Can't create shared memory %d\n", MAXANPT);
    quit();
  }
  fshmp = shmp + buffsize;
    
  /* Semaphore */
  if(!(semid = initsem(ANSEMKEY+MAXANPT*2, &semunion))){
    printf("Can't create semaphore %d\n", MAXANPT);
    quit();
  }

  memset(data, 0, buffsize);
  memset(shmp, 0, buffsize + sizeof(blocknum));

  /* List Data Socket */
  while(1){
    usleep(100000); // 10evt/s 
    if((sock = infcon(ebhostname))){
      eb_get(sock, INF_GET_BLOCKNUM, (char *)&num);
      close(sock);
    }else{
      continue;
    }
    if(num != orgnum){
      orgnum = num;
      if((sock = infcon(ebhostname))){
	eb_get(sock, INF_GET_RAWDATA, data);
	close(sock);
	//DB(printf("received new data size=%d\n", recvsz));
      }else{
	continue;
      }
    }else{
      continue;
    }
      
    memcpy((char *)&hd, data, sizeof(hd));
    rhd = ridf_dechd(hd);

    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      sem_p(semid, &semb);
      memcpy(shmp, data, rhd.blksize * WORDSIZE);
      if(blocknum == 0xffffff00){
	blocknum = 0;
      }else if(data[0] == 0x0a){
	blocknum = 0xffffffff;
      }else{
	blocknum++;
      }
      memcpy(fshmp, (char *)&blocknum, sizeof(blocknum));
      sem_v(semid, &semb);
      break;
    default:
      printf("\n ****** Error header ******\n");
      printf("layer = %d, classid = %d\n", rhd.layer, rhd.classid);
      printf("blksize = %d, efn = %d\n", rhd.blksize, rhd.efn);
      printf("\n");
      break;
    }
  }
  
  return 0;
}

