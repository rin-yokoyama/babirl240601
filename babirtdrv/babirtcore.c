/*
  bbrl.c
  Nov 10, 2003 for babarl
  
  mbuff version

*/
#include <string.h>
#include <mbuff.h>

void init_driver(void);
void end_driver(void);
void init_block(void);
void init_event(short fid);
void init_segment(short segid);
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
void delay_us(void);

static unsigned short *data,*cdata;
static int mp,eventid,eventhmp,eventsize,segmenthmp,segmentsize;
int i;
volatile char *shmptr;
static int shmsize = BLKSIZE*2+8;

void init_driver(void){
  if((shmptr=(volatile char*)mbuff_alloc(BBRL_SHM,shmsize))==NULL){
    printk("Can't make shared memory (mbuff).\n");
  }
  data = (unsigned short *)shmptr;
  cdata = (unsigned short *)(shmptr+BLKSIZE*2);
  memset(data,0,shmsize);                   /* Memory set to '0' */
}

void end_driver(void){
  cdata[0] = 0xeeee;
  mbuff_free(BBRL_SHM,(void*)shmptr);
}

void init_block(void){
  memset(data,0,4);                       /* Memory set to '0' */
  mp = 4;
  eventid = 0;
}

void init_pblock(void){
  data[0] = 0x0010;
  memset(data+1,0,3);                       /* Memory set to '0' */
  mp = 4;
  eventid = 0;
}

void init_event(short fid){
  eventid++;                               /* Count up event ID */
  eventhmp = mp;                           /* Store mp for event size */
  eventsize = 3;                           /* Initialize event size */
  mp++;
  data[mp++] = fid;                        /* Write FID */
  data[mp++] = eventid;                    /* Write event ID */
}

void init_segment(short segid){
  segmenthmp = mp;                         /* Store mp for segment size */
  segmentsize = 2;                         /* Initialize segment size */
  mp++;
  data[mp++] = segid;                      /* Write segment ID */
}

int end_block(void){
  data[mp++] = 0xffff;                     /* Write END */
  data[mp++] = 0xffff;                     /* Write END */

  return mp;
}

int end_event(void){
  data[eventhmp] = eventsize | 0x8000;     /* Write event size */
  return mp;
}

int end_segment(void){
  data[segmenthmp] = segmentsize;          /* Write segment size */
  eventsize += segmentsize;                /* Count up event size */

  return mp;
}

void init_shmem(void){
  char put = 0x01;
  cdata[0] = 0x0101;
  rtf_put(3,&put,1);
}

void dinit_shmem(int sh){
  if(sh == 0){
    cdata[0] = 0x0101;
  }else{
    cdata[1] = 0x0202;
  }
}

void change_shmem(int sh){
  if(sh == 0 ){
    data = (unsigned short *)shmptr;
    /* memset(data,0,0x4000); */
  }else{
    data = (unsigned short *)(shmptr+BLKSIZE);
    /* memset(data,0,0x4000); */
  }
}

void clear_shmem(void){
  cdata[0] = 0x0000;
}

void dclear_shmem(int sh){
  if(sh == 0){
    cdata[0] = 0x0000;
  }else{
    cdata[1] = 0x0000;
  }
}

short dget_flag(int sh){
  return cdata[sh];
}

void put_smem(void){
  cdata[0] = 0x0001;
}

int get_mp(void){
  return mp;
}

void delay_us(void){
  outb(1,0x80);
}

