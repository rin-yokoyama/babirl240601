/* babirl : babirldrv/babirldrvridf.c
 * last modified : 08/08/29 18:08:33 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * RIDF format functions for driver
 *
 */

void init_mem(void);
void init_block(void);
void init_event(void);
void init_segment(int segid);
int end_block(void);
int end_event(void);
int end_eventts(void);
int end_segment(void);
int get_evtn(void);
void delay_us(void);

#ifdef VME
static unsigned int dmadata[DMAMAX];
#endif
#ifndef DBUFF
static unsigned short data[EB_EFBLOCK_BUFFSIZE];
#else
unsigned short *data;
static unsigned short databuff[EB_EFBLOCK_BUFFSIZE*2];
volatile int dn=0;
#endif

static volatile int eventhmp,eventsize,segmenthmp,segmentsize;
static volatile int mp;
#ifdef DBUFF
static volatile int mpa, mpb;
#endif
static unsigned int evtn;
//static volatile int buffsize = EB_EFBLOCK_BUFFSIZE * WORDSIZE;
//int thd;

static volatile int scrhmp, scrsize;
static volatile unsigned long long int timestamp; 

/** Initialize data buffer memory
 */
#ifndef DBUFF
void init_mem(void){
  memset(data, 0, buffsize);
}
#else
void chmem(void){
  if(dn==0){
    data = databuff + EB_EFBLOCK_BUFFSIZE;
    mpa = mp;
    dn = 1;
  }else{
    data = databuff;
    mpb = mp;
    dn = 0;
  }
}

void init_mem(void){
  dn = 1;
  mp = 0;
  mpa = 0;
  mpb = 0;
  chmem();
  memset(databuff,0,sizeof(databuff));             /* Memory set to '0' */
}
#endif

void end_driver(void){
}

/** Initialize data block
 */
void init_block(void){
  eventsize = 0;
  mp = RIDF_HD_SZ;
}

/** Initialize evtn
 *  This function should be call from startup.c just once.
 */
void init_evtn(void){
  evtn = 0;
}

/** Get evtn
 *  @return evtn event number
 */
int get_evtn(void){
  return evtn;
}

/** Initialize event block
 */
void init_event(void){
  evtn++;                               // Count up event number
  eventhmp = mp;                        // Store mp for event header
  eventsize = RIDF_HDEVT_SZ;            // Initialize event size
  mp += RIDF_HDEVT_SZ;

  // copy evtn
  memcpy((char *)(data+eventhmp)+SIZEOFINT*2, (char *)&evtn, SIZEOFINT);
}

void init_eventts(void){
  evtn++;                               // Count up event number
  eventhmp = mp;                        // Store mp for event header
  eventsize = RIDF_HDEVTTS_SZ;            // Initialize event size
  mp += RIDF_HDEVTTS_SZ;

  // copy evtn
  memcpy((char *)(data+eventhmp)+SIZEOFINT*2, (char *)&evtn, SIZEOFINT);
}
 

/** Initialize segment block
 */
void init_segment(int segid){
  segmenthmp = mp;                      // Store mp for segment header
  segmentsize = RIDF_HDSEG_SZ;          // Initialize segment size
  mp += RIDF_HDSEG_SZ;

  // copy segment id
  memcpy((char *)(data+segmenthmp)+SIZEOFINT*2, (char *)&segid, SIZEOFINT);
}

/** End of data block */
int end_block(void){
  int thd;

  thd = RIDF_MKHD1(RIDF_LY0, RIDF_EF_BLOCK, mp);  // make header 1
  memcpy((char *)data, (char *)&thd, SIZEOFINT);  // copy header 1 to buff
  thd = EFN;                                      // make header 2
  memcpy((char *)data+SIZEOFINT, (char *)&thd, SIZEOFINT); // copy 

  return mp;
}

