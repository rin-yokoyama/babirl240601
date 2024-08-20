/* babirl : babirldrv/babirldrivridfrt.c
 * last modified : 08/03/22 16:49:12 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * RIDF format functions for rt driver
 *
 */

#include <string.h>
//#include <mbuff.h>

//#define DEBUG

#ifdef DEBUG
#define DB(x) x
#else
#define DB(x) 
#endif

void init_driver(void);
void end_driver(void);
void init_block(void);
void init_event(void);
void init_segment(int segid);
int end_block(void);
int end_event(void);
int end_segment(void);
void init_shmem(void);
void dinit_shmem(int sh);
void change_shmem(int sh);
void dclear_shmem(int sh);
void clear_shmem(void);
void put_data(char flag);
void put_smem(void);
int get_mp(void);
void print_shmem(int lines);

int get_evtn(void);

#ifdef VME
static unsigned int dmadata[DMAMAX];
#endif

static int eventhmp,eventsize,segmenthmp,segmentsize;
static volatile int mp;
static unsigned int evtn;
//int thd;


static unsigned short *data;
static int eventhmp,eventsize,segmenthmp,segmentsize;
static int scrhmp, scrsize;
int i;
char *shmptr;

char *flag1, *flag2, *flag3, *flags;

void init_driver(void){
  if((shmptr = ioremap(RTSHMEM, MAPSIZE))==NULL){
    printk("babirtdrv: Can't make shared memory.\n");
  }

  DB(printk("babirtdrv: shmptr = %p\n", shmptr));

  data = (unsigned short *)shmptr;
  flag1 = (char *)(shmptr + EF_SHM_FLAG1);
  flag2 = (char *)(shmptr + EF_SHM_FLAG2);
  flag3 = (char *)(shmptr + EF_SHM_FLAG3);
  flags = (char *)(shmptr + EF_SHM_SSF);
  memset(data, 0, MAPSIZE);    /* Memory set to '0' */
}

void ssm_start(void){
  flags[0] = STAT_RUN_START;
}

void ssm_stop(void){
  flags[0] = STAT_RUN_IDLE;
}

void end_driver(void){
  ssm_stop();
  iounmap(shmptr);
}

void dinit_shmem(int sh){
  if(sh == 0){
    flag1[0] = 0x01;
  }else if(sh == 1){
    flag2[0] = 0x02;
  }else if(sh == 3){
    flag3[0] = 0x03;
  }
}

void change_shmem(int sh){
  if(sh == 0 ){
    data = (unsigned short *)(shmptr + EF_SHM_DATA1);
    /* memset(data,0,0x4000); */
  }else if(sh == 1){
    data = (unsigned short *)(shmptr + EF_SHM_DATA2);
    /* memset(data,0,0x4000); */
  }else if(sh == 2){
    data = (unsigned short *)(shmptr + EF_SHM_DATA3);
  }
}

void clear_shmem(void){
  flag1[0] = 0x00;
  flag2[0] = 0x00;
  flag3[0] = 0x00;
}

void dclear_shmem(int sh){
  if(sh == 0){
    flag1[0] = 0x00;
  }else if(sh == 1){
    flag2[0] = 0x00;
  }else if(sh == 2){
    flag3[0] = 0x00;
  }
}

char dget_flag(int sh){
  if(sh == 0){
    return flag1[0];
  }else if(sh == 1){
    return flag2[0];
  }else{
    return flag3[0];
  }
}

int get_mp(void){
  return mp;
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

int store_evtn(void){
  memcpy((char *)(shmptr + EF_SHM_EVTN), (char *)&evtn, sizeof(evtn));
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
  int thd;

  thd = RIDF_MKHD1(RIDF_LY0, RIDF_EF_BLOCK, mp);  // make header 1
  memcpy((char *)data, (char *)&thd, SIZEOFINT);  // copy header 1 to buff
  thd = EFN;                                      // make header 2
  memcpy((char *)data+SIZEOFINT, (char *)&thd, SIZEOFINT); // copy 

  return mp;
}

/** End of event block */
int end_event(void){
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


/** Initialize event block
 */
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

