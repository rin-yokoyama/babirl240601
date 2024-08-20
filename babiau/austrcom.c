/* babirl/babild/ebstrcom.c
 * last modified : 09/07/31 15:11:08 
 *
 * String configuration setter
 *
 * Hidetada Baba
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bi-config.h>
#include <bi-common.h>

#include "austrcom.h"

extern struct stdaqinfo daqinfo;
extern struct struninfo runinfo;
extern struct stssminfo ssminfo;
/* extern struct stclinfo clinfo[MAXCLI]; */
extern unsigned long long int totsize;
extern unsigned int gloevtn;

char textbuf[32768];
char parsbuf[1024];

/*
  set\ndaqinfo\nrunname=hogehoge:ebsize=10000 ....
  get=daqinfo
   -> get=daqinfo:runname=hogehoge:ebsize=10000 ...
  get=eflist
   -> get=eflist:idx=1:ex=1:of=1:name=hogehoge:host=hogehoge:idx=10:ex=....
  set=runinfo:header=hogehoge
  set=runinfo:ender=hogehoge
  set=eflist:id=1:of=1:name=test:host=hoge:id=2:of=0
*/

//* If command includs 'set' return 1, else return 0 //
int chksetcom(char *com){
  int pt, len, ret, sz, endpt;
  char *tcom;
  char func[1024];

  pt = 0;
  ret = 0;
  len = strlen(com);

  while(pt < len){
    if((tcom = index(com+pt, '\n')) == NULL){
      printf("Error command\n");
      break;
    }else{
      sz = tcom - com - (unsigned int)pt;
      memcpy(func, com+pt, sz);
      func[sz] = '\0';
      
      pt += sz + 1;
      if((tcom = index(com+pt, '\n')) == NULL){
	endpt = len;
      }else{
	endpt = (int)(tcom - com);
      }

      sz = endpt - pt;

      pt += sz + 1;

      if(!strcmp(func, "set")){
	ret = 1;
	break;
      }
    }
  }

  return ret;
}

char *parsetextcom(char *com, int *asccom){
  int idx, len, pt, endpt, sz, set;
  int id;
  char *tcom;
  char func[1024], val[1024];

  id = 0;
  pt = 0;
  len = strlen(com);
  idx = 0;
  set = NONE;
  *asccom = 0;

  while(pt < len){
    if((tcom = index(com+pt, '\n')) == NULL){
      printf("Error command\n");
      break;
    }else{
      sz = tcom - com - pt;
      memcpy(func, com+pt, sz);
      func[sz] = '\0';
      
      pt += sz + 1;
      if((tcom = index(com+pt, '\n')) == NULL){
	endpt = len;
      }else{
	endpt = (int)(tcom - com);
      }

      sz = endpt - pt;
      memcpy(val, com+pt, sz);
      val[sz] = '\0';

      pt += sz + 1;

      DB(printf("%s = %s\n", func, val));

      if(!strcmp(func, "get")){
	if(!strcmp(val, "daqinfo")){
	  idx += getdaqinfo(&daqinfo, textbuf+idx);
	}
	if(!strcmp(val, "runinfo")){
	  idx += getruninfo(&runinfo, textbuf+idx);
	}
	if(!strcmp(val, "eflist")){
	  idx += geteflist(&daqinfo, textbuf+idx);
	}
	if(!strcmp(val, "ebinfo")){
	  idx += getebinfo(textbuf+idx);
	}
      }else if(!strcmp(func, "set")){
	if(!strcmp(val, "daqinfo")){
	  set = DAQINFON;
	}
	if(!strcmp(val, "runinfo")){
	  set = RUNINFON;
	} 
	if(!strcmp(val, "eflist")){
	  set = EFLISTN;
	}
	if(!strcmp(val, "hdlist")){
	  set = HDLISTN;
	}
	if(!strcmp(val, "ssminfo")){
	  set = SSMINFON;
	}
	/*
	  if(!strcmp(val, "clinfo")){
	  set = CLINFON;
	  }
	*/
	if(!strcmp(val, "mtlist")){
	  set = MTLISTN;
	}
      }else if(!strcmp(func, "id")){
	id = strtol(val, NULL, 0);
      }else if(!strcmp(func, "A")){
	set = NONE;
      }else if(!strcmp(func, "daq")){
	if(!strcmp(val, "start")){
	  *asccom |= ASC_EB_RUN_START;
	}
	if(!strcmp(val, "nssta")){
	  *asccom |= ASC_EB_RUN_NSSTA;
	}
	if(!strcmp(val, "stop")){
	  *asccom |= ASC_EB_RUN_STOP;
	}
	if(!strcmp(val, "end")){
	  *asccom |= ASC_EB_RUN_CLOSE;
	}
      }else{
	switch (set){
	case DAQINFON:
	  *asccom |= setdaqinfo(&daqinfo, func, val);
	  break;
	case RUNINFON:
	  setruninfo(&runinfo, func, val);
	  break;
	case EFLISTN:
	  *asccom |= seteflist(&daqinfo.eflist[id], func, val);
	  break;
	case HDLISTN:
	  *asccom |= sethdlist(&daqinfo.hdlist[id], func, val);
	  break;
	case MTLISTN:
	  *asccom |= setmtlist(&daqinfo.mtlist[id], func, val);
	  break;
	  /*
	    case CLINFON:
	    *asccom |= setclinfo(&clinfo[id], func, val);
	    break;
	  */
	case SSMINFON:
	  *asccom |= setssminfo(&ssminfo, func, val);
	  break;
	default:
	  /* Noop */
	  break;
	}
      }
    }
  }

  if(idx != 0){
    textbuf[idx-1] = '\0';
  }else{
    textbuf[0] = '\0';
  }

  return textbuf;
}

