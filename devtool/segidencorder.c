#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MKSEGID(device,focal,detector,module) ((((device<<6 | focal) << 6) | detector)<<8 | module) 

int main(int argc, char *argv[]){
  int device, focal, detector, module;
  unsigned int segid;

  if(argc != 5){
    printf("segidencorder device focal detector module\n");
    exit(0);
  }

  device   = strtol(argv[1], NULL, 0);
  focal    = strtol(argv[2], NULL, 0);
  detector = strtol(argv[3], NULL, 0);
  module   = strtol(argv[4], NULL, 0);

  segid = MKSEGID(device,focal,detector,module);

  printf("0x%08x\n", segid);

  return 0;
}
