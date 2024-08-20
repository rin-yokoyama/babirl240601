/* devtool/dexecuter.c
 * last modified : 09/08/19 13:34:32 
 *
 * Dummy data generator for babies (CC/NET mode)
 * Trigger = serial RTS 
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define DEVICE "/dev/ttyS0"

#include "dexecuter.h"

#define SEGID 5
#define EFSIZE 60000

#define COMFIFO "/tmp/babirldbfifo"

//#define TIMEOUT 60000
#define TIMEOUT 0
#define FRAME_LEN 300
#define BUF_LEN FRAME_LEN*2

volatile unsigned int evtn, oevtn, devtn, blkn;
unsigned long long int dt;
struct timeval ta, tb;
int l, efsize;
int sfd, arg;
char *babiesrun;
int j, i;

/* Prototypes for library */
int initshm();

int fd, cfd, dispersion, evtlen;
/* for babies */
int efn, bshmid, idx, size, mp;
char *buff, *buffat;
RIDFHD hd;
RIDFHDEVT evthd;
RIDFHDSEG seghd;
RIDFHDBLKN blknhd;
RIDFHDEOB eobhd;

/* for event fragment */
unsigned short lbuff[EFSIZE*2];
unsigned int segid;
int hds, evthds, seghds;
int segs, evts;
volatile char ssflag, ssf;

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

  rmpid("sexecuter");

  exit(0);
}



int datacopy(void){
  int bsz;

  if(buff[EF_SHM_FLAG1] && buff[EF_SHM_FLAG2]){
    DB(printf("Both buffer full\n"));
    usleep(10000);
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
    
    bsz = mp + sizeof(eobhd)/WORDSIZE;

    hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, bsz, efn);
    memcpy((char *)lbuff, (char *)&hd, sizeof(hd));

    blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER,
			    sizeof(blknhd)/WORDSIZE, efn, blkn);
    memcpy((char *)(lbuff+sizeof(hd)/WORDSIZE),
	   (char *)&blknhd, sizeof(blknhd));

    eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sizeof(eobhd)/WORDSIZE,
			  efn, bsz);
    memcpy((char *)(lbuff + mp), (char *)&eobhd, sizeof(eobhd));

    memcpy(buffat, (char *)lbuff, bsz*WORDSIZE);

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
  blkn = 0;
  segid = SEGID;
  ssflag = 0;
  ssf = 0;
  sfd = 0;
  arg = 0;
  l = 1;
  
  if(argc != 6){
    printf("sexecuter EFN EVTLEN DISPERSION EFSIZE\n\n");
    printf("EFN        : Event Fragment Number 0-254\n");
    printf("EVTLEN     : Length of one event (short word) 1-500\n");
    printf("DISPERSION : Dispersion of the length of one event (short word) 1-200\n");
    printf("EFSIZE     : Block size 1-%d (short word)\n", EFSIZE);
    printf("EVT/TRIG   : Events per trigger 1-1000\n");
    exit(0);
  }
  efn = strtol(argv[1], NULL, 0);
  actual_length = strtol(argv[2], NULL, 0);
  dispersion = strtol(argv[3], NULL, 0);
  efsize = strtol(argv[4], NULL, 0);
  l = strtol(argv[5], NULL, 0);

  if(efn < 1 || efn > 254){
    printf("0 < EFN < 255\n");
    exit(0);
  }

  if(actual_length < 1 || actual_length > 500){
    printf("0 < EVTLEN < 200\n");
    exit(0);
  }

  if(dispersion < 1 || dispersion > 200){
    printf("0 < DISPERSION < 100\n");
    exit(0);
  }

  if(dispersion < 1 || dispersion > EFSIZE){
    printf("0 < DISPERSION < %d\n", EFSIZE);
    exit(0);
  }

  if(l < 1 || l > 1000){
    printf("1 < EVT/TRIG < 1000\n");
  }


  /* PID file */
  if(chkpid("sexecuter")){
    printf("sexecuter: Error, another sexecuter may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/sexecuter\n");
    exit(0);
  }
  /* PID file */
  if(!mkpid("sexecuter")){
    printf("sexecuter: Error, Can't create PID file or another babinfo may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/sexecuter\n");
    exit(0);
  }


  if((cfd = open(COMFIFO, O_RDWR)) < 0){
    printf("Can't open COMFIFO\n");
    exit(1);
  }


  if((sfd = open(DEVICE,  O_RDWR | O_NOCTTY | O_NDELAY)) < 0){
    printf("Can't open %s\n", DEVICE);
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

  memset(buff, 0, EF_SHM_SIZE);

  babiesrun = buff + EF_SHM_RUN;

  memset(lbuff, 0, sizeof(lbuff));

  evthds = sizeof(evthd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;
  hds = (sizeof(hd) + sizeof(blknhd))/WORDSIZE;
  blkn = 0;

  mp = hds;
  idx = 0;

  arg = TIOCM_DTR | TIOCM_RTS;
  ioctl(sfd, TIOCMBIC, &arg ); 

  for (;;){
    while(*babiesrun == 1){
      if(ssflag == 0){
	DB(printf("DAQ start\n"));

	ssflag = 1;
	evtn = 0;
	blkn = 0;
	ssf = 1;
	memcpy(buff + EF_SHM_SSF, (char *)&ssf, sizeof(ssf));
	DB(printf("ssf %d\n", ssf));
      }
      status = 1;

      arg = TIOCM_DTR | TIOCM_RTS;
      ioctl(sfd, TIOCMBIC, &arg ); 

      arg = TIOCM_CTS | TIOCM_CD;
      ioctl(sfd, TIOCMIWAIT, arg );	

      arg = TIOCM_DTR | TIOCM_RTS;
      ioctl(sfd, TIOCMBIS, &arg ); 
	
      if(*babiesrun != 0){
	for(i=0;i<l;i++){
	  evtn++; DB(printf("evtn=%d\n", evtn));
	  evtlen = actual_length + 
	    (int)((double)dispersion*rand()/(RAND_MAX + 1.0));
	  memset(databuf, 0x01, evtlen*WORDSIZE);
	  
	  /* RIDF */
	  evts = evtlen + evthds + seghds;
	  evthd = ridf_mkhd_evt(RIDF_LY1, RIDF_EVENT, evts, efn, evtn);
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
	    blkn++;
	  }
	}
      }else{
	goto stoproutine;
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
	DB(printf("sexecuter: put end-flag to fifo\n"));
	flock(cfd, LOCK_EX);
	write(cfd, (char *)&idx, sizeof(idx));
	flock(cfd, LOCK_UN);
	idx = 0;

	arg = TIOCM_DTR | TIOCM_RTS;
	ioctl(sfd, TIOCMBIC, &arg ); 
      }else{
	usleep(10000);
      }
    }
  }
}
