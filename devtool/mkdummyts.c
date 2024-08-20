/* devtool/mkdummyts.c
 * 
 * Make dummy TS for test
 *
 * Hidetada Baba (RIKEN)
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define MAXBUFF 4000

int main(int argc, char *argv[]){
  FILE *fd;
  int efn, evtsize, segid;
  int mp, evtn, bsz, sz;
  short segbuff[MAXBUFF];
  short buff[MAXBUFF*2];
  int i, pf, blkn;
  unsigned long long int ts;

  double stoptime, deadtime, rate;
  double t0, t1, newtime, tottime;

  RIDFHD hd;
  RIDFHDEVTTS evthd;
  RIDFHDSEG   seghd;
  RIDFHDEOB   eobhd;
  RIDFHDBLKN  blknhd;

  if(argc != 8){
    printf("mkdummyts EFN SEGID EVTSIZE RATE DEADTIME STOPTIME OUTFILE\n");
    printf("      EFN      : Event fragment number\n");
    printf("      SEGID    : Segment ID\n");
    printf("      EVTSIZE  : Event size\n");
    printf("      RATE     : Event rate (evt/s)\n");
    printf("      DEADTIME : Deat time per 1 event (us)\n");
    printf("      STOPTIME : Stop time of this file (s)\n");
    printf("      OUTFILE  : Output file name\n");
    exit(0);
  }

  init_nt();

  efn     = strtol(argv[1], NULL, 0);
  segid   = strtol(argv[2], NULL, 0);
  evtsize = strtol(argv[3], NULL, 0);
  rate     = strtod(argv[4], NULL);
  deadtime = strtod(argv[5], NULL);
  stoptime = strtod(argv[6], NULL);

  if(evtsize > MAXBUFF){
    printf("EVTSIZE <= %d\n", MAXBUFF);
  }

  if((fd = fopen(argv[7], "w")) == NULL){
    printf("Can't open %s\n", argv[7]);
    exit(1);
  }

  printf("OUTFILE  = %s\n", argv[6]);
  printf("EFN      = %d\n", efn);
  printf("SEDIG    = %d\n", segid);
  printf("EVTSIZE  = %d\n", evtsize);
  printf("rate     = %f\n", rate);
  printf("DEADTIME = %f\n", deadtime);
  printf("STOPTIME = %f\n", stoptime);

  deadtime = deadtime / 1000000.;  // us to s
  tottime = 0.;

  evtn = 0;
  blkn = 0;
  for(i=0;i<evtsize;i++){
    segbuff[i] = i;
  }

  t0 = 0.;
  t1 = 0.;
  pf = 1;

  mp = sizeof(hd)/WORDSIZE;
  sz = sizeof(blknhd)/2;
  blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER, sz, efn, blkn);
  memcpy((char *)(buff+mp), (char *)&blknhd, sizeof(blknhd));
  mp += sizeof(blknhd)/2;

  
  while(tottime < stoptime){
    if(pf){
      t0 = 0.;
      t1 = 0.;
    }

    newtime = nt(rate);
    tottime += newtime;
    t1 = t1 + newtime;

    if(ne(t0, t1, deadtime)){
      // Event accepted
      pf = 1;
      
      evtn ++;

      sz = evtsize + (sizeof(evthd) + sizeof(seghd))/WORDSIZE;
      ts = (unsigned long long int)(tottime * 100000000); // to 10 ns/period
      evthd = ridf_mkhd_evtts(RIDF_LY1, RIDF_EVENT_TS, sz, efn, evtn, ts);
      memcpy((char *)(buff+mp), (char *)&evthd, sizeof(evthd));
      mp += sizeof(evthd)/WORDSIZE;
      sz = evtsize + sizeof(seghd)/WORDSIZE;
      seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT, sz, efn, segid);
      memcpy((char *)(buff+mp), (char *)&seghd, sizeof(seghd));
      mp += sizeof(seghd)/WORDSIZE;
      memcpy((char *)(buff+mp), (char *)segbuff, evtsize*sizeof(short));
      mp += evtsize;
	
      if(mp > MAXBUFF){
	sz = sizeof(eobhd)/2;
	bsz = mp + sz;
	eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sz, efn, bsz);
	memcpy((char *)(buff+mp), (char *)&eobhd, sizeof(eobhd));

	hd = ridf_mkhd(RIDF_LY0, RIDF_EAF_BLOCK, bsz, efn);
	memcpy((char *)buff, (char *)&hd, sizeof(hd));
    
	fwrite(buff, 2, bsz, fd);

	// New buffer
	blkn ++;
	mp = sizeof(hd)/WORDSIZE;
	sz = sizeof(blknhd)/WORDSIZE;
	blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER, sz, efn, blkn);
	memcpy((char *)(buff+mp), (char *)&blknhd, sizeof(blknhd));
	mp += sizeof(blknhd)/2;

      }

    }else{
      // Event rejected
      pf = 0;
    }


  }


  fclose(fd);

  return 0;
}
