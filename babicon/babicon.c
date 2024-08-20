/* babicon/babicon.c
 * last modified : 17/02/03 17:35:14 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * DAQ Manager/Configurator
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <netinet/in.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include "babicon.h"
#include "comfuncs.c"

#include "../babild/contains_colon.h"

int getscrdata(int);

int chkex(int ex, char *mes){
  printf("ex %d\n", ex);
  
  if(ex){
    return 1;
  }else{
    printf("%s %d is not exist\n", mes, ex);
    return 0;
  }
}

int chknex(int ex, char *mes){
  if(!ex){
    return 1;
  }else{
    printf("%s %d is already exist\n", mes, ex);
    return 0;
  }
}

int escon(char *arg){
  int essock, efid;

  efid = -1;
  essock = 0;
  if(arg == NULL){
    printf("no EFSID\n");
    return 0;
  }
  
  efid = strtol(arg, NULL, 0);
  if(efid < 0 || efid > MAXEF){
    printf("0 < EFSID < %d\n", MAXEF);
    return 0;
  }
  if(!daqinfo.eflist[efid].ex){
    printf("Invalid EFSID\n");
    return 0;
  }

  if(!(essock = mktcpsend_tout(daqinfo.eflist[efid].host, ESCOMPORT+efid, 10))){
    printf("Can't connet to babies id=%d.\n", efid);
    return 0;
  }

  return essock;
}

int mocon(char *host){
  int mosock;

  /* Connect to babild */
  if(!(mosock = mktcpsend(host, BABIMOPORT))){
    printf("babicon: Can't connet to babimo.\n");
    return 0;
  }
  
  return mosock;
}

void quit(void){
  printf("babicon: quit\n");
  exit(0);
}

int mokill(char *arg){
  int sock, com, len;
  char buff[1024];

  DB(printf("babicon: mokill %s\n", arg));

  memset(buff ,0, sizeof(buff));

  if(!strlen(arg)){
    printf("kill babild/babinfo/babies ...\n");
    return 0;
  }

  strncpy(buff, arg, sizeof(buff)-1);
  com = MON_KILLPID;

  sock = mocon(ebhostname);

  len = sizeof(com) + strlen(buff);
  send(sock,(char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(com), 0);
  send(sock, buff, strlen(buff), 0);

  close(sock);

  return 0;
}

int getevtnumber(char *arg){
  int sock;
  unsigned int ebn;

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_EVTN, (char *)&ebn);
  close(sock);

  if(ebn > 0){
    printf("Event built number = %d\n", ebn);
  }else{
    printf("No event build\n");
  }

  return 0;
}

int getebrate(char *arg){
  int sock, dt;
  double ts, rs;
  time_t now;

  get_runinfo();

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_TOTSIZE, (char *)&totsize);
  time(&now);
  close(sock);

  ts = (double)totsize/1024./1024.*2.;

  if(runinfo.runstat){
    dt = (int)now - runinfo.starttime;
  }else{
    dt = runinfo.stoptime - runinfo.starttime;
  }

  if(totsize > 0){
    rs = ts / (double)dt;
    printf("Total event built size = %7.2f (MB)\n", ts);
    printf("Total event built rate = %7.2f (MB/s)\n", rs);
  }else{
    printf("No event build\n");
  }

  return 0;
}

int get_runinfo(void){
  int sock;

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_RUNINFO, (char *)&runinfo);
  close(sock);

  return 0;
}

void show_scrlist(void){
  int i, j, tn, ix;
  int st[MAXSCRANA];

  for(i=0;i<scranan;i++){
    tn = scrana[i].scrid;
    ix = 0;
    for(j=0;j<scranan;j++){
      if(i!=j && tn > scrana[j].scrid){
	ix++;
      }
    }
    st[ix] = i;
  }

  printf("   ID SCRN : NAME\n");
  for(i=0;i<scranan;i++){
    DB(printf("i=%d , scranan=%d\n",i,scranan));
    printf("%5d  %3d : %s", scrana[st[i]].scrid,
	   scrana[st[i]].scrn, scrana[st[i]].idname);
    if(scrana[st[i]].classid == RIDF_SCALER){
      printf(" (Non clear 24bit)\n");
    }else if(scrana[st[i]].classid == RIDF_NCSCALER32){
      printf(" (Non clear 32bit)\n");
    }else{
      printf("\n");
    }
  }

}


int show_scrdata(int id){
  int tdx, i;

  if((tdx = fndscrid(id)) < 0){
    printf("No such scaler id = %d\n", id);
    return 0;
  }

  printf("   ID SCRN : NAME\n");
  printf("%5d  %3d : %s\n", scrana[tdx].scrid,
	 scrana[tdx].scrn, scrana[tdx].idname);
  printf("\n");

  printf("   SCRRATE (ch=%d, rate=%dcps)\n", scrana[tdx].ratech, scrana[tdx].rate);

  /*
    if(scrana[tdx].classid == RIDF_SCALER){
    printf("CHN %-17s      Total   \n", " Name");
    for(i=0;i<scrana[tdx].scrn;i++){
    printf("%3d (%-15s) : %10u\n", i, scr[tdx][i].name,
    scr[tdx][i].cur);
    }
    }else{
  */
  printf("CHN %-17s      Current /      Total\n", " Name");
  for(i=0;i<scrana[tdx].scrn;i++){
    printf("%3d (%-15s) : %10u / %10Lu\n", i, scr[tdx][i].name,
	   scr[tdx][i].cur, scr[tdx][i].tot);
  }
  /* } */

  return 1;
}

void show_efrc(struct stefrc efrc){
  printf("ID      : %d\n", efrc.efid);
  printf("erport  : %d\n", efrc.erport);
  printf("erhost  : %s\n", efrc.erhost);
  printf("hd1     : %d\n", efrc.hd1);
  printf("hd1dir  : %s\n", efrc.hd1dir);
  printf("hd2     : %d\n", efrc.hd2);
  printf("hd2dir  : %s\n", efrc.hd2dir);
  printf("isdrv   : %d", efrc.mt);
  //if(!efrc.mt) cprintf(FG_RED, "(Error in driver dir)");
  printf("\n");
  printf("rtdrv   : %s\n", efrc.mtdir);
  printf("connect : %d\n", efrc.connect);
}

