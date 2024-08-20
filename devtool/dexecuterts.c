/* devtool/dexecuter.c
 * last modified : 09/08/19 14:18:29 
 *
 * Dummy data generator for babies (CC/NET mode)
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 */


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/file.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#include "dexecuter.h"

#define SEGID 5
#define EFSIZE 20000

#define COMFIFO "/tmp/babirldbfifo"

//#define TIMEOUT 60000
#define TIMEOUT 0
#define FRAME_LEN 300
#define BUF_LEN FRAME_LEN*2

double evtr, us;
volatile unsigned int evtn, oevtn, devtn;
unsigned long long int dt;
struct timeval ta, tb;
double tevtr, devtr;
int l, efsize;
char *babiesrun;

int j, i;

/* Prototypes for library */
int initshm();

int fd, cfd, dispersion, evtlen;
/* for babies */
int efn, bshmid, idx, size, mp;
char *buff, *buffat;
RIDFHD hd;
RIDFHDEVTTS evthd;
RIDFHDSEG seghd;

/* for event fragment */
unsigned short lbuff[EFSIZE*2];
unsigned int segid;
int hds, evthds, seghds;
int segs, evts;
volatile char ssflag, ssf;
long long int lots;

/*    int data,q,x,lam_pattern; */
int len,len1,status,actual_length,length;
int commentlen, q, x; 
int cmdbuf[BUF_LEN+2],rplybuf[BUF_LEN+2];
int clrbuf[BUF_LEN+2];
unsigned short databuf[BUF_LEN+2];
unsigned short data;

#ifdef DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

void quit(void){
  if(fd) close(fd);
  if(cfd) close(cfd);

  exit(0);
}


void ini(void){
  gettimeofday(&tb, NULL);
  oevtn = 0;
  alarm(1);
}

void chk(void){
  if(ssflag){

    gettimeofday(&ta, NULL);
    
    dt = (ta.tv_sec - tb.tv_sec)*1000*1000
      + (ta.tv_usec - tb.tv_usec);

    devtn = evtn - oevtn;
    tevtr = (double)devtn*1000.*1000./(double)dt;
    
    devtr = (double)evtr - tevtr;
    
    us -= devtr;
    if(us < 1){
      us = 1;
      l++;
    }
  }

  alarm(1);
}


