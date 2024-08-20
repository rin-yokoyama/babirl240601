#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "nbbqvio.h"


#define RPVADDR 0x8000
#define LEVEL   0x0a

#define A16     0x29
#define A32     0x09


void sta(void){
  short val;

  init_nbbqvio();
  vme_amsr(A16);
  val = 0x13;
  vme_write16(RPVADDR | LEVEL, val);
  val = 0x23;
  vme_write16(RPVADDR | LEVEL, val);
  val = 0x00;
  vme_write16(RPVADDR | LEVEL, val);
  vme_amsr(A32);
  release_nbbqvio();

}


void sto(void){
  short val;

  init_nbbqvio();
  vme_amsr(A16);
  val = 0x43;
  vme_write16(RPVADDR | LEVEL, val);
  val = 0x03;
  vme_write16(RPVADDR | LEVEL, val);
  vme_amsr(A32);
  release_nbbqvio();

}

int main(int argc, char *argv[]){

  if(argc != 2){
    printf("ssmrpv130 start/stop\n");
    exit(0);
  }

  if(!strncmp(argv[1], "sta", 3)){
    sta();
  }else if(!strncmp(argv[1], "sto", 3)){
    sto();
  }else{
    printf("Invalid command\n");
    printf("ssmrpv130 start/stop\n");
  }

  return 0;
}
