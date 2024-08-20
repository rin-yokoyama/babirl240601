/* babirl : babian/babian.c
 * last modified : 08/10/14 21:18:25 
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
#include <pthread.h>

/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <isoba.h>

struct sttharg{
  int com;
  int id;
  int sock;
};

struct stwtharg{
  /*** Streaming Sockets ***/
  int comfd;
  int maxsockid;
};

/****** Globals *******/
int sock[MAXANPT], listlen, shmid[MAXANPT], semid[MAXANPT], clen[MAXANPT];
struct sockaddr_in saddr[MAXANPT], caddr[MAXANPT];
char *shmp[MAXANPT], *fshmp[MAXANPT];
struct sembuf semb[MAXANPT];
union semun semunion[MAXANPT];

/* constatns */
static int maxconnection = 20;    // max connection
pthread_t stthre[ISOBA_MAX_CONNECTION];         // Thread for clients
pthread_t stwthre;         // Thread for waiting process
static pthread_mutex_t stmemmutex = PTHREAD_MUTEX_INITIALIZER; // for memory
static pthread_mutex_t stnummutex = PTHREAD_MUTEX_INITIALIZER; // for connection
struct stbrstat st;

/* buffer */
char *data[MAXANPT];

/* variables */
int connectionnumber, connectid[ISOBA_MAX_CONNECTION];

/*********/

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

int stmain(struct sttharg *tharg){
  int id, com, sock, last, size;
  unsigned int lblkn, rblkn;
  char buff[256*1024];

  // Copy arg to local variables
  pthread_mutex_lock(&stnummutex);
  id = tharg->id;
  com = tharg->com;
  sock = tharg->sock;
  pthread_mutex_unlock(&stnummutex);

  // Check first/last block number

  switch(com){
  case ISOBA_COM_FROMLAST:
    last = 1;
    break;
  case ISOBA_COM_FROMFIRST:
    last = 0;
    break;
  default :
    last = 1;
    break;
  }

  DB(printf("stmain[%d] com=%d\n", id, com));

  if(!last){
    /* check first blkn */
    pthread_mutex_lock(&stmemmutex);
    rblkn = st.first->blkn;
    pthread_mutex_unlock(&stmemmutex);
  }else{
    rblkn = 0;
  }

  while(1){
    usleep(10000);
    if(last){
      pthread_mutex_lock(&stmemmutex);
      lblkn = st.last->blkn;
      if(rblkn != lblkn){
        size = st.last->size;
        memcpy(buff, st.last->data, size);
      }
      pthread_mutex_unlock(&stmemmutex);
      if(rblkn != lblkn){
        rblkn = lblkn;
        if(send(sock, buff, size, 0) < 0){
          printf("stmain[%d]: Connection is closed\n", id);
          /* goto exit */
          break;
        }else{
          /* noop */
          DB(printf("stmain[%d]: Sent ldata blkn=%d size=%d\n", id, lblkn, size));
        }
      }
    }else{
      /* from first */
      pthread_mutex_lock(&stmemmutex);
      if(getrbuff(&st, buff, &size, rblkn)){
        pthread_mutex_unlock(&stmemmutex);
        if(send(sock, buff, size, 0) < 0){
          printf("stmain[%d]: Connection is closed\n", id);
          /* goto exit */
          break;
        }else{
          DB(printf("stmain[%d]: Sent data blkn=%d\n", id, rblkn));
          rblkn++;
        }
      }else{
        pthread_mutex_unlock(&stmemmutex);
        DB(printf("stmain[%d]: Reach last block\n", id));
        last = 1;
      }
    }
  }

  close(sock);
  pthread_mutex_lock(&stnummutex);
  connectid[id] = 0;
  connectionnumber--;
  pthread_mutex_unlock(&stnummutex);

  return 0;
}

