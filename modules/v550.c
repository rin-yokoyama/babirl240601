#include "v550.h"

/*
 * vread16(unsigned int address, unsinged short *val);
 * address : address of register to be read
 * buffer  : pointer to the data buffer where one short word is to be stored.
 */
int v550_segdata(unsigned int maddr,unsigned int geo){
  short nData = 0;
  short nCh1, nCh2;
  int i;
  vread16(maddr+V550_NUMBER_OF_CHANNELS,&nData);
  nCh1 = (nData&0x0fc0)>>6;
  nCh2 = (nData&0x003f);
  vread16(maddr+V550_WORD_COUNTER_REGISTERS0,&nData);
  nData &= 0x7ff;
  *(int*)(data+mp) = (geo&0x1f)<<27 | nCh1 << 16 | nData;
  mp+=2;
  segmentsize += 2;
  for( i=0; i < nData; i++){
//    if (mp > MAXBUFF) {
//      printk("ch0 break at i=%d\n",i);
//      break;
//    }
    vread32(maddr+V550_FIFO_CHANNEL0,(int *)(data+mp));
    mp+=2;
    segmentsize += 2;
  }
  vread16(maddr+V550_WORD_COUNTER_REGISTERS1,&nData);
  nData &= 0x7ff;
  *(int*)(data+mp) = (geo&0x1f)<<27 | nCh2 << 16 | nData;
  mp+=2;
  segmentsize += 2;
  for( i=0; i < nData; i++){
//    if (mp > MAXBUFF) {
//      printk("ch1 break at i=%d\n",i);
//      break;
//    }
    vread32(maddr+V550_FIFO_CHANNEL1,(int *)(data+mp));
    *((int*)(data+mp)) |= (1<<28);
    mp+=2;
    segmentsize += 2;
  }
  return segmentsize;
} 

int v550_dma_segdata(unsigned int maddr, int n1, int n2, int geo){
  short nData = 0;
  short nCh1, nCh2;
  //int i;

  vread16(maddr+V550_NUMBER_OF_CHANNELS,&nData);
  nCh1 = (nData&0x0fc0)>>6;
  nCh2 = (nData&0x003f);
  vread16(maddr+V550_WORD_COUNTER_REGISTERS0,&nData);
  nData &= 0x7ff;
  *(int*)(data+mp) = (geo&0x1f)<<27 | nCh1 << 16 | nData;
  mp+=2;
  segmentsize += 2;

  univ_dma_read((char *)(data+mp), nData*4, n1);
  segmentsize += nData*4/2;
  mp += nData*4/2;


  vread16(maddr+V550_WORD_COUNTER_REGISTERS1,&nData);
  nData &= 0x7ff;
  *(int*)(data+mp) = (geo&0x1f)<<27 | nCh2 << 16 | nData;
  mp+=2;
  segmentsize += 2;

  univ_dma_read((char *)(data+mp), nData*4, n2);
  segmentsize += nData*4/2;
  mp += nData*4/2;

  

  return segmentsize;
}


int v550_map_segdata(unsigned int maddr,int n, unsigned int geo){
  short nData = 0;
  short nCh1, nCh2;
  int i;

  univ_map_read16(V550_NUMBER_OF_CHANNELS,&nData,n);
  nCh1 = (nData&0x0fc0)>>6;
  nCh2 = (nData&0x003f);
  univ_map_read16(V550_WORD_COUNTER_REGISTERS0,&nData,n);
 
  nData &= 0x7ff;
  *(int*)(data+mp) = (geo&0x1f)<<27 | nCh1 << 16 | nData;
  mp+=2;
  segmentsize += 2;
  for( i=0; i < nData; i++){
    univ_map_read32(V550_FIFO_CHANNEL0, (int *)(data+mp), n);
    mp+=2;
    segmentsize += 2;
  }
 
  univ_map_read16(V550_WORD_COUNTER_REGISTERS1,&nData,n);
   nData &= 0x7ff;
  *(int*)(data+mp) = (geo&0x1f)<<27 | nCh2 << 16 | nData;
  mp+=2;
  segmentsize += 2;
  for( i=0; i < nData; i++){
    univ_map_read32(V550_FIFO_CHANNEL1, (int *)(data+mp), n);
    *((int*)(data+mp)) |= (1<<28);
    mp+=2;
    segmentsize += 2;
  }
  return segmentsize;
} 
