#define SCRADDR 0x12340000

#include "nbbqvio.h"
#include "sis3800.c"

int scr(void){
  int size;
  
  init_nbbqvio();
  size = sis3800_ridf_ncscaler(SCRADDR);
  release_nbbqvio();

  return size;
}

void scrinit(void){
  init_nbbqvio();
  sis3800_initialize(SCRADDR);
  sis3800_clear_all(SCRADDR);
  sis3800_setinhibit(SCRADDR);
  release_nbbqvio();
}

