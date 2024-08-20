/* madc32 */

#include "madc32.h"

void madc32_clear(unsigned int maddr){
  short val;

  val = 1;
  vwrite16(maddr+MADC32_READOUT_RESET, &val);

}

void madc32_start_acq(unsigned int maddr){
  short val;

  val = 1;
  vwrite16(maddr+MADC32_START_ACQ, &val);
}

void madc32_stop_acq(unsigned int maddr){
  short val;

  val = 0;
  vwrite16(maddr+MADC32_START_ACQ, &val);
}

void madc32_irq_level(unsigned int maddr, short val){

  vwrite16(maddr+MADC32_IRQ_LEVEL, &val);
}

void madc32_resol_8khires(unsigned int maddr){
  short val;

  val = 4;
  vwrite16(maddr+MADC32_ADC_RESOLUTION, &val);
}

void madc32_input_range(unsigned int maddr, short val){
  vwrite16(maddr+MADC32_INPUT_RANGE, &val);
}


void madc32_module_id(unsigned int maddr, short val){
  vwrite16(maddr+MADC32_MODULE_ID, &val);
}

int madc32_segdata(unsigned int maddr){
  volatile short cnt, i;

  vread16(maddr + MADC32_BUFFER_DATA_LENGTH, (short *)&cnt);
  cnt = cnt & 0x3fff;

  for(i=0;i<cnt;i++){
    vread32(maddr + MADC32_DATA, (long *)(data+mp));
    mp += 2;
    segmentsize += 2;
  }

  return segmentsize;
}


#ifdef UNIV
void madc32_map_clear(int n){
  short val;

  val = 1;
  univ_map_write16(MADC32_READOUT_RESET, &val, n);

}

void madc32_map_start_acq(int n){
  short val;

  val = 1;
  univ_map_write16(MADC32_START_ACQ, &val, n);
}

void madc32_map_stop_acq(int n){
  short val;

  val = 0;
  univ_map_write16(MADC32_START_ACQ, &val, n);
}

void madc32_map_irq_level(short val, int n){

  univ_map_write16(MADC32_IRQ_LEVEL, &val, n);
}

void madc32_map_resol_8khires(int n){
  short val;

  val = 4;
  univ_map_write16(MADC32_ADC_RESOLUTION, &val, n);
}

void madc32_map_input_range(short val, int n){
  univ_map_write16(MADC32_INPUT_RANGE, &val, n);
}


void madc32_map_module_id(short val, int n){
  univ_map_write16(MADC32_MODULE_ID, &val, n);
}

int madc32_map_segdata(int n){
  volatile short cnt, i;

  univ_map_read16(MADC32_BUFFER_DATA_LENGTH, (short *)&cnt, n);
  cnt = cnt & 0x3fff;

  for(i=0;i<cnt;i++){
    univ_map_read32(MADC32_DATA, (long *)(data+mp), n);
    mp += 2;
    segmentsize += 2;
  }

  return segmentsize;
}
#endif
