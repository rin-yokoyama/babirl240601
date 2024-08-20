/* devtool/mkdummyridf.c
 * 
 * Make dummy RIDF for test
 *
 * Hidetada Baba (RIKEN)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define MAXBUFF 4000
#define SEGID   1

int main(int argc, char *argv[]){
  FILE *fd;
  int efn, evtsize, scrid, scrn, blkn, sz;
  short data[MAXBUFF*2];
  int i, j, mp, evtn, intt, bsz;
  unsigned int scrval;
  short segbuff[MAXBUFF];
  time_t t;

  RIDFHD hd;
  RIDFHDEVT  evthd;
  RIDFHDSEG  seghd;
  RIDFHDEOB  eobhd;
  RIDFHDSCR  scrhd;
  RIDFHDBLKN blknhd;

  if(argc != 7){
    printf("mkdummyridf EFN EVTSIZE BLKN SCRID SCRN OUTFILE\n");
    exit(0);
  }

  if((fd = fopen(argv[6], "w")) == NULL){
    printf("Can't open %s\n", argv[6]);
    exit(1);
  }

  efn     = strtol(argv[1], NULL, 0);
  evtsize = strtol(argv[2], NULL, 0);
  blkn    = strtol(argv[3], NULL, 0);
  scrid   = strtol(argv[4], NULL, 0);
  scrn    = strtol(argv[5], NULL, 0);

  printf("OUTFILE = %s\n", argv[6]);
  printf("EFN     = %d\n", efn);
  printf("EVTSIZE = %d\n", evtsize);
  printf("BLKN    = %d\n", blkn);
  printf("SCRID   = %d\n", scrid);
  printf("SCRN    = %d\n", scrn);

  for(i=0;i<evtsize;i++){
    segbuff[i] = i;
  }

  evtn = 0;
  scrval = 0;
  for(i=0;i<blkn;i++){
    mp = sizeof(hd)/2;

    sz = sizeof(blknhd)/2;
    blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER, sz, efn, i);
    memcpy((char *)(data+mp), (char *)&blknhd, sizeof(blknhd));
    mp += sizeof(blknhd)/2;

    while(mp < MAXBUFF){
      sz = evtsize + sizeof(evthd)/2 + sizeof(seghd)/2;
      evthd = ridf_mkhd_evt(RIDF_LY1, RIDF_EVENT, sz, efn, evtn);
      memcpy((char *)(data+mp), (char *)&evthd, sizeof(evthd));
      mp += sizeof(evthd)/2;

      sz = evtsize + sizeof(seghd)/2;
      seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT, sz, efn, SEGID);
      memcpy((char *)(data+mp), (char *)&seghd, sizeof(seghd));
      mp += sizeof(seghd)/2;

      memcpy((char *)(data+mp), (char *)segbuff, evtsize*2);
      mp += evtsize;

      evtn++;
    }
    time(&t);
    intt = (int)t;
    sz = scrn*2 + sizeof(scrhd)/2;
    scrhd = ridf_mkhd_scr(RIDF_LY1, RIDF_NCSCALER32, sz, efn, intt, scrid);
    memcpy((char *)(data+mp), (char *)&scrhd, sizeof(scrhd));
    mp += sizeof(scrhd)/2;

    for(j=0;j<scrn;j++){
      scrval++;
      memcpy((char *)(data+mp), (char *)&scrval, sizeof(scrval));
      mp += sizeof(scrval)/2;
    }

    sz = sizeof(eobhd)/2;
    bsz = mp + sz;
    eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sz, efn, bsz);
    memcpy((char *)(data+mp), (char *)&eobhd, sizeof(eobhd));

    hd = ridf_mkhd(RIDF_LY0, RIDF_EAF_BLOCK, bsz, efn);
    memcpy((char *)data, (char *)&hd, sizeof(hd));
    
    fwrite(data, 2, bsz, fd);
  }
  

  fclose(fd);

  return 0;
}
