#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "nbbqvio.h"


#define RPVADDR 0x8000
#define LEVEL   0x0a
#define PULSE   0x08

#define A16     0x29
#define A32     0x09


void pulse(unsigned short val){

  init_nbbqvio();
  vme_amsr(A16);
  vme_write16(RPVADDR | PULSE, val);
  vme_amsr(A32);
  release_nbbqvio();

}

int main(int argc, char *argv[]){
  unsigned short val;

  if(argc != 2){
    printf("pulserpv130 VAL\n");
    exit(0);
  }

  val = strtol(argv[1], NULL, 0);
  pulse(val);


  return 0;
}
