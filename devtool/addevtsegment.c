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
#include <sys/stat.h>
#include <sys/types.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>


int main(int argc, char *argv[]){
  RIDFHD hd, thd;
  RIDFRHD rhd;
  //RIDFHDEVT evthd;
  //RIDFHDSEG seghd;
  int evtn = 0, addn;
  unsigned int segid, segdatas;
  //int hds, evthds, seghds;
  unsigned short *ibuff;
  static FILE *ifd;
  static FILE *ofd;
  char name[128] = {0};
  
  if(argc < 3){
    printf("addevtsegment INFILE EVTN\n");
    printf(" if EVTN=0, add 1 event at the first\n");
    exit(0);
  }

  // Input File
  if((ifd = fopen(argv[1], "r")) == NULL){
    printf("Can't open input file %s\n", argv[2]);
    exit(1);
  }

  // make output directory
  sprintf(name, "%s.ad", argv[1]);
  ofd = fopen(name, "w");
  if(ofd){
  }else{
    printf("Invalid input file %s\n", name);
    fclose(ifd);
    exit(0);
  }


  addn = strtol(argv[2], NULL, 0);
  printf("add %d + 1th event data\n", addn);

  ibuff = malloc(EB_EFBLOCK_SIZE);
  memset(ibuff, 0, EB_EFBLOCK_SIZE);
  //hds = sizeof(hd)/WORDSIZE;
  //evthds = sizeof(evthd)/WORDSIZE;
  //seghds = sizeof(seghd)/WORDSIZE;


  while(fread((char *)&hd, sizeof(hd), 1, ifd) == 1){
    rhd = ridf_dechd(hd);
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      break;
    case RIDF_SEGMENT:
      fread((char *)&segid, sizeof(segid), 1, ifd);
      segdatas = rhd.blksize-(sizeof(hd)+sizeof(segid))/WORDSIZE;
      fread((char *)ibuff, 2, segdatas, ifd);
      if(evtn == addn){
	thd = ridf_mkhd(rhd.layer, rhd.classid, sizeof(thd)/WORDSIZE+sizeof(segid)/WORDSIZE, rhd.efn);
	fwrite((char *)&thd, sizeof(thd), 1, ofd);
	fwrite((char *)&segid, sizeof(segid), 1, ofd);
      }
      fwrite((char *)&hd, sizeof(hd), 1, ofd);
      fwrite((char *)&segid, sizeof(segid), 1, ofd);
      fwrite((char *)ibuff, 2, segdatas, ofd);

      break;
    default:
      segdatas = rhd.blksize-sizeof(hd)/WORDSIZE;
      fread((char *)ibuff, 2, segdatas, ifd);
      break;
    }

    evtn++;
  }

  printf("Last event number = %d\n", evtn);

  fclose(ofd);

  fclose(ifd);

  free(ibuff);

  return 0;
}
