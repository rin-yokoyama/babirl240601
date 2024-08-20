/* devtool/chksumridf.c
 * 
 * last modified : 07/03/30 14:59:31 
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

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbcpri.h>

#define MAXBUFFSIZE 100000

#define CHKRIDF_NEXT_HD    0x01
#define CHKRIDF_NEXT_EVT   0x02
#define CHKRIDF_NEXT_BLK   0x04
#define CHKRIDF_NEXT_SCR   0x08
#define CHKRIDF_NEXT_COM   0x10

/* Not implemented yet */
#define CHKRIDF_PREV_HD   11
#define CHKRIDF_PREV_EVT  12
#define CHKRIDF_PREV_BLK  13

#define CHKRIDF_BLK_COL   FG_BLACK
#define CHKRIDF_EVT_COL   FG_BLUE
#define CHKRIDF_SEG_COL   FG_GREEN
#define CHKRIDF_SCR_COL   FG_MAGENTA
#define CHKRIDF_COM_COL   FG_YELLOW


//#define NSTOP

void print_rhd(RIDFRHD rhd){
  printf("ly=%d, cid=%d, size=%d, efn=%d\n",
	  rhd.layer, rhd.classid, rhd.blksize, rhd.efn);
}

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  unsigned short buff[MAXBUFFSIZE];
  FILE *fd;
  unsigned int blkn, evtn, segid;
  int segdatas, date, scrid, scrdatas;
  unsigned int blksz, sumsz;
  int tdata, nn = 0, tt, i;
  int nevt = 0, xflag = 0;

  if(argc != 2){
    printf("chkridf FILENAME\n");
    exit(0);
  }

  if((fd = fopen(argv[1], "r")) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }
  
  blkn = -1;
  memset(buff, 0, sizeof(buff));
  blksz = 0;
  sumsz = 0;

  while(fread((char *)&hd, sizeof(hd), 1, fd) == 1){
    rhd = ridf_dechd(hd);

    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      if(blksz != sumsz){
	printf("blksz = %d / sumsz = %d\n", blksz, sumsz);
      }
      blksz = 0;
      sumsz = sizeof(hd)/WORDSIZE;
      blksz = rhd.blksize;
      blkn++;
      break;
    case RIDF_EVENT:
      fread((char *)&evtn, sizeof(evtn), 1, fd);
      sumsz += rhd.blksize;
      nevt = 0;
      break;
    case RIDF_SEGMENT:
      fread((char *)&segid, sizeof(segid), 1, fd);
      segdatas = rhd.blksize-(sizeof(hd)+sizeof(segid))/WORDSIZE;
      fread((char *)buff, 2, segdatas, fd);
      memcpy((char *)&tdata, buff, 4);

      if(nn == 0){
	nn = tdata;
      }else{
	if(nevt == 0){
	  if(tdata != (nn+1)){
	    printf("error %d / %d\n", nn, tdata);
	    getchar();
	  }
	  nn = tdata;
	}
      }

      for(i=1;i<segdatas/2;i++){
	memcpy((char *)&tt, (char *)(buff+2*i), 4);
	//if(tdata != tt){
	if(tdata+1 != tt){
	  printf("segid = 0x%08x\n", segid);
	  printf("der %d %d / %d\n", i, tt, tdata);
	  printf("segdatas=%d\n", segdatas);
	  xflag = 1;
	  getchar();
	}
	tdata = tt;
	nn = tdata;
      }


      if(xflag){
	printf("segid = 0x%08x\n", segid);
	printf("segdatas=%d\n", segdatas);
	for(i=0;i<segdatas/2;i++){
	  memcpy((char *)&tt, (char *)(buff+2*i), 4);
	  printf("der %d %d / %d\n", i, tt, tdata);
	}
	xflag = 0;
      }

      nevt ++;

      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
      sumsz += rhd.blksize;
      fread((char *)&date, sizeof(date), 1, fd);
      fread((char *)&scrid, sizeof(scrid), 1, fd);
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      fread((char *)buff, 2, scrdatas, fd);
      break;
    case RIDF_COMMENT:
      sumsz += rhd.blksize;
      fread((char *)&date, sizeof(date), 1, fd);
      fread((char *)&scrid, sizeof(scrid), 1, fd);
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      fread((char *)buff, 2, scrdatas, fd);
      break;
    default:
      sumsz += rhd.blksize;
      fread((char *)buff, WORDSIZE, rhd.blksize - sizeof(hd)/WORDSIZE, fd);
    }
  }

  fclose(fd);

  return 0;
}
