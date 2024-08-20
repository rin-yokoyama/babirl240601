#include "v767.h"
#define V767_MAXEVT 256

int v767_segdata(unsigned long maddr){
  int wordcnt;

  wordcnt = 0;
  vread32(maddr+V767_OUTBUFF,(long *)(data+mp));
  mp += 2;
  segmentsize += 2;
  wordcnt++;

  if((data[mp-1] & V767_TYPE_MASK_S) == V767_HEADER_BIT_S){
    while(wordcnt < V767_MAXEVT){
      vread32(maddr+V767_OUTBUFF,(long *)(data+mp));
      mp += 2;
      segmentsize += 2;
      wordcnt++;
      if((data[mp-1] & (V767_TYPE_MASK_S)) != V767_DATA_BIT_S){
        break;
      }
    }
  }

  return segmentsize;
}


int v767_segmod(short segid,unsigned long maddr){
  int tmp,wordcnt;

  wordcnt = 0;
  tmp = mp;

  mp += 2;

  vread32(maddr+V767_OUTBUFF,(long *)(data+mp));
  mp += 2;
  wordcnt++;

  if((data[mp-1] & V767_TYPE_MASK_S) == V767_HEADER_BIT_S){
    while(wordcnt < V767_MAXEVT){
      vread32(maddr+V767_OUTBUFF,(long *)(data+mp));
      mp += 2;
      wordcnt++;
      if((data[mp-1] & (V767_TYPE_MASK_S)) != V767_DATA_BIT_S){
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

void v767_reset(unsigned long maddr){
  unsigned short sval = 1;
  vwrite16(maddr+V767_RESET,&sval);
}

