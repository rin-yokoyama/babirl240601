#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <bi-config.h>
#include <bi-common.h>
#include <bbxml.h>

#include "babildxcom.h"

extern struct stdaqinfo daqinfo;
extern struct struninfo runinfo;
extern struct stssminfo ssminfo;
extern void store_daqinfo();
extern void store_runinfo();
extern void store_eflist();
extern void store_hdlist();
extern void store_ssminfo();
extern int eb_set_header(char *hd);
extern int eb_set_ender(char *ed);
extern int eb_run_start(int com, char *err);
extern int eb_run_stop(char *err);
extern int eb_run_close();
extern int addaliasname(char *aliasname);
extern int fchkerhost;
extern int tgig;
extern char aliasnames[10][128];

int babildxcom(char *buff, char *ret){
  BBXMLEL *dom, *top, *tnode, *rdom, *rtop;
  int sz=0, i;

  rdom = bbxml_DomDocument();
  dom = bbxml_parsebuff(buff, strlen(buff));
  rtop = bbxml_appendChild(rdom, bbxml_createElement("babild", ""));
			   
  if((top = bbxml_node(dom, "babildxcom"))){
    i = 0;
    while(xcom[i].name){
      tnode = bbxml_node(top, xcom[i].name);
      if(tnode){
	(*xcom[i].func)(tnode, rtop);
      }
      i++;
    }
  }else{
    bbxml_appendChild(rtop, bbxml_createElement("error", "This is babild(XML), but commond is not babildxcom"));
  }
  
  bbxml_free(dom);
  sz = bbxml_sprintall(rdom, ret);
  bbxml_free(rdom);

  return sz;
}


/* babild functions */
int xgeteflist(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  int i;
  BBXMLEL *dinfo, *eflist;

  if(strcmp(ret->tag, "daqinfo")){
    dinfo = bbxml_appendChild(ret, bbxml_createElement("daqinfo", ""));
  }else{
    dinfo = ret;
  }

  for(i=0;i<MAXEF;i++){
    if(daqinfo.eflist[i].ex){
      eflist = bbxml_appendChild(dinfo, bbxml_createElement("eflist", ""));
      sprintf(tstr, "%d", i);
      bbxml_appendChild(eflist, bbxml_createElement("efn", tstr));
      sprintf(tstr, "%d", daqinfo.eflist[i].ex);
      bbxml_appendChild(eflist, bbxml_createElement("ex", tstr));
      sprintf(tstr, "%d", daqinfo.eflist[i].of);
      bbxml_appendChild(eflist, bbxml_createElement("of", tstr));
      bbxml_appendChild(eflist, bbxml_createElement("name", daqinfo.eflist[i].name));
      bbxml_appendChild(eflist, bbxml_createElement("host", daqinfo.eflist[i].host));
    }
  }

  return 1;
}

int xgethdlist(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  int i;
  BBXMLEL *dinfo, *hdlist;

  if(strcmp(ret->tag, "daqinfo")){
    dinfo = bbxml_appendChild(ret, bbxml_createElement("daqinfo", ""));
  }else{
    dinfo = ret;
  }

  for(i=0;i<MAXHD;i++){
    if(daqinfo.hdlist[i].ex){
      hdlist = bbxml_appendChild(dinfo, bbxml_createElement("hdlist", ""));
      sprintf(tstr, "%d", i);
      bbxml_appendChild(hdlist, bbxml_createElement("hdn", tstr));
      sprintf(tstr, "%d", daqinfo.hdlist[i].ex);
      bbxml_appendChild(hdlist, bbxml_createElement("ex", tstr));
      sprintf(tstr, "%d", daqinfo.hdlist[i].of);
      bbxml_appendChild(hdlist, bbxml_createElement("of", tstr));
      bbxml_appendChild(hdlist, bbxml_createElement("path", daqinfo.hdlist[i].path));
      sprintf(tstr, "%llu", daqinfo.hdlist[i].free);
      bbxml_appendChild(hdlist, bbxml_createElement("free", tstr));
      sprintf(tstr, "%llu", daqinfo.hdlist[i].full);
      bbxml_appendChild(hdlist, bbxml_createElement("full", tstr));
      sprintf(tstr, "%llu", daqinfo.hdlist[i].maxsize);
      bbxml_appendChild(hdlist, bbxml_createElement("maxsize", tstr));
    }
  }

  return 1;
}

