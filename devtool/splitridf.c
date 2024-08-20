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

#define MAXSEGID        50
#define MAXOBLK       5000

int hds, evthds, seghds;
unsigned short obuff[EB_EFBLOCK_SIZE];
unsigned short ibuff[EB_EFBLOCK_SIZE];
FILE *ifd, *ofd;

void storefile(unsigned int blks, unsigned int efn){
  RIDFHD hd;

  hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, blks, efn);
  memcpy((char *)obuff, (char *)&hd, hds*WORDSIZE);
  fwrite(obuff, 2, blks, ofd);
}

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  RIDFHDEVT evthd;
  RIDFHDSEG seghd;

  unsigned int blkn, evtn, segid, efn;
  int segdatas, i, tsegid;
  char segflag[MAXSEGID]; // Flag for segment id
  char segfind[MAXSEGID]; // Flag for found segment
  int segflagcnt;        // Count for segment id flag
  int evts, blks;
  int idx, tidx;
  int scrflag, scrdatas;

  segflagcnt = 0;
  scrflag = 0;
  memset((char *)segflag, 0, sizeof(segflag));
  memset((char *)segfind, 0, sizeof(segfind));

  if(argc < 5){
    printf("splitridf EFN INFILE OUTFILE SEGID [SEGID] ...\n");
    printf(" SEGID = 0 to 30, s=scaler\n");
    exit(0);
  }

  // EFN
  efn = strtol(argv[1], NULL, 0);
  if(efn == 0){
    printf("0 < EFN < 0xffffffff\n");
    exit(0);
  }

  // Segment IDs
  for(i=4;i<argc;i++){
    if(argv[i][0] == 's'){
      scrflag = 1;
    }else{
      tsegid = strtol(argv[i], NULL, 0);
      if(tsegid == 0 || tsegid > MAXSEGID){
	printf("0 < SEGID < %d\n", MAXSEGID);
	exit(0);
      }
      segflagcnt++;
      segflag[tsegid] = 1;
    }
  }

  // Input File
  if((ifd = fopen(argv[2], "r")) == NULL){
    printf("Can't open input file %s\n", argv[2]);
    exit(1);
  }

  // Output File
  if((ofd = fopen(argv[3], "w")) == NULL){
    printf("Can't open output file %s\n", argv[3]);
    exit(1);
  }


  // Comment
  printf("Now stating...\n");
  printf("Segment ID : ");
  for(i=0;i<MAXSEGID;i++){
    if(segflag[i]){
      printf("%d ", i);
    }
  }
  if(scrflag){
    printf("scaler");
  }
  printf("\n");

  blkn = -1;
  memset(ibuff, 0, sizeof(ibuff));
  memset(obuff, 0, sizeof(obuff));
  hds = sizeof(hd)/WORDSIZE;
  evthds = sizeof(evthd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;

  evts = 0;
  blks = hds;
  idx = hds;
  tidx = 0;


  while(fread((char *)&hd, sizeof(hd), 1, ifd) == 1){
    rhd = ridf_dechd(hd);
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      blkn++;
      break;
    case RIDF_EVENT:
      if(evts){
	blks += evts;
	evthd = ridf_mkhd_evt(RIDF_LY1, RIDF_EVENT,
			      evts, efn, evtn);
	// Evt header
	memcpy((char *)(obuff+tidx), (char *)&evthd, evthds*WORDSIZE);
	//printf("evts event idx : %d , tidx : %d\n", idx, tidx);
	if(blks > MAXOBLK){
	  storefile(blks, efn);
	  //printf("mx event idx : %d , tidx : %d\n", idx, tidx);
	  blks = hds;
	  idx = hds;
	}
	evts = 0;
      }

      fread((char *)&evtn, sizeof(evtn), 1, ifd);
      tidx = idx;
      idx += evthds;
      evts += evthds;
      //printf("event idx : %d , tidx : %d\n", idx, tidx);
      break;
    case RIDF_SEGMENT:
      fread((char *)&segid, sizeof(segid), 1, ifd);
      segdatas = rhd.blksize-(sizeof(hd)+sizeof(segid))/WORDSIZE;
      fread((char *)ibuff, 2, segdatas, ifd);
      if(segid > 0 && segid < MAXSEGID){
	if(segflag[segid]){
	  if(!segfind[segid]){
	    segfind[segid] = 1;
	  }
	  seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT,
				rhd.blksize, efn, segid);
	  memcpy((char *)(obuff+idx), (char *)&seghd, seghds*WORDSIZE);
	  idx += seghds;
	  memcpy((char *)(obuff+idx), (char *)ibuff, segdatas*WORDSIZE);
	  idx += segdatas;
	  evts += seghds + segdatas;
	}
      }else{
	printf("Out range of segment id %d\n", segid);
	printf("MAXSEGID shoud be modified\n");
      }
      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
      if(rhd.layer == RIDF_LY1 && scrflag){
	/* Store layer 1 only (async. event data) */
	scrdatas = rhd.blksize - sizeof(hd)/WORDSIZE;
	fread((char *)ibuff, 2, scrdatas, ifd);
	memcpy((char *)(obuff+idx), (char *)&hd, sizeof(hd));
	idx += hds;
	memcpy((char *)(obuff+idx), (char *)ibuff, scrdatas * WORDSIZE);
	idx += scrdatas;
	blks += hds + scrdatas;
      }
      break;
    case RIDF_STATUS:
    case RIDF_COMMENT:
      scrdatas = rhd.blksize - sizeof(hd)/WORDSIZE;
      fread((char *)ibuff, 2, scrdatas, ifd);
      break;
    }
  }

  fclose(ifd);
  fclose(ofd);

  for(i=0;i<MAXSEGID;i++){
    if(segflag[i]){
      if(!segfind[i]){
	printf("Segment ID %d was not found\n", i);
      }
    }
  }

  return 0;
}
