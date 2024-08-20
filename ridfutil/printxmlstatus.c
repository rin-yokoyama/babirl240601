/* ribfutil/printxmlstatus.c
 * 
 * last modified : 12/02/21 19:35:41 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 * 
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ridf.h>
#include <bbzfile.h>

#define MAXBUFFSIZE 128*1024
#define WORDSIZE 2

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  char buff[MAXBUFFSIZE*2];
  char outbuff[MAXBUFFSIZE*2];
  bgFile *fd;
  int cid;
  int siz;

  if(argc != 2){
    printf("printxmlstatus FILENAME\n");
    printf(" \n");
    exit(0);
  }
  
  if((fd = bgropen(argv[1])) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }

  memset(buff, 0, sizeof(buff));

  siz = 0;

  while(bgread(fd, (char *)&hd, sizeof(hd)) == sizeof(hd)){
    rhd = ridf_dechd(hd);
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      break;
    case RIDF_EVENT:
    case RIDF_EVENT_TS:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
      break;
    case RIDF_SEGMENT:
    case RIDF_TIMESTAMP:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
    case RIDF_NCSCALER32:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
      break;
    case RIDF_COMMENT:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
      break;
    case RIDF_STATUS:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
      memcpy((char *)&cid, buff+sizeof(int), sizeof(cid));
      if(cid == RIDF_STATUS_START_XML||
	 cid == RIDF_STATUS_STOP_XML){
	memset(outbuff, 0, sizeof(outbuff));
	memcpy(outbuff, buff+sizeof(int)*2, siz-sizeof(int)*2);
	printf("%s\n", outbuff);
      }
      break;
    case RIDF_BLOCK_NUMBER:
    case RIDF_END_BLOCK:
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
      break;
    default:
      printf("Unknown Header\n");
      siz = rhd.blksize*WORDSIZE - sizeof(hd);
      bgread(fd, (char *)buff, siz);
    }
  }

  bgclose(fd);

  return 0;
}
