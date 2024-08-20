/* babirl : babirldrv/babirldrvridf.c
 * last modified : 07/03/07 14:54:01 
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
int end_segment(void);
int get_evtn(void);

#ifdef VME
static unsigned int dmadata[DMAMAX];
#endif
static unsigned short data[EB_EFBLOCK_BUFFSIZE];
static int eventhmp,eventsize,segmenthmp,segmentsize;
static volatile int mp;
static unsigned int evtn;
int buffsize = EB_EFBLOCK_BUFFSIZE * WORDSIZE;
int thd;

/** Initialize data buffer memory
 */
void init_mem(void){
  memset(data, 0, buffsize);
}

void end_driver(void){
}

/** Initialize data block
 */
void init_block(void){
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
  thd = RIDF_MKHD1(RIDF_LY0, RIDF_EF_BLOCK, mp);  // make header 1
  memcpy((char *)data, (char *)&thd, SIZEOFINT);  // copy header 1 to buff
  thd = EFN;                                      // make header 2
  memcpy((char *)data+SIZEOFINT, (char *)&thd, SIZEOFINT); // copy 

  return mp;
}

/** End of event block */
int end_event(void){
  // make header 1
  thd = RIDF_MKHD1(RIDF_LY1, RIDF_EVENT, eventsize);
  // copy header 1
  memcpy((char *)(data+eventhmp), (char *)&thd, SIZEOFINT);
  thd = EFN;
  // copy header 2
  memcpy((char *)(data+eventhmp)+SIZEOFINT, (char *)&thd, SIZEOFINT);

  return mp;
}

/** End of segment block */
int end_segment(void){
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