void show_eflist(void){
  int i;

  cprintf(FG_BLACK|BG_GREEN, "Event flagment\n");
  printf("%3s %-10s %-10s on/off\n","ID","Hostname","Nickname");
  for(i=0; i<MAXEF; i++){
    if(daqinfo.eflist[i].ex){
      printf("%3d %-10s %-10s (%s)\n", i, daqinfo.eflist[i].host,
	     daqinfo.eflist[i].name, ofstr[daqinfo.eflist[i].of]);
    }
  }
  printf("\n");
}

void show_mtlist(void){
  int i;

  cprintf(FG_BLACK|BG_GREEN, "MT list\n");
  for(i=0; i<MAXMT; i++){
    if(daqinfo.mtlist[i].ex){
      printf("%2d %-15s (%s)\n", i, daqinfo.mtlist[i].path,
	     ofstr[daqinfo.mtlist[i].of]);
    }
  }
  printf("\n");
}

void show_runinfo(void){
  time_t ttime;
  char chst[80];
  struct tm *strtime;

  memset(chst, 0, sizeof(chst));

  cprintf(FG_BLACK|BG_MAGENTA, "Run information");
  printf("\n");
  printf("  Run name   : %s\n", daqinfo.runname);
  printf("  Run number : %d\n", runinfo.runnumber);
  printf("  Run status : %s\n", runstatstr[runinfo.runstat]);
  ttime = runinfo.starttime;
  strtime = localtime(&ttime);
  strftime(chst, sizeof(chst), "%d-%b-%y %X", strtime);
  printf("  Start date : %s\n", chst);
  if(!runinfo.runstat){
    ttime = runinfo.stoptime;
    strtime = localtime(&ttime);
    strftime(chst, sizeof(chst), "%d-%b-%y %X", strtime);
    printf("  Stop  date : %s\n", chst);
  }
  printf("  Header     : %s\n", runinfo.header);
  if(!runinfo.runstat){
    printf("  Ender      : %s\n", runinfo.ender);
  }
  printf("\n");
  
}

void show_ebinfo(void){
  cprintf(FG_BLACK|BG_YELLOW, "EB Information\n");
  printf("- EF Number        : %d\n", daqinfo.efn);
  printf("- Event Build Size : %d", daqinfo.ebsize);
  printf(" (%d kB)", daqinfo.ebsize*WORDSIZE/1024);
  printf("\n");
  printf("- Babildes mode    : %s\n\n", ofstr[daqinfo.babildes]);
	 
}

void show_ssminfo(void){
  cprintf(FG_BLACK|BG_GREEN, "SSM Information\n");
  if(ssminfo.ex){
    printf("SSM hostname : %s (%s)\n", ssminfo.host, ofstr[ssminfo.of]);
    printf("Start comand : %s\n", ssminfo.start);
    printf("Stop comand  : %s\n", ssminfo.stop);
  }else{
    printf("SSM is not exist\n");
  }
  printf("\n");
}

void show_daqinfo(void){
  show_ssminfo();
  show_eflist();
  show_mtlist();
  show_hdlist();
  show_ebinfo();
  show_runinfo();
}

int start(char *arg){
  int sock, ret;

  getconfig("noshow\0");

  sock = ebcon(ebhostname);
  eb_get(sock, EB_RUN_START, buff);
  close(sock);
  memcpy((char *)&ret, buff, sizeof(ret));
  if(!ret){
    if(daqinfo.babildes){
      cprintf(FG_RED, "Can't start, babild is babildes mode\n");
    }else{
      cprintf(FG_RED, "Can't start, because %s\n",
	      buff+sizeof(ret));
      printf("  (if you don't want to check the hostname, try to use setchkerhost off)\n");
    }
  }else{
    get_runinfo();
    show_runinfo();
  }

  return 0;
}

int nssta(char *arg){
  int sock, ret;

  getconfig("noshow\0");

  sock = ebcon(ebhostname);
  eb_get(sock, EB_RUN_NSSTA, buff);
  close(sock);
  memcpy((char *)&ret, buff, sizeof(ret));
  if(!ret){
    if(daqinfo.babildes){
      cprintf(FG_RED, "Can't start, babild is babildes mode\n");
    }else{
      cprintf(FG_RED, "Can't start, because %s\n",
	      buff+sizeof(ret));
      printf("  (if you don't want to check the hostname, try to use setchkerhost off)\n");
    }
  }else{
    get_runinfo();
    show_runinfo();
  }

  return 0;
}

int stop(char *arg){
  int sock, ret;
  char *line;
  char tender[80];

  getconfig("noshow\0");

  memset(buff, 0, sizeof(buff));

  sock = ebcon(ebhostname);
  eb_get(sock, EB_RUN_STOP, buff);
  close(sock);

  memcpy((char *)&ret, buff, sizeof(ret));

  if(!ret){
    cprintf(FG_RED, "Can't stop, now stopping or some error or babildes mode\n");
    if(strlen(buff+4)){
      cprintf(FG_GREEN, "  (%s)\n", buff+4);
    }
    return 0;
  }

  if(runinfo.runstat == STAT_RUN_START){
    memset(tender, 0, sizeof(tender));
    if(!strlen(arg)){
      line = readline("Ender : ");
      strncpy(tender, line, sizeof(tender)-1);
      free(line);
    }else{
      strncpy(tender, arg, sizeof(tender)-1);
    }

  }
  usleep(300000);
  get_runinfo();

  while(runinfo.runstat != STAT_RUN_IDLE
	&& runinfo.runstat != STAT_RUN_WAITSTOP){
#ifdef DEBUG
    usleep(100000);
#else
    usleep(500000);
    printf("Waiting for run end... (stat=%d)\n", runinfo.runstat);
#endif
    get_runinfo();
  }
  if(runinfo.runstat == STAT_RUN_WAITSTOP){
    sock = ebcon(ebhostname);
    ret = eb_set(sock, EB_SET_ENDER, tender, sizeof(tender));
    close(sock);

    sock = ebcon(ebhostname);
    ret = eb_get(sock, EB_RUN_CLOSE, (char *)&ret);
    close(sock);
  }
  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_RUNINFO, (char *)&runinfo);
  close(sock);

  show_runinfo();

  show_hdlist();
  printf("\n");

  getevtnumber(NULL);
  printf("\n");
  getevtn(NULL);

  if(strlen(buff+4)){
    cprintf(FG_RED, "\n*** %s\n", buff+4);
  }



  return 0;
}

