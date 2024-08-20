/* babirl: ridfutil/csvruninfo
 * last modified : 08/12/13 02:19:22 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Print RIDF Header in CSV format
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

int main(int argc, char *argv[]){
  FILE *fd;
  char buff[RIDF_COMMENT_RUNINFO_ASC_SIZE];
  RIDFHDCOM hd;
  RIDFRHD rhd;
  RIDFHD chd;
  struct ridf_comment_runinfost info;
  char start[20], stop[20];
  int i;
  struct stat st;
  double size;

  if(argc < 2){
    printf("prinfo FILENAME [FILENAME]...\n");
    exit(0);
  }

  printf("\"Run name\",\"Run number\",\"Start time\",");
  printf("\"Stop time\",\"Date\",\"Header\",\"Ender\",\"Size (MB)\"\n");

  for(i=1;i<argc;i++){
    if(stat(argv[i], &st)){
      printf("Can't open %s\n", argv[i]);
      exit(1);
    }

    size = (double)(st.st_size/1024)/1024;

    if(!(fd = fopen(argv[i], "r"))){
      printf("Can't open %s\n", argv[i]);
      exit(1);
    }

    fread(buff, 1, sizeof(chd), fd);
    fread(buff, 1, sizeof(buff), fd);
    
    fclose(fd);
    
    memcpy((char *)&hd, buff, sizeof(hd));
    rhd = ridf_dechd(hd.chd);
    
    if(rhd.classid != RIDF_COMMENT){
      fprintf(stderr, "%s don't include header information.\n", argv[i]);
      continue;
    }
    
    memset((char *)&info, 0, sizeof(info));
    memcpy((char *)&info, buff+sizeof(hd), sizeof(info));
    memset(start, 0, sizeof(start));
    memset(stop, 0, sizeof(stop));
    
    memcpy(start, info.starttime+9, sizeof(start)-9);
    memcpy(stop, info.stoptime+8, sizeof(stop)-8);
    
    printf("\"%s\",", info.runname);
    printf("\"%s\",", info.runnumber);
    printf("\"%s\",", start);
    printf("\"%s\",", stop);
    printf("\"%s\",", info.date);
    printf("\"%s\",", info.header);
    printf("\"%s\",",  info.ender);
    printf("\"%2.1f\"\n",  size);
  }    
  return 1;
}
