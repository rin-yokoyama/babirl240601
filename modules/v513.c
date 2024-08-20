/* V513 I/O Reg */

#include "v513.h"

int v513_segdata(unsigned int maddr){

  vread16(maddr+V513_INPUT,data+mp);
  mp++;
  segmentsize++;

  return segmentsize;
}

int v513_segdata_v(unsigned int maddr, unsigned short *sval){

  vread16(maddr+V513_INPUT,data+mp);
  memcpy((char *)sval,(char *)(data+mp),2);
  mp++;
  segmentsize++;

  return segmentsize;
}

void v513_cldata(unsigned int maddr){
  short val;
  val = 1;
  vwrite16(maddr+V513_CLR_INPUT,&val);
}

void v513_clstrobe(unsigned int maddr){
  short val;
  val = 1;
  vwrite16(maddr+V513_CLR_STROBE,&val);
}

void v513_clint(unsigned int maddr){
  short val;
  val = 1;
  vwrite16(maddr+V513_CLR_INT,&val);
}