int setrunnumber(char *arg){
  char *line;
  char *argv[10];
  int newn, argc, sock, ret;

  DB(printf("babicon: setnumber %s\n", arg));

  newn = -1;
  argc = striparg(arg, argv);

  if(argc == 1){
    newn = strtol(argv[0], NULL, 0);
  }else{
    line = readline("New runnumber: ");
    newn = strtol(line, NULL, 0);
    free(line);
  }

  if(newn < 0){
    printf("runnumber >= 0\n");
    return 0;
  }

  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_RUNNUMBER, (char *)&newn, sizeof(newn));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameters\n");
    return 0;
  }

  getconfig(NULL);

  return 1;
}

int setebsize(char *arg){
  char *line;
  char *argv[10];
  int newn, argc, sock, ret;

  DB(printf("babicon: setebsize %s\n", arg));

  newn = -1;
  argc = striparg(arg, argv);

  if(argc == 1){
    newn = strtol(argv[0], NULL, 0);
  }else{
    line = readline("New event build size: ");
    newn = strtol(line, NULL, 0);
    free(line);
  }

  if(newn < 0 || newn > EB_EFBLOCK_MAXSIZE){
    printf("Please set 0 < ebsize < %d (%d kB)\n", EB_EFBLOCK_MAXSIZE,
	   EB_EFBLOCK_MAXSIZE*2/1024);
    //printf("64 kB is not a real limit of event build, it will be improved.\n");
    return 0;
  }

  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_EBSIZE, (char *)&newn, sizeof(newn));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameters\n");
    return 0;
  }

  getconfig(NULL);

  return 1;
}

int setbabildes(char *arg){
  char *line, str[80];
  char *argv[10];
  int babildes, argc, sock, ret;

  DB(printf("babicon: setbabildes %s\n", arg));

  argc = striparg(arg, argv);

  if(argc == 1){
    strcpy(str, argv[0]);
  }else{
    line = readline("Set babildes mode on/off: ");
    strcpy(str, line);
    free(line);
  }

  if(strcmp(str, "on") == 0){
    babildes = 1;
  }else if(strcmp(str, "off") == 0){
    babildes = 0;
  }else{
    printf("setbabildes on/off\n");
    return 0;
  }
    

  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_BABILDES, (char *)&babildes, sizeof(babildes));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameters or babiau\n");
    return 0;
  }

  getconfig(NULL);

  return 1;
}

int setrunname(char *arg){
  char *line;
  char *argv[10];
  char nname[80];
  int argc, sock, ret;

  DB(printf("babicon: setnumber %s\n", arg));

  memset(nname, 0, sizeof(nname));
  argc = striparg(arg, argv);

  if(argc == 1){
    strncpy(nname, argv[0], sizeof(nname)-1);
  }else{
    line = readline("New name: ");
    strncpy(nname, line, sizeof(nname)-1);
    free(line);
  }

  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_RUNNAME, nname, sizeof(nname));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameters\n");
    return 0;
  }

  getconfig(NULL);

  return 1;
}

int setconfig(char *arg){
  DB(printf("babicon: setconfig %s\n", arg));
  printf("This command is not implemented yet.\n");
  return 0;
}

int sethdlist(char *arg){
  int argc;
  char *argv[10];
  char this[] = "sethdlist";
  int hdn, of, fl;
  char hdp[80];
  int sock, ret, ofc, pathc, dlc, er;

  fl = 1;
  of = 0;
  hdn = 0;
  ret = 0;
  ofc = 0;
  pathc = 0;
  dlc = 0;
  er = 1;

  DB(printf("babicon: sethdlist %s\n", arg));
  argc = striparg(arg, argv);

  if(argc == 2 || argc == 3){
    hdn = strtol(argv[0], NULL, 0);
    if(hdn < 0 || hdn >= MAXHD){
      printf("0 < HDN < %d\n", MAXHD);
      fl = 0;
    }
    
    if(!strcmp(argv[1], "on")){
      of = 1;
      ofc = 1;
    }else if(!strcmp(argv[1], "off")){
      of = 0;
      ofc = 1;
    }else if(!strcmp(argv[1], "del")){
      dlc = 1;
    }else if(!strcmp(argv[1], "path")){
      if(argc != 3){
	fl = 0;
      }else{
	memset(hdp, 0, sizeof(hdp));
	strncpy(hdp, argv[2], sizeof(hdp)-1);
	sock = ebcon(ebhostname);
	ret = eb_set(sock, EB_CHK_DIR, hdp, strlen(hdp)+1);
	close(sock);
	if(contains_colon(hdp))
	{
	  printf("Kafka-server %s\n", hdp);
	  pathc = 2;
	}
	else if(!ret){
	  printf("Can't open dir %s\n", hdp);
	  fl = 0;
	}else{
	  pathc = 1;
	}
      }
    }else{
      printf("COMMAND = on/off/del/path\n");
      fl = 0;
    }
  }else{
    fl = 0;
  }

  if(!fl){
    synopsis(this);
    return 0;
  }

  if(fl){
    sock = ebcon(ebhostname);
    eb_get(sock, EB_GET_HDLIST, (char *)daqinfo.hdlist);
    close(sock);

    if(ofc){
      if(daqinfo.hdlist[hdn].ex){
	daqinfo.hdlist[hdn].of = of;
      }else{
	printf("Don't exist such HDN %d\n", hdn);
	er = 0;
      }
    }else if(pathc){
      daqinfo.hdlist[hdn].ex = pathc;
      strncpy(daqinfo.hdlist[hdn].path, hdp,
	      sizeof(daqinfo.hdlist[hdn].path));
    }else if(dlc){
      daqinfo.hdlist[hdn].ex = 0;
      memset(daqinfo.hdlist[hdn].path, 0,
	     sizeof(daqinfo.hdlist[hdn].path));
    }
    if(er){
      sock = ebcon(ebhostname);
      ret = eb_set(sock, EB_SET_HDLIST, (char *)daqinfo.hdlist,
		   sizeof(daqinfo.hdlist));
      close(sock);
      if(!ret){
	printf("Run is now running, can't change parameters\n");
      }
    }

    sock = ebcon(ebhostname);
    eb_get(sock, EB_GET_HDLIST, (char *)daqinfo.hdlist);
    close(sock);

    show_hdlist();
  }

  return 1;
}

