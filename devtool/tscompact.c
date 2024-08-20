/* babirl : devtool/tscompact.c
 * last modified : 09/12/06 20:52:55 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Time stamp sorting and compaction program
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>

/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

struct tsbst{
  unsigned long long int ts;
  unsigned long long int fp;
};

int rfd, wfd;
struct tsbst *tsb;
FILE *ifd, *ofd, *fltfd;
short buff[EB_EFBLOCK_SIZE];
short segts[EB_EFBLOCK_SIZE];
short segdata[EB_EFBLOCK_SIZE];
int maxmp = 8192, mp;

#define TS(x) (x &  0x0000ffffffffffffLL)
#define FP(x) (x &  0x0000ffffffffffffLL)
#define EFN(x) (int)((x & 0xff00000000000000LL) >> 56)
#define DET(x) ((x & 0x00003f00LL) >> 8)
#define TSDET  60
#define DSPDET 30

void quit(void);

void quit(void){
  if(rfd) close(rfd);
  if(wfd) close(wfd);
  if(tsb) free(tsb);
  if(ifd) fclose(ifd);
  if(ofd) fclose(ofd);

  exit(0);
}

int comparets(const void *a, const void *b){
  long long int ats=0, bts=0;

  memcpy((char *)&ats, (char *)a, 7);
  memcpy((char *)&bts, (char *)b, 7);

  if(ats > bts){
    return 1;
  }else if(ats < bts){
    return -1;
  }else{
    return 0;
  }

}


int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFHDEVTTS evthd;
  RIDFHDSEG seghd;
  struct stat fst;
  char itsfile[256], otsfile[256], iridffile[256], oridffile[256];
  char fltfile[256];
  int wid, tsn, idx, odx, sz;
  int segs, evts, evtn, flag;
  int segtssize, segdatasize;
  unsigned int hds, evthds, seghds, efn, segdataid, segtsid;
  unsigned long long int fp;
  int r;

  evthds = sizeof(evthd)/WORDSIZE;
  seghds = sizeof(seghd)/WORDSIZE;
  hds = sizeof(hd)/WORDSIZE;

  if(argc !=3){
    printf("tscompact NAME WID\n");
    exit(0);
  }

  wid = strtol(argv[2], NULL, 0);

  sprintf(itsfile, "%s.tst", argv[1]);
  sprintf(otsfile, "%s_sc.tst", argv[1]);
  sprintf(iridffile, "%s.ridf", argv[1]);
  sprintf(oridffile, "%s_sc.ridf", argv[1]);
  sprintf(fltfile, "%s_sc.flt", argv[1]);
  
  if((rfd = open(itsfile, O_RDONLY)) < 0){
    printf("Can't open time stamp table %s\n", itsfile);
    quit();
  }

  if((wfd = open(otsfile, O_WRONLY|O_CREAT, 0644)) < 0){
    printf("Can't open output table %s\n", otsfile);
    quit();
  }

  if((ifd = fopen(iridffile, "r")) == NULL){
    printf("Can't open input ridf file %s\n", iridffile);
    quit();
  }    

  if((ofd = fopen(oridffile, "w")) == NULL){
    printf("Can't open output ridf file %s\n", oridffile);
    quit();
  }    

  if((fltfd = fopen(fltfile, "w")) == NULL){
    printf("Can't open output flt %s\n", fltfile);
    quit();
  }
  fprintf(fltfd, "1 %s\n", oridffile);
  fclose(fltfd);
  

  fstat(rfd, &fst);
  tsb = malloc(fst.st_size);
  
  if(tsb == NULL){
    printf("Can't malloc memory (size=%llu)\n", (unsigned long long int )fst.st_size);
    quit();
  }

  printf("tscompact\n");
  printf(" name=%s, wid=%d\n", argv[1], wid);

  tsn = fst.st_size/sizeof(struct tsbst);
  r = read(rfd, tsb, fst.st_size);

  qsort(tsb, tsn, sizeof(struct tsbst), comparets);

  close(rfd); rfd = 0;

  idx = 0;
  odx = 0;
  evtn = 0;
  flag = 1;
  mp = hds;
  fp = hds;
  efn = 0;
  segtssize = 0;
  segdatasize = 0;
  segtsid = 0;
  segdataid = 0;
  while(idx < tsn){
    if(flag == 1){
      // init event and segment
      segtssize = 0;
      segdatasize = 0;
      evts = 0;
      odx = idx;

      // store first event
      fseeko(ifd, FP(tsb[odx].fp), SEEK_SET);
      r = fread(&evthd, sizeof(evthd), 1, ifd);
      efn = RIDF_EF(evthd.chd.hd2);
      
      fread(&seghd, sizeof(seghd), 1, ifd);
      segtsid = seghd.segid;
      sz = RIDF_SZ(seghd.chd.hd1) - seghds;
      fread(segts+segtssize, sizeof(short), sz, ifd);
      segtssize += sz;

      fread(&seghd, sizeof(seghd), 1, ifd);
      segdataid = seghd.segid;
      sz = RIDF_SZ(seghd.chd.hd1) - seghds;
      fread(segdata+segdatasize, sizeof(short), sz, ifd);
      segdatasize += sz;

      //printf("1 : r = %d / efn = %d %08x %08x/ segtssize=%d / segdatasize=%d\n",
      //r, efn, segtsid, segdataid, segtssize, segdatasize);

      if(r == 0){
	printf("Error, can't read from ridf file. Tabun ridf to tst no ga seigoushitenai.\n");
	printf("mp=%d / idx=%d / tsn=%d / fp = %llu / evtn=%d\n", mp, idx, tsn, fp, evtn);
	flag = 3;
	idx = tsn - 1;
	continue;
      }

      flag = 2;
      idx++;
    }else if(flag == 2){
      if((TS(tsb[odx].ts) + wid) >= TS(tsb[idx].ts)){
	// store
	// store first event
	fseeko(ifd, FP(tsb[idx].fp), SEEK_SET);
	r = fread(&evthd, sizeof(evthd), 1, ifd);
	
	fread(&seghd, sizeof(seghd), 1, ifd);
	sz = RIDF_SZ(seghd.chd.hd1) - seghds;
	fread(segts+segtssize, sizeof(short), sz, ifd);
	segtssize += sz;
	
	fread(&seghd, sizeof(seghd), 1, ifd);
	sz = RIDF_SZ(seghd.chd.hd1) - seghds;
	fread(segdata+segdatasize, sizeof(short), sz, ifd);
	segdatasize += sz;

	//printf("2 : r=%d / segtssize=%d / segdatasize=%d\n", r, segtssize, segdatasize);
	//getchar();

	if(idx == tsn - 1){
	  flag = 3;
	}else{
	  idx++;
	}
      }else{
	flag = 3;
      }
    }else if(flag == 3){
      // close event
      flag = 1;

      evts = evthds + (seghds * 2) + segtssize + segdatasize;
      // copy evtheader
      evthd = ridf_mkhd_evtts(RIDF_LY1, RIDF_EVENT_TS, evts,
			      efn, evtn, TS(tsb[odx].ts));
      memcpy((char *)(buff+mp), (char *)&evthd, evthds*WORDSIZE);
      mp += evthds;;
      // copy segment ts
      segs = segtssize + seghds;
      seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT, segs, efn, segtsid);
      memcpy((char *)(buff+mp), (char *)&seghd, seghds*WORDSIZE);
      mp += seghds;
      memcpy((char *)(buff+mp), (char *)segts, segtssize*WORDSIZE);
      mp += segtssize;
      // copy segment data
      segs = segdatasize + seghds;
      seghd = ridf_mkhd_seg(RIDF_LY2, RIDF_SEGMENT, segs, efn, segdataid);
      memcpy((char *)(buff+mp), (char *)&seghd, seghds*WORDSIZE);
      mp += seghds;
      memcpy((char *)(buff+mp), (char *)segdata, segdatasize*WORDSIZE);
      mp += segdatasize;

      tsb[evtn].ts = (((unsigned long long int)efn << 56) | tsb[odx].ts);
      tsb[evtn].fp = (((unsigned long long int)1 << 48) | fp);

      //printf("3 : mp=%d / idx=%d / tsn=%d / fp = %llu / evtn=%d\n", mp, idx, tsn, fp, evtn);
      fp += evts;

      evtn++;
      

      if((mp > maxmp) || (idx == tsn - 1)){
	hd = ridf_mkhd(RIDF_LY0, RIDF_EF_BLOCK, mp, efn);
	memcpy((char *)buff, (char *)&hd, hds*WORDSIZE);
	fwrite(buff, 2, mp, ofd);

	mp = hds;
	fp += hds;
      }

      idx ++;
    }

    //printf("%llu %llu %d\n", TS(tsb[idx].ts),
    //   FP(tsb[idx].fp),EFN(tsb[idx].ts));
  }


  write(wfd, tsb, evtn*sizeof(struct tsbst));
  close(wfd); wfd = 0;

  quit();

  return 0;
}

