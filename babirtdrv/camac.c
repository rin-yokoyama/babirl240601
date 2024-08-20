#ifdef CAMAC

#ifdef CC7700
#define CNAFGEN(c,n,a,f) (0x00073fff & (c<<16 | (n-1)<<9 | a <<5 | f))
#define NAFGEN(n,a,f) ( 0x3fff & ((n-1)<<9 | a <<5 | f) )
#endif

#ifdef K2915
#define CNAFGEN(c,n,a,f) (0x00073fff & (c<<16 | n<<9 | a <<5 | f))
#define NAFGEN(n,a,f) ( 0x3fff & (n<<9 | a <<5 | f) )
#endif

#define QSTOP            0x00000002  /* Q-Stop   Block Transfer Mode */
#define QIGNORE          0x00000004  /* Q-Ignore Block Transfer Mode */
#define QREPEAT          0x00000006  /* Q-Repeat Block Transfer Mode */
#define QSCAN            0x00000008  /* Q-Scan   Block Transfer Mode */


int read_segdata(short c,short n,short a,short f);
int read_segndata(short len,short c,short n,short a,short f);
int read_segsdata(short len,short c,short n,short a,short f);
int read_segbdata(short len,short c,short n,short a,short f);
int read_segmod(short len,short segid,short c,short n,short a,short f);
int read_segsmod(short len,short segid,short c,short n,short a,short f);
int read_segbmod(short len,short segid,short c,short n,short a,short f);
int read_segmemmod(short segid,short c,short n);
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

int read_segmod(short len,short segid,short c,short n,short a,short f){
  data[mp++] = len+2;                      /* Write word count */
  data[mp++] = segid;                      /* Write segment ID */
  for(i=0;i<len;i++){
    read16(CNAFGEN(c,n,a,f),data+mp);      /* Read data */
    mp++;
    a++;
  }

  eventsize += len + 2;                    /* Count up event size */

  return len + 2;
}

int read_segsmod(short len,short segid,short c,short n,short a,short f){
  segmenthmp = mp;
  mp++;
  data[mp++] = segid;                      /* Write segment ID */
  block_read16(QSCAN,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  data[segmenthmp] = len + 2;              /* Write word count */
  eventsize += len + 2;                    /* Count up event size */

  return len + 2;
}

int read_segbmod(short len,short segid,short c,short n,short a,short f){
  segmenthmp = mp;
  mp++;
  data[mp++] = segid;                      /* Write segment ID */
  block_read16(QIGNORE,CNAFGEN(c,n,a,f),data+mp,len); /* Read data */
  mp += len;
  data[segmenthmp] = len + 2;              /* Write word count */
  eventsize += len + 2;                    /* Count up event size */

  return len + 2;
}


int read_segmemmod(short segid,short c,short n){
  short wdata,wordcnt;

  wdata = 0x0001;
  write16(CNAFGEN(c,n,1,17),&wdata);       /* Change to CAMAC-mode */
  read16(CNAFGEN(c,n,0,1),&wordcnt);       /* Read word count */
  wdata = 0x0000;
  write16(CNAFGEN(c,n,0,17),&wdata);       /* Reset read buffer pointer */
  data[mp++] = wordcnt+3;                  /* Write word count */
  data[mp++] = segid;                      /* Write segment ID */
  data[mp++] = wordcnt;                    /* Write FERA word count */
  if(wordcnt != 0){
    block_read16(QIGNORE,CNAFGEN(c,n,0,0),data+mp,wordcnt); /* Read */
  }
  mp += wordcnt;
  wdata = 0x0003;
  write16(CNAFGEN(c,n,1,17),&wdata);       /* Change to ECL-mode */

  eventsize += wordcnt + 3;

  return wordcnt + 3;
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
#endif
