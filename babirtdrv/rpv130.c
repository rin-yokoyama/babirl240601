#include "rpv130.h"

int rpv130_write( unsigned long maddr, unsigned short val){
  vwrite16(maddr, &val);
  return 1;
}

int rpv130_output(unsigned long maddr, unsigned short val){
  set_amsr(0x29);
  rpv130_write(maddr+RPV130_PULSE, val);
  set_amsr(0x09);

  return 1;
}

int rpv130_level(unsigned long maddr, unsigned short val){
  set_amsr(0x29);
  rpv130_write(maddr+RPV130_LEVEL, val);
  set_amsr(0x09);

  return 1;
}


int rpv130_segdata(unsigned long maddr){
  vread16(maddr,(short *)(data+mp));
  mp += 1;
  segmentsize += 1;
  
  return segmentsize;
}

