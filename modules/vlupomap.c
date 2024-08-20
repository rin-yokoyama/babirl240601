#include "vlupo.h"

int vlupoio_nimin_segdata_map(int n){

  univ_map_read16(VLUPO_IO_NIMIN, (short *)(data+mp), n);
  mp +=1;
  segmentsize +=1;

  return segmentsize;
}

int vlupoio_nimcoin_segdata_map(int n){

  univ_map_read16(VLUPO_IO_NIMCOIN, (short *)(data+mp), n);
  mp += 1;
  segmentsize += 1;

  return segmentsize;
}

int vlupoio_lvdsin_segdata_map(int n){

  univ_map_read16(VLUPO_IO_LVDSIN, (short *)(data+mp), n);
  mp += 1;
  segmentsize += 1;

  return segmentsize;
}

int vlupoio_lvdscoin_segdata_map(int n){

  univ_map_read16(VLUPO_IO_LVDSCOIN, (short *)(data+mp), n);
  mp += 1;
  segmentsize += 1;

  return segmentsize;
}

void vlupoio_clear_map(int n){
  short val;

  univ_map_read16(VLUPO_IO_CLEAR, &val, n);
}

void vlupoio_enable_interrupt_map(int n){
  short val = 1;

  univ_map_write16(VLUPO_IO_INTENABLE, &val, n);

}

void vlupoio_disable_interrupt_map(int n){
  short val = 1;

  univ_map_write16(VLUPO_IO_INTDISABLE, &val, n);

}

void vlupoio_pulse_map(int n, short val){

  univ_map_write16(VLUPO_IO_PULSE, &val, n);
}

void vlupoio_level_map(int n, short val){

  univ_map_write16(VLUPO_IO_LEVEL, &val, n);
}

void vlupoio_intdelay_map(int n, short val){

  univ_map_write16(VLUPO_IO_INTDELAY, &val, n);
}

void vlupoio_pulsewidth_map(int n, short val){

  univ_map_write16(VLUPO_IO_PULSEWIDTH, &val, n);
}

int vlupotrg_clk_map(int n){
  univ_map_read32(0x00, (int *)(data+mp), n);
  mp += 2;
  univ_map_read32(0x04, (int *)(data+mp), n);
  mp += 2;

  segmentsize += 4;

  return segmentsize;
}

int vlupots_segts_map(int n){

  univ_map_read32(VLUPO_TS_DATA32L, (int *)(data+mp), n);
  mp += 2;
  univ_map_read32(VLUPO_TS_DATA32H, (int *)(data+mp), n);
  mp += 1;
  segmentsize += 3;
  
  memcpy((char *)&timestamp , (char *)(data+mp-3), 8);
  
  
  return segmentsize;
}

int vlupots_segtsfly_map(int n){
  univ_map_read32(VLUPO_TS_DATAFLY, (int *)(data+mp), n);
  mp += 2;
  segmentsize += 2;

  return segmentsize;
}

void vlupots_intdelay_map(int n, short val){

  univ_map_write16(VLUPO_TS_INTDELAY, &val, n);
}

void vlupots_pulse_map(int n, short val){

  univ_map_write16(VLUPO_TS_PULSE, &val, n);
}

void vlupots_pulsewidth_map(int n, short val){

  univ_map_write16(VLUPO_TS_PULSEWIDTH, &val, n);
}

void vlupots_clear_map(int n){
  short val = 1;

  univ_map_read16(VLUPO_TS_CLEAR, &val, n);
}

void vlupots_clear_counter_map(int n){
  short val = 1;

  univ_map_read16(VLUPO_TS_CLEARCOUNTER, &val, n);
}

int vlupots_segfifocnt_map(int n){
   int lval;
   
   univ_map_read32(VLUPO_TS_FIFOCNT, &lval, n);
  data[mp] = lval & 0x0000ffff;
  mp ++;
  segmentsize ++;

  return segmentsize;
}

