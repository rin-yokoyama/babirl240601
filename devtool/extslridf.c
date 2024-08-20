/* devtool/extslridf.c
 * 
 * last modified : 07/03/30 14:59:23 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Extract slow data from RIDF
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
  memcpy((char *)(obuff+hds), (char *)ibuff, blks - hds);
  fwrite(obuff, 2, blks, ofd);
}

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;

  unsigned int efn;

  if(argc != 4){
    printf("splitridf EFN INFILE OUTFILE\n");
    exit(0);
  }

  // EFN
  efn = strtol(argv[1], NULL, 0);
  if(efn == 0){
    printf("0 < EFN < 0xffffffff\n");
    exit(0);
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
  printf("\n");

  memset(ibuff, 0, sizeof(ibuff));
  memset(obuff, 0, sizeof(obuff));
  hds = sizeof(hd)/WORDSIZE;

  while(fread((char *)&hd, sizeof(hd), 1, ifd) == 1){
    rhd = ridf_dechd(hd);
    if(rhd.layer != RIDF_LY0){
      memcpy((char *)ibuff, (char *)&hd, sizeof(hd));
      fread((char *)(ibuff+hds), 2, rhd.blksize - hds, ifd);
    }

    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      break;
    case RIDF_EVENT:
    case RIDF_SEGMENT:
      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
    case RIDF_STATUS:
    case RIDF_COMMENT:
      if(rhd.layer == RIDF_LY1){
	/* Store layer 1 only (async. event data) */
	storefile(rhd.blksize + hds, efn);
      }else{
      }	
      break;
    }
  }

  fclose(ifd);
  fclose(ofd);

  return 0;
}