int seteflist(char *arg){
  char this[] = "seteflist";
  int argc;
  char *argv[10];
  int efn, fl;
  char ch1[80], ch2[80];
  int sock, ret;
  int com, upf;
  enum comlist {ADD, DEL, HOST, NAME, ON, OFF, SCR, PAS};

  //DB(printf("babicon: seteflist %s\n", arg));
  argc = striparg(arg, argv);

  if(argc != 2 && argc != 3 && argc != 4){
    synopsis(this);
    return 0;
  }


  fl = 1;
  efn = 0;
  ret = 0;
  upf = 0;

  efn = strtol(argv[0], NULL, 0);
  if(efn < 0 || efn >= MAXEF){
    printf("0 < EFN < %d\n", MAXEF);
    return 0;
  }

  if((com = chkcom(argv[1], comseteflist)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  switch(com){
  case ADD:
    if(argc != 4){
      fl = 0;
    }else{
      memset(ch1, 0, sizeof(ch1));
      memset(ch2, 0, sizeof(ch2));
      strncpy(ch1, argv[2], sizeof(ch1)-1);
      strncpy(ch2, argv[3], sizeof(ch2)-1);
    }
    break;
  case HOST:
  case NAME:
    if(argc != 3){
      fl = 0;
    }else{
      memset(ch1, 0, sizeof(ch1));
      strncpy(ch1, argv[2], sizeof(ch1)-1);
    }
    break;
  }
    
  if(fl){
    sock = ebcon(ebhostname);
    eb_get(sock, EB_GET_EFLIST, (char *)daqinfo.eflist);
    close(sock);

    switch(com){
    case ADD:
      if((upf = chknex(daqinfo.eflist[efn].ex, "EFN"))){
	daqinfo.eflist[efn].ex = 1;
	daqinfo.eflist[efn].of = EB_EFLIST_ON;
	strncpy(daqinfo.eflist[efn].host, ch1,
		sizeof(daqinfo.eflist[efn].host));
	strncpy(daqinfo.eflist[efn].name, ch2,
		sizeof(daqinfo.eflist[efn].name));
      }
      break;
    case DEL:
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	daqinfo.eflist[efn].ex = 0;
	daqinfo.eflist[efn].of = EB_EFLIST_OFF;
	memset(daqinfo.eflist[efn].host, 0, sizeof(daqinfo.eflist[efn].host));
	memset(daqinfo.eflist[efn].name, 0, sizeof(daqinfo.eflist[efn].name));
      }
      break;
    case HOST:
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	strncpy(daqinfo.eflist[efn].host, ch1,
		sizeof(daqinfo.eflist[efn].host));
      }
      break;
    case NAME:
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	strncpy(daqinfo.eflist[efn].name, ch1,
		sizeof(daqinfo.eflist[efn].name));
      }
      break;
    case ON:
      printf("on\n");
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	daqinfo.eflist[efn].of = EB_EFLIST_ON;
      }
      break;
    case OFF:
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	daqinfo.eflist[efn].of = EB_EFLIST_OFF;
      }
      break;
    case SCR:
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	daqinfo.eflist[efn].of = EB_EFLIST_SCR;
      }
      break;
    case PAS:
      if((upf = chkex(daqinfo.eflist[efn].ex, "EFN"))){
	daqinfo.eflist[efn].of = EB_EFLIST_PAS;
      }
      break;
    }

    if(upf){
      sock = ebcon(ebhostname);
      ret = eb_set(sock, EB_SET_EFLIST, (char *)daqinfo.eflist,
		   sizeof(daqinfo.eflist));
      close(sock);
      if(!ret){
	printf("Run is now running, can't change parameters\n");
      }

      sock = ebcon(ebhostname);
      eb_get(sock, EB_GET_EFLIST, (char *)daqinfo.eflist);
      close(sock);
    }

    show_eflist();
  }else{
    synopsis(this);
  }

  return 1;
}

int synopsis(char *arg){
  int i, m;

  m = 0;
  i = 0;

  while(manual[i].name){
    if(strcmp(manual[i].name, arg) == 0){
      m = i;
      break;
    }
    i++;
  }

  cprintf(AT_BOLD, "SYNOPSIS\n");
  printf("%s", tab);
  printf("%s", manual[m].name);
  printf(" %s\n", manual[m].synopsis);
  printf("\n");

  cprintf(AT_BOLD, "COMMAND\n");
  if(manual[m].command){
    i=0;
    printf("%s", tab);
    while(manual[m].command[i]){
      if(i != 0) printf(" / ");
      printf("%s%s", manual[m].command[i], manual[m].command[i+1]);
      i += 3;
    }
    printf("\n");
  }
  printf("\n");

  return 0;
}

int help(char *arg){
  int i, f, argc, m;
  char *argv[10];
  char tmpchar[256];

  DB(printf("babicon: help %s\n", arg));
  i = 0;
  f = -1;
  m = -1;
  argc = striparg(arg, argv);
  if(argc > 0){
    while(commands[i].name){
      if(strcmp(commands[i].name, argv[0]) == 0){
	f = i;
	break;
      }
      i++;
    }
    i =0;
    while(manual[i].name){
      if(strcmp(manual[i].name, argv[0]) == 0){
	m = i;
	break;
      }
      i++;
    }
  }

  if(f < 0){
    i = 0;
    while(1){
      if(commands[i].name == NULL) break;
      printf("%-12s : %s\n",commands[i].name, commands[i].doc);
      i++;
    }
    printf("\n");
    printf("help [command]  <-  show detail of command\n\n");
  }else{
    if(m > -1){
      cprintf(AT_BOLD, "NAME\n");
      printf("%s%s - %s\n", tab, commands[m].name, commands[m].doc);
      printf("\n");
      cprintf(AT_BOLD, "SYNOPSIS\n");
      printf("%s", tab);
      printf("%s", manual[m].name);
      printf(" %s\n", manual[m].synopsis);
      printf("\n");
      cprintf(AT_BOLD, "DESCRIPSTION\n");
      printf("%s%s\n", tab, manual[m].description);
      printf("\n");
      if(manual[m].command){
	cprintf(AT_BOLD, "COMMAND\n");
	i=0;
	while(manual[m].command[i]){
	  printf("     ");
	  sprintf(tmpchar, "%s %s",
		  manual[m].command[i], manual[m].command[i+1]);
	  printf("%-20s: %s\n",
		 tmpchar, manual[m].command[i+2]);
	  i+=3;
	}
      }
    }else{
      printf("%-10s : %s\n",commands[f].name, commands[f].doc);
    }
  }
  
  return 0;
}

