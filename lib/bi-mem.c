/* bi-mem.c
 * last modified : 17/01/06 09:43:44 
 *
 * List like event memory management
 * 
 * Hidetada Baba (RIKEN)
 * baba@rarfaxp.riken.jp
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>

struct evtmem *first = NULL;    ///< Pointer for first evtmem
struct evtmem *last  = NULL;    ///< Pointer for last evtmem
struct nevtmem *nfirst = NULL;  ///< Pointer for first nevtmem
struct tsmem  *tsfirst = NULL;  ///< Pointer for first time stamp
static int memn = 0;

/** Store event data to event memory
 *  @param *data pointer of source data
 *  @param size size of source data
 *  @param efn EF number
 *  @evtn event number
 *  @return Pointer for event memory
 */
struct evtmem *newevt(char *data, int size, int efn, unsigned int evtn, int *flag){ 
  struct evtmem *new = NULL, *p;
  int i, exflag;
  unsigned int tevtn;

  *flag = 0;

  if(memn > MAXEVTMEM){
    //printf("bi-mem: Fatal!! memn > MAXEVEMEM, %d\n", memn);
    *flag = MEM_OVER_EVTMEM;
    return NULL;
  }

  p = first;
  exflag = 0;

  if(!first){
    if(!(new = (struct evtmem *)malloc(sizeof(struct evtmem)))){
      *flag = MEM_NMALLOC_EVTMEM_FST;
      return NULL;
    }
    new->next = NULL;
    first = new;
    last  = new;
    for(i=0; i<MAXEF; i++){
      new->evtdata[i] = NULL;
    }
    if(!(new->evtdata[efn] = malloc(size))){
      *flag = MEM_NMALLOC_EVTDATA_FST;
      return NULL;
    }
    new->size[efn] = size;
    memcpy(new->evtdata[efn], data, size);
    new->evtn = evtn;
    p = new;
    memn = 1;
  }else{
    if(evtn <= last->evtn || first->evtn >= last->evtn){
      while(p){
	if(evtn == p->evtn){
	  if(!p->evtdata[efn]){
	    // If evtdata is NULL, to avoid duplication of evtn
	    if(!(p->evtdata[efn] = malloc(size))){
	      printf("Can't malooc efn=%d, evtn=%d size=%d\n", efn, evtn, size);
	      *flag = MEM_NMALLOC_EVTDATA_ADD;
	      return NULL;
	    }
	    p->size[efn] = size;
	    memcpy(p->evtdata[efn], data, size);
	  }
	  exflag = 1;
	  break;
	}
	p = p->next;
      }
    }

    if(!exflag){
      if(last->evtn != evtn - 1){
	tevtn = last->evtn;
	*flag = MEM_NMALLOC_EVTN_JUMP;

	// for new evtn > last->evtn + 1
	while(tevtn < evtn){
	  if(!(new = (struct evtmem *)malloc(sizeof(struct evtmem)))){
	    *flag = MEM_NMALLOC_EVTMEM_NEV;
	    return NULL;
	  }
	  new->next = NULL;
	  for(i=0; i<MAXEF; i++){
	    new->evtdata[i] = NULL;
	  }
	  new->evtn = tevtn;
	  last->next = new;
	  last = new;
	  memn++;
	  tevtn++;
	}
      }

      if(first->evtn < evtn){
	if(!(new = (struct evtmem *)malloc(sizeof(struct evtmem)))){
	  *flag = MEM_NMALLOC_EVTMEM_NEV;
	  return NULL;
	}
	new->next = NULL;
	for(i=0; i<MAXEF; i++){
	  new->evtdata[i] = NULL;
	}
	if(!(new->evtdata[efn] = malloc(size))){
	  *flag = MEM_NMALLOC_EVTDATA_NEV;
	  return NULL;
	}
	new->size[efn] = size;
	memcpy(new->evtdata[efn], data, size);
	new->evtn = evtn;
	last->next = new;
	last = new;
	p = new;
	memn++;
      }else{
	/* For first->evtn > evtn */
	*flag = MEM_NMALLOC_EVTN_JUMP;
	return NULL;
      }
    }
  }

  return p;
}

/*! Check existence of event data */
int chkefdata(struct steflist *eflist){
  int i, ret = 1;
  
  if(!first) return 0;

  for(i=0; i<MAXEF; i++){
    if(eflist[i].ex && eflist[i].of ==  EB_EFLIST_ON){
      if(!first->evtdata[i]){
	DB(printf("chkevtdata: without evtdata %d\n", i));
	ret = 0;
      }else{
	//DB(printf("chkevtdata: efn=%d evtn = %d\n", i, first->evtn));
      }
    }
  }

  return ret;
}

/*! Check total size of event data
 *  If at least one event data not exist, return 0
 */