int xgetdaqinfo(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  BBXMLEL *dinfo;

  dinfo = bbxml_appendChild(ret, bbxml_createElement("daqinfo", ""));
  bbxml_appendChild(dinfo, bbxml_createElement("runname", daqinfo.runname));
  sprintf(tstr, "%d", daqinfo.runnumber);
  bbxml_appendChild(dinfo, bbxml_createElement("runnumber", tstr));
  sprintf(tstr, "%d", daqinfo.ebsize);
  bbxml_appendChild(dinfo, bbxml_createElement("ebsize", tstr));
  sprintf(tstr, "%d", daqinfo.efn);
  bbxml_appendChild(dinfo, bbxml_createElement("efn", tstr));
  sprintf(tstr, "%d", daqinfo.babildes);
  bbxml_appendChild(dinfo, bbxml_createElement("babildes", tstr));

  xgeteflist(NULL, dinfo);
  xgethdlist(NULL, dinfo);


  return 1;
}

int xgetruninfo(BBXMLEL *nd, BBXMLEL *ret){
  char tstr[BBXML_TEXTMAX];
  BBXMLEL *rinfo;
  time_t ttime;
  struct tm *strtime;

  rinfo = bbxml_appendChild(ret, bbxml_createElement("runinfo", ""));
  sprintf(tstr, "%d", runinfo.runnumber);
  bbxml_appendChild(rinfo, bbxml_createElement("runnumber", tstr));
  switch(runinfo.runstat){
  case STAT_RUN_IDLE:
    sprintf(tstr, "IDLE");
    break;
  case STAT_RUN_START:
    sprintf(tstr, "START");
    break;
  case STAT_RUN_NSSTA:
    sprintf(tstr, "NSSTA");
    break;
  case STAT_RUN_WAITSTOP:
    sprintf(tstr, "WAITSTOP");
    break;
  default:
    sprintf(tstr, "UNKNOWN");
    break;
  }
  bbxml_appendChild(rinfo, bbxml_createElement("runstat", tstr));

  ttime = runinfo.starttime;
  strtime = localtime(&ttime);
  strftime(tstr, sizeof(tstr), "%d-%b-%y %X", strtime);
  bbxml_appendChild(rinfo, bbxml_createElement("starttime", tstr));

  ttime = runinfo.stoptime;
  strtime = localtime(&ttime);
  strftime(tstr, sizeof(tstr), "%d-%b-%y %X", strtime);
  bbxml_appendChild(rinfo, bbxml_createElement("stoptime", tstr));
  bbxml_appendChild(rinfo, bbxml_createElement("header", runinfo.header));
  bbxml_appendChild(rinfo, bbxml_createElement("ender", runinfo.ender));


  return 1;
}

int xseteflist(BBXMLEL *nd, BBXMLEL *ret){
  BBXMLEL *eflist, *efn, *host, *name, *of, *ex;
  int nefn = 0;

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "seteflist: Now running!"));
    return 0;
  }

  DB(printf("babldxcom xseteflist"));
  eflist = bbxml_node(nd, "eflist");
  while(eflist){
    efn = bbxml_node(eflist, "efn");
    host = bbxml_node(eflist, "host");
    name = bbxml_node(eflist, "name");
    of = bbxml_node(eflist, "of");
    ex = bbxml_node(eflist, "ex");

    if(efn){
      nefn = strtol(efn->text, NULL, 0);
      if(nefn > 0 && nefn < 256){
	if(host) strcpy(daqinfo.eflist[nefn].host, host->text);
	if(name) strcpy(daqinfo.eflist[nefn].name, name->text);
	if(of)  daqinfo.eflist[nefn].of = strtol(of->text, NULL, 0);
	if(ex)  daqinfo.eflist[nefn].ex = strtol(ex->text, NULL, 0);
      }
    }
    eflist = bbxml_next(eflist, "eflist");
  }
  store_eflist();
  store_daqinfo();

  return 1;
}

int xsethdlist(BBXMLEL *nd, BBXMLEL *ret){
  BBXMLEL *hdlist, *hdn, *path, *of, *ex;
  int nhdn = 0;

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "sethdlist: Now running!"));
    return 0;
  }

  hdlist = bbxml_node(nd, "hdlist");
  while(hdlist){
    hdn = bbxml_node(hdlist, "hdn");
    path = bbxml_node(hdlist, "path");
    of = bbxml_node(hdlist, "of");
    ex = bbxml_node(hdlist, "ex");

    if(hdn){
      nhdn = strtol(hdn->text, NULL, 0);
      if(nhdn >= 0 && nhdn < MAXHD){
	if(path) strcpy(daqinfo.hdlist[nhdn].path, path->text);
	if(of)  daqinfo.hdlist[nhdn].of = strtol(of->text, NULL, 0);
	if(ex)  daqinfo.hdlist[nhdn].ex = strtol(ex->text, NULL, 0);
      }
    }
    hdlist = bbxml_next(hdlist, "hdlist");
  }
  store_hdlist();

  return 1;
}

