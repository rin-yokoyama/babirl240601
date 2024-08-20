/* devtool/extblkridf.c
 * 
 * last modified : 10/01/05 16:52:23 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Extract 1 block
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbcpri.h>

#define MAXBUFFSIZE 1024*256

int main(int argc, char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  char buff[MAXBUFFSIZE*2];
  FILE *ifd, *ofd;
  int blkn, iblkn;


  if(argc != 4){
    printf("chkridf INFILE OUTFILE BLKN\n");
    exit(0);
  }

  if((ifd = fopen(argv[1], "r")) == NULL){
    printf("Can't open %s\n", argv[1]);
    exit(1);
  }

  if((ofd = fopen(argv[2], "w")) == NULL){
    printf("Can't open %s\n", argv[2]);
    fclose(ifd);
    exit(1);
  }

  iblkn = strtol(argv[3], NULL, 0);
  if(iblkn < 0){
    printf("Invalid blkn = %d\n", iblkn);
    fclose(ifd);
    fclose(ofd);
    exit(0);
  }
  
  blkn = 0;
  memset(buff, 0, sizeof(buff));
  
  while(fread((char *)&hd, sizeof(hd), 1, ifd) == 1){
    rhd = ridf_dechd(hd);
    memcpy(buff, (char *)&hd, sizeof(hd));
    fread(buff+sizeof(hd), rhd.blksize - sizeof(hd)/2, 2, ifd);

    printf("iblkn %d / blkn %d (%d)\n", iblkn, blkn, rhd.blksize);

    if(rhd.classid == RIDF_EF_BLOCK ||
       rhd.classid == RIDF_EA_BLOCK ||
       rhd.classid == RIDF_EAEF_BLOCK){
      if(blkn == iblkn){
	fwrite(buff, rhd.blksize, 2, ofd);
	break;
      }
      blkn++;
    }else{
      printf("Invalid header 0x%08x\n", hd.hd1);
      break;
    }
  }

  if(blkn != iblkn){
    printf("Can't find blkn = %d (last blkn = %d)\n", iblkn, blkn);
  }

  fclose(ifd);
  fclose(ofd);

  return 0;
}