int setdaqinfo(struct stdaqinfo *st, char *mem, char *val){
  int ival, ret = 0;

  if(!strcmp(mem, "runname")){
    memset(st->runname, 0, sizeof(st->runname));
    snprintf(st->runname, sizeof(st->runname), val);
    ret = ASC_EB_SET_RUNNAME;
  }else if(!strcmp(mem, "runnumber")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->runnumber, (char *)&ival, sizeof(ival));
    ret = ASC_EB_SET_RUNNUMBER;
  }else if(!strcmp(mem, "ebsize")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->ebsize, (char *)&ival, sizeof(ival));
    ret = ASC_EB_SET_EBSIZE;
  }else if(!strcmp(mem, "efn")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->efn, (char *)&ival, sizeof(ival));
    ret = ASC_EB_SET_EFN;
  }else if(!strcmp(mem, "babildes")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->babildes, (char *)&ival, sizeof(ival));
    ret = ASC_EB_SET_BABILDES;
  }

  return ret;
}

int setruninfo(struct struninfo *st, char *mem, char *val){

  if(!strcmp(mem, "header")){
    memset(st->header, 0, sizeof(st->header));
    snprintf(st->header, sizeof(st->header), val);
  }else if(!strcmp(mem, "ender")){
    memset(st->ender, 0, sizeof(st->ender));
    snprintf(st->ender, sizeof(st->ender), val);
  }

  return 1;
}


int setssminfo(struct stssminfo *st, char *mem, char *val){
  int ival;

  if(!strcmp(mem, "of")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->of, (char *)&ival, sizeof(ival));
  }else if(!strcmp(mem, "host")){
    memset(st->host, 0, sizeof(st->host));
    snprintf(st->host, sizeof(st->host), val);
  }else if(!strcmp(mem, "start")){
    memset(st->start, 0, sizeof(st->start));
    snprintf(st->start, sizeof(st->start), val);
  }else if(!strcmp(mem, "stop")){
    memset(st->stop, 0, sizeof(st->stop));
    snprintf(st->stop, sizeof(st->stop), val);
  }

  return ASC_EB_SET_SSMINFO;
}


int seteflist(struct steflist *st, char *mem, char *val){
  int ival;

  if(!strcmp(mem, "of")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->of, (char *)&ival, sizeof(ival));
  }else if(!strcmp(mem, "name")){
    memset(st->name, 0, sizeof(st->name));
    snprintf(st->name, sizeof(st->name), val);
  }else if(!strcmp(mem, "host")){
    memset(st->host, 0, sizeof(st->host));
    snprintf(st->host, sizeof(st->host), val);
  }

  if(!strcmp(mem, "del")){
    st->ex = 0;
  }else{
    st->ex = 1;
  }

  return ASC_EB_SET_EFLIST;
}


