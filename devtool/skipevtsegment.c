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
  RIDFHD hd;
  RIDFRHD rhd;
  //RIDFHDEVT evthd;
  //RIDFHDSEG seghd;
  int evtn = 0, skipn = 0;
  unsigned int segid, segdatas;
  //int hds, evthds, seghds, endn = 0;
  int endn = 0;
  unsigned short *ibuff;
  static FILE *ifd;
  static FILE *ofd;
  char name[128] = {0};

  ibuff = malloc(EB_EFBLOCK_SIZE);
  memset(ibuff, 0, EB_EFBLOCK_SIZE);

  if(argc < 2){
    printf("skipevtsegment INFILE [EVTN] [Number of Events]\n");
    printf(" if EVTN=1, remove the first event\n");
    printf("  EVTN is the number of n-th event in the file\n");
    printf(" (some cases, EVTN is not the same as the event number written in RIDF Event Header)\n");
    exit(0);
  }

  // Input File
  if((ifd = fopen(argv[1], "r")) == NULL){
    printf("Can't open input file %s\n", argv[2]);
    exit(1);
  }

  // make output directory
  sprintf(name, "%s.sk", argv[1]);
  ofd = fopen(name, "w");
  if(ofd){
  }else{
    printf("Invalid input file %s\n", name);
    fclose(ifd);
    exit(0);
  }


  if(argc >=3 ){
    skipn = strtol(argv[2], NULL, 0);
  }

  endn = skipn + 1;
  if(argc == 4){
    endn = strtol(argv[3], NULL, 0);
    endn = skipn + endn;
  }

  printf("skip %d th - %d th event data\n", skipn, endn-1);

  //hds = sizeof(hd)/WORDSIZE;
  //evthds = sizeof(evthd)/WORDSIZE;
  //seghds = sizeof(seghd)/WORDSIZE;


  while(fread((char *)&hd, sizeof(hd), 1, ifd) == 1){
    evtn++;
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
      //if(evtn != skipn){
      if((evtn < skipn) || (evtn >= endn)){
	fwrite((char *)&hd, sizeof(hd), 1, ofd);
	fwrite((char *)&segid, sizeof(segid), 1, ofd);
	fwrite((char *)ibuff, 2, segdatas, ofd);
      }
      break;
    default:
      segdatas = rhd.blksize-sizeof(hd)/WORDSIZE;
      fread((char *)ibuff, 2, segdatas, ifd);
      break;
    }

  }

  printf("Last event number = %d\n", evtn);

  fclose(ofd);

  fclose(ifd);

  free(ibuff);

  return 0;
}
