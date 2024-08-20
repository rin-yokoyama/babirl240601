#include "v792.h"

int v792_segdata(unsigned int maddr){
  int wordcnt;

  wordcnt = 0;
  vread32(maddr+V792_OUTBUFF,(int *)(data+mp));
  mp += 2;
  segmentsize += 2;
  wordcnt++;

  if((data[mp-1] & V792_TYPE_MASK_S) == V792_HEADER_BIT_S){
    while(wordcnt < 34){
      /* vread32(maddr+V792_OUTBUFF+wordcnt*32,(int *)(data+mp)); */
      vread32(maddr+V792_OUTBUFF,(int *)(data+mp));
      mp += 2;
      segmentsize += 2;
      wordcnt++;
      if((data[mp-1] & (V792_TYPE_MASK_S)) != V792_DATA_BIT_S){
        break;
      }
    }
  }

  return segmentsize;
}

int v792_segdata2(unsigned int maddr){
  int wordcnt;
  int tdata;
  
  wordcnt = 0;
  vread32(maddr+V792_OUTBUFF,&tdata);

  if((tdata & V792_TYPE_MASK) != V792_HEADER_BIT){
    return 0;
  }
  data[mp++] = (tdata)&0xffff;
  data[mp++] = (tdata >> 16)&0xffff;
  segmentsize += 2;
  wordcnt++;

  while(wordcnt < 34){
    vread32(maddr+V792_OUTBUFF,(int *)(data+mp));
    mp += 2;
    segmentsize += 2;
    wordcnt++;
    if((data[mp-1] & (V792_TYPE_MASK_S)) != V792_DATA_BIT_S){
      break;
    }
  }

  return segmentsize;
}


int v792_segmod(short segid,unsigned int maddr){
  int tmp,wordcnt;

  wordcnt = 0;
  tmp = mp;

  mp += 2;

  vread32(maddr+V792_OUTBUFF,(int *)(data+mp));
  mp += 2;
  wordcnt++;

  if((data[mp-1] & V792_TYPE_MASK_S) == V792_HEADER_BIT_S){
    while(wordcnt < 34){
      vread32(maddr+V792_OUTBUFF+wordcnt*32,(int *)(data+mp));
      mp += 2;
      wordcnt++;
      if((data[mp-1] & (V792_TYPE_MASK_S)) != V792_DATA_BIT_S){
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

int v792_dmasegdata(unsigned int maddr, int rsize){
  /* rsize : nanko data wo yomuka (int word wo nanko ka) */
  int wordcnt, csize;
  int staadr, stoadr, staflag, vsize;
  volatile int dmadelay, dmaflag, loop;

  wordcnt = 0;
  dmaflag = 0;

  csize = rsize * 4;  /* int word -> char size */
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
      if((dmadata[loop] & V792_TYPE_MASK) == V792_HEADER_BIT){
	staadr = loop;
	staflag = 1;
	//}else if(staflag &&
	//     (dmadata[loop] & V792_TYPE_MASK) == V792_DATA_BIT){
      }else if(staflag && 
	       (((dmadata[loop] & V792_TYPE_MASK) == V792_EOB_BIT) ||
		((dmadata[loop] & V792_TYPE_MASK) == V792_ERROR_BIT))){
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

void v792_clear(unsigned int maddr){
  short sval;
 
  sval = 0x04;
 
  vwrite16(maddr + V792_BIT_SET2, &sval);
  vwrite16(maddr + V792_BIT_CLE2, &sval);
 
}

