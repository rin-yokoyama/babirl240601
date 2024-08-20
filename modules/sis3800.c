#include "sis3800.h"
/* 
 * original by Masaru-san 
 * reviced by S.Ota 6.Nov.2003.
 * reviced by H.Baba 29.Mar.2008. (erase i-series)
 */
void sis3800_initialize(unsigned int maddr) {
  int dummy = 0x1;
  vwrite32(maddr+SIS3800_GLOBAL_COUNT_ENABLE, &dummy);
}

void sis3800_inputmode(unsigned int maddr, int mode) {
  vwrite32(maddr+SIS3800_CONTROL_REGISTER, (int *)&mode);
}

int sis3800_clear_all(unsigned int maddr){
  int dummy = 0x1;
  vwrite32(maddr+SIS3800_CLEAR_ALL_COUNTERS, &dummy);
  vwrite32(maddr+SIS3800_CLOCK_SHADOW_REGISTER, &dummy);
  return 0;
}

int sis3800_read_scaler(short pos, short len, unsigned int maddr, unsigned short offset){
  volatile int i;
  // 
  // len must be 12*i
  int dummy = 0x1;
  //  mp = blksize/2 - (len * 2 * pos);           /* Set mp */
  // implicit declaration of blksize
#ifndef BLKSIZE
#define BLKSIZE 0x4000
#endif
  mp = BLKSIZE/2 - (len * 2 * pos);           /* Set mp */
  vwrite32(maddr+SIS3800_CLOCK_SHADOW_REGISTER, &dummy);
  vwrite32(maddr+SIS3800_CLEAR_ALL_COUNTERS, &dummy);
  for(i=0;i<len;i++){
    /* Read Scaler data */
    if(i<32){
      vread32(maddr+SIS3800_READ_SHADOW_REGISTER+(offset+i)*4,(int *)(data+mp));
    }
    mp += 2;
  }

  return len * 2;
}


int sis3800_ridf_scaler(unsigned int maddr){
  volatile int i;
  int dummy = 0x1;

  vwrite32(maddr+SIS3800_CLOCK_SHADOW_REGISTER, &dummy);
  vwrite32(maddr+SIS3800_CLEAR_ALL_COUNTERS, &dummy);
  for(i=0;i<SIS3800_NCH;i++){
    /* Read Scaler data */
    vread32(maddr+SIS3800_READ_SHADOW_REGISTER+i*4,(int *)(data+mp));
    mp += 2;
  }

  scrsize += 2 * SIS3800_NCH;
  if(eventsize > 0) eventsize += 2* SIS3800_NCH;

  return scrsize;
}

int sis3800_ridf_ncscaler(unsigned int maddr){
  volatile int i;
  int dummy = 0x1;

  vwrite32(maddr+SIS3800_CLOCK_SHADOW_REGISTER, &dummy);
  for(i=0;i<SIS3800_NCH;i++){
    /* Read Scaler data */
    vread32(maddr+SIS3800_READ_SHADOW_REGISTER+i*4,(int *)(data+mp));
    mp += 2;
  }

  scrsize += 2 * SIS3800_NCH;
  if(eventsize > 0) eventsize += 2* SIS3800_NCH;

  return scrsize;
}

int sis3800_segdata(unsigned int maddr, int ch){
  int dummy = 0x1;

  vwrite32(maddr+SIS3800_CLOCK_SHADOW_REGISTER, &dummy);
  vwrite32(maddr+SIS3800_CLEAR_ALL_COUNTERS, &dummy);
  vread32(maddr+SIS3800_READ_SHADOW_REGISTER+ch*4,(int *)(data+mp));
  mp += 2;
  segmentsize += 2;

  return segmentsize;
}

int sis3800_segndata(unsigned int maddr, int len){
  int dummy = 0x1;
  int i;
  vwrite32(maddr+SIS3800_CLOCK_SHADOW_REGISTER, &dummy);
  vwrite32(maddr+SIS3800_CLEAR_ALL_COUNTERS, &dummy);
  for(i=0;i<len;i++){
    if(i < 32){
      vread32(maddr+SIS3800_READ_SHADOW_REGISTER+i*4,(int *)(data+mp));
      mp += 2;
      segmentsize += 2;
    }
  }

  return segmentsize;
}

void sis3800_setinhibit(unsigned int maddr){
  int dummy = SIS3800_SET_INPUT_BIT0;

  vwrite32(maddr+SIS3800_CONTROL_REGISTER, &dummy);

}
