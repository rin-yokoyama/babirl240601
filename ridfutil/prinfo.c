/* babirl: devtool/prinfo.c 
 * last modified : 11/03/31 12:57:52 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Print RIDF Header information
 * gzip support
 * xml support
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbzfile.h>

int main(int argc, char *argv[]){
  bgFile *fd;
  char buff[RIDF_COMMENT_RUNINFO_ASC_SIZE];
  //char xbuff[1024*128];
  RIDFHDCOM hd;
  RIDFRHD rhd;
  RIDFHD chd;
  //RIDFHD xhd;
  struct ridf_comment_runinfost info;
  char start[20], stop[20];
  int x = 0;

  if(argc < 2){
    printf("prinfo FILENAME [FILENAME]\n");
    exit(0);
  }

  x = 1;
  while(x < argc){
    if(!(fd = bgropen(argv[x]))){
      printf("Can't open %s\n", argv[x]);
      exit(1);
    }
    

    bgread(fd, buff, sizeof(chd));
    bgread(fd, buff, sizeof(buff));

    bgclose(fd);

    memcpy((char *)&hd, buff, sizeof(hd));
    rhd = ridf_dechd(hd.chd);
    
    if(rhd.classid != RIDF_COMMENT){
      printf("This file don't include header information.\n");
      exit(0);
    }
    
    memset((char *)&info, 0, sizeof(info));
    memcpy((char *)&info, buff+sizeof(hd), sizeof(info));
    memset(start, 0, sizeof(start));
    memset(stop, 0, sizeof(stop));
    
    memcpy(start, info.starttime+9, sizeof(start)-9);
    memcpy(stop, info.stoptime+8, sizeof(stop)-8);
    
    printf("%-10s : %s\n", "Run name", info.runname);
    printf("%-10s : %s\n", "Run number", info.runnumber);
    printf("%-10s : %s\n", "Start time", start);
    printf("%-10s : %s\n", "Stop time", stop);
    printf("%-10s : %s\n", "Date", info.date);
    printf("%-10s : %s\n", "Header", info.header);
    printf("%-10s : %s\n", "Ender", info.ender);

    x++;
    printf("\n");
  }

  return 1;
}