int sethdlist(struct sthdlist *st, char *mem, char *val){
  int ival;

  if(!strcmp(mem, "of")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->of, (char *)&ival, sizeof(ival));
  }else if(!strcmp(mem, "path")){
    memset(st->path, 0, sizeof(st->path));
    snprintf(st->path, sizeof(st->path), val);
  }

  if(!strcmp(mem, "del")){
    st->ex = 0;
  }else{
    st->ex = 1;
  }

  return ASC_EB_SET_EFLIST;;
}

int setmtlist(struct stmtlist *st, char *mem, char *val){
  int ival;

  if(!strcmp(mem, "of")){
    ival = strtol(val, NULL, 0);
    memcpy((char *)&st->of, (char *)&ival, sizeof(ival));
  }else if(!strcmp(mem, "path")){
    memset(st->path, 0, sizeof(st->path));
    snprintf(st->path, sizeof(st->path), val);
  }

  if(!strcmp(mem, "del")){
    st->ex = 0;
  }else{
    st->ex = 1;
  }

  return ASC_EB_SET_MTLIST;;
}

/*
  int setclinfo(struct stclinfo *st, char *mem, char *val){
  int ival;
  
  if(!strcmp(mem, "shmid")){
  ival = strtol(val, NULL, 0);
  memcpy((char *)&st->ex, (char *)&ival, sizeof(ival));
  }else if(!strcmp(mem, "host")){
  memset(st->clihost, 0, sizeof(st->clihost));
  snprintf(st->clihost, sizeof(st->clihost), val);
  }
  
  if(!strcmp(mem, "del")){
  st->ex = 0;
  }
  
  return ASC_EB_SET_CLINFO;;
  }
*/


int getdaqinfo(struct stdaqinfo *st, char *str){
  return sprintf(str, "get\ndaqinfo\nrunname\n%s\nrunnumber\n%d\nebsize\n%d\nefn\n%d\nbabildes\n%d\n", st->runname, st->runnumber, st->ebsize, st->efn, st->babildes);
}

int getruninfo(struct struninfo *st, char *str){
  return sprintf(str, "get\nruninfo\nrunnumber\n%d\nrunstat\n%d\nstarttime\n%d\nstoptime\n%d\nheader\n%s\nender\n%s\n", st->runnumber, st->runstat, st->starttime, st->stoptime, st->header, st->ender);
}

int geteflist(struct stdaqinfo *st, char *str){
  int i, off;
  
  off = sprintf(str, "get\eflist\n");
  for(i=0;i<MAXEF;i++){
    if(st->eflist[i].ex){
      off += sprintf(str+off, "id\n%d\nof\n%d\nname\n%s\nhost\n%s\n",
		     i, st->eflist[i].of, st->eflist[i].name, st->eflist[i].host);
    }
  }

  return off;
}

int gethdlist(struct stdaqinfo *st, char *str){
  int i, off;
  
  off = sprintf(str, "get\nmtlist\n");
  for(i=0;i<MAXHD;i++){
    if(st->hdlist[i].ex){
      off += sprintf(str+off, "id\n%d\nof\n%d\npath\n%s\nfree\n%llu\nfull\n%llu\nmaxsize\n%llu\n",
		     i, st->hdlist[i].of, st->hdlist[i].path, st->hdlist[i].free, st->hdlist[i].full, st->hdlist[i].maxsize);
    }
  }

  return off;
}

int getmtlist(struct stdaqinfo *st, char *str){
  int i, off;
  
  off = sprintf(str, "get\nmtlist\n");
  for(i=0;i<MAXMT;i++){
    if(st->mtlist[i].ex){
      off += sprintf(str+off, "id\n%d\nof\n%d\npath\n%s\nmaxsize\n%llu\n",
		     i, st->mtlist[i].of, st->mtlist[i].path, st->mtlist[i].maxsize);
    }
  }

  return off;
}

int getebinfo(char *str){
  return sprintf(str, "get\nebinfo\nevtn\n%u\ntotsize\n%llu\n",
		 gloevtn, totsize);
}
