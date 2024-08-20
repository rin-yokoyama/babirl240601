/* devtool/splitridf.c
 * 
 * last modified : 08/03/19 14:27:02 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * RIDF splitter
 * Event assembly data to event fragment data 
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define MAXSEGID       100
#define MAXBUFF       4000

static int hds=4, evthds=6, seghds=6;
//unsigned short obuff[EB_EFBLOCK_SIZE];
//unsigned short ibuff[EB_EFBLOCK_SIZE];
unsigned short *obuff;
unsigned short *ibuff;
static FILE *ifd[MAXSEGID], *ofd;
static int efn = 1, idx=4;
static int segn = 0;
static int blkn, evtn=0;

void storefile(unsigned int blks, unsigned int efn){
  RIDFHD hd;

  if(blks > hds){
    hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, blks, efn);
    memcpy((char *)obuff, (char *)&hd, hds*WORDSIZE);
    fwrite(obuff, 2, blks, ofd);

    idx = hds;

  }
}

void quit(void){
  int i;

  storefile(idx, efn);
  printf("Last event number = %d\n", evtn);

  fclose(ofd);

  for(i=0;i<segn;i++){
    fclose(ifd[i]);
  }

  free(ibuff);
  free(obuff);

  exit(0);
}

int main(int argc, char *argv[]){
  RIDFHD hd[MAXSEGID];
  RIDFRHD rhd;
  RIDFHDEVT evthd;
  RIDFHDSEG seghd;

  int evts;
  int segid;
  int i, evtidx, segdatas;

  if(argc<3){
    printf("combinesegments OUTFILE INFILE [INFILE] ...\n");
    exit(0);
  }

  // Segment IDs
  for(i=2;i<argc;i++){
    ifd[segn] = fopen(argv[i], "r");
    if(!ifd[segn]){
      printf("Cannot open input file %s\n", argv[i]);
    }else{
      //printf("open %s\n", argv[i]);
      segn++;
    }
  }

  // Output File
  if((ofd = fopen(argv[1], "w")) == NULL){
    printf("Can't open output file %s\n", argv[3]);
    exit(1);
  }


  // Comment
  printf("Now combinig is started..., number of segments=%d\n", segn);

  printf("\n");

  blkn = -1;
  ibuff = malloc(EB_EFBLOCK_SIZE);
  obuff = malloc(EB_EFBLOCK_SIZE);
  memset(ibuff, 0, EB_EFBLOCK_SIZE);
  memset(obuff, 0, EB_EFBLOCK_SIZE);
  hds = sizeof(RIDFHD)/WORDSIZE;
  evthds = sizeof(evthd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;

  evts = evthds;
  idx = hds;

  while(1){
    for(i=0;i<segn;i++){
      if(feof(ifd[i])){
	quit();
	break;
      }
    }

    // new event
    evtn++;
    evts = evthds;
    evtidx = idx;
    idx += evthds;
    // read header from ifd
    for(i=0;i<segn;i++){
      if(fread((char *)&hd[i], sizeof(hd[i]), 1, ifd[i]) != 1){
	quit();
      }
    }

    // read data
    for(i=0;i<segn;i++){
      rhd = ridf_dechd(hd[i]);
      switch(rhd.classid){
      case RIDF_SEGMENT:
	fread((char *)&segid, sizeof(segid), 1, ifd[i]);
	segdatas = rhd.blksize-(sizeof(hd[i])+sizeof(segid))/WORDSIZE;
	fread((char *)ibuff, 2, segdatas, ifd[i]);
	memcpy((char *)(obuff+idx), (char *)&hd[i], sizeof(hd[i]));
	idx += hds;
	memcpy((char *)(obuff+idx), (char *)&segid, sizeof(segid));
	idx += 2;
	memcpy((char *)(obuff+idx), (char *)ibuff, segdatas*WORDSIZE);
	idx += segdatas;
	evts += seghds + segdatas;
      break;
    default:
      printf("error : non segment header\n");
      quit();
      break;
      }
    }

    evthd = ridf_mkhd_evt(RIDF_LY1, RIDF_EVENT, evts, efn, evtn);
    memcpy((char *)(obuff+evtidx), (char *)&evthd, sizeof(evthd));

    if(idx > MAXBUFF){
      //printf("store file %d %d\n", idx, evtn);
      storefile(idx, efn);
    }
  }

  quit();

  return 0;
}
