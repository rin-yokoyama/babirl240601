#include "sis3300.h"

int sis3300_segdata(unsigned int maddr,int gr){
  int wordcnt;
  int graddr;
  int max;

  wordcnt = 0;
  graddr = SIS3300_BANKSIZE*gr;

  vread32(maddr+SIS3300_EVT_DIR_BANK1+SIS3300_EVT_DIR_OFF*gr,&max);
  if(max > 3500){
    max = 3500;
  }

  while(wordcnt < max){

    vread32(maddr+SIS3300_MEMBASE_BANK1+graddr+wordcnt*4,
	    (int *)(data+mp));
    mp += 2;
    wordcnt++;
    segmentsize += 2;

    if(data[mp-2] & (SIS3300_ORBIT_S | SIS3300_USRBIT_S)){
      break;
    }
  }
  
  return segmentsize;
}

int sis3300_segdata_pre(unsigned int maddr,int gr,int max){
  int wordcnt;
  int graddr;
  int adptr;

  wordcnt = 0;
  graddr = SIS3300_BANKSIZE*gr;
  if(max > 3500){
    max = 3500;
  }

  vread32(maddr+SIS3300_TRIGGER_EVT_DIR_BANK1,&adptr);
  adptr = (adptr & 0x001ffff) - max;

  if(adptr < 0){
    adptr = 0x00020000 + adptr;
  }

  while(wordcnt < max){
    adptr += 1;
    if(adptr >= 0x00020000){
      adptr -= 0x00020000;
    }
    vread32(maddr+SIS3300_MEMBASE_BANK1+graddr+(adptr*4),
	    (int *)(data+mp));
    mp += 2;
    wordcnt++;
    segmentsize += 2;

    if(data[mp-2] & (SIS3300_ORBIT_S | SIS3300_USRBIT_S)){
      break;
    }
  }
  
  return segmentsize;
}

int sis3300_dmasegdata_pre_i(unsigned int maddr,int gr,int max, int dn){
  int wordcnt;
  int graddr, rsize, csize;
  int adptr;
  volatile int dmadelay, dmaflag;
  
  wordcnt = 0;
  graddr = SIS3300_BANKSIZE*gr;
  if(max > 3500){
    max = 3500;
  }
  
  rsize = max;
  
  vread32(maddr+SIS3300_TRIGGER_EVT_DIR_BANK1,&adptr);

  adptr = (adptr & 0x001ffff) - max;
  
  if(adptr < 0){
    rsize = 0 - adptr;
    adptr = 0x00020000 + adptr;
  }

  dmaflag = 0;
  csize = rsize * 4;  /* int word -> char size */
  vme_dma_vread32_start(maddr+SIS3300_MEMBASE_BANK1+graddr+(adptr*4), csize);
  delay_us();  /* delay about 1us */
  dmadelay = 0;
  for(dmadelay=0;dmadelay<1000;dmadelay++){
    if(vme_dma_vread32_store((char *)dmadata,csize)){
      dmadelay = 5555;
      dmaflag = 1;
    }else{
      delay_us();
    }
  }
  
  if(dmaflag){
    memcpy((char *)(data+mp),(char *)(dmadata),csize);
    wordcnt += rsize;
    segmentsize += rsize*2;
    mp += rsize*2;
  }else{
    data[mp++] = 0xfeee;
    data[mp++] = dmadelay & 0xffff;
    wordcnt++;
    segmentsize += 2;
  }

  if(rsize < max){
    dmaflag = 0;
    adptr = 0;
    rsize = max - rsize;
    //printk("***  rsize %d   max %d\n",rsize,max);
    csize = rsize * 4;  /* int word -> char size */
    vme_dma_vread32_start(maddr+SIS3300_MEMBASE_BANK1+graddr+(adptr*4), csize);

    delay_us();  /* delay about 1us */
    dmadelay = 0;
    for(dmadelay=0;dmadelay<1000;dmadelay++){
      if(vme_dma_vread32_store((char *)dmadata,csize)){
	dmadelay = 5555;
	dmaflag = 1;
      }else{
	delay_us();
      }
    }
    if(dmaflag){
      memcpy((char *)(data+mp),(char *)(dmadata),csize);
      wordcnt += rsize;
      segmentsize += rsize*2;
      mp += rsize*2;
    }else{
      data[mp++] = 0xfeee;
      data[mp++] = dmadelay & 0xffff;
      wordcnt++;
      segmentsize += 2;
    }
  }
  
  return segmentsize;
}

int sis3300_dmasegdata_pre(unsigned int maddr,int gr,int max){
  return sis3300_dmasegdata_pre(maddr,gr,max);
}


