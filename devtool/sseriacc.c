/* devtool/seriacc.c
 * last modified : 09/05/28 12:08:49 
 *
 * Serial port access program
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 */


#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <bi-config.h>
#include <bi-common.h>

#define DEVICE "/dev/ttyS0"

int sfd;

void quit(void){
  close(sfd);
  rmpid("seriacc");
}

int main(int argc, char *argv[]){
  int arg;
  // int cont;

  if(argc != 3 && argc != 1){
    printf("seriacc dtr/rts on/off/cont\n\n");
    exit(0);
  }

  signal(SIGINT, (void *)quit);

  if((sfd = open(DEVICE,  O_RDWR | O_NOCTTY | O_NDELAY)) < 0){
    printf("Can't open %s\n", DEVICE);
    exit(1);
  }

  //cont = 0;
  if(argc == 3){
    arg = 0;
    if(!strcmp(argv[1], "dtr")){
      arg = TIOCM_DTR;
      printf("DTR ");
    }else if(!strcmp(argv[1], "rts")){
      arg = TIOCM_RTS;
      printf("RTS ");
    }else{
      printf("seriacc dtr/rts on/off\n\n");
    }
    
    if(arg){
      if(!strcmp(argv[2], "on") || !strcmp(argv[2], "cont")){
	ioctl(sfd, TIOCMBIS, &arg ); 
	printf("ON\n");
	//cont = 1;
      }else if(!strcmp(argv[2], "off")){
	ioctl(sfd, TIOCMBIC, &arg ); 
	printf("OFF\n");
      }else{
	printf("seriacc dtr/rts on/off\n\n");
      }
    }
  }else{
    ioctl(sfd, TIOCMGET, &arg ); 
    printf("read = 0x%04x\n", arg);
    printf("RTS  = 0x%04x\n", TIOCM_RTS);
    printf("CTS  = 0x%04x\n", TIOCM_CTS);
    printf("DTR  = 0x%04x\n", TIOCM_DTR);
    printf("CD   = 0x%04x\n", TIOCM_CD);
  }
  
  quit();

  return 0;
}
