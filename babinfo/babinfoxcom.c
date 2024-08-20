#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bi-config.h>
#include <bi-common.h>
#include <bbxml.h>

#include "babinfoxcom.h"
#include "babinfodb.h"

extern int getesconfig(int efn, struct stefrc *, char *);
extern int setesconfig(int efn, struct stefrc *);
extern int reloadesdrv(int efn);
extern void scrfromdb();
extern void scrtodb();
extern struct stdaqinfo daqinfo;
extern struct struninfo runinfo;
extern struct stdbcon dbcon;

int babinfoxcom(char *buff, char *ret){
  BBXMLEL *dom, *top, *tnode, *rdom, *rtop;
  int sz=0, i;

  rdom = bbxml_DomDocument();
  dom = bbxml_parsebuff(buff, strlen(buff));
  rtop = bbxml_appendChild(rdom, bbxml_createElement("babinfo", ""));
  DB(printf("babinfoxcom\n"));
  if((top = bbxml_node(dom, "babinfoxcom"))){
    i = 0;
    while(xcom[i].name){
      tnode = bbxml_node(top, xcom[i].name);
      if(tnode){
	(*xcom[i].func)(tnode, rtop);
      }
      i++;
    }
  }else{
    bbxml_appendChild(rtop, bbxml_createElement("error", "babinfoxcom: This is babinfo(XML), but commond is not babinfoxcom"));
  }
  
  bbxml_free(dom);
  sz = bbxml_sprintall(rdom, ret);
  bbxml_free(rdom);

  return sz;
}


/* babinfo functions */
int xgetesconfig(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX], myhost[128];
  int efn;
  BBXMLEL *efnel, *esconfig, *efar;
  struct stefrc efrc;

  efnel = bbxml_node(nd, "efn");
  esconfig = bbxml_appendChild(ret, bbxml_createElement("esconfig", ""));
  while(efnel){
    efar = bbxml_appendChild(esconfig, bbxml_createElement("node", ""));
    bbxml_appendChild(efar, bbxml_createElement("efn", efnel->text));
    efn = strtol(efnel->text, NULL, 0);
    memset(myhost, 0, sizeof(myhost));
    if(getesconfig(efn, &efrc, myhost)){
      DB(printf("xcom: efn=%d, erhost=%s, myhost=%s\n", efn, efrc.erhost, myhost));
      bbxml_appendChild(efar, bbxml_createElement("myhost", myhost));
      bbxml_appendChild(efar, bbxml_createElement("host", efrc.erhost));
      sprintf(tstr, "%d", efrc.mt);
      bbxml_appendChild(efar, bbxml_createElement("isdrv", tstr));
      bbxml_appendChild(efar, bbxml_createElement("rtdrv", efrc.mtdir));
      sprintf(tstr, "%d", efrc.connect);
      bbxml_appendChild(efar, bbxml_createElement("connect", tstr));
    }else{
      sprintf(tstr, "xgetesconfig: connection error for %s(efn=%d)", myhost, efn);
      bbxml_appendChild(efar, bbxml_createElement("error", tstr));
    }
    efnel = bbxml_next(efnel, "efn");
  }

  return 1;
}


int xsetesconfig(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX], myhost[128];
  int efn;
  BBXMLEL *efnel, *host, *rtdrv;
  struct stefrc efrc;

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "setesconfig: Now running!"));
    return 0;
  }

  efnel = bbxml_node(nd, "efn");
  if(efnel){
    efn = strtol(efnel->text, NULL, 0);
    memset(myhost, 0, sizeof(myhost));
    if(getesconfig(efn, &efrc, myhost)){
      host = bbxml_node(nd, "host");
      rtdrv = bbxml_node(nd, "rtdrv");
      if(host) strcpy(efrc.erhost, host->text);
      if(rtdrv) strcpy(efrc.mtdir, rtdrv->text);
      setesconfig(efn, &efrc);
    }else{
      sprintf(tstr, "xsetesconfig: connection error for %s(efn=%d)", myhost, efn);
      bbxml_appendChild(ret, bbxml_createElement("error", tstr));
    }
  }else{
    sprintf(tstr, "xsetesconfig: Invalid command (no efn)");
    bbxml_appendChild(ret, bbxml_createElement("error", tstr));
  }

  return 1;
}

int xreloadesdrv(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  int efn;
  BBXMLEL *efnel;
  
  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "reloadesdrv: Now running!"));
    return 0;
  }

  efnel = bbxml_node(nd, "efn");
  if(efnel){
    efn = strtol(efnel->text, NULL, 0);
    if(efn > 0 && efn < 256){
      reloadesdrv(efn);
    }else{
      sprintf(tstr, "xreloadesdrv: Invalid efn %d", efn);
      bbxml_appendChild(ret, bbxml_createElement("error", tstr));
    }
  }else{
    sprintf(tstr, "xreloadesdrv: Invalid command (no efn)");
    bbxml_appendChild(ret, bbxml_createElement("error", tstr));
  }
  
  return 1;
}