int sis3300_dmasegdata_pre2_i(unsigned int maddr,int gr,int max, int off, int dn){
  int wordcnt;
  int graddr, rsize, csize;
  int adptr;
  volatile int dmadelay, dmaflag;
  
  wordcnt = 0;
  graddr = SIS3300_BANKSIZE*gr;
  if(max > 3500){
    max = 3500;
  }
  
  rsize = max;
  
  vread32(maddr+SIS3300_TRIGGER_EVT_DIR_BANK1,&adptr);

  adptr = (adptr & 0x001ffff) - max - off;
  
  if(adptr < 0){
    rsize = 0 - adptr;
    adptr = 0x00020000 + adptr;
  }

  dmaflag = 0;
  csize = rsize * 4;  /* int word -> char size */
  vme_dma_vread32_start(maddr+SIS3300_MEMBASE_BANK1+graddr+(adptr*4), csize);
  delay_us();  /* delay about 1us */
  dmadelay = 0;
  for(dmadelay=0;dmadelay<1000;dmadelay++){
    if(vme_dma_vread32_store((char *)dmadata,csize)){
      dmadelay = 5555;
      dmaflag = 1;
    }else{
      delay_us();
    }
  }
  
  if(dmaflag){
    memcpy((char *)(data+mp),(char *)(dmadata),csize);
    wordcnt += rsize;
    segmentsize += rsize*2;
    mp += rsize*2;
  }else{
    data[mp++] = 0xfeee;
    data[mp++] = dmadelay & 0xffff;
    wordcnt++;
    segmentsize += 2;
  }

  if(rsize < max){
    dmaflag = 0;
    adptr = 0;
    rsize = max - rsize;
    //printk("***  rsize %d   max %d\n",rsize,max);
    csize = rsize * 4;  /* int word -> char size */
    vme_dma_vread32_start(maddr+SIS3300_MEMBASE_BANK1+graddr+(adptr*4), csize);

    delay_us();  /* delay about 1us */
    dmadelay = 0;
    for(dmadelay=0;dmadelay<1000;dmadelay++){
      if(vme_dma_vread32_store((char *)dmadata,csize)){
	dmadelay = 5555;
	dmaflag = 1;
      }else{
	delay_us();
      }
    }
    if(dmaflag){
      memcpy((char *)(data+mp),(char *)(dmadata),csize);
      wordcnt += rsize;
      segmentsize += rsize*2;
      mp += rsize*2;
    }else{
      data[mp++] = 0xfeee;
      data[mp++] = dmadelay & 0xffff;
      wordcnt++;
      segmentsize += 2;
    }
  }
  
  return segmentsize;
}

int sis3300_dmasegdata_pre2(unsigned int maddr,int gr,int max,int off){
  return sis3300_dmasegdata_pre2(maddr,gr,max,off);
}

void sis3300_set_mode(unsigned int maddr, int mode){
  int lval;

  lval = mode;
  vwrite32(maddr+SIS3300_EVENT_CONFIG_ADC12,&lval);
  vwrite32(maddr+SIS3300_EVENT_CONFIG_ADC34,&lval);
  vwrite32(maddr+SIS3300_EVENT_CONFIG_ADC56,&lval);
  vwrite32(maddr+SIS3300_EVENT_CONFIG_ADC78,&lval);
  vwrite32(maddr+SIS3300_EVENT_CONFIG_ALL_ADC,&lval);
}

void sis3300_acq_ctrl(unsigned int maddr, int mode){
  int lval;

  lval = mode;
  vwrite32(maddr+SIS3300_ACQ_CTRL,&lval);
}

int sis3300_header(short mid, short bank, short size){
  data[mp++] = (mid<<8 | bank | 0x8000) & 0xffff;
  data[mp++] = size;
  segmentsize += 2;

  return segmentsize;
}

void sis3300_stop_delay(unsigned int maddr, int size){
  int lval = size;
  vwrite32(maddr+SIS3300_STOP_DELAY,&lval);
}

int sis3300_segth(unsigned int maddr,int gr){
  int graddr;

  graddr = SIS3300_TRIGGER_THRESHOLD_ADC12+SIS3300_BANKSIZE*gr;

  vread32(maddr+graddr,(int *)(data+mp));
  mp += 2;
  segmentsize += 2;
  
  return segmentsize;
}

void sis3300_writeth(unsigned int maddr,int gr,int val){
  int graddr;
  int lval;

  graddr = SIS3300_TRIGGER_THRESHOLD_ADC12+SIS3300_BANKSIZE*gr;

  lval = val;
  vwrite32(maddr+graddr,&lval);
}

int sis3300_readth(unsigned int maddr,int gr){
  int graddr;
  int lval;

  graddr = SIS3300_TRIGGER_THRESHOLD_ADC12+SIS3300_BANKSIZE*gr;

  vread32(maddr+graddr,(int *)(&lval));
  
  return lval;
}

int sis3300_segtrig(unsigned int maddr){

  vread32(maddr+SIS3300_TRIGGER_EVT_DIR_BANK1,(int *)(data+mp));
  mp += 2;
  segmentsize += 2;
  
  return segmentsize;
}

void sis3300_userout(unsigned int maddr, int onoff){
  int lval;

  if(onoff){
    lval = 0x00000002;
    vwrite32(maddr+SIS3300_CTRL_STAT, &lval);
  }else{
    lval = 0x00020000;
    vwrite32(maddr+SIS3300_CTRL_STAT, &lval);
  }
}
