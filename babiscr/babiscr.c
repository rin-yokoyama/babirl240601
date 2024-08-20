/* last modified : 08/09/13 20:32:07 
 *
 * babirl/babiscr/babiscr.c
 *
 * Scaler reader for babies (with -s mode)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/file.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define  COMFIFO "/tmp/babirldbfifo"
#define  EFSIZE 20000

#define SCRTIMER 1

/* Globals */
char pidpath[128];
unsigned short data[EFSIZE*2];
int mp, tmp, scrhds, hds;
RIDFHD hd;
RIDFHDSCR scrhd;

int efn, shmid, idx, cfd;
char *buff, *buffat;
int scrsize, eventsize = 0, segmentsize = 0;

#include "scr.c"

/* Prototype */
int datacopy(void);
void quit(void);
void readscr(void);

static volatile int last=0;

void quit(void){

  last = 1;

  readscr();
  usleep(10000);

  idx = -1;
  flock(cfd, LOCK_EX);
  write(cfd, (char *)&idx, sizeof(idx));
  flock(cfd, LOCK_UN);

  unlink(pidpath);

  exit(0);
}

void readscr(void){
  int t;
  time_t tt;

  mp = hds + scrhds;
  tmp = mp;
  scrsize = 0;
  scrsize = scr();
  scrsize += scrhds;

  time(&tt);
  t = (int)tt;
  scrhd = ridf_mkhd_scr(RIDF_LY1, RIDF_NCSCALER32, scrsize, efn, t, efn);
  memcpy((char *)(data+hds), (char *)&scrhd, sizeof(scrhd));
  while(!datacopy());

}

int datacopy(void){
  if(buff[EF_SHM_FLAG1] && buff[EF_SHM_FLAG2]){
    DB(printf("Both buffer full\n"));
    printf("Both buffer full\n");
    sleep(1);
    if(last == 1){
      return 1;
    }else{
      return 0;
    }
  }else{
    if(idx == 0 && !buff[EF_SHM_FLAG1]){
      DB(printf("idx %d : memcpy\n", idx));
      buffat = buff + EF_SHM_DATA1;
      *(buff + EF_SHM_FLAG1) = EF_SHM_READY1;
    }else if(idx == 1 && !buff[EF_SHM_FLAG2]){
      DB(printf("idx %d : memcpy\n", idx));
      buffat = buff + EF_SHM_DATA2;
      *(buff + EF_SHM_FLAG2) = EF_SHM_READY2;
    }else{
      printf("Invalid shared memory management\n");
      printf("idx   = %d\n", idx);
      printf("flag1 = %d\n", buff[EF_SHM_FLAG1]);
      printf("flag2 = %d\n", buff[EF_SHM_FLAG2]);
      if(last == 1){
	return 1;
      }else{
	return 0;
      }
    }
    
    hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, mp, efn);
    memcpy((char *)data, (char *)&hd, hds*WORDSIZE);
    memcpy(buffat, (char *)data, mp*WORDSIZE);
    
    mp = hds;
    
    flock(cfd, LOCK_EX);
    write(cfd, (char *)&idx, sizeof(idx));
    flock(cfd, LOCK_UN);
    if(idx == 0){
      idx = 1;
    }else{
      idx = 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[]){
  int pid;
  FILE *fd;

  memset(pidpath, 0, sizeof(pidpath));
  cfd = 0;

  if(argc != 2){
    printf("babiscr EFN\n");
    exit(0);
  }

  efn = strtol(argv[1], NULL, 0);

  sprintf(pidpath, "%s.pid", argv[0]);
  printf("%s\n", pidpath);

  pid = (int)getpid();

  if((fd = fopen(pidpath, "w")) == NULL){
    printf("babiscr: Can't open PID file %s\n", pidpath);
    exit(1);
  }

  fprintf(fd, "%d", pid);
  fclose(fd);

  if((cfd = open(COMFIFO, O_RDWR)) < 0){
    printf("Can't open COMFIFO\n");
    unlink(pidpath);
    exit(1);
  }

  signal(SIGINT, (void *)quit);

  buff = 0;
  if((shmid = initshm(EFSHMKEY+efn, EF_SHM_SIZE, &buff)) == -1){
    printf("initshm failed  shmid = %d\n", shmid);
    exit(0);
  }
  if(!buff){
    printf("Can't allocate shared memory %p\n", buff);
    exit(0);
  }
  DB(printf("shmid = %d / addr = %p\n", shmid, buff));
  memset(buff, 0, EF_SHM_SIZE);

  scrhds = sizeof(scrhd)/WORDSIZE;
  hds = sizeof(hd)/WORDSIZE;

  scrinit();
  while(1){
    readscr();

    sleep(SCRTIMER);
  }

  return 0;
}