int exec_sh(char *arg){
  DB(printf("babicon: sh %s\n", arg));
  system(arg);
  return 0;
}

int exec_nrsh(char *arg){
  int sock, com, len;
  char buff[1024];

  DB(printf("babicon: nrsh %s\n", arg));

  memset(buff ,0, sizeof(buff));
  strncpy(buff, arg, sizeof(buff)-1);
  com = MON_EXEC;

  sock = mocon(ebhostname);

  len = sizeof(com) + strlen(buff);
  send(sock,(char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(com), 0);
  send(sock, buff, strlen(buff), 0);

  close(sock);
  

  return 0;
}

int exec_rsh(char *arg){
  int sock, com, len;
  char buff[1024];

  DB(printf("babicon: rsh %s\n", arg));

  memset(buff ,0, sizeof(buff));
  strncpy(buff, arg, sizeof(buff)-1);
  com = MON_EXEC;

  sock = mocon(ebhostname);

  len = sizeof(com) + strlen(buff);
  send(sock,(char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(com), 0);
  send(sock, buff, strlen(buff), 0);

  close(sock);
  

  return 0;
}

int getconfig(char *arg){
  int sock;

  DB(printf("babicon: getconfig %s\n", arg));

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_SSMINFO, (char *)&ssminfo);
  close(sock);
  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_DAQINFO, (char *)&daqinfo);
  close(sock);
  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_RUNINFO, (char *)&runinfo);
  close(sock);

  if(arg){
    if(!strcmp("noshow", arg)){
      /* noop */
    }else{
      show_daqinfo();
    }
  }else{
    show_daqinfo();
  }

  return 0;
}



int setssminfo(char *arg){
  char this[] = "setssminfo";
  int argc;
  char *argv[10];
  char ch1[80];
  int sock, ret;
  int com;
  char *line;
  enum comlist {ON, OFF, HOST, START, STOP};

  argc = striparg(arg, argv);

  if(argc != 1 && argc != 2){
    synopsis(this);
    return 0;
  }


  ret = 0;
  
  if((com = chkcom(argv[0], comssminfo)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  memset(ch1, 0, sizeof(ch1));
  switch(com){
  case HOST:
    if(argc != 2){
      line = readline("HOSTNAME : ");
      strncpy(ch1, line, sizeof(ch1)-1);
      free(line);
    }else{
      strncpy(ch1, argv[1], sizeof(ch1)-1);
    }
    break;
  case START:
  case STOP:
    if(argc != 2){
      line = readline("PATH : ");
      strncpy(ch1, line, sizeof(ch1)-1);
      free(line);
    }else{
      strncpy(ch1, argv[1], sizeof(ch1)-1);
    }
    break;
  }

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_SSMINFO, (char *)&ssminfo);
  close(sock);
  
  switch(com){
  case ON:
    ssminfo.of = 1;
    break;
  case OFF:
    ssminfo.of = 0;
    break;
  case HOST:
    ssminfo.ex = 1;
    strncpy(ssminfo.host, ch1, sizeof(ssminfo.host));
    break;
  case START:
    strncpy(ssminfo.start, ch1, sizeof(ssminfo.start));
    break;
  case STOP:
    strncpy(ssminfo.stop, ch1, sizeof(ssminfo.stop));
    break;
  }

  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_SSMINFO, (char *)&ssminfo,
	 sizeof(ssminfo));
  close(sock);
  if(!ret){
    printf("Run is now running, can't change parameters\n");
  }

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_SSMINFO, (char *)&ssminfo);
  close(sock);

  show_ssminfo();

  return 1;
}


int getclinfo(char *arg){
  int sock;
  if(!(sock = infcon(ebhostname))) return 0;

  eb_get(sock, INF_GET_CLIHOSTS, (char *)clinfo);
  close(sock);

  show_clinfo();
  
  return 1;
}


