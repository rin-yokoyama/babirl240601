/* babirl/devtool/stsrvridf.c
 * last modified : 10/12/10 13:14:54 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Serve RIDF data with streaming connection
 *  from file with 1s interval
 *
 *
 * stsrvridf [-i INTERVAL] [-m MAXCONNECTION] [-h] FILENAME
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>

/* babirl */
#include <libbabirl.h>
#include <ridf.h>
#include <isoba.h>
#include <bbzfile.h>

/* macros */
#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

struct sttharg{
  int com;
  int id;
  int sock;
};

/****** Globals *******/
/* constatns */
static int interval = 1;          // interval time (s)
static int maxconnection = 20;    // max connection
bgFile *fd = NULL;
pthread_t stthre[ISOBA_MAX_CONNECTION];         // Thread for clients
pthread_t filethre;                            // Thread for file reader
static pthread_mutex_t stmemmutex = PTHREAD_MUTEX_INITIALIZER; // for memory
static pthread_mutex_t stnummutex = PTHREAD_MUTEX_INITIALIZER; // for connection
fd_set fdset;
struct stbrstat st;


/* file descriptor */
int comfd;

/* buffer */

/* variables */
int connectionnumber, connectid[ISOBA_MAX_CONNECTION];

/*********/

/* Prototype */
void quit(void);
void help(void);

/* quit function */
void quit(void){
  FD_ZERO(&fdset);
  if(fd) bgclose(fd);
  if(comfd) close(comfd);
}

/* help usage */
void help(void){
  printf("stsrvridf [-i INTERVAL] [-m MAXCONNECTION] [-h] FILENAME\n");
  printf(" -i INTERVAL\n");
  printf("  INTERVAL       : interval time (s), default = 1s\n");
  printf(" -m MAXCONNECTION\n");
  printf("  MAXCONNECTION  : number of available connections, default = 10\n");
  printf(" -h\n");
  printf("   This help\n");
}

/* check option */
int chkopt(int argc, char *argv[]){
  int val;

  while((val=getopt(argc, argv, "i:m:")) != -1){
    switch(val){
    case 'h':
      help();
      return 0;
      break;
    case 'i':
      interval = strtol(optarg, NULL, 0);
      printf("Interval = %d\n", interval);
      break;
    case 'm':
      maxconnection = strtol(optarg, NULL, 0);
      if(maxconnection > ISOBA_MAX_CONNECTION || maxconnection <= 0){
	printf("0 < maxconnection < %d\n", ISOBA_MAX_CONNECTION);
	return 0;
      }
      printf("Max connection = %d\n", maxconnection);
      break;
    default:
      printf("Invalid option %c\n", val);
      return 0;
      break;
    }
  }

  return 1;
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
	if(send(sock, buff, size, 0) <= 0){
	  printf("stmain[%d]: Connection is closed\n", id);
	  /* goto exit */
	  break;
	}else{
	  /* noop */
	  DB(printf("stmain[%d]: Sent data blkn=%d size=%d\n", id, lblkn, size));
	}
      }
    }else{
      /* from first */
      pthread_mutex_lock(&stmemmutex);
      if(getrbuff(&st, buff, &size, rblkn)){
	pthread_mutex_unlock(&stmemmutex);
	if(send(sock, buff, size, 0) <= 0){
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


int st_connect(void){
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

void filemain(void){
  RIDFHD hd;
  RIDFRHD rhd;
  char buff[256*1024];
  int sz;

  memset((char *)&st, 0, sizeof(st));
  st.max = ISOBA_MAX_BUFFER;

  while(bgread(fd, (char *)&hd, sizeof(hd)) > 0){
    memcpy(buff, (char *)&hd, sizeof(hd));
    rhd = ridf_dechd(hd);
    printf("read classid=%d sz=%d\n", rhd.classid, rhd.blksize);
    bgread(fd, buff+sizeof(hd), rhd.blksize*2 - sizeof(hd));
    sz = rhd.blksize*2;
    pthread_mutex_lock(&stmemmutex);
    addrbuff(&st, buff, sz);
    pthread_mutex_unlock(&stmemmutex);
    sleep(interval);
  }

  bgclose(fd);
  fd = NULL;
  printf("End of file\n");
}


int main(int argc, char *argv[]){
  if(!chkopt(argc, argv)){
    exit(0);
  }

  /* Signal SIGINT -> quit() */
  signal(SIGINT, (void *)quit);
  signal(SIGPIPE, SIG_IGN);

  if((fd = bgropen(argv[argc-1])) == NULL){
    printf("Can't open %s\n", argv[argc]);
    exit(1);
  }

  /* Initialize */
  connectionnumber = 0;
  memset(connectid, 0, sizeof(connectid));
  
  /* Make command port */
  if((comfd = mktcpsock(ISOBA_STCOMPORT)) == -1) quit();

  printf("comfd = %d\n", comfd);

  /* Thread Create */
  pthread_create(&filethre, NULL, (void *)filemain, NULL);
  pthread_detach(filethre);


  pthread_mutex_init(&stmemmutex, NULL);
  pthread_mutex_init(&stnummutex, NULL);

  while(1){
    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    
    if(select(comfd+1, &fdset, NULL, NULL, NULL) != 0){
      if(FD_ISSET(comfd, &fdset)){
	if(!st_connect()) break;
      }
    }
  }
  

  return 0;
}
