/* babirl/devtool/rdf2ridf.c
 *
 * last modified : 08/06/07 18:13:45 
 *
 * Data format converter
 * RDF to RIDF (for all segments)
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>
#include <rdf.h>
#include <ridf.h>

#define BLKSIZE 8192
#define MAXOBLK 10000

#define MAXSCRN 12 * 5

#define SCRCID RIDF_CSCALER
#define SCRID  15

int main(int argc, char *argv[]){
  FILE *ifd, *ofd;
  unsigned int efn;
  unsigned int scr[MAXSCRN];
  short ibuff[BLKSIZE], obuff[EB_EFBLOCK_SIZE];
  struct rdf_blkinfost info;
  int evts, segs, segdatas;
  unsigned int ievtn, evtn, idx;
  int totblk, tidx, seghds, evthds;
  int i, j, scrn, scrsz, scrblksz;
  RIDFHD hd;
  RIDFHDEVT evthd;
  RIDFHDSEG seghd;
  RIDFHDSCR scrhd;

  scrn = 0;
  memset(scr, 0, sizeof(scr));

  if(argc < 4){
    printf("rdf2ridf EFN INFILE OUTFILE [ScrNum]\n");
    printf(" ScrNum 1=12ch, 2=24ch, 3=36ch (Max=5)\n");
    exit(0);
  }

  if(argc == 5){
    scrn = strtol(argv[4], NULL, 0);
    if(scrn > 5 || scrn < 0){
      printf("0 <= ScrNum <= 5\n");
      exit(0);
    }
  }

  efn = strtol(argv[1], NULL, 0);
  if(efn == 0){
    printf("0 < EFN < 0xffffff\n");
    exit(0);
  }

  if((ifd = fopen(argv[2], "r")) == NULL){
    printf("Can't open input file %s\n", argv[2]);
    exit(1);
  }

  if((ofd = fopen(argv[3], "w")) == NULL){
    printf("Can't open output file %s\n", argv[3]);
    exit(1);
  }

  idx = 4;
  evtn = 0;
  totblk = 0;

  //hds = sizeof(hd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;
  evthds = sizeof(evthd)/WORDSIZE;

  // Main loop
  while(fread(ibuff, 2, BLKSIZE, ifd) == BLKSIZE){
    memset((char *)&info, 0, sizeof(info));
    // Analyze rdf data
    ievtn = rdf_scanblk((unsigned short*)ibuff, &info, BLKSIZE);


    // Scaler
    if(ievtn && scrn){
      // Calc. scaler block size
      scrblksz = sizeof(scrhd) + scrn * 12 * sizeof(int);
      scrsz = scrn * 12 * sizeof(int);

      // Copy scaler data to scr[] 
      memcpy((char *)scr,
	     (char *)(ibuff)+sizeof(ibuff)-scrsz, scrsz);
      // Make scr header (scaler id = SCRID, date = 0)
      scrhd = ridf_mkhd_scr(RIDF_LY1, SCRCID,
			    scrblksz/WORDSIZE, efn, 0, SCRID);
      // Copy scaler block to buff
      memcpy((char *)(obuff+idx), (char *)&scrhd, sizeof(scrhd));
      idx += sizeof(scrhd)/WORDSIZE;
      memcpy((char *)(obuff+idx), (char *)scr, scrsz);
      idx += scrsz/WORDSIZE;
    }

    for(i=0;i<ievtn;i++){
      // New event
      tidx = idx;   // Index for Event Header
      idx += 6;     // Move idx to Segment Header
      evts = 6;     // Init. event size
      // Loop for segments
      for(j=0;j<RDF_MAXSEGID;j++){
	if(info.segptr[i][j]){
	  // Calc. segment size, segdata size
	  segs = info.segsize[i][j] + 4;
	  segdatas = segs - seghds;
	  // Make segment header
	  seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT, segs, efn,
				info.segid[i][j]);
	  // Memcopy seghd to obuff
	  memcpy((char *)(obuff+idx), (char *)&seghd, seghds*WORDSIZE);
	  // Increment idx += seghd size
	  idx += seghds;
	  // Memcopy segment data to obuff
	  memcpy((char *)(obuff+idx), (char *)(info.segptr[i][j]+2),
		 segdatas*WORDSIZE);
	  // Increment idx += segment data size
	  idx += segdatas;
	  // Add segment size to event size
	  evts += segs;
	}
      }
      // Make event header
      evthd = ridf_mkhd_evt(RIDF_LY1, RIDF_EVENT, evts, efn, evtn);
      // Memcopy evthd to obuff (pointer = tidx)
      memcpy((char *)(obuff+tidx), (char *)&evthd, evthds*WORDSIZE);

      evtn++;

      // Block end
      if(idx > MAXOBLK){
	totblk += idx;
	hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, idx, efn);
	memcpy((char *)obuff, (char *)&hd, sizeof(hd));
	//printf("evtn %d / idx %d\n", evtn, idx);
	fwrite(obuff, 2, idx, ofd);
	idx = 4;
      }
    }
  }

  // Residual data
  if(idx > 4){
    totblk += idx;
    hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, idx, efn);
    memcpy((char *)obuff, (char *)&hd, sizeof(hd));
    fwrite(obuff, 2, idx, ofd);
    printf("%d\n", idx);
  }


  printf("Finiched...\n");
  printf("Total size = %d / event number =%d\n", totblk, evtn);

  
  fclose(ifd);
  fclose(ofd);

  return 0;
}