int chkevtsize(struct steflist *eflist){
  int i, ret = 0;

  if(!first) return 0;

  for(i=0; i<MAXEF; i++){
    if(eflist[i].ex && eflist[i].of ==  EB_EFLIST_ON){
      if(!first->evtdata[i]){
	return 0;
      }else{
	ret += first->size[i];
      }
    }
  }

  return ret;
}

/*! Do event-build of first event and copy event builded data to rdata.
 *  Before do event-build, check event flagment data by chkefdata()
 */
int mkef(char *rdata, struct steflist *eflist, int *evtn){
  int i, idx = 0;

  if(!first){
    DB(printf("mkef: without first\n"));
    return 0;
  }
  if(!chkefdata(eflist)){
    DB(printf("mkef: without chkefdata\n"));
    return 0;
  }

  for(i=0; i<MAXEF; i++){
    if(eflist[i].ex && eflist[i].of ==  EB_EFLIST_ON){
      memcpy(rdata+idx, first->evtdata[i], first->size[i]);
      idx += first->size[i];
      *evtn = first->evtn;
    }
  }

  return idx;
}


/*! Do event-build of first event and copy event builded data to rdata.
 *  Before do event-build, check event flagment data by chkefdata()
 *  Time stamp version.
 */
int mkefts(char *rdata, struct steflist *eflist, int *evtn,
	   long long int *ts){
  int i, idx = 0;

  if(!first){
    DB(printf("mkef: without first\n"));
    return 0;
  }
  if(!chkefdata(eflist)){
    DB(printf("mkef: without chkefdata\n"));
    return 0;
  }

  *evtn = first->evtn;
  *ts = chkts(*evtn, -10);

  if(*ts != -1){
    rdata += sizeof(long long int);
  }

  for(i=0; i<MAXEF; i++){
    if(eflist[i].ex && eflist[i].of ==  EB_EFLIST_ON){
      memcpy(rdata+idx, first->evtdata[i], first->size[i]);
      idx += first->size[i];
    }
  }

  return idx;
}


  

/*! Clean first event memory */
int cleanfirst(struct steflist *eflist){
  struct evtmem *p = NULL;
  int i;

  if(first){
    p = first->next;
    for(i=0; i<MAXEF; i++){
      //if(eflist[i].ex && eflist[i].of ==  EB_EFLIST_ON){
      if(eflist[i].ex){
	free(first->evtdata[i]);
	first->evtdata[i] = NULL;
      }
    }

    free(first);

    first = p;
    memn--;
    if(!first){
      last = NULL;
    }
  }else{
    return 0;
  }

  return memn;
}

/*! Clean all event memory */
int cleanall(struct steflist *eflist){
  struct evtmem *p;
  int i;

  while(first){
    p = first->next;
    for(i=0; i<MAXEF; i++){
      //if(eflist[i].ex && eflist[i].of ==  EB_EFLIST_ON){
      if(eflist[i].ex){
	free(first->evtdata[i]);
	first->evtdata[i] = NULL;
      }
    }
    
    free(first);
    first = p;
  }

  memn = 0;
  last = NULL;

  return 1;
}

int showevtlist(void){
  int i;
  struct evtmem *p;

  if(!first) return 0;

  p = first;

  while(p){
    for(i=0;i<MAXEF;i++){
      printf("EFN%d %s (%d)\n", i, p->evtdata[i], p->size[i]);
    }
    p = p->next;
  }

  return 1;
}


int getmemn(void){
  return memn;
}

/** Get first event number
 *  @return First event number
 */
unsigned int getfirstevtn(void){
  if(first){
    return first->evtn;
  }else{
    return 0;
  }
}

/** Get last event number
 *  @return Last event number
 */
unsigned int getlastevtn(void){
  if(last){
    return last->evtn;
  }else{
    return 0;
  }
}


/* nevtmem series */

/** Store nevtdata to nevt memory
 *  @param *data pointer of source data
 *  @param size size of source data
 *  @return Pointer for event memory
 */
struct nevtmem *newnevt(char *data, int size){
  struct nevtmem *new = NULL, *t = NULL;

  if(!(new = (struct nevtmem *)malloc(sizeof(struct nevtmem)))){
    printf("bi-mem: nevtmem can't make new nevtmem\n");
    return NULL;
  }
  new->next = NULL;
  new->nevtdata = NULL;
  if(!(new->nevtdata = malloc(size))){
    printf("bi-mem: nevtmem can't make new nevtdata\n");
    free(new);
    new = NULL;
    return NULL;
  }
  new->size = size;
  memcpy(new->nevtdata, data, size);

  if(!nfirst){
    nfirst = new;
  }else{
    t = nfirst;
    while(t->next){
      t = t->next;
    }
    t->next = new;
  }

  return new;
}

/** Clean first nevt memory
 */
int ncleanfirst(void){
  struct nevtmem *p = NULL;

  if(nfirst){
    p = nfirst->next;
    free(nfirst->nevtdata);
    nfirst->nevtdata = NULL;
    free(nfirst);
    nfirst = p;
  }else{
    return 0;
  }

  return 1;
}

