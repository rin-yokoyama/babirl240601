/* devtool/chkridf.c
 * 
 * last modified : 14/04/17 12:37:31 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Check RIDF file
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbcpri.h>
#include <bbzfile.h>


#define CHKRIDF_NEXT_HD    0x01
#define CHKRIDF_NEXT_EVT   0x02
#define CHKRIDF_NEXT_BLK   0x04
#define CHKRIDF_NEXT_SCR   0x08
#define CHKRIDF_NEXT_COM   0x10
#define CHKRIDF_NEXT_STA   0x20

/* Not implemented yet */
#define CHKRIDF_PREV_HD   11
#define CHKRIDF_PREV_EVT  12
#define CHKRIDF_PREV_BLK  13

#define CHKRIDF_BLK_COL   FG_BLACK
#define CHKRIDF_EVT_COL   FG_BLUE
#define CHKRIDF_SEG_COL   FG_GREEN
#define CHKRIDF_SCR_COL   FG_MAGENTA
#define CHKRIDF_COM_COL   FG_YELLOW
#define CHKRIDF_STA_COL   FG_CYAN

//#define NSTOP

#define TMPDATA "/tmp/tmponlinedata"

void print_rhd(RIDFRHD rhd){
  printf("hd1 = 0x%08x\n", RIDF_MKHD1(rhd.layer,rhd.classid,rhd.blksize));
  printf("ly=%d, cid=%d, size=%d, efn=%d\n",
	  rhd.layer, rhd.classid, rhd.blksize, rhd.efn);
}

