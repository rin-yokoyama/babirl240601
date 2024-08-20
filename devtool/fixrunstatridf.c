/* devtool/fixrunstatridf.c
 * 
 * last modified : 10/11/17 19:58:27 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Bug fix for RIBF runstatus global block header
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  FILE *rfd, *wfd;
  char oname[256];
  int hd, sz;
  char buff[1024*256];

  if(argc != 2){
    printf("fixrunsstatridf FILENAME\n");
    exit(0);
  }

  if((rfd = fopen(argv[1], "r")) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }

  memset(oname, 0, sizeof(oname));
  strcpy(oname, argv[1]);
  strcat(oname, ".fixed");
  if((wfd = fopen(oname, "w")) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }

  // for first header
  fread(&hd, 1, 4, rfd);
  sz = (hd & 0x003fffff) - 2;
  fread(buff, 2, sz, rfd);
  fwrite(&hd, 1, 4, wfd);
  fwrite(buff, 2, sz, wfd);

  // for status header
  fread(&hd, 1, 4, rfd);
  sz = (hd & 0x003fffff) - 2;
  printf("hd org = %08x\n", hd);
  printf("sz org = %d\n", sz);
  sz = sz + 6;
  fread(buff, 2, sz, rfd);
  sz += 2;
  hd = (hd & 0xfff00000) | (sz & 0x000fffff);
  printf("hd new = %08x\n", hd);
  sz -= 2;
  fwrite(&hd, 1, 4, wfd);
  fwrite(buff, 2, sz, wfd);

  while(!feof(rfd)){
    // for all data
    if(fread(&hd, 1, 4, rfd) > 0){
      sz = (hd & 0x003fffff) - 2;
      fread(buff, 2, sz, rfd);
      fwrite(&hd, 1, 4, wfd);
      fwrite(buff, 2, sz, wfd);
    }
  }

  fclose(rfd);
  fclose(wfd);

  return 0;
}
