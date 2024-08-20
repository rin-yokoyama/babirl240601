#include "babirldrvcaen.h"

int v7XX_segdata(unsigned long maddr){
  int wordcnt;

  wordcnt = 0;
  vread32(maddr+V7XX_OUTBUFF,(long *)(data+mp));
  mp += 2;
  segmentsize += 2;
  wordcnt++;

  if((data[mp-1] & V7XX_TYPE_MASK_S) == V7XX_HEADER_BIT_S){
    while(wordcnt < 34){
      /* vread32(maddr+V775_OUTBUFF+wordcnt*32,(long *)(data+mp)); */
      vread32(maddr+V7XX_OUTBUFF,(long *)(data+mp));
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

void v7XX_clear(unsigned long maddr){
  short sval;

  sval = 0x04;

  vwrite16(maddr + V7XX_BIT_SET2, &sval);
  vwrite16(maddr + V7XX_BIT_CLE2, &sval);

}
