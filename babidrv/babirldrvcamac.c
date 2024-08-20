/* babirl : babirldrv/babirldrvcamac.c
 * last modified : 08/07/18 13:59:41 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * CAMAC functions for driver
 *
 */

int read_segdata(short c,short n,short a,short f);
int read_segdata24(short c,short n,short a,short f);
int read_segndata(short len,short c,short n,short a,short f);
int read_segsdata(short len,short c,short n,short a,short f);
int read_segbdata(short len,short c,short n,short a,short f);
int read_segmod(short len,short segid,short c,short n,short a,short f);
int read_segsmod(short len,short segid,short c,short n,short a,short f);
int read_segbmod(short len,short segid,short c,short n,short a,short f);
int read_segmemmod(short segid,short c,short n);
int read_scaler(short pos,short len,short c,short n);
int read_sscaler(short pos,short len,short c,short n);
int read_pedestal(short type,short c,short n,short vsn);
void write_mod(short c,short n,short a,short f,short *data);
void write_data(short c,short n,short a,short f,short data);
void control_mod(short c,short n,short a,short f);

int read_segdata(short c,short n,short a,short f){
  read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
  mp++;
  segmentsize++;                           /* Count up segment size */

  return segmentsize;
}

int read_segdata24(short c,short n,short a,short f){
  read24(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
  mp+=2;
  segmentsize+=2;                           /* Count up segment size */

  return segmentsize;
}

int read_segndata(short len,short c,short n,short a,short f){
  int i;
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);        /* Read data */
    a++;
    mp++;
    segmentsize++;                           /* Count up segment size */
  }

  return segmentsize;
}

int read_segsdata(short len,short c,short n,short a,short f){
  int i;

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
