#include "sis3700.h"

int sis3700_segdata(unsigned int maddr){

  int  wordcnt;     /* 1 word = 16 bits */
  int longWord;    /* temporary data storage */
  int thisWordCnt;  /* word count in this event */
  //  int  packed;      /* if packed = 1 pack mode is enabled */

  /*--- read word count ---*/
  vread32(maddr+SIS3700_COUNTERFIFO,&longWord);
  thisWordCnt = (longWord & SIS3700_WORD_COUNT);

  /*--- read pack status ---*/
  //  vread32(maddr+SIS3700_STATUS_REG,&longWord);
  //  packed = (longWord & SIS3700_PACK_MODE);

  /*
   * only pack mode is avilable 
   */
  //  if (!packed) return 0; 
 
  /*--- read and store data ---*/
  if (thisWordCnt == 0x1fff){
    thisWordCnt = 0;
    /*
      printk("SIS3700 : word cnt = 0x1fff\n");
      vread32(maddr+SIS3700_CTRL_REG,&longWord);
      printk("status     : %08x\n",longWord);
      vread32(maddr+SIS3700_COUNTERFIFO,&longWord);
      printk("EVT Number : %08x\n",longWord);
      vread32(maddr+SIS3700_DATAFIFO,&longWord);
      printk("Data 0     : %08x\n",longWord);
    */
  }

  data[mp] = (short)thisWordCnt;
  mp++;
  segmentsize++;
 
  wordcnt = 0;
  while(wordcnt < thisWordCnt) {
    
    vread32(maddr+SIS3700_DATAFIFO,(int*)(data+mp));
    mp+=2;
    segmentsize+=2;
    wordcnt+=2;

  }

  longWord = SIS3700_CLEAR | SIS3700_ENABLE_OUT_VME | SIS3700_ENABLE_PACK;
  vwrite32(maddr+SIS3700_CTRL_REG,&longWord);
  return segmentsize;

}


int sis3700_segmod(short segid,unsigned int maddr){
  int tmp,wordcnt;
  int longWord;    /* temporary data storage */
  int thisWordCnt; /* word count in this event */
  //  int  packed;      /* if packed = 1 pack mode is enabled */

  wordcnt = 0;
  tmp = mp;
  mp += 2;

  /*--- read word count ---*/
  vread32(maddr+SIS3700_COUNTERFIFO,&longWord);
  thisWordCnt = (longWord & SIS3700_WORD_COUNT);

  /*--- read pack status ---*/
  //  vread32(maddr+SIS3700_STATUS_REG,&longWord);
  //  packed = (longWord & SIS3700_PACK_MODE);
  /*
   * only pack mode is avilable 
   */
  //  if (!packed) {
  //    mp = tmp;
  //    data[mp++] = 2;                  /* Write word count */
  //    data[mp++] = segid;                      /* Write segment ID */
  //    eventsize += 2;
  //    return 2; 
  //  }

  /*--- read data ---*/
  data[mp] = (short)thisWordCnt;
  mp++;
  wordcnt++;

  while(wordcnt-1<thisWordCnt) {
 
    vread32(maddr+SIS3700_DATAFIFO,(int*)(data+mp));
    
    mp+=2;
    wordcnt+=2;

  }

  mp = tmp;
  data[mp++] = wordcnt+2;                  /* Write word count */
  data[mp++] = segid;                      /* Write segment ID */

  mp += wordcnt;

  eventsize += wordcnt + 2;                    /* Count up event size */

  longWord = SIS3700_CLEAR;
  vwrite32(maddr+SIS3700_CTRL_REG,&longWord);
  return wordcnt + 2;

}