/** End of event block */
int end_event(void){
#ifdef TIMESTAMPEF
  return end_eventts();
#else
  int thd;

  // make header 1
  thd = RIDF_MKHD1(RIDF_LY1, RIDF_EVENT, eventsize);
  // copy header 1
  memcpy((char *)(data+eventhmp), (char *)&thd, SIZEOFINT);
  thd = EFN;
  // copy header 2
  memcpy((char *)(data+eventhmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);

  eventsize = 0;

  return mp;
#endif
}

/** End of event block with Time Stamp*/
int end_eventts(void){
  int thd;

  // make header 1
  thd = RIDF_MKHD1(RIDF_LY1, RIDF_EVENT_TS, eventsize);
  // copy header 1
  memcpy((char *)(data+eventhmp), (char *)&thd, SIZEOFINT);
  thd = EFN;
  // copy header 2
  memcpy((char *)(data+eventhmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);
  // copy timestamp
  memcpy((char *)(data+eventhmp)+SIZEOFINT*3, (char *)&timestamp, SIZEOFTS);

  eventsize = 0;

  return mp;
}   

/** End of segment block */
int end_segment(void){
  int thd;

  // make header 1
  thd = RIDF_MKHD1(RIDF_LY2, RIDF_SEGMENT, segmentsize);
  // copy header 1
  memcpy((char *)(data+segmenthmp), (char *)&thd, SIZEOFINT);
  // make header 2
  thd = EFN;
  // copy header 2
  memcpy((char *)(data+segmenthmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);
  
  eventsize += segmentsize;                /* Count up event size */

  return mp;
}


void init_scaler(int scrid){
  struct timeval t;
  int tim;

  scrhmp = mp;                        // Store mp for event header
  scrsize = RIDF_HDSCR_SZ;            // Initialize event size
  mp += RIDF_HDSCR_SZ;

  do_gettimeofday(&t);
  tim = (int)t.tv_sec;

  // copy date
  memcpy((char *)(data+scrhmp)+SIZEOFINT*2, (char *)&tim, SIZEOFINT);
  // copy scrid
  memcpy((char *)(data+scrhmp)+SIZEOFINT*3, (char *)&scrid, SIZEOFINT);

  if(eventsize > 0) eventsize += scrsize;
}

void init_ncscaler(int scrid){
  init_scaler(scrid);
}

/** End of scaler (clear type) */
int end_scaler(void){
  int thd;

  // make header 1
  if(eventsize > 0){
    // Layer = 2 (Scaler in event)
    thd = RIDF_MKHD1(RIDF_LY2, RIDF_CSCALER, scrsize);
  }else{
    // Layer = 1
    thd = RIDF_MKHD1(RIDF_LY1, RIDF_CSCALER, scrsize);
  }
  // copy header 1
  memcpy((char *)(data+scrhmp), (char *)&thd, SIZEOFINT);
  thd = EFN;
  // copy header 2
  memcpy((char *)(data+scrhmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);

  scrsize = 0;

  return mp;
}

/** End of non clear scaler */
int end_ncscaler(void){
  int thd;

  // make header 1
  if(eventsize > 0){
    // Layer = 2 (Scaler in event)
    thd = RIDF_MKHD1(RIDF_LY2, RIDF_SCALER, scrsize);
  }else{
    // Layer = 1
    thd = RIDF_MKHD1(RIDF_LY1, RIDF_SCALER, scrsize);
  }
  // copy header 1
  memcpy((char *)(data+scrhmp), (char *)&thd, SIZEOFINT);

  thd = EFN;
  // copy header 2
  memcpy((char *)(data+scrhmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);

  scrsize = 0;

  return mp;
}

/** End of non clear scaler */
int end_ncscaler32(void){
  volatile int thd;

  // make header 1
  if(eventsize > 0){
    // Layer = 2 (Scaler in event)
    thd = RIDF_MKHD1(RIDF_LY2, RIDF_NCSCALER32, scrsize);
  }else{
    // Layer = 1
    thd = RIDF_MKHD1(RIDF_LY1, RIDF_NCSCALER32, scrsize);
  }

  // copy header 1
  memcpy((char *)(data+scrhmp), (char *)&thd, SIZEOFINT);

  thd = EFN;
  // copy header 2
  memcpy((char *)(data+scrhmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);

  scrsize = 0;

  return mp;
}

void delay_us(void){
  outb(1,0x80);
}