int setauclinfo(char *arg){
  char this[] = "setauclinfo";
  int argc;
  char *argv[10];
  int cln, fl;
  int sock, ret;
  int com, efn;
  enum comlist {ADD, DEL};


  argc = striparg(arg, argv);

  if(argc != 2 && argc !=3){
    synopsis(this);
    return 0;
  }


  fl = 1;
  cln = 0;
  efn = 0;

  ret = 0;

  efn = strtol(argv[0], NULL, 0);
  if(efn < 0 || efn >= MAXEF){
    printf("0 < EFN < %d\n", MAXEF);
    return 0;
  }


  if((com = chkcom(argv[1], comsetauclinfo)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  sock = infcon(ebhostname);
  eb_get(sock, AU_GET_CLINFO, (char *)auclinfo);
  close(sock);

  switch(com){
  case ADD:
    if(argc != 3){
      fl = 0;
    }else{
      cln = strtol(argv[2], NULL, 0);
      if(cln < 0 || cln >= MAXCLI){
	printf("0 < CLIN < %d\n", MAXCLI);
	fl = 0;
      }else{
	auclinfo[efn] = cln;
      }
    }
    break;
  case DEL:
    auclinfo[efn] = -1;
    break;
  }
    
  if(fl){
    sock = infcon(ebhostname);
    ret = eb_set(sock, AU_SET_CLINFO, (char *)auclinfo,
		   sizeof(auclinfo));
    close(sock);
    if(!ret){
      printf("Run is now running, can't change parameters\n");
    }

    getauclinfo();
  }else{
    synopsis(this);
  }
  

  return 1;
}

int getauclinfo(char *arg){
  //char this[] = "getauclinfo";
  int sock, i;

  sock = infcon(ebhostname);
  eb_get(sock, AU_GET_CLINFO, (char *)auclinfo);
  close(sock);

  printf("\n");
  cprintf(FG_BLACK|BG_CYAN, "AUEB UDP client definition\n");
  for(i=0;i<MAXEF;i++){
    if(auclinfo[i] != -1){
      printf(" EFN[%d] -> Client ID[%d]\n", i, auclinfo[i]);
    }
  }
  printf("\n");
  return 1;
}

int getscrdata(int id){
  int len, com, sock;
  int idx, tid, i;

  if(!(sock = infcon(ebhostname))) return 0;
  eb_get(sock, INF_GET_SCRLIST, (char *)buff);
  memcpy((char *)&scranan, buff, sizeof(scranan));
  memcpy((char *)scrana, buff+sizeof(scranan), sizeof(scrana));
  close(sock);

  if(!(sock = infcon(ebhostname))) return 0;
  com = INF_GET_SCRDATA;
  len = sizeof(com) + sizeof(id);
  memcpy(buff, (char *)&com, sizeof(com));
  memcpy(buff+sizeof(com), (char *)&id, sizeof(id));

  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, buff, len, 0);

  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);


  memcpy((char *)&tid, buff, sizeof(tid));
  if(tid < 0){
    return 0;
  }

  idx = sizeof(int);
  memcpy((char *)&scrana[tid].classid, buff+idx, sizeof(int));
  idx += sizeof(int);
  memcpy((char *)&scrana[tid].scrid, buff+idx, sizeof(int));
  idx += sizeof(int);
  memcpy((char *)&scrana[tid].scrn, buff+idx, sizeof(int));
  idx += sizeof(int);
  memcpy((char *)&scrana[tid].ratech, buff+idx, sizeof(int));
  idx += sizeof(int);
  memcpy((char *)&scrana[tid].rate, buff+idx, sizeof(int));
  idx += sizeof(int);
  memcpy(scrana[tid].idname, buff+idx, sizeof(scrana[tid].idname));
  idx += sizeof(scrana[tid].idname);

  if(!scr[tid]){
    scr[tid] = (struct ridf_scr_contst *)
      malloc(sizeof(struct ridf_scr_contst)*scrana[tid].scrn);
  }

  for(i=0;i<scrana[tid].scrn;i++){
    memcpy((char *)&scr[tid][i], buff+idx, sizeof(scr[tid][i]));
    idx += sizeof(scr[tid][id]);
  }

  return 1;
}

int getscrlist(char *arg){
  int sock, id;

  if(arg){
    if(strlen(arg)){
      id = strtol(arg, NULL, 0);
      if(id < 0){
	printf("0 =< scrid\n");
	return 0;
      }
      if(getscrdata(id)){
	printf("before show_scrdata\n");
	show_scrdata(id);
	return 1;
      }
    }
  }

  if(!(sock = infcon(ebhostname))) return 0;
  eb_get(sock, INF_GET_SCRLIST, (char *)buff);
  memcpy((char *)&scranan, buff, sizeof(scranan));
  memcpy((char *)scrana, buff+sizeof(scranan), sizeof(scrana));
  close(sock);

  printf("scranan %d\n", scranan);

  show_scrlist();

  return 1;
}

int setscrname(char *arg){
  int id, tdx = -1, newf = 0, tn, i, yes = 0;
  int sock, len;
  char *line;
  char pr[128];

  if(!strlen(arg)){
    printf("setscrname SCRID\n");
    return 0;
  }

  id = strtol(arg, NULL, 0);
  if(id < 0){
    printf("0 < scrid\n");
    return 0;
  }

  printf("Scaler id = %d\n", id);
  getscrlist(NULL);

  tdx = fndscrid(id);
  DB(printf("tdx = %d\n", tdx));
  if(tdx != -1){
    getscrdata(id);
    sprintf(pr, "ID name [%s] : ", scrana[tdx].idname);
  }else{
    sprintf(pr, "ID name : ");
    tdx = scranan;
    newf = 1;
    if(tdx == MAXSCRANA){
      printf("Too many scaler ananlysis (max=%d)\n", MAXSCRANA);
      return 0;
    }
  }

  line = readline(pr);
  if(strlen(line)){
    strncpy(scrana[tdx].idname, line, sizeof(scrana[tdx].idname));
  }
  free(line);

  if(newf){
    line = readline("Clear scaler ? [y(=def)/n] : ");
    if(line[0] == 'n'){
      scrana[tdx].classid = RIDF_SCALER;
    }else{
      scrana[tdx].classid = RIDF_CSCALER;
    }
    free(line);
    line = readline("How many channels ? : ");
    tn = strtol(line, NULL, 0);
    free(line);
    if(tn < 1){
      printf("0 < channel number\n");
      return 0;
    }
    scrana[tdx].scrid = id;
    scrana[tdx].scrn = tn;
    scr[tdx] = (struct ridf_scr_contst *)
      malloc(sizeof(struct ridf_scr_contst)*scrana[tdx].scrn);
  }

  sprintf(pr, "Rate Ch [%d] : ", scrana[tdx].ratech);
  line = readline(pr);
  if(strlen(line)){
    scrana[tdx].ratech = strtol(line, NULL, 0);
  }
  free(line);
  sprintf(pr, "Rate [%d]cps : ", scrana[tdx].rate);
  line = readline(pr);
  if(strlen(line)){
    scrana[tdx].rate = strtol(line, NULL, 0);
  }
  free(line);


  for(i=0;i<scrana[tdx].scrn;i++){
    if(!newf){
      sprintf(pr, "SCR%d [%s] : ", i, scr[tdx][i].name);
    }else{
      sprintf(pr, "SCR%d : ", i);
    }
    line = readline(pr);
    if(strlen(line)){
      strncpy(scr[tdx][i].name, line, sizeof(scr[tdx][i]));
    }
    free(line);
  }

  sprintf(pr, "Set scrid=%d [y/n] ? : ", id);
  yes = 0;
  while(1){
    line = readline(pr);
    if(strlen(line)){
      if(line[0] == 'y'){
	yes = 1;
	break;
      }else if(line[0] == 'n'){
	yes = 0;
	break;
      }else{
	printf(" y or n \n");
      }
    }
    free(line);
  }
  if(yes){
    printf("Updated scaler name\n");
    memset(buff, 0, sizeof(buff));
    memcpy(buff, (char *)&scrana[tdx].classid, sizeof(int));
    len = sizeof(int);
    memcpy(buff+len, (char *)&scrana[tdx].scrid, sizeof(int));
    len += sizeof(int);
    memcpy(buff+len, (char *)&scrana[tdx].scrn, sizeof(int));
    len += sizeof(int);
    memcpy(buff+len, (char *)&scrana[tdx].ratech, sizeof(int));
    len += sizeof(int);
    memcpy(buff+len, (char *)&scrana[tdx].rate, sizeof(int));
    len += sizeof(int);
    memcpy(buff+len, scrana[tdx].idname, sizeof(scrana[tdx].idname));
    len += sizeof(scrana[tdx].idname);
    for(i=0;i<scrana[tdx].scrn;i++){
      memcpy(buff+len, scr[tdx][i].name, sizeof(scr[tdx][i].name));
      len += sizeof(scr[tdx][i].name);
    }
      if(!(sock = infcon(ebhostname))) return 0;
      eb_set(sock, INF_SET_SCRNAME, buff, len);
  }else{
    printf("Canceled\n");
  }

  return 1;
}