int datacopy(void){
  if(buff[EF_SHM_FLAG1] && buff[EF_SHM_FLAG2]){
    DB(printf("Both buffer full\n"));
    printf("Both buffer full\n");
    sleep(1);
    return 0;
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
      return 0;
    }
    
    hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, mp, efn);
    memcpy((char *)lbuff, (char *)&hd, hds*WORDSIZE);
    memcpy(buffat, (char *)lbuff, mp*WORDSIZE);

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
  fd = 0;
  cfd = 0;
  evtn = 0;
  segid = SEGID;
  ssflag = 0;
  ssf = 0;
  
  if(argc != 6){
    printf("dexecuter EFN EVTRATE EVTLEN DISPERSION EFSIZE\n\n");
    printf("EFN        : Event Fragment Number 0-254\n");
    printf("EVTRATE    : Event rate (evt/s) 1-10000\n");
    printf("EVTLEN     : Length of one event (short word) 1-200\n");
    printf("DISPERSION : Dispersion of the length of one event (short word) 1-100\n");
    printf("EFSIZE     : Block size 1-%d (short word)\n", EFSIZE);
    exit(0);
  }
  efn = strtol(argv[1], NULL, 0);
  evtr = (double)strtol(argv[2], NULL, 0);
  actual_length = strtol(argv[3], NULL, 0);
  dispersion = strtol(argv[4], NULL, 0);
  efsize = strtol(argv[5], NULL, 0);

  if(efn < 1 || efn > 254){
    printf("0 < EFN < 255\n");
    exit(0);
  }

  if(evtr < 1 || evtr > 10000){
    printf("0 < EVTRATE < 10000\n");
    exit(0);
  }

  if(actual_length < 1 || actual_length > 200){
    printf("0 < EVTLEN < 200\n");
    exit(0);
  }

  if(dispersion < 1 || dispersion > 100){
    printf("0 < DISPERSION < 100\n");
    exit(0);
  }

  if(dispersion < 1 || dispersion > EFSIZE){
    printf("0 < DISPERSION < %d\n", EFSIZE);
    exit(0);
  }


  if((cfd = open(COMFIFO, O_RDWR)) < 0){
    printf("Can't open COMFIFO\n");
    exit(1);
  }

  signal(SIGINT, (void *)quit);

  buff = 0;
  if((bshmid = initshm(EFSHMKEY+efn, EF_SHM_SIZE, &buff)) == -1){
    printf("initshm failed  shmid = %d\n", bshmid);
    exit(0);
  }
  if(!buff){
    printf("Can't allocate shared memory %p\n", buff);
    exit(0);
  }
  DB(printf("shmid = %d / addr = %p\n", bshmid, buff));
  memset(buff, 0, EF_SHM_SIZE);
  babiesrun = buff + EF_SHM_RUN;

  memset(lbuff, 0, sizeof(lbuff));

  evthds = sizeof(evthd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;
  hds = sizeof(hd)/WORDSIZE;

  mp = hds;
  idx = 0;

  signal(SIGALRM, (void *)chk);
  us = 1000*1000/evtr;
  evtn = 0;
  l = 10;
  
  for (;;){
    while(*babiesrun == 1){
      if(ssflag == 0){
	DB(printf("DAQ start\n"));
	ini();

	ssflag = 1;
	evtn = 0;
	ssf = 1;
	memcpy(buff + EF_SHM_SSF, (char *)&ssf, sizeof(ssf));
	DB(printf("ssf %d\n", ssf));
      }
      status = 1;
      while(status != 0){
	usleep((int)(us*(double)l));
	status = 0;
	if (status > 0){
	  DB(printf("status=%d\n",status));
          if (*babiesrun == 0){
            goto stoproutine;
          }
	}
      }
      for(i=0;i<l;i++){
	evtn++; DB(printf("evtn=%d\n", evtn));
	evtlen = actual_length + 
	  (int)((double)dispersion*rand()/(RAND_MAX + 1.0));
	memset(databuf, 0x01, evtlen*WORDSIZE);
	
	/* RIDF */

	evts = evtlen + evthds + seghds;
	lots = evtn;
	evthd = ridf_mkhd_evtts(RIDF_LY1, RIDF_EVENT_TS, evts, efn, evtn, lots);
	memcpy((char *)(lbuff+mp), (char *)&evthd, evthds*WORDSIZE);
	mp += evthds;
	segs = evtlen + seghds;
	seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT, segs, efn, segid);
	memcpy((char *)(lbuff+mp), (char *)&seghd, seghds*WORDSIZE);
	mp += seghds;
	memcpy((char *)(lbuff+mp), (char *)databuf,
	       evtlen*sizeof(short));
	mp += evtlen;

	
	if(mp > efsize){
	  while(!datacopy());
	}
      }
    }
stoproutine:

    if(*babiesrun == 0){
      if(ssflag == 1){
	ssflag = 0;
        DB(printf("DAQ stop\n"));
	if(mp > hds){
	  while(!datacopy());
	}

	memcpy(buff + EF_SHM_EVTN, (char *)&evtn, sizeof(evtn));
	DB(printf("evtn %d\n", evtn));
	ssf = 0;
	memcpy(buff + EF_SHM_SSF, (char *)&ssf, sizeof(ssf));
	DB(printf("ssf %d\n", ssf));

	usleep(10000);
	idx = -1;
	DB(printf("bexecuter: put end-flag to fifo\n"));
	flock(cfd, LOCK_EX);
	write(cfd, (char *)&idx, sizeof(idx));
	flock(cfd, LOCK_UN);
	idx = 0;
      }else{
	usleep(10000);
      }
    }
  }
}
