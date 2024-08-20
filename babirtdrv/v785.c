#include "v785.h"

int v785_segdata(unsigned long maddr){
  int wordcnt;

  wordcnt = 0;
  vread32(maddr+V785_OUTBUFF,(long *)(data+mp));
  mp += 2;
  segmentsize += 2;
  wordcnt++;

  if((data[mp-1] & V785_TYPE_MASK_S) == V785_HEADER_BIT_S){
    while(wordcnt < 34){
      /* vread32(maddr+V785_OUTBUFF+wordcnt*32,(long *)(data+mp)); */
      vread32(maddr+V785_OUTBUFF,(long *)(data+mp));
      mp += 2;
      segmentsize += 2;
      wordcnt++;
      if((data[mp-1] & (V785_TYPE_MASK_S)) != V785_DATA_BIT_S){
        break;
      }
    }
  }

  return segmentsize;
}

int v785_segdata2(unsigned long maddr){
  int wordcnt;
  long tdata;
  
  wordcnt = 0;
  vread32(maddr+V785_OUTBUFF,&tdata);

  if((tdata & V785_TYPE_MASK) != V785_HEADER_BIT){
    return 0;
  }
  data[mp++] = (tdata)&0xffff;
  data[mp++] = (tdata >> 16)&0xffff;
  segmentsize += 2;
  wordcnt++;

  while(wordcnt < 34){
    vread32(maddr+V785_OUTBUFF,(long *)(data+mp));
    mp += 2;
    segmentsize += 2;
    wordcnt++;
    if((data[mp-1] & (V785_TYPE_MASK_S)) != V785_DATA_BIT_S){
      break;
    }
  }

  return segmentsize;
}


int v785_segmod(short segid,unsigned long maddr){
  int tmp,wordcnt;

  wordcnt = 0;
  tmp = mp;

  mp += 2;

  vread32(maddr+V785_OUTBUFF,(long *)(data+mp));
  mp += 2;
  wordcnt++;

  if((data[mp-1] & V785_TYPE_MASK_S) == V785_HEADER_BIT_S){
    while(wordcnt < 34){
      vread32(maddr+V785_OUTBUFF+wordcnt*32,(long *)(data+mp));
      mp += 2;
      wordcnt++;
      if((data[mp-1] & (V785_TYPE_MASK_S)) != V785_DATA_BIT_S){
        break;
      }
    }
  }
  mp = tmp;
  data[mp++] = wordcnt*2+2;                  /* Write word count */
  data[mp++] = segid;                      /* Write segment ID */

  mp += wordcnt*2;

  eventsize += wordcnt*2 + 2;                    /* Count up event size */

  return wordcnt*2 + 2;
}

int v785_dmasegdata(unsigned long maddr, int rsize){
  /* rsize : nanko data wo yomuka (long word wo nanko ka) */
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
      if((dmadata[loop] & V785_TYPE_MASK) == V785_HEADER_BIT){
	staadr = loop;
	staflag = 1;
	//}else if(staflag &&
	//     (dmadata[loop] & V785_TYPE_MASK) == V785_DATA_BIT){
      }else if(staflag && 
	       (((dmadata[loop] & V785_TYPE_MASK) == V785_EOB_BIT) ||
		((dmadata[loop] & V785_TYPE_MASK) == V785_ERROR_BIT))){
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

void v785_clear(unsigned long maddr){
  short sval;
 
  sval = 0x04;
 
  vwrite16(maddr + V785_BIT_SET2, &sval);
  vwrite16(maddr + V785_BIT_CLE2, &sval);
 
}

