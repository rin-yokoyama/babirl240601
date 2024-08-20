#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <bi-config.h>
#include <bi-common.h>

#define COMFIFO "/tmp/babirldbfifo"

#define SIZ 100

int main(int argc, char *argv[]){
  int fd;
  int len, i;
  char ddata[SIZ];

  if((fd = open(COMFIFO, O_WRONLY)) < 1){
    printf("Can't open %s\n", COMFIFO);
    exit(0);
  }

  for(i=0;i<SIZ;i++){
    ddata[i] = i;
  }

  len = sizeof(ddata);


  flock(fd, LOCK_EX);
  write(fd, (char *)&len, sizeof(len));
  write(fd, ddata, len);
  flock(fd, LOCK_UN);

  close(fd);

  return 0;
}

