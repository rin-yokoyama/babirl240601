#include "babirldrvcaen.h"

/** v7XX_set_interrupt
 *  @maddr base address
 *  @level interrupt level
 *  @n     event counter for the interrupt generation
 **/
void v7XX_set_interrupt(unsigned int maddr, short level, short n){
  vwrite16(maddr + V7XX_INT_REG, &level);
  vwrite16(maddr + V7XX_EVT_TRIG_REG, &n);
}

void v7XX_intlevel(unsigned int maddr, short level){
  vwrite16(maddr + V7XX_INT_REG, &level);
}

int v7XX_segdata(unsigned int maddr){
  int wordcnt;

  wordcnt = 0;
  vread32(maddr+V7XX_OUTBUFF,(int *)(data+mp));
  mp += 2;
  segmentsize += 2;
  wordcnt++;

  if((data[mp-1] & V7XX_TYPE_MASK_S) == V7XX_HEADER_BIT_S){
    while(wordcnt < 34){
      /* vread32(maddr+V775_OUTBUFF+wordcnt*32,(int *)(data+mp)); */
      vread32(maddr+V7XX_OUTBUFF,(int *)(data+mp));
      mp += 2;
      segmentsize += 2;
      wordcnt++;
      if((data[mp-1] & (V7XX_TYPE_MASK_S)) != V7XX_DATA_BIT_S){
        break;
      }
    }
  }

  return segmentsize;
}


int v7XX_segdata2(unsigned int maddr){
  int wordcnt;
  int tdata;

  wordcnt = 0;
  vread32(maddr+V7XX_OUTBUFF,&tdata);

  if((tdata & V7XX_TYPE_MASK) != V7XX_HEADER_BIT){
    return 0;
  }
  data[mp++] = (tdata)&0xffff;
  data[mp++] = (tdata >> 16)&0xffff;
  segmentsize += 2;
  wordcnt++;

  while(wordcnt < 34){
    vread32(maddr+V7XX_OUTBUFF,(int *)(data+mp));
    mp += 2;
    segmentsize += 2;
    wordcnt++;
    if((data[mp-1] & (V7XX_TYPE_MASK_S)) != V7XX_DATA_BIT_S){
      break;
    }
  }

  return segmentsize;
}

int v7XX_dmasegdata(unsigned int maddr, int rsize){
  /* rsize : nanko data wo yomuka (int word wo nanko ka) */
  int wordcnt, csize;
  int staadr, stoadr, staflag, vsize;
  volatile int dmadelay, dmaflag, loop;

  wordcnt = 0;
  dmaflag = 0;

  csize = rsize * 4;  /* long word -> char size */
  vme_dma_vread32_start(maddr, csize);
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

  staadr = 0; stoadr = 0;
  staflag = 0; vsize = 0;

  if(dmaflag){
    for(loop=0;loop<rsize;loop++){
      if((dmadata[loop] & V7XX_TYPE_MASK) == V7XX_HEADER_BIT){
        staadr = loop;
        staflag = 1;
        //}else if(staflag &&
        //     (dmadata[loop] & V7XX_TYPE_MASK) == V7XX_DATA_BIT){
      }else if(staflag &&
               (((dmadata[loop] & V7XX_TYPE_MASK) == V7XX_EOB_BIT) ||
                ((dmadata[loop] & V7XX_TYPE_MASK) == V7XX_ERROR_BIT))){
        stoadr = loop;
        break;
      }else if(!staflag && loop > 1){
        stoadr = 0;
        break;
      }
    }
    if(loop >= rsize){
      stoadr = rsize - 1;
    }
    vsize = stoadr - staadr + 1;

    memcpy((char *)(data+mp),(char *)(dmadata+staadr),vsize*4);

    wordcnt += vsize;
    segmentsize += vsize*2;
    mp += vsize*2;
  }else{
    data[mp++] = dmadelay & 0xffff;
    data[mp++] = 0x0600;
    wordcnt++;
    segmentsize += 2;
  }

  return segmentsize;
}

#ifdef V2718
int v7XX_fxdmasegdata(unsigned int maddr, int rsize){
  /* rsize : int word to be read */
  int csize, ret, vsize;

  csize = rsize * 4;  /* long word -> char size */
  ret = dma_vread32(maddr, (char *)(data+mp), csize);

  vsize = 0;
  if(ret >= 0){
    vsize = ret/2;
  }

  segmentsize += vsize;
  mp += vsize;

  return segmentsize;
}
#endif

void v7XX_clear(unsigned int maddr){
  short sval;

  sval = 0x04;

  vwrite16(maddr + V7XX_BIT_SET2, &sval);
  vwrite16(maddr + V7XX_BIT_CLE2, &sval);

}

#ifdef UNIV
int v7XX_multi_map_segdata(unsigned int moff, int n){
  int wordcnt;

  wordcnt = 0;
  univ_map_vread32(moff+V7XX_OUTBUFF,(int *)(data+mp), n);
  mp += 2;
  segmentsize += 2;
  wordcnt++;

  if((data[mp-1] & V7XX_TYPE_MASK_S) == V7XX_HEADER_BIT_S){
    while(wordcnt < 34){
      univ_map_vread32(moff+V7XX_OUTBUFF,(int *)(data+mp), n);
      mp += 2;
      segmentsize += 2;
      wordcnt++;
      if((data[mp-1] & (V7XX_TYPE_MASK_S)) != V7XX_DATA_BIT_S){
        break;
      }
    }
  }

  return segmentsize;
}

int v7XX_map_segdata(int n){
  return v7XX_multi_map_segdata(0, n);
}


void v7XX_multi_map_clear(unsigned int moff, int n){
  short sval;

  sval = 0x04;

  univ_map_write16(moff + V7XX_BIT_SET2, &sval, n);
  univ_map_write16(moff + V7XX_BIT_CLE2, &sval, n);
}

void v7XX_map_clear(int n){
  v7XX_multi_map_clear(0, n);
}


int v1X90_multi_map_dma_segdata(int dman, int n){
  int dmacnt;

  univ_map_read32(V1X90_EVT_FIFO, (int *)(&dmacnt), n);
  dmacnt = (dmacnt & 0x0000ffff) * 4;
  return univ_dma_segdata(dmacnt, dman);
}

int v1X90_map_dma_segdata(int n){
  /* dman = n */
  return v1X90_multi_map_dma_segdata(n, n);
}

void v1X90_multi_map_clear(unsigned int moff, int n){
  short sval = V1X90_SOFT_CLEAR;

  univ_map_write16(moff+V1290_SOFT_CLEAR, &sval, n); 
}

void v1X90_map_clear(int n){
  v1X90_multi_map_clear(0, n);
}

void v1X90_multi_map_intlevel(short level, unsigned int moff, int n){
  univ_map_write16(moff+V1X90_INT_LEVEL, &level, n);
}

void v1X90_map_intlevel(short level, int n){
  v1X90_multi_map_intlevel(level, 0, n);
}


#endif