int st_connect(int comfd){
  struct sockaddr_in caddr;
  int tsock, clen, ret, i;
  struct sttharg tharg;

  ret = 0;

  DB(printf("st_connect: \n"));

  clen = sizeof(caddr);
  if((tsock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("Error in accept st_connect\n");
    return 0;
  }
  DB(printf("st_connect: receiving\n"));
  /* Receive options from client */
  recv(tsock, (char *)&tharg.com, sizeof(int), MSG_WAITALL);
  DB(printf("st_connect: received com=%d\n", tharg.com));

  /* lock connection numbers */
  pthread_mutex_lock(&stnummutex);
  connectionnumber++;
  if(connectionnumber > maxconnection){
    ret = ISOBA_CONNECTION_EXCEED;
    connectionnumber--;
    pthread_mutex_unlock(&stnummutex);
    send(tsock, (char *)&ret, sizeof(ret), 0);
    close(tsock);
    printf("Reach maximum connection\n");
    return 1;
  }

  for(i=0;i<maxconnection;i++){
    if(!connectid[i]){
      tharg.id = i;
      connectid[i] = 1;
      break;
    }
  }
  tharg.sock = tsock;
  pthread_mutex_unlock(&stnummutex);

  ret = ISOBA_CONNECTION_OK;
  send(tsock, (char *)&ret, sizeof(ret), 0);

  /* Thread Create */
  pthread_create(&stthre[tharg.id], NULL, (void *)stmain, &tharg);
  pthread_detach(stthre[tharg.id]);
  usleep(10);

  return 1;
}

/* process to wait the streaming connection */
int stwaitmain(struct stwtharg *tharg){

  fd_set fdset;
  int comfd = tharg->comfd;
  int maxsockid = tharg->maxsockid;
  while(1){
    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    if(select(maxsockid+1, &fdset, NULL, NULL, NULL) != 0){
      if(FD_ISSET(comfd, &fdset)){
        if(!st_connect(comfd)) break;
      }
    }
  }

  return 0;

}

int main(int argc, char *argv[]){
  unsigned int blocknum[MAXANPT];
  RIDFHD hd;
  RIDFRHD rhd;
  int recvsz, i, rid;
  struct stwtharg tharg;

  /* for fetching data from babinfo */
  unsigned int orgnum = 0xffffffff, num = 0;
  char ebhostname[128];

  for(i=0;i<MAXANPT;i++){
    if((data[i] = malloc(EB_EFBLOCK_BUFFSIZE * WORDSIZE)) == NULL){
      printf("too large buffer %d\n", i);
      quit();
    }
  }

  for(i=0;i<MAXANPT;i++){
    blocknum[i] = 0;
    semb[i].sem_num = 0;
    semb[i].sem_flg = SEM_UNDO;
    sock[i] = 0;
  }

  if(argc != 2){
    printf("babianisopull SERVERNAME\n");
    printf(" for the moment, data copied into shmid=0\n");
    printf(" for the moment, take data each 100ms\n");
    exit(0);
  }else{
    strncpy(ebhostname, argv[1], sizeof(ebhostname));
  }


  /* PID file */
  if(chkpid("babianisopull")){
    printf("babianisopull: Error, another babian may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babianisopull\n");
    exit(0);
  }

  /* Signal */
  signal(SIGINT, (void *)quit);

#ifndef DEBUG
  daemon(1, 0);
#endif
  if(!mkpid("babianisopull")){
    printf("babianisopull: Error, another babianisopull may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babianisopull\n");
    exit(0);
  }

  /* Shared Memory */
  for(i=0;i<MAXANPT;i++){
    if(!(shmid[i] = initshm(ANSHMKEY+i*2, (EB_EFBLOCK_BUFFSIZE * WORDSIZE) + sizeof(blocknum[i]), &shmp[i]))){
      printf("Can't create shared memory %d\n", i);
      exit(0);
    }
    fshmp[i] = shmp[i] + (EB_EFBLOCK_BUFFSIZE * WORDSIZE);
    
    /* Semaphore */
    if(!(semid[i] = initsem(ANSEMKEY+i*2, &semunion[i]))){
      printf("Can't create semaphore %d\n", i);
      quit();
    }
  }

  tharg.maxsockid = 0;
  for(i=0;i<MAXANPT;i++){
    memset(data[i], 0, (EB_EFBLOCK_BUFFSIZE * WORDSIZE));
    memset(shmp[i], 0, (EB_EFBLOCK_BUFFSIZE * WORDSIZE) + sizeof(blocknum[i]));
  }
  
  /* Initialize */
  connectionnumber = 0;
  memset(connectid, 0, sizeof(connectid));

  /* Make command port, and register to fd */
  if((tharg.comfd = mktcpsock(ISOBA_STCOMPORT)) == -1) quit();
  if(tharg.comfd > tharg.maxsockid) tharg.maxsockid = tharg.comfd;

  /* Thread Create for waiting streaming connection*/
  pthread_create(&stwthre, NULL, (void *)stwaitmain, &tharg);
  pthread_detach(stwthre);

  rid = 0; //for the moment, only use shmid = 0
  /* List Data Socket */
  while(1){
    usleep(100000);
    if((sock[0] = infcon(ebhostname))){
      recvsz = eb_get(sock[0], INF_GET_BLOCKNUM, (char *)&num);
      close(sock[0]);
    }else{
      continue;
    }
    if(num != orgnum){
      orgnum = num;
      if((sock[0] = infcon(ebhostname))){
        recvsz = eb_get(sock[0], INF_GET_RAWDATA, data[rid]);
        close(sock[0]);
      }
    }else{
      continue;
    }

    memcpy((char *)&hd, data[rid], sizeof(hd));
    rhd = ridf_dechd(hd);

    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      sem_p(semid[rid], &semb[rid]);
      memcpy(shmp[rid], data[rid], rhd.blksize * WORDSIZE);

      if(rid == 0){ // currently only rid==0 is supported, T.I. 
	pthread_mutex_lock(&stmemmutex);
	addrbuff(&st, data[rid], rhd.blksize * WORDSIZE);
	pthread_mutex_unlock(&stmemmutex);
      }

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

  return 0;
}