#ifdef USEDB
/* babinfo functions */
int xdbsetexp(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  BBXMLEL *idel, *setexpel;
  int id, rid;

  DB(printf("xdbsetexp\n"));

  idel = bbxml_node(nd, "id");
  setexpel = bbxml_appendChild(ret, bbxml_createElement("exp", ""));
  if(idel){
    id = strtol(idel->text, NULL, 0);
    rid = babinfodb_setexpid(dbcon, id);
    if(rid == id){
      DB(printf("xcom: setdbexpbyid id=%d \n", id));
      sprintf(tstr, "%d", rid);
      bbxml_appendChild(setexpel, bbxml_createElement("id", tstr));
    }else{
      if(rid == DBCONNECTIONERROR){
	sprintf(tstr, "xsetdbexp: Cannot connect to DB");
      }else{
	rid = babinfodb_getexpid();
	sprintf(tstr, "%d", rid);
	bbxml_appendChild(setexpel, bbxml_createElement("id", tstr));
	sprintf(tstr, "xsetdbexp: Cannot set expid");
      }
      bbxml_appendChild(setexpel, bbxml_createElement("error", tstr));
      rid = babinfodb_getexpid();
    }
  }else{
    sprintf(tstr, "xsetdbexp: XML error, no <id> in <exp>");
    bbxml_appendChild(setexpel, bbxml_createElement("error", tstr));
  }

  return 1;
}

/* babinfo functions */
int xdbgetexp(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  BBXMLEL *getexpel;
  int rid;

  getexpel = bbxml_appendChild(ret, bbxml_createElement("exp", ""));
  rid = babinfodb_getexpid();
  sprintf(tstr, "%d", rid);
  bbxml_appendChild(getexpel, bbxml_createElement("id", tstr));

  if(babinfodb_getexpname(dbcon, tstr) > 0){
    bbxml_appendChild(getexpel, bbxml_createElement("name", tstr));
  }

  return 1;
}

/* babinfo functions */
int xdbgetscr(BBXMLEL *nd, BBXMLEL *ret){
  scrfromdb();
  bbxml_appendChild(ret, bbxml_createElement("status", "ok"));
  return 1;
}

int xdbsetscr(BBXMLEL *nd, BBXMLEL *ret){
  scrtodb();
  bbxml_appendChild(ret, bbxml_createElement("status", "ok"));
  return 1;
}


int xdbsetdaqname(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  char name[64]={0};
  char server[64]={0};
  BBXMLEL *nameel, *daqel, *serverel;
  int expid, r;


  DB(printf("xdbsetdaqname\n"));

  expid = babinfodb_getexpid();
  nameel = bbxml_node(nd, "name");
  serverel = bbxml_node(nd, "server");
  daqel = bbxml_appendChild(ret, bbxml_createElement("daq", ""));
  if(nameel){
    strcpy(name, nameel->text);
    if(serverel){
      strcpy(server, serverel->text);
    }
    if((r = babinfodb_setdaqname(dbcon, expid, name, server)) != 1){
      if(r == DBCONNECTIONERROR){
	sprintf(tstr, "xsetdbexp: Cannot connect to DB");
      }else{
	sprintf(tstr, "xsetdaqname: Wrong DAQ Name %s", name);
      }
      bbxml_appendChild(daqel, bbxml_createElement("error", tstr));
    }else{
      bbxml_appendChild(daqel, bbxml_createElement("status", "ok"));
    }
  }else{
    sprintf(tstr, "xsetdaqname: XML error, no <name> in <daq>");
    bbxml_appendChild(daqel, bbxml_createElement("error", tstr));
  }

  return 1;

}

int xdbgetdaqname(BBXMLEL *nd, BBXMLEL *ret){
  BBXMLEL *daqel;
  char daqname[64]={0};

  babinfodb_getdaqname(daqname);
  daqel = bbxml_appendChild(ret, bbxml_createElement("daq", ""));
  if(strlen(daqname)){
    bbxml_appendChild(daqel, bbxml_createElement("name", daqname));
  }else{
    bbxml_appendChild(daqel, bbxml_createElement("error", "No DAQ name"));
  }

  return 1;
}

#endif

/* babinfo initial function */
int xgetinitialize(char *ret){
  BBXMLEL *rdom, *initel;
#ifdef USEDB
  char tstr[BBXML_TEXTMAX];
  int rid;
  BBXMLEL *dbsetexp, *dbsetdaqname;
  char name[64];
#endif

  rdom = bbxml_DomDocument();
  initel = bbxml_appendChild(rdom, bbxml_createElement("babinfoxcom", ""));

#ifdef USEDB
  /* DB Exp ID */
  rid = babinfodb_getexpid();
  if(rid){
    dbsetexp = bbxml_appendChild(initel, bbxml_createElement("dbsetexp", ""));
    sprintf(tstr, "%d", rid);
    bbxml_appendChild(dbsetexp, bbxml_createElement("id", tstr));
  }
  if(babinfodb_getdaqname(name)){
    dbsetdaqname = bbxml_appendChild(initel, 
				     bbxml_createElement("dbsetdaqname", ""));
    bbxml_appendChild(dbsetdaqname, bbxml_createElement("name", name));
  }
#endif

  bbxml_sprintall(rdom, ret);

  return 1;
}