/** Clean all nevt memory */
int ncleanall(void){

  while(ncleanfirst()){}

  return 1;
}

/** Store nevt memory
 *  @param buff Pointer of buffer which will be stored nevt data
 *  @return Size of nevtdata (0 = non)
 */
int storenevt(char *buff){
  if(!nfirst){
    return 0;
  }else{
    memcpy(buff, nfirst->nevtdata, nfirst->size);
  }
  return nfirst->size;
}


/** Return first event number
 *  @return first event number
 */
unsigned int getebn(void){
  unsigned int ret;

  if(first){
    ret = first->evtn;
  }else{
    ret = 0;
  }

  return ret;
}


/** Allocate new time stamp 
 *  @param evtn event number
 *  @param ts time stamp
 *  @return pointer for ts memory
 */
struct tsmem *newts(unsigned int evtn, long long int ts, int efn){
  struct tsmem *new = NULL, *t = NULL;

  if(!(new = (struct tsmem *)malloc(sizeof(struct tsmem)))){
    printf("bi-mem: newts can't make new tsmem\n");
    return NULL;
  }
  new->next = NULL;
  memcpy((char *)&new->ts, (char *)&ts, sizeof(ts));
  memcpy((char *)&new->evtn, (char *)&evtn, sizeof(evtn));
  memcpy((char *)&new->efn, (char *)&efn, sizeof(efn));

  DB(printf("%u / %lld : %u / %lld\n", evtn, ts, new->evtn, new->ts));

  if(!tsfirst){
    tsfirst = new;
  }else{
    t = tsfirst;
    while(t->next){
      t = t->next;
    }
    t->next = new;
  }

  return new;
}

/** Update ts value
 *  @param evtn event number
 *  @param ts time stamp
 *  @return pointer for ts memory
 */
struct tsmem *updatets(unsigned int evtn, long long int ts, int efn){
  struct tsmem  *p = NULL;

  p = tsfirst;

  while(p){
    if(evtn == p->evtn){
      memcpy((char *)&p->ts, (char *)&ts, sizeof(ts));
      memcpy((char *)&p->efn, (char *)&efn, sizeof(efn));
      return p;
    }else{
      p = p->next;
    }
  }

  return NULL;
}


/** Return stored time stamp
 *  @param evtn event number
 *  @return time stamp (-1 = no time stamp)
 */
long long int chkts(unsigned int evtn, int efn){
  struct tsmem  *p = NULL;

  p = tsfirst;

  //DB(printf("chkts %u : %u / %lld\n", evtn, tsfirst->evtn, tsfirst->ts));
  while(p){
    if(evtn == p->evtn){
      if(efn >= p->efn || efn == -10){
	return p->ts;
      }else{
	return -2;
      }
    }else{
      p = p->next;
    }
  }

  return -1;
}

/** Clean first ts memory
 */
int tscleanfirst(void){
  struct tsmem *p = NULL;

  if(tsfirst){
    p = tsfirst->next;
    free(tsfirst);
    tsfirst = p;
  }else{
    return 0;
  }

  return 1;
}

/** Clean all nevt memory */
int tscleanall(void){

  while(tscleanfirst()){}

  return 1;
}


/** Add data to Ring Buffer
 *
 */
int addrbuff(struct stbrstat *st, char *data, int sz){
  char *td;
  struct stbrbuff *trb, *p;

  DB(printf("addrbuff: st->n=%d, st->max=%d, st->blkn=%d\n", st->n,st->max,st->blkn));

  // copy data
  td = malloc(sz);
  memcpy(td, data, sz);

  // for new rbuff
  trb = malloc(sizeof(struct stbrbuff));
  trb->data = td;
  trb->blkn = st->blkn + 1;
  trb->next = NULL;
  trb->size = sz;
  DB(printf("addrbuff: newrbuff blkn=%d\n", trb->blkn));

  // for first rbuff
  if(!st->first){
    st->n = 1;
    st->blkn = 1;
    st->first = trb;
    st->last = trb;
    DB(printf("addrbuff: first rbuff\n"));
    return 1;
  }

  st->last->next = trb;
  st->last = trb;

  // delete exeed rbuff
  if(st->n == st->max){
    DB(printf("addrbuff: delete first blkn=%d\n", st->first->blkn));
    p = st->first->next;
    free(st->first->data);
    free(st->first);
    st->first = p;
  }else{
    DB(printf("addrbuff: increased n n=%d\n", st->n));
    st->n++;
  }

  st->blkn++;

  return 1;
}

/* Get data for blkn
 * Copy data, return 1 = copied, 0 = not copied
 */
int getrbuff(struct stbrstat *st, char *data, int *sz, unsigned int blkn){
  struct stbrbuff *p;

  p = st->first;
  if(!p)
    return 0;
  while(blkn > p->blkn){
    p = p->next;
    if(!p){
      return 0;
    }
  }

  memcpy(data, p->data, p->size);
  *sz = p->size;

  return 1;
}