int vlupots_segtrgcnt_map(int n){
   
  univ_map_read32(VLUPO_TS_TRGCOUNTER, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

void vlupots_clkmode_map(int n, short val){

  univ_map_write16(VLUPO_TS_CLKMODE, &val, n);
}


void vlupots_enable_interrupt_map(int n){
  short val = 1;

  univ_map_write16(VLUPO_TS_INTENABLE, &val, n);

}

void vlupots_disable_interrupt_map(int n){
  short val = 1;

  univ_map_write16(VLUPO_TS_INTDISABLE, &val, n);

}


int vlupodm_segclk10m_map(int n){
   
  univ_map_read32(VLUPO_DM_CLK10M, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrclk10m_map(int n){

  univ_map_read32(VLUPO_DM_CLK10M, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}

int vlupodm_segclk10k_map(int n){
   
  univ_map_read32(VLUPO_DM_CLK10K, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrclk10k_map(int n){

  univ_map_read32(VLUPO_DM_CLK10K, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}

int vlupodm_segclk1k_map(int n){
   
  univ_map_read32(VLUPO_DM_CLK1K, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrclk1k_map(int n){

  univ_map_read32(VLUPO_DM_CLK1K, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}

void vlupodm_pulse_map(int n, short val){

  univ_map_write16(VLUPO_DM_PULSE, &val, n);
}

void vlupodm_level_map(int n, short val){

  univ_map_write16(VLUPO_DM_LEVEL, &val, n);
}

int vlupodm_segungated_map(int n){
   
  univ_map_read32(VLUPO_DM_UNGATED, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrungated_map(int n){

  univ_map_read32(VLUPO_DM_UNGATED, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}

int vlupodm_seggated_map(int n){
   
  univ_map_read32(VLUPO_DM_GATED, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrgated_map(int n){

  univ_map_read32(VLUPO_DM_GATED, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}

int vlupodm_segscr0_map(int n){
   
  univ_map_read32(VLUPO_DM_SCR0, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrscr0_map(int n){

  univ_map_read32(VLUPO_DM_SCR0, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}
int vlupodm_segscr1_map(int n){
   
  univ_map_read32(VLUPO_DM_SCR1, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrscr1_map(int n){

  univ_map_read32(VLUPO_DM_SCR1, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}
int vlupodm_segscr2_map(int n){
   
  univ_map_read32(VLUPO_DM_SCR2, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrscr2_map(int n){

  univ_map_read32(VLUPO_DM_SCR2, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}
int vlupodm_segscr3_map(int n){
   
  univ_map_read32(VLUPO_DM_SCR3, (int *)(data+mp), n);
  mp +=2;
  segmentsize +=2;

  return segmentsize;
}

int vlupodm_scrscr3_map(int n){

  univ_map_read32(VLUPO_DM_SCR3, (int *)(data+mp), n);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;

  return scrsize;
}
int vlupodm_segtrgsrc_map(int n){

  univ_map_read16(VLUPO_DM_TRGSRC, (short *)(data+mp), n);
  mp +=1;
  segmentsize +=1;

  return segmentsize;
}
void vlupodm_pulsewidth_map(int n, short val){

  univ_map_write16(VLUPO_DM_PULSEWIDTH, &val, n);
}
void vlupodm_intdelay_map(int n, short val){

  univ_map_write16(VLUPO_DM_INTDELAY, &val, n);
}
void vlupodm_intevtcnt_map(int n, short val){

  univ_map_write16(VLUPO_DM_INTEVTCNT, &val, n);
}
void vlupodm_outconf0_map(int n, short val){

  univ_map_write16(VLUPO_DM_OUTCONF0, &val, n);
}
void vlupodm_outconf1_map(int n, short val){

  univ_map_write16(VLUPO_DM_OUTCONF1, &val, n);
}
void vlupodm_outconf2_map(int n, short val){

  univ_map_write16(VLUPO_DM_OUTCONF2, &val, n);
}
void vlupodm_outconf3_map(int n, short val){

  univ_map_write16(VLUPO_DM_OUTCONF3, &val, n);
}

void vlupodm_andor0_map(int n, short val){

  univ_map_write16(VLUPO_DM_ANDOR0, &val, n);
}
void vlupodm_andor1_map(int n, short val){

  univ_map_write16(VLUPO_DM_ANDOR1, &val, n);
}
void vlupodm_andor2_map(int n, short val){

  univ_map_write16(VLUPO_DM_ANDOR2, &val, n);
}
void vlupodm_andor3_map(int n, short val){

  univ_map_write16(VLUPO_DM_ANDOR3, &val, n);
}
void vlupodm_trgconf_map(int n, short val){

  univ_map_write16(VLUPO_DM_TRGCONF, &val, n);
}
void vlupodm_trgact_map(int n, short val){

  univ_map_write16(VLUPO_DM_TRGACT, &val, n);
}

void vlupodm_clear_map(int n){
  short val;

  univ_map_read16(VLUPO_DM_CLEAR, &val, n);
}
void vlupodm_clearscr_map(int n){
  short val;

  univ_map_read16(VLUPO_DM_CLEARSCR, &val, n);
}
void vlupodm_cleartrgscr_map(int n){
  short val;

  univ_map_read16(VLUPO_DM_CLEARTRGSCR, &val, n);
}
void vlupodm_clearall_map(int n){
  short val;

  univ_map_read16(VLUPO_DM_CLEARALL, &val, n);
}
void vlupodm_enable_interrupt_map(int n){
  short val = 1;

  univ_map_write16(VLUPO_DM_INTENABLE, &val, n);

}
void vlupodm_disable_interrupt_map(int n){
  short val = 1;

  univ_map_write16(VLUPO_DM_INTDISABLE, &val, n);

}

int vlupodm_clk_map(int n){
  univ_map_read32(VLUPO_DM_CLK10M, (int *)(data+mp), n);
  mp += 2;
  univ_map_read32(VLUPO_DM_CLK10K, (int *)(data+mp), n);
  mp += 2;

  segmentsize += 4;

  return segmentsize;
}

int vlupodm_scrdummy(void){
  int zr=0;
 
  memcpy((char *)(data+mp), (char *)&zr, 4);
  mp += 2;
  scrsize += 2;
  if(eventsize > 0) eventsize += 2;
 
  return scrsize;
}
 
