/* babirl/devtool/rdf2ridf_seg.c
 *
 * last modified : 06/12/23 12:35:33 
 *
 * Data format converter
 * RDF to RIDF (specified segment only)
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/shm.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define BLKSIZE 8192
#define MAXOBLK 10000


int nextevt(int segid, unsigned short *idata, int *ridx, int *isegs){
  static int nidx=0;
  int i, hflag, esize, ts, tidx;
  int ssize, tidf;


  hflag = 0;
  if(nidx == 0){
    for(i=0;i<4;i++){
      if(idata[i] != 0){
	hflag++;
      }
    }
    if(hflag){
      return 0;
    }else{
      nidx = 4;
    }
  }

  if((unsigned short)idata[nidx] == 0xffff){
    nidx = 0;
    return 0;
  }

  /* Event Header */
  tidx = nidx;
  esize = idata[nidx] & 0x7fff;
  nidx += 3;
  ts = 0;
  while(ts < esize){
    ssize = idata[nidx] & 0x7fff;
    if(ssize == 0) exit(0);
    nidx ++;
    tidf = idata[nidx];
    if(tidf == segid){
      *ridx = nidx;
      *isegs = ssize;
      nidx = tidx + esize;

      return 1;
    }else{
      nidx += ssize -1;
    }
    ts += ssize;
  }
  /* Out of range */
  printf("Can't find segid %d\n", segid);
  nidx = 0;
    
  return 0;
}

int main(int argc, char *argv[]){
  FILE *ifd, *ofd;
  int efn, segid;
  unsigned short idata[BLKSIZE];
  unsigned short odata[EB_EFBLOCK_SIZE];
  int isegs, ridx;
  RIDFHD hd;
  unsigned int evtn;
  int blks, evts, segs, idx;
  int totblk;

  isegs = 0;

  evtn = 0;
  blks = 0;
  segs = 0;
  idx = 0;
  ridx = 0;
  totblk = 0;

  if(argc != 5){
    printf("rdf2ridf EFN SEGID INFILE OUTFILE\n");
    exit(0);
  }

  efn = strtol(argv[1], NULL, 0);
  if(efn < 1 || efn > 254){
    printf("0 < EFN < 255\n");
    exit(0);
  }

  segid = strtol(argv[2], NULL, 0);
  if(segid < 1 || segid > 254){
    printf("0 < segid < 255\n");
    exit(0);
  }

  if((ifd = fopen(argv[3], "r")) == NULL){
    printf("Can't open input file %s\n", argv[2]);
    exit(1);
  }
  if((ofd = fopen(argv[4], "w")) == NULL){
    printf("Can't open output file %s\n", argv[3]);
    exit(1);
  }

  memset(idata, 0, sizeof(idata));
  memset(odata, 0, sizeof(odata));

  printf("Now converting...\n");

  while(!feof(ifd)){
    if(fread(idata, 2, BLKSIZE, ifd)==0) break;
    while((nextevt(segid, idata, &ridx, &isegs))){
      if(idx == 0){
	blks = 4;
	idx = 4;
      }
      evtn ++;
      segs = isegs + 4;
      evts = segs + 6;
      blks += evts;
      
      hd = ridf_mkhd(RIDF_LY1, RIDF_EVENT, evts, efn);
      memcpy((char *)(odata+idx), (char *)&hd, sizeof(hd));
      idx += 4;
      memcpy((char *)(odata+idx), (char *)&evtn, sizeof(evtn));
      idx += 2;
      hd = ridf_mkhd(RIDF_LY2, RIDF_SEGMENT, segs, efn);
      memcpy((char *)(odata+idx), (char *)&hd, sizeof(hd));
      idx += 4;
      memcpy((char *)(odata+idx), (char *)&segid, sizeof(segid));
      idx += 2;
      memcpy((char *)(odata+idx), (char *)(idata+ridx+1), (isegs-2)*2);
      idx += isegs-2;
      if(blks > MAXOBLK){
	totblk += blks;
	hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, blks, efn);
	memcpy((char *)odata, (char *)&hd, sizeof(hd));
	printf("evtn %d / blks %d  /  idx %d\n", evtn, blks, idx);
	fwrite(odata, 2, blks, ofd);
	idx = 0;
      }
    }
  }

  if(idx != 0){
    totblk += blks;
    hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, blks, efn);
    memcpy((char *)odata, (char *)&hd, sizeof(hd));
    printf("evtn %d / blks %d  /  idx %d\n", evtn, blks, idx);
    fwrite(odata, 2, blks, ofd);
  }

  printf("Total events : %d\n", evtn);
  printf("Data size    : %d\n", totblk*2);
  printf("Finished\n");

  fclose(ifd);
  fclose(ofd);

  return 0;
}
