/*
  bbdaq.c
  Feb 24, 2002

*/

void init_mem(void);
void init_block(void);
void init_segment(void);
int end_block(void);
int end_event(void);
int end_segment(void);
int read_segdata(short c,short n,short a,short f);
int read_segndata(short len,short c,short n,short a,short f);
int read_segsdata(short len,short c,short n,short a,short f);
int read_segbdata(short len,short c,short n,short a,short f);
void write_mod(short c,short n,short a,short f,short *data);
void write_data(short c,short n,short a,short f,short data);
void control_mod(short c,short n,short a,short f);
int get_mp(void);

static unsigned short data[0x2000];
static int mp,blkid,eventsize,segmenthmp,segmentsize;
int i;


void init_mem(void){
  memset(data,0,0x4000);                   /* Memory set to '0' */
  blkid = 0;
}

void end_driver(void){
}

void init_block(void){
  eventsize = 0;

  data[0] = 0;
  data[1] = blkid;
  mp = 3;
  blkid++;
}

void init_segment(void){
  data[mp++] = 0xcc77;
  segmenthmp = mp;                         /* Store mp for segment size */
  segmentsize = 0;                         /* Initialize segment size */
  mp++;
}

int end_block(void){
  data[mp++] = 0xffff;                     /* Write END */
  data[2] = eventsize + 1;

  return mp;
}

int end_segment(void){
  data[segmenthmp] = segmentsize;          /* Write segment size */
  eventsize += segmentsize+2;                /* Count up event size */

  return mp;
}

int end_event(void){
  return mp;
}

int read_segdata(short c,short n,short a,short f){
  read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
  mp++;
  segmentsize++;                           /* Count up segment size */

  return segmentsize;
}

int read_segndata(short len,short c,short n,short a,short f){
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
    a++;
    mp++;
    segmentsize++;                           /* Count up segment size */
  }

  return segmentsize;
}

int read_segsdata(short len,short c,short n,short a,short f){
#ifdef K2915
  block_read16(QSCAN,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  segmentsize += len;
#else
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
    a++;
    mp++;
    segmentsize++;                           /* Count up segment size */
  }
#endif

  return segmentsize;
}

int read_segbdata(short len,short c,short n,short a,short f){
  block_read16(QIGNORE,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  segmentsize += len;

  return segmentsize;
}

void write_mod(short c,short n,short a,short f,short *data){
  write16(CNAFGEN(c,n,a,f),data);
}

void write_data(short c,short n,short a,short f,short data){
  write16(CNAFGEN(c,n,a,f),&data);
}

void control_mod(short c,short n,short a,short f){
  control(CNAFGEN(c,n,a,f));
}

int get_mp(void){
  return mp;
}