int getesconfig(char *arg){
  int sock, len, com;
  int chk;
  fd_set fdset;
  struct timeval timeout;

  DB(printf("babicon: getesconfig %s\n", arg));

  if(!(sock = escon(arg))) return 0;
  
  com = ES_GET_CONFIG;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, len, 0);
  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  if((chk = select(sock+1, &fdset, NULL, NULL, &timeout))){
    recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
    recv(sock, (char *)&efrc, len, MSG_WAITALL);
  }else{
    printf("No reply from EFN=%s\n", arg);
    close(sock);
    return 0;
  }
  close(sock);

  show_efrc(efrc);

  return 1;
}

int esquit(char *arg){
  int sock, len, com;
  DB(printf("babicon: esquit %s\n", arg));

  if(!(sock = escon(arg))) return 0;
  
  com = ES_QUIT;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, len, 0);
  close(sock);

  return 1;
}

int setesconfig(char *arg){
  int sock, len, com, ret;
  char *rarg[80];
  int ar, fl;

  fl = 1;
  com = 0;
  DB(printf("babicon: setesconfig %s\n", arg));
  ar = striparg(arg, rarg);
  if(ar != 3) fl = 0;

  if(!getesconfig(rarg[0])){
    return 0;
  }

  if(ar == 3){
    if(!strcmp(rarg[1], "hd1")){
      if(!strcmp(rarg[2], "on")){
	efrc.hd1 = 1;
      }else if(!strcmp(rarg[2], "off")){
	efrc.hd1 = 0;
      }else{
	fl = 0;
      }
    }else if(!strcmp(rarg[1], "hd2")){
      if(!strcmp(rarg[2], "on")){
	efrc.hd2 = 1;
      }else if(!strcmp(rarg[2], "off")){
	efrc.hd2 = 0;
      }else{
	fl = 0;
      }
    }else if(!strcmp(rarg[1], "mt")){
      if(!strcmp(rarg[2], "on")){
	efrc.mt = 1;
      }else if(!strcmp(rarg[2], "off")){
	efrc.mt = 0;
      }else{
	fl = 0;
      }
    }else if(!strcmp(rarg[1], "hd1dir")){
      strncpy(efrc.hd1dir, rarg[2], sizeof(efrc.hd1dir)-1);
    }else if(!strcmp(rarg[1], "hd2dir")){
      strncpy(efrc.hd2dir, rarg[2], sizeof(efrc.hd2dir)-1);
    }else if(!strcmp(rarg[1], "rtdrv")){
      strncpy(efrc.mtdir, rarg[2], sizeof(efrc.mtdir)-1);
    }else if(!strcmp(rarg[1], "host")){
      strncpy(efrc.erhost, rarg[2], sizeof(efrc.erhost)-1);
    }
  }

  printf("\n");

  if(fl){
    if(!(sock = escon(rarg[0]))) return 0;
  
    com = ES_SET_CONFIG;
    len = sizeof(com) + sizeof(efrc);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&com, sizeof(com), 0);
    send(sock, (char *)&efrc, sizeof(efrc), 0);
    recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
    recv(sock, (char *)&ret, len, MSG_WAITALL);
    close(sock);

    getesconfig(rarg[0]);
    
  }else{
    printf("setesconfig EFID COMMAND VALUE\n");
    printf("COMMAND : host  EBHOSTNAME\n");
    printf("        : hd1 on/off\n");
    printf("        : hd2 on/off\n");
    printf("        : mt on/off\n");
    printf("        : hd1dir PATH\n");
    printf("        : hd2dir PATH\n");
    printf("        : rtdrv  PATH\n");

    return 0;
  }


  return 1;
}

int reloadesdrv(char *arg){
  int sock, len, com;
  char *rarg[80];

  striparg(arg, rarg);
  com = 0;
  DB(printf("babicon: setesconfig %s\n", arg));

  get_runinfo();
  if(runinfo.runstat){
    printf("Run is now running, can't change parameters\n");
    return 0;
  }

  if(!(sock = escon(rarg[0]))) return 0;
  com = ES_RELOAD_DRV;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(com), 0);
  close(sock);

  getesconfig(rarg[0]);
  show_efrc(efrc);

  return 1;
}


void show_evtn(char *arg){
  int i, j, t, e;

  printf("Last event number\n");

  //printf("evtnidx = %d / %d\n", evtnidx, nofef);

  printf("%3s %-10s %s\n","EFN","Nickname","EVTN");
  for(i=0; i<MAXEF; i++){
    if(daqinfo.eflist[i].ex && daqinfo.eflist[i].of == EB_EFLIST_ON){
      printf("%3d %-10s %d\n", i, daqinfo.eflist[i].name, evtn[i]);
    }
  }

  
  if(evtnidx <= nofef*3){
    // noop
  }else{
    // show babildes breakdown
    printf("\n\n");
    printf("-- babildes tree --\n");
    t = 0;
    /*
      for(i=0;i<evtnidx;i++){
      printf("%d\n", evtnbuff[i]);
      }
    */
    for(i=0;i<evtnidx;i++){
      e = evtnbuff[i];
      switch(e){
      case 0:
	// noop
	break;
      case -1:
	t +=2;
	continue;
      case -2:
	t -= 2;
	break;
      case -3:
	t = 0;
	break;
      default:
	// noop
	break;
      }
      if(t < 0){
	printf("show_evtn indent error\n");
	t = 0;
      }
      
      for(j=0;j<t;j++){
	printf(" ");
      }
      if(t>0){
	printf("|-");
      }
      printf("%3d : %u\n", evtnbuff[i+1], evtnbuff[i+2]);
      i+=2;
    }
  }

}