int main(int argc, char *argv[]){
  bgFile *bfd;
  RIDFHD hd;
  RIDFRHD rhd;
  unsigned short *buff;
  FILE *fd;
  unsigned int blkn, evtn, segid;
  int segdatas, date, scrid, scrdatas, i;
  int com = CHKRIDF_NEXT_HD;
  char tcom = 0, omit = 0;
  char tcombuff[1024];
  struct stbsegid bsid;

  struct sembuf semb;
  union semun semunion;
  char *shmp;
  int shmid, semid, sid, sz;

  unsigned long long int ts;
  int efn, skevtn;
  int shmsize;

  int nostop;

  if(argc > 3 || argc < 2){
    printf("chkridf [-n/o] [FILENAME]\n");
    printf("chkridf -s SHMID\n");
    exit(0);
  }

  buff = malloc(EB_EFBLOCK_SIZE);
  if(!buff){
    printf("Can't malloc %d\n", EB_EFBLOCK_SIZE);
  }

  sid = -1; fd = 0; bfd = 0;
  nostop = 0;

  if(argc == 3 && (strcmp(argv[1], "-s") == 0)){
    sid = strtol(argv[2], NULL, 0);
  }
  if(argc == 3 && (strcmp(argv[1], "-n") == 0)){
    nostop = 1;
  }
  if(argc == 3 && (strcmp(argv[1], "-o") == 0)){
    omit = 1;
  }
  if(sid != -1){
    cprintf(FG_RED, "Online mode\n");

    if(sid == 6){
      shmsize = 8 * 1024 * 1024; // 8MB for SID=6
    }else{
      shmsize = BABIAN_SHM_BUFF_SIZE;
    }    
    /* Shared Memory */
    if(!(shmid = initshm(ANSHMKEY+sid*2,
			 shmsize + sizeof(int),
			 &shmp))){
      printf("Can't create shared memory\n");
      exit(0);
    }

    /* Semaphore */
    if(!(semid = initsem(ANSEMKEY+sid*2, &semunion))){
      printf("Can't create semaphore\n");
      exit(0);
    }

    if((fd = fopen(TMPDATA, "w")) == NULL){
      printf("Can't create tmpfile %s\n", TMPDATA);
      exit(1);
    }

    sem_p(semid, &semb);     // Lock shared memory
    memcpy((char *)&hd, shmp, sizeof(hd));
    rhd = ridf_dechd(hd);
    fwrite(shmp, WORDSIZE, rhd.blksize, fd);
    sem_v(semid, &semb);     // Unlock shared memory

    fclose(fd);
    chmod(TMPDATA, 0666);
    
    
    if((bfd = bgropen(TMPDATA)) == NULL){
      printf("Can't open %s\n", TMPDATA);
      exit(1);
    }
  }else{
    if((bfd = bgropen(argv[argc-1])) == NULL){
      printf("Can't open %s\n", argv[argc-1]);
      exit(1);
    }
  }

  if(bfd){
    if(bfd->gfd){
      cprintf(FG_BLUE, "GZIPPED RIDF\n");
    }
  }

  printf("Commands... \n");
  printf("         : Next header (default)\n");
  printf(" e       : Next event\n");
  printf(" b       : Next block\n");
  printf(" s       : Next scaler\n");
  printf(" t       : Next status\n");
  printf(" c       : Next comment\n");
  printf(" q       : Quit program\n");
  printf("\n\n");

  blkn = -1;
  memset(buff, 0, EB_EFBLOCK_SIZE);

  skevtn = -1;
  while(bgread(bfd, (char *)&hd, sizeof(hd)) > 0){

    if(tcom == 'e'){
      com = CHKRIDF_NEXT_EVT;
      tcombuff[strlen(tcombuff)-1] = 0;
      if(strlen(tcombuff) > 1 && skevtn < 0){
	skevtn = strtol(tcombuff+1, NULL, 0);
	printf("Skip to evtn = %d ...\n", skevtn);
      }
    }else if(tcom == 'b'){
      com = CHKRIDF_NEXT_BLK;
    }else if(tcom == 's'){
      com = CHKRIDF_NEXT_SCR;
    }else if(tcom == 'c'){
      com = CHKRIDF_NEXT_COM;
    }else if(tcom == 't'){
      com = CHKRIDF_NEXT_STA;
    }else if(tcom == 'q'){
      break;
    }else{
      com = CHKRIDF_NEXT_HD;
    }

    rhd = ridf_dechd(hd);
    //printf("cid %d\n", rhd.classid);
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      blkn++;
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_BLK)){
	cprintf(CHKRIDF_BLK_COL, "EF Block Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_END_BLOCK:
      bgread(bfd, (char *)&sz, sizeof(sz));
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_BLK)){
	cprintf(CHKRIDF_BLK_COL, "EF Block Ender / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("Size of this block = %d\n", sz);
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_BLOCK_NUMBER:
      bgread(bfd, (char *)&sz, sizeof(sz));
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_BLK)){
	cprintf(CHKRIDF_BLK_COL, "EF Block Number / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("The number of this block = %d\n", sz);
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_EVENT:
    case RIDF_EVENT_TS:
      bgread(bfd, (char *)&evtn, sizeof(evtn));
      if(rhd.classid == RIDF_EVENT_TS){
	bgread(bfd, (char *)&ts, sizeof(ts));
	ts &= 0x0000ffffffffffff;
      }	
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_EVT)){
	if(skevtn < 0 || skevtn == evtn){
	  if(rhd.classid == RIDF_EVENT_TS){
	    cprintf(CHKRIDF_EVT_COL, "Event Header with Time Stamp / blkn=%d\n", blkn);
	  }else{
	    cprintf(CHKRIDF_EVT_COL, "Event Header / blkn=%d\n", blkn);
	  }
	  print_rhd(rhd);
	  printf("Event Number = %d\n", evtn);
	  if(rhd.classid == RIDF_EVENT_TS){
	    printf("Time Stamp = %lld\n", ts);
	  }
	  printf("\n: ");
	  if(!nostop){
	    fgets(tcombuff, sizeof(tcombuff), stdin);
	    tcom = tcombuff[0];
	  }
	  skevtn = -1;
	}
      }
      break;
    case RIDF_SEGMENT:
      bgread(bfd, (char *)&segid, sizeof(segid));
      segdatas = rhd.blksize-(sizeof(hd)+sizeof(segid))/WORDSIZE;
      bgread(bfd, (char *)buff, segdatas*2);
      if(com & CHKRIDF_NEXT_HD){
	cprintf(CHKRIDF_SEG_COL, "Segment Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("Segment ID = %d (0x%08x)\n", segid, segid);
	memcpy((char *)&bsid, (char *)&segid, sizeof(segid));
	printf("Rev %d / Dev %d / FP  %d / Det %d / Mod %d\n",
	       bsid.revision, bsid.device, bsid.focal,
	       bsid.detector, bsid.module);
	if(!omit){
	  for(i=0;i<segdatas;i++){
	    printf("%04x ", buff[i]);
	    if((i+1)%8 == 0){
	      printf("\n");
	    }
	  }
	  if(i%8 != 0){
	    printf("\n");
	  }
	}
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_TIMESTAMP:
      segdatas = rhd.blksize-sizeof(hd)/WORDSIZE;
      bgread(bfd, (char *)buff, segdatas*2);
      if(com & CHKRIDF_NEXT_HD){
	cprintf(CHKRIDF_SEG_COL, "Timestamp Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	for(i=0;i<segdatas;i+=4){
	  memcpy((char *)&ts, (char *)(buff+i), sizeof(ts));
	  efn = ts >> 56;
	  ts = ts & 0x0000ffffffffffffLL;
	  printf("efn[%3d] : TS=%llu\n", efn, ts);
	}
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
    case RIDF_NCSCALER32:
      bgread(bfd, (char *)&date, sizeof(date));
      bgread(bfd, (char *)&scrid, sizeof(scrid));
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      bgread(bfd, (char *)buff, scrdatas*2);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_SCR)){
	cprintf(CHKRIDF_SCR_COL, "Scaler Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("Scaler Date = %d\n", date);
	printf("Scaler ID = %d\n", scrid);
	for(i=0;i<scrdatas;i++){
	  printf("%04x ", buff[i]);
	  if((i+1)%8 == 0){
	    printf("\n");
	  }
	}
	if(i%8 != 0){
	  printf("\n");
	}
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_COMMENT:
      bgread(bfd, (char *)&date, sizeof(date));
      bgread(bfd, (char *)&scrid, sizeof(scrid));
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      bgread(bfd, (char *)buff, scrdatas*2);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_COM)){
	cprintf(CHKRIDF_COM_COL, "Comment Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("Comment Date = %d\n", date);
	printf("Comment ID = %d\n", scrid);
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    case RIDF_STATUS:
      bgread(bfd, (char *)&date, sizeof(date));
      bgread(bfd, (char *)&scrid, sizeof(scrid));
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      bgread(bfd, (char *)buff, scrdatas*2);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_STA)){
	cprintf(CHKRIDF_STA_COL, "Status Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	printf("Status Date = %d\n", date);
	printf("Status ID = %d\n", scrid);
	printf("\n: ");
	if(!nostop){
	  fgets(tcombuff, sizeof(tcombuff), stdin);
	  tcom = tcombuff[0];
	}
      }
      break;
    default:
      printf("Unknown Header\n");
      print_rhd(rhd);
      printf("\n: ");
      bgread(bfd, (char *)buff, (rhd.blksize - sizeof(hd)/WORDSIZE)*2);
      if(!nostop){
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
      }
    }
  }

  bgclose(bfd);

  return 0;
}