int xsetruninfo(BBXMLEL *nd, BBXMLEL *ret){
  BBXMLEL *runname, *runnumber;
  int n = 0;

  DB(printf("xsetruninfo\n"));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "setrunname: Now running!"));
    return 0;
  }

  runname = bbxml_node(nd, "runname");
  runnumber = bbxml_node(nd, "runnumber");

  if(runname){
    strcpy(daqinfo.runname, runname->text);
  }

  if(runnumber){
    n = strtol(runnumber->text, NULL, 0);
    if(n >= 0){
      daqinfo.runnumber = n;
      runinfo.runnumber = n;
    }
  }
  store_daqinfo();

  return 1;
}


int xsetaliasname(BBXMLEL *nd, BBXMLEL *ret){
  DB(printf("xsetaliasname\n"));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "aliasname: Now running!"));
    return 0;
  }

  if(nd->text){
    addaliasname(nd->text);
  }

  return 1;
}

/* babild initial function */
int xgetinitialize(char *ret){
  char tstr[BBXML_TEXTMAX];
  BBXMLEL *rdom, *initel;
  int i;

  rdom = bbxml_DomDocument();
  initel = bbxml_appendChild(rdom, bbxml_createElement("babildxcom", ""));

  /* chkerhost */
  sprintf(tstr, "%d", fchkerhost);
  bbxml_appendChild(initel, bbxml_createElement("setchkerhost", tstr));
  /* tgig */
  sprintf(tstr, "%d", tgig);
  bbxml_appendChild(initel, bbxml_createElement("settgig", tstr));
  for(i=0;i<10;i++){
    if(strlen(aliasnames[i])){
      bbxml_appendChild(initel, bbxml_createElement("setaliasname", aliasnames[i]));    }
  }

  bbxml_sprintall(rdom, ret);

  return 1;
}

int xsetchkerhost(BBXMLEL *nd, BBXMLEL *ret){
  DB(printf("xseterhost %s\n", nd->text));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "setchkerhost: Now running!"));
    return 0;
  }

  fchkerhost = (int)strtol(nd->text, NULL, 0);
  store_daqinfo();

  return 1;
}

int xsettgig(BBXMLEL *nd, BBXMLEL *ret){
  DB(printf("xsettgig %s\n", nd->text));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "settgig: Now running!"));
    return 0;
  }

  tgig = (int)strtol(nd->text, NULL, 0);
  store_daqinfo();

  return 1;
}


int xwth(BBXMLEL *nd, BBXMLEL *ret){
  DB(printf("xwth\n"));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "wth: Now running!"));
    return 0;
  }

  if(nd->text){
    eb_set_header(nd->text);
  }

  return 1;
}

int xstart(BBXMLEL *nd, BBXMLEL *ret){
  char err[1024]={0};
  DB(printf("xstart\n"));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "start: Now running!"));
    return 0;
  }

  if(nd->text){
    eb_set_header(nd->text);
  }

  if(!eb_run_start(EB_RUN_START, err)){
    bbxml_appendChild(ret, bbxml_createElement("error", err));
    return 0;
  }

  return 1;
}

int xnssta(BBXMLEL *nd, BBXMLEL *ret){
  char err[1024]={0};
  DB(printf("xnssta\n"));

  if(runinfo.runstat){
    bbxml_appendChild(ret, bbxml_createElement("error", "nssta: Now running!"));
    return 0;
  }

  if(!eb_run_start(EB_RUN_NSSTA, err)){
    bbxml_appendChild(ret, bbxml_createElement("error", err));
    return 0;
  }

  return 1;
}

int xstop(BBXMLEL *nd, BBXMLEL *ret){
  char err[1024]={0};
  int loop=0;
  DB(printf("xstop\n"));
  if(!eb_run_stop(err)){
    bbxml_appendChild(ret, bbxml_createElement("error", err));
    return 0;
  }

  while(loop<200){ // timeout = 20 sec
    if(runinfo.runstat == STAT_RUN_WAITSTOP){
      if(nd->text){
	eb_set_ender(nd->text);
      }
      usleep(10000);
      eb_run_close();
      break;
    }else if(runinfo.runstat == STAT_RUN_IDLE){
      break;
    }else{
      loop++;
      usleep(100000); // wait 100ms
    }
  }

  return 1;
}

