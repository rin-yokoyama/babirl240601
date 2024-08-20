#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
  FILE *fd;
  char buf[1024] = {0};
  int i=0;

  fd = popen("/home/baba/daq/babirl/babicon/babiconjson bbdaqsrv", "w");
  if(fd == NULL){
    printf("cannot open babiconjson\n");    return 0;
  }

  for(i=0;i<10;i++){
    sprintf(buf, "getconfig\n");
    printf("************* %d ****************\n", i);
    fprintf(fd, "%s", buf);
    fgets(buf, sizeof(buf), fd);
    printf("%s\n", buf);
    sleep(1);
    }
  if(fd) pclose(fd);

  return 0;
}
