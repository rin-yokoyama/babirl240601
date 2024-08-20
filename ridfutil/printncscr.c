/* ribfutil/printncscr.c
 * 
 * last modified : 11/10/05 20:22:30 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * 
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbcpri.h>

#define MAXBUFFSIZE 100000

#define CHKRIDF_NEXT_HD    0x01
#define CHKRIDF_NEXT_EVT   0x02
#define CHKRIDF_NEXT_BLK   0x04
#define CHKRIDF_NEXT_SCR   0x08
#define CHKRIDF_NEXT_COM   0x10
#define CHKRIDF_NEXT_STA   0x20

/* Not implemented yet */
#define CHKRIDF_PREV_HD   11
#define CHKRIDF_PREV_EVT  12
#define CHKRIDF_PREV_BLK  13

#define CHKRIDF_BLK_COL   FG_BLACK
#define CHKRIDF_EVT_COL   FG_BLUE
#define CHKRIDF_SEG_COL   FG_GREEN
#define CHKRIDF_SCR_COL   FG_MAGENTA
#define CHKRIDF_COM_COL   FG_YELLOW
#define CHKRIDF_STA_COL   FG_CYAN

void print_rhd(RIDFRHD rhd){
  printf("ly=%d, cid=%d, size=%d, efn=%d\n",
	  rhd.layer, rhd.classid, rhd.blksize, rhd.efn);
}

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  unsigned short buff[MAXBUFFSIZE];
  unsigned short scrbuff[MAXBUFFSIZE];
  FILE *fd;
  int i, date, scrid;
  int siz, scrsiz;
  unsigned int sv;
  struct tm *strtime;
  char ch[128];
  int tscrid;
  time_t tdate;

  if(argc != 3){
    printf("printncscr SCRID FILENAME\n");
    exit(0);
  }
  

  if((fd = fopen(argv[2], "r")) == NULL){
    printf("Can't open %s\n", argv[2]);
    exit(1);
  }

  memset(buff, 0, sizeof(buff));

  tscrid = strtol(argv[1], NULL, 0);

  siz = 0;
  scrsiz = 0;

  while(fread((char *)&hd, sizeof(hd), 1, fd) == 1){
    rhd = ridf_dechd(hd);
    //printf("0x%08x  0x%08x\n", hd.hd1, hd.hd2);
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      break;
    case RIDF_EVENT:
    case RIDF_EVENT_TS:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      fseeko(fd, siz, SEEK_CUR);
      break;
    case RIDF_SEGMENT:
    case RIDF_TIMESTAMP:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      fseeko(fd, siz, SEEK_CUR);
      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
    case RIDF_NCSCALER32:
      fread((char *)&date, sizeof(date), 1, fd);
      fread((char *)&scrid, sizeof(scrid), 1, fd);
      siz = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      scrsiz = siz;
      fread((char *)buff, 2, siz, fd);

      if(scrid == tscrid){
	memcpy((char *)scrbuff, (char *)buff, 2 * siz);
      }
      
      break;
    case RIDF_COMMENT:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      fseeko(fd, siz, SEEK_CUR);
      break;
    case RIDF_STATUS:
    case RIDF_BLOCK_NUMBER:
    case RIDF_END_BLOCK:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      fseeko(fd, siz, SEEK_CUR);
      break;
    default:
      printf("Unknown Header\n");
      print_rhd(rhd);
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      fseeko(fd, siz, SEEK_CUR);
    }
  }

  fclose(fd);

  if(scrsiz > 0){
    tdate = date;
    strtime = localtime(&tdate);
    strftime(ch, sizeof(ch), "%d-%b-%y %X", strtime);
    printf("%s, %s, %d\n", argv[1], ch, scrid);
    
    for(i=0;i<32;i++){
      memcpy((char *)&sv, scrbuff+(i*2), sizeof(sv));
      printf("SCR%02d: %d\n", i, sv);
    }
    printf("\n");
  }else{
    printf("Can't find scaler data\n");
  }

  return 0;
}
