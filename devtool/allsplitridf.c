/* devtool/splitridf.c
 * 
 * last modified : 17/02/13 10:59:07 
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

#include <bbzfile.h>

#define MAXSEGID  200

int hds, evthds, seghds;
unsigned short *ibuff;
//static FILE *ifd;
static bgFile *ifd;
static FILE *ofd[MAXSEGID] = {NULL};
int seglist[MAXSEGID] = {0};
int efnlist[256] = {0};
static int segn = 0;
char name[128] = {0};

int findsegn(int efn, int segid){
  int i;
  char ofile[256] = {0};
  //struct stbsegid bsid;

  for(i=0; i<MAXSEGID; i++){
    if(efnlist[i] == efn && 
       seglist[i] == segid) return i;
  }

  efnlist[segn] = efn;
  seglist[segn] = segid;

  sprintf(ofile, "%s/%03d_%08x.segment", name, efn, segid);
  ofd[segn] = fopen(ofile, "w");
  if(!ofd[segn]){
    printf("Cannot open output file %s\n", ofile);
    exit(1);
  }else{
    //memcpy((char *)&bsid, (char *)&segid, sizeof(segid));
    //printf("Write Segid(%08x) to %s (segn=%d)\n", segid, ofile, segn);
    //printf("Rev %d / Dev %d / FP  %d / Det %d / Mod %d\n\n",
    //bsid.revision, bsid.device, bsid.focal,
    //bsid.detector, bsid.module);
    //printf("%s ", ofile);
  }

  segn++;
  return segn-1;
}

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  RIDFHDEVT evthd;
  RIDFHDSEG seghd;
  int evtn, i, n;
  char *idx;
  unsigned long long int ts;
  unsigned int segid, segdatas;

  if(argc < 2){
    printf("allsplitridf INFILE [WORKDIRECTORY]\n");
    exit(0);
  }

  // Initialize
  for(i=0;i<MAXSEGID;i++){
    seglist[i] = 0;
    ofd[i] = 0;
  }

  // Input File
  //if((ifd = fopen(argv[1], "r")) == NULL){
  //printf("Can't open input file %s\n", argv[2]);
  //exit(1);
  //}
  if((ifd = bgropen(argv[1])) == NULL){
    printf("Can't open input file %s\n", argv[2]);
    exit(1);
  }
  if(ifd){
    if(ifd->gfd){
      printf("GZIPPED RIDF\n");
    }
  }
  

  // make output directory
  strcpy(name, argv[1]);
  idx = strstr(name, ".ridf");
  if(idx){
    *idx = 0;
  }else{
    printf("Invalid input file %s\n", name);
    bgclose(ifd);
    //fclose(ifd);
    exit(0);
  }

  // output directory
  if(argc==3){
    strcpy(name, argv[2]);
    mkdir(name, 0755);
  }else{
    mkdir(name, 0755);
  }
  printf("Working directory = %s\n", name);

  printf("\n");

  ibuff = malloc(EB_EFBLOCK_SIZE);
  memset(ibuff, 0, EB_EFBLOCK_SIZE);
  hds = sizeof(hd)/WORDSIZE;
  evthds = sizeof(evthd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;

  //while(fread((char *)&hd, sizeof(hd), 1, ifd) == 1){
  while(bgread(ifd, (char *)&hd, sizeof(hd)) > 0){
    rhd = ridf_dechd(hd);
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      break;
    case RIDF_EVENT:
      bgread(ifd, (char *)&evtn, sizeof(evtn));
      //fread((char *)&evtn, sizeof(evtn), 1, ifd);
      break;
    case RIDF_EVENT_TS:
      bgread(ifd, (char *)&evtn, sizeof(evtn));
      bgread(ifd, (char *)&ts, sizeof(ts));
      //fread((char *)&evtn, sizeof(evtn), 1, ifd);
      //fread((char *)&ts, sizeof(ts), 1, ifd);
      break;
    case RIDF_SEGMENT:
      bgread(ifd, (char *)&segid, sizeof(segid));
      //fread((char *)&segid, sizeof(segid), 1, ifd);

      n = findsegn(rhd.efn, segid);

      segdatas = rhd.blksize-(sizeof(hd)+sizeof(segid))/WORDSIZE;
      bgread(ifd, (char *)ibuff, segdatas*2);
      //fread((char *)ibuff, 2, segdatas, ifd);

      fwrite((char *)&hd, sizeof(hd), 1, ofd[n]);
      fwrite((char *)&segid, sizeof(segid), 1, ofd[n]);
      fwrite((char *)ibuff, 2, segdatas, ofd[n]);

      break;
    default:
      segdatas = rhd.blksize-sizeof(hd)/WORDSIZE;
      bgread(ifd, (char *)ibuff, segdatas*2);
      //fread((char *)ibuff, 2, segdatas, ifd);
      break;
    }
  }

  printf("\n Last event number = %d\n", evtn);

  for(i=0;i<MAXSEGID;i++){
    if(ofd[i]){
      fclose(ofd[i]);
    }
  }

  bgclose(ifd);
  //fclose(ifd);
  free(ibuff);

  return 0;
}
