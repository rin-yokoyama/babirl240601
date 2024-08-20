#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include <bbzfile.h>

#include <bi-config.h>
#include <bi-common.h>


bgFile *bgropen(char *file){
  int tfd;
  bgFile *fd;

  if(!file){
    printf("zoro length filename\n");
    return 0;
  }
  if(strlen(file) < 4){
    printf("too short file name\n");
    return 0;
  }

  if((tfd = open(file, O_RDONLY)) < 1){
    return 0;
  }else{
    fd = malloc(sizeof(bgFile));
    if(!strncmp(file+strlen(file)-3, ".gz", 3)){
      fd->gfd = gzdopen(tfd, "r");
      fd->nfd = 0;
    }else{
      fd->nfd = tfd;
      fd->gfd = 0;
    }
  }

  return fd;
}

void bgclose(bgFile *fd){
  if(fd){
    if(fd->nfd){
      close(fd->nfd);
    }else{
      gzclose(fd->gfd);
    }
    free(fd);
  }
}

int bgread(bgFile *fd, void *buff, int cnt){
  if(fd->nfd){
    return read(fd->nfd, buff, cnt);
  }else{
    return gzread(fd->gfd, buff, cnt);
  }
}
