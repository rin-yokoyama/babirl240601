#include "v1190.h"
void v1190_clear(unsigned int maddr);
int v1190_segdata(unsigned int maddr);

/* Segment data */
int v1190_segdata(unsigned int maddr){
  int wordcnt;

  wordcnt = 0;
  /* Global Header */
  vread32(maddr + V1190_OUTBUFF, (int *)(data + mp));
  mp += 2;
  segmentsize += 2;
  wordcnt ++;

  if((data[mp-1] & V1190_TYPE_MASK_S) == V1190_GLOBAL_HEADER_BIT_S){
    while(wordcnt < 256){
      /* TDC Header */
      vread32(maddr + V1190_OUTBUFF, (int *)(data +mp));
      mp += 2;
      segmentsize += 2;
      wordcnt ++;
      if((data[mp-1] & V1190_TYPE_MASK_S) == V1190_TDC_HEADER_BIT_S){
	while(wordcnt < 256){
	  vread32(maddr + V1190_OUTBUFF, (int *)(data + mp));
	  mp += 2;
	  segmentsize += 2;
	  wordcnt ++;
	  if((data[mp-1] & V1190_TYPE_MASK_S) != V1190_TDC_DATA_BIT_S){
	    /* TDC Trailer or TDC Error */
	    break;
	  }
	}
      }else{
	break;
	/* Global Trailer */
      }
    }
  }

  return segmentsize;
}

/* Software clear */
void v1190_clear(unsigned int maddr){
  short sval;

  sval = V1190_SOFT_CLEAR_BIT;
  vwrite16(maddr + V1190_SOFT_CLEAR, &sval);
}

/* Set Almost full register */
void v1190_tdcfull(unsigned int maddr, short fl){
  short sval;

  sval = fl;
  vwrite16(maddr + V1190_ALMOST_FULL, &sval);
}