int getevtn(char *arg){
  int sock, len, com, i;
  char chefn[80];
  int x = 0;
  int tbuff[1024];

  DB(printf("babicon: getevtn %s\n", arg));
  memset((char *)evtnbuff, 0, sizeof(evtnbuff));
  evtnidx = 0;

  nofef = 0;
  for(i=0;i<MAXEF;i++){
    if(daqinfo.eflist[i].ex && daqinfo.eflist[i].of == EB_EFLIST_ON){
      memset(chefn, 0, sizeof(chefn));
      sprintf(chefn, "%d", i);
      if(!(sock = escon(chefn))) return 0;
      com = ES_GET_EVTN;
      len = sizeof(com);
      send(sock, (char *)&len, sizeof(len), 0);
      send(sock, (char *)&com, len, 0);
      recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
      recv(sock, (char *)tbuff, len, MSG_WAITALL);
      evtn[i] = tbuff[0];
      close(sock);
      if(x > 4){
	evtnbuff[evtnidx] = -3;
      }else{
	evtnbuff[evtnidx] = 0;
      }
      evtnidx ++;
      evtnbuff[evtnidx] = i;
      evtnidx ++;
      memcpy((char *)(evtnbuff + evtnidx), (char *)tbuff, len);
      evtnidx += len/4;
      x = len;
      nofef ++;
    }
  }

  show_evtn(NULL);
  
  return 1;
}

int getesevtnumber(char *arg){
  getevtn(NULL);

  return 1;
}

int whoareyou(char *arg){
  int sock, len, com;

  DB(printf("babicon: whoareyou %s\n", arg));

  if(!(sock = escon(arg))) return 0;
  
  com = WHOAREYOU;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, len, 0);
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, (char *)&esname, len, MSG_WAITALL);
  close(sock);

  printf("EF[%s] = %s\n", arg, esname);

  return 1;
}

int getebname(void){
  int sock;

  sock = ebcon(ebhostname);
  eb_get(sock, WHOAREYOU, ebname);
  close(sock);

  return 1;
}


int wth(char *arg){
  char *line;
  int sock;

  DB(printf("babicon: wth %s\n", arg));

  get_runinfo();
  if(!runinfo.runstat){
    if(!strlen(arg)){
      line = readline("Header : ");
      strncpy(runinfo.header, line, sizeof(runinfo.header)-1);
      free(line);
    }else{
      strncpy(runinfo.header, arg, sizeof(runinfo.header)-1);
    }
  }else{
    printf("Now running...\n");
    return 0;
  }

  sock = ebcon(ebhostname);
  eb_set(sock, EB_SET_HEADER, runinfo.header,
	       sizeof(runinfo.header));
  close(sock);


  return 1;
}

int shutdownall(char *arg){
  int sock, ret;
  char *line;

  DB(printf("babicon: shutdown %s\n", arg));

  line = readline("Shutdown all ? [y/n] : ");
  if(line[0] == 'y'){
    sock = infcon(ebhostname);
    eb_get(sock, INF_QUIT, (char *)&ret);
    close(sock);
    
    sock = ebcon(ebhostname);
    eb_get(sock, EB_QUIT, (char *)&ret);
    close(sock);

    printf("Shutdown\n");
  }else{
    printf("Canceled\n");
  }
  free(line);

  return 0;
}

int esconnect(char *arg){
  int sock, len, com;

  DB(printf("babicon: esconnect %s\n", arg));
  if(!(sock = escon(arg))) return 0;
  com = ES_CON_EFR;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, len, 0);
  close(sock);

  return 1;
}

int esdisconnect(char *arg){
  int sock, len, com;

  DB(printf("babicon: esdisconnect %s\n", arg));

  if(!(sock = escon(arg))) return 0;
  com = ES_DIS_EFR;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, len, 0);
  close(sock);

  return 1;
}

int main(int argc,char *argv[]){
  char *com;
  char *line;
  char prompt[64];
  int i;

  cl_screen();

  /* Signal SIGINT -> quit() */
  signal(SIGINT,(void *)quit);

  /* Get environmnet */
  if(argc == 2){
    strncpy(ebhostname, argv[1], sizeof(ebhostname)-1);
  }else if(getenv("BABILD") != NULL){
    strncpy(ebhostname, getenv("BABILD"), sizeof(ebhostname)-1);
  }else{
    strcpy(ebhostname, "localhost");
  }

  cprintf(FG_WHITE|BG_BLUE, "babicon");
  printf("\n");

  getebname();
  cprintf(FG_MAGENTA, "*");
  printf(" %s = %s\n", ebhostname, ebname);


  /* Initialize commands */
  init_hcom(commands);
  /* Initialize prompt */
  csprintf(prompt, AT_BOLD|FG_BLUE, "%s>", ebhostname);
  strcat(prompt," ");


  for(i=0;i<MAXSCRANA;i++){
    scr[i] = NULL;
  }

  get_runinfo();
  getconfig(NULL);
  getstatcom(NULL);
  getdbcon(NULL);
  getclinfo(NULL);
  if(dbcon.of){
    cprintf(FG_BLUE, "\n*** Run information will be stored into DB ***\n");
    dbgetexpid(NULL);
    dbgetdaqname(NULL);
    printf("\n\n");
  }


  /* Main loop */
  while(1){
    line = readline(prompt);
    if(!line){
      break;
    }
    com = stripwhite(line);
    if(*com){
      add_history(com);
      execute_line(com, commands);
    }
    free(line);
  }

  quit();

  return 0;
}
