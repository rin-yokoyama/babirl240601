#include <bbxml.h>

static BBXMLEL *xmlel = NULL;
/* common functions for babicon and babicmd */
int infcon(char *host){
  int infsock;

  /* Connect to babild */
  if(!(infsock = mktcpsend(host, INFCOMPORT))){
    printf("babicon: Can't connet to babinfo.\n");
    exit(0);
    return 0;
  }
  
  return infsock;
}

int ebcon(char *host){
  int ebsock;

  /* Connect to babild */
  if(!(ebsock = mktcpsend(host, EBCOMPORT))){
    printf("babicon: Can't connet to babild.\n");
    exit(0);
    return 0;
  }
  
  return ebsock;
}

int chkcom(char *com, char **comlist){
  int ret = -1, i;

  i = 0;
  while(comlist[i]){
    if(!strcmp(com, comlist[i])){
      ret = i/3;
      break;
    }
    i += 3;
  }

  return ret;
}

int fndscrid(int id){
  int ret, i;

  ret = -1;
  for(i=0;i<scranan;i++){
    if(scrana[i].scrid == id){
      ret = i;
    }
  }

  return ret;
}



int exec_psh(char *arg){
  FILE *pfp;
  char buff[1024];

  memset(buff, 0, sizeof(buff));

  if(strlen(arg) <= 0) return 0;

  if((pfp = popen(arg, "r")) == NULL){
    printf("Can't exec %s\n", arg);
    return 0;
  }else{
    fread(buff, 1, sizeof(buff)-1, pfp);
    if(pclose(pfp) < 0){
      printf("Unknown error : %s", arg);
      return 0;
    }
    printf("%s", buff);
    if(buff[0] == 'O' && buff[1] == 'K'){
      // OK
      return 1;
    }else{
      // Error
      return 0;
    }
  }
}

int exec_nsh(char *arg){
  char buff[1024];

  memset(buff, 0, sizeof(buff));
  sprintf(buff, "%s &", arg);
  system(buff);

  return 1;
}


int pstart(char *arg){
  if(strlen(arg) <= 0){
    printf("pstart COMMAND\n");
    return 0;
  }

  if(!exec_nsh(arg)){
    return 0;
  }else{
    start();
  }

  return 0;
}

int pnssta(char *arg){
  if(strlen(arg) <= 0){
    printf("pnssta COMMAND\n");
    return 0;
  }

  if(!exec_nsh(arg)){
    return 0;
  }else{
    nssta();
  }

  return 0;
}

int pstop(char *arg){
  if(strlen(arg) <= 0){
    printf("pstop COMMAND\n");
    return 0;
  }

  if(!exec_psh(arg)){
    return 0;
  }else{
    stop();
  }

  return 0;
}

void show_hdlist(void){
  int i;
  float avail;

  cprintf(FG_BLACK|BG_GREEN, "HD list\n");
  for(i=0; i<MAXHD; i++){
    if(daqinfo.hdlist[i].ex){
      printf("%2d %-25s (%2s)", i, daqinfo.hdlist[i].path,
	     ofstr[daqinfo.hdlist[i].of]);
      if(daqinfo.hdlist[i].of == 1){
	printf(" ");
      }
      avail = (float)(daqinfo.hdlist[i].free)/1024./1024./1024.;

      if(avail < 10.0){
	printf("   %3.1fGB free\n", avail);
      }else if(avail < 1000.){
	avail = avail;
	printf("   %3.0fGB free\n", avail);
      }else if(avail < 10000.){
	avail = avail / 1024.;
	printf("   %3.1fTB free\n", avail);
      }else{
	avail = avail / 1024.;
	printf("   %3.0fTB free\n", avail);
      }
    }
  }
  printf("\n");
  gettgig();

}


int show_scrlive(void){
  printf("Scaler live time information\n");
  printf("   GATED id=%3d ch=%2d\n", scrlive.gatedid, scrlive.gatedch);
  printf(" UNGATED id=%3d ch=%2d\n", scrlive.ungatedid, scrlive.ungatedch);
  printf("\n");

  return 1;
}

int show_efblkn(unsigned int blkn[MAXEF]){
  int i;
  for(i=0;i<MAXEF;i++){
    if(blkn[i]){
      printf("EFN[%3d] : %u\n", i, blkn[i]);
    }
  }
  return 1;
}

int getefblkn(char *arg){
  int sock;
  unsigned int blkn[MAXEF];

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_EFBLKN, (char *)blkn);
  close(sock);

  show_efblkn(blkn);

  return 1;
}


int setscrlive(char *arg){
  char this[] = "setscrlive";
  int argc;
  char *argv[10];
  int sock, ret;
  int com, tid, tch;
  enum comlist {GATED, UNGATED};

  argc = striparg(arg, argv);

  if(argc != 3){
    synopsis(this);
    return 0;
  }


  ret = 0;
  
  if((com = chkcom(argv[0], comsetscrlive)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_SCRLIVE, (char *)&scrlive);
  close(sock);
  
  tid = strtol(argv[1], NULL, 0);
  tch = strtol(argv[2], NULL, 0);

  switch(com){
  case GATED:
    scrlive.gatedid = tid;
    scrlive.gatedch = tch;
    break;
  case UNGATED:
    scrlive.ungatedid = tid;
    scrlive.ungatedch = tch;
    break;
  }

  sock = infcon(ebhostname);
  ret = eb_set(sock, INF_SET_SCRLIVE, (char *)&scrlive,
	 sizeof(scrlive));
  close(sock);
  if(!ret){
    printf("setscrlive, faled\n");
  }

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_SCRLIVE, (char *)&scrlive);
  close(sock);

  show_scrlive();

  return 1;
}


int getscrlive(char *arg){
  int sock;

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_SCRLIVE, (char *)&scrlive);
  close(sock);

  show_scrlive();

  return 1;
}

void show_statcom(void){
  cprintf(FG_BLACK|BG_RED, "Run status command\n");
  printf("On/Off: %s\n", ofstr[statcom.of]);
  printf("Start : %s\n", statcom.start);
  printf("Stop  : %s\n", statcom.stop);
  printf("\n");
}

int setstatcom(char *arg){
  int sock, argc;
  char *argv[10];
  char this[] = "setstatcom";
  enum comlist {ON, OFF, START, STOP};
  char ch1[80];
  int ret;
  int com;
  char *line;

  argc = striparg(arg, argv);
  if(argc != 1 && argc != 2){
    synopsis(this);
    return 0;
  }

  ret = 0;

  if((com = chkcom(argv[0], comstatcom)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }
  memset(ch1, 0, sizeof(ch1));
  switch(com){
  case START:
  case STOP:
      line = readline("PATH : ");
      strncpy(ch1, line, sizeof(ch1)-1);
      free(line);
      break;
  }

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_STAT_COMMAND, (char *)&statcom);
  close(sock);


  switch(com){
  case ON:
    statcom.of = 1;
    break;
  case OFF:
    statcom.of = 0;
    break;
  case START:
    strncpy(statcom.start, ch1, sizeof(statcom.start));
    break;
  case STOP:
    strncpy(statcom.stop, ch1, sizeof(statcom.stop));
    break;
  }

  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_STAT_COMMAND, (char *)&statcom, sizeof(statcom));
  close(sock);
  if(!ret){
    printf("Run is now running, can't change parameters\n");
  }

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_STAT_COMMAND, (char *)&statcom);
  close(sock);

  show_statcom();

  return 1;
}

int getstatcom(char *arg){
  int sock;

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_STAT_COMMAND, (char *)&statcom);
  close(sock);
  show_statcom();

  return 1;
}

int gettcpclinfo(char *arg){
  int sock;

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_TCPCLIHOSTS, (char *)&tcpclinfo);
  close(sock);
  show_tcpclinfo();

  return 1;
}


// DB connection
void show_dbcon(void){
  cprintf(FG_BLACK|BG_MAGENTA, "DB connection\n");
  printf("On/Off : %s\n", ofstr[dbcon.of]);
  printf("DBHost : %s\n", dbcon.host);
  printf("DBName : %s\n", dbcon.dbname);
  printf("DBUser : %s\n", dbcon.user);
  printf("DBPass : *******\n");
  printf("\n");
}

int setdbcon(char *arg){
  int sock, argc;
  char *argv[10];
  char this[] = "setdbcon";
  enum comlist {ON, OFF, SET};
  char ch1[80], ch2[80], ch3[80], ch4[80], prop[128];
  int ret;
  int com;
  char *line;

  argc = striparg(arg, argv);
  if(argc != 1 && argc != 5){
    synopsis(this);
    return 0;
  }

  ret = 0;

  if((com = chkcom(argv[0], comdbcon)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_DBCON, (char *)&dbcon);
  close(sock);

  memset(ch1, 0, sizeof(ch1));
  memset(ch2, 0, sizeof(ch2));
  memset(ch3, 0, sizeof(ch3));
  memset(ch4, 0, sizeof(ch4));

  switch(com){
  case SET:
    if(argc == 1){
      sprintf(prop, "HOST [def=%s] : ", dbcon.host);
      line = readline(prop);
      strncpy(ch1, line, sizeof(ch1)-1);
      free(line);

      sprintf(prop, "DBNAME [def=%s] : ", dbcon.dbname);
      line = readline(prop);
      strncpy(ch2, line, sizeof(ch2)-1);
      free(line);

      sprintf(prop, "DBUSER [def=%s] : ", dbcon.user);
      line = readline(prop);
      strncpy(ch3, line, sizeof(ch3)-1);
      free(line);

      sprintf(prop, "DBPASS [def=*****] : ");
      line = readline(prop);
      strncpy(ch4, line, sizeof(ch4)-1);
      free(line);
    }else{
      strncpy(ch1, argv[2], sizeof(ch1)-1);
      strncpy(ch2, argv[3], sizeof(ch2)-1);
      strncpy(ch3, argv[4], sizeof(ch3)-1);
      strncpy(ch4, argv[5], sizeof(ch4)-1);
    }
    break;
  }


  switch(com){
  case ON:
    dbcon.of = 1;
    break;
  case OFF:
    dbcon.of = 0;
    break;
  case SET:
    if(strlen(ch1) > 1){
      strncpy(dbcon.host, ch1, sizeof(dbcon.host));
    }
    if(strlen(ch2) > 1){
      strncpy(dbcon.dbname, ch2, sizeof(dbcon.dbname));
    }
    if(strlen(ch3) > 1){
      strncpy(dbcon.user, ch3, sizeof(dbcon.user));
    }
    if(strlen(ch4) > 1){
      strncpy(dbcon.passwd, ch4, sizeof(dbcon.passwd));
    }
    break;
  }

  sock = infcon(ebhostname);
  ret = eb_set(sock, INF_SET_DBCON, (char *)&dbcon, sizeof(dbcon));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameters\n");
  }

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_DBCON, (char *)&dbcon);
  close(sock);

  show_dbcon();

  return 1;
}

int getchkerhost(char *arg){
  int chkerhost;
  int sock;

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_CHKERHOST, (char *)&chkerhost);
  close(sock);

  printf("CHKERHOST = %s\n", ofstr[chkerhost]);

  return 1;
}

int getdbcon(char *arg){
  int sock;

  sock = infcon(ebhostname);
  eb_get(sock, INF_GET_DBCON, (char *)&dbcon);
  close(sock);

  show_dbcon();

  return 1;
}

int chkdbcon(char *arg){
  int sock, ret;

  sock = infcon(ebhostname);
  eb_get(sock, INF_CHK_DBCON, (char *)&ret);
  close(sock);

  if(ret){
    cprintf(FG_BLUE, "Connection OK\n");
  }else{
    cprintf(FG_MAGENTA, "Connection NG\n");
  }

  return 1;
}

void show_clinfo(void){
  int i;
  cprintf(FG_BLACK|BG_CYAN, "UDP Client list\n");
  printf("ID  : HOSTNAME\n");
  for(i=0;i<MAXCLI;i++){
    if(clinfo[i].ex){
      printf("%3d : %s (SHMID = %d)\n", i, clinfo[i].clihost, clinfo[i].ex-1);
    }
  }
}


void show_tcpclinfo(void){
  int i;
  cprintf(FG_BLACK|BG_CYAN, "TCP Client list\n");
  printf("ID  : HOSTNAME\n");
  for(i=0;i<MAXTCPCLI;i++){
    if(tcpclinfo[i].port){
      printf("%3d : %s (port = %d) ", i, tcpclinfo[i].host, tcpclinfo[i].port);
      if(tcpclinfo[i].sock){
	printf("Available\n");
      }else{
	printf("No connection\n");
      }
    }
  }
}


int dbsetexpid(char *arg){
  char this[] = "dbsetexpid";
  int argc, expid;
  char *argv[10];
  char com[1024];
  BBXMLEL *exp=NULL, *id=NULL, *er=NULL, *name=NULL, *babinfo=NULL;
  argc = striparg(arg, argv);
  if(argc != 1){
    synopsis(this);
    return 0;
  }

  expid = strtol(arg, NULL, 0);
  memset(com, 0, sizeof(com));
  sprintf(com, "<dbsetexp><id>%d</id></dbsetexp>\n", expid);
  babinfoxcom(com);

  DB(bbxml_printall(xmlel));
  
  if(!(babinfo = bbxml_node(xmlel, "babinfo"))){
    printf("Error xml, this is not babinfo xml\n");
  }

  if(!(exp = bbxml_node(babinfo, "exp"))){
    printf("Error in xml\n");
  }

  if((er = bbxml_node(exp, "error"))){
    printf("Error : %s\n", er->text);
  }
  
  if(!(id = bbxml_node(exp, "id"))){
    printf("No ExpID\n");
  }else{
    printf(" ExpID = %s\n", id->text);
    if((name = bbxml_node(exp, "name"))){
      printf(" ExpName = %s", name->text);
    }
    printf("\n");
  }
  

  return 1;
}

int dbgetexpid(char *arg){
  BBXMLEL *exp=NULL, *id=NULL, *er=NULL, *name=NULL, *babinfo=NULL;
  char com[1024];
  memset(com, 0, sizeof(com));
  sprintf(com, "<dbgetexp/>");
  babinfoxcom(com);

  if(!(babinfo = bbxml_node(xmlel, "babinfo"))){
    printf("Error xml, this is not babinfo xml\n");
  }

  if(!(exp = bbxml_node(babinfo, "exp"))){
    printf("Error in xml\n");
  }

  if((er = bbxml_node(exp, "error"))){
    printf("Error : %s\n", er->text);
  }
  
  if(!(id = bbxml_node(exp, "id"))){
    printf("No ExpID\n");
  }else{
    printf(" ExpID = %s\n", id->text);
    if((name = bbxml_node(exp, "name"))){
      printf(" ExpName = %s", name->text);
    }
    printf("\n");
  }
  
  return 1;
}

int dbgetscr(char *arg){
  char com[1024];
  memset(com, 0, sizeof(com));
  sprintf(com, "<dbgetscr/>");
  babinfoxcom(com);

  return 1;
}

int dbsetscr(char *arg){
  char com[1024];
  memset(com, 0, sizeof(com));
  sprintf(com, "<dbsetscr/>");
  babinfoxcom(com);

  return 1;
}

void babinfoxcom(char *com){
  char buff[EB_BUFF_SIZE];
  int idx = 0, len;
  int sock;

  if(xmlel) bbxml_free(xmlel);

  len = strlen(com);
  memset(buff, 0, sizeof(buff));
  idx = sprintf(buff, "<?xml version=\"1.0\"?>\n");
  idx += sprintf(buff+idx, "<babinfoxcom>\n");
  memcpy(buff+idx, com, len);
  idx += len;
  idx += sprintf(buff+idx, "\n</babinfoxcom>\n");

  DB(printf("%s\n", buff));

  sock = infcon(ebhostname);
  send(sock, (char *)&idx, sizeof(idx), 0);
  send(sock, buff, idx, 0);
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);
  close(sock);
  buff[len] = 0;
  
  xmlel = bbxml_parsebuff(buff, len);
}

int setchkerhost(char *arg){
  char *line, str[80];
  char *argv[10];
  int chkerhost, argc, sock, ret;

  DB(printf("babicon: setsetchkerhost %s\n", arg));

  argc = striparg(arg, argv);
  if(argc == 1){
    strcpy(str, argv[0]);
  }else{
    line = readline("Set check event receiver's host on/off: ");
    strcpy(str, line);
    free(line);
  }

  if(strcmp(str, "on") == 0){
    chkerhost = 1;
  }else if(strcmp(str, "off") == 0){
    chkerhost = 0;
  }else{
    printf("setchkerhost on/off\n");
    return 0;
  }
    
  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_CHKERHOST, (char *)&chkerhost, sizeof(chkerhost));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameter of chkerhost\n");
    return 0;
  }

  getchkerhost(NULL);

  return 1;
}

int dbgetdaqname(char *arg){
  //char this[] = "dbgetdaqname";
  BBXMLEL *exp=NULL, *id=NULL, *er=NULL, *babinfo=NULL;
  char com[1024];
  memset(com, 0, sizeof(com));
  sprintf(com, "<dbgetdaqname/>");
  babinfoxcom(com);

  if(!(babinfo = bbxml_node(xmlel, "babinfo"))){
    printf("Error xml, this is not babinfo xml\n");
  }

  if(!(exp = bbxml_node(babinfo, "daq"))){
    printf("Error in xml\n");
  }

  if((er = bbxml_node(exp, "error"))){
    printf("Error : %s\n", er->text);
  }
  
  if(!(id = bbxml_node(exp, "name"))){
    printf("No Name\n");
  }else{
    printf(" DAQName = %s", id->text);
    printf("\n");
  }
  return 1;
}

int dbsetdaqname(char *arg){
  //char this[] = "dbsetdaqname";
  int argc;
  char *argv[10], *line;
  char str[64];
  char com[1024];
  BBXMLEL *exp=NULL, *babinfo=NULL, *er=NULL;

  argc = striparg(arg, argv);
  if(argc == 1){
    strncpy(str, argv[0], sizeof(str)-1);
  }else{
    line = readline("Enter DAQ Name: ");
    strncpy(str, line, sizeof(str)-1);
    free(line);
  }

  memset(com, 0, sizeof(com));
  sprintf(com, "<dbsetdaqname><name>%s</name><server>%s</server></dbsetdaqname>", str, ebhostname);
  babinfoxcom(com);

  //bbxml_printall(xmlel);

  if(!(babinfo = bbxml_node(xmlel, "babinfo"))){
    printf("Error xml, this is not babinfo xml\n");
    return 0;
  }

  if(!(exp = bbxml_node(babinfo, "daq"))){
    printf("Error in xml\n");
    return 0;
  }

  if((er = bbxml_node(exp, "error"))){
    printf("Error : %s\n", er->text);
    return 0;
  }
  
  dbgetdaqname(NULL);

  return 1;
}


int setclinfo(char *arg){
  char this[] = "setclinfo";
  int argc;
  char *argv[10];
  int cln, fl;
  char ch1[80];
  int sock, ret;
  int com, upf, id;
  enum comlist {ADD, DEL, ID};

  argc = striparg(arg, argv);

  if(argc != 2 && argc != 3){
    synopsis(this);
    return 0;
  }


  fl = 1;
  cln = 0;
  ret = 0;
  upf = 1;
  id = 0;

  cln = strtol(argv[0], NULL, 0);
  if(cln < 0 || cln >= MAXCLI){
    printf("0 < CLIN < %d\n", MAXCLI);
    return 0;
  }

  if((com = chkcom(argv[1], comsetclinfo)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  switch(com){
  case ADD:
    if(argc != 3){
      fl = 0;
    }else{
      memset(ch1, 0, sizeof(ch1));
      strncpy(ch1, argv[2], sizeof(ch1)-1);
    }
    break;
  case ID:
    if(argc != 3){
      fl = 0;
    }else{
      id = strtol(argv[2], NULL, 0);
      if(id < 0 || id >= MAXANPT){
	printf("Error : 0 <= ID < %d\n\n", MAXANPT);
	fl = 0;
      }
    }
    break;
  }
    
  if(fl){
    sock = infcon(ebhostname);
    eb_get(sock, INF_GET_CLIHOSTS, (char *)clinfo);
    close(sock);
    
    switch(com){
    case ADD:
      clinfo[cln].ex = 1;
      strncpy(clinfo[cln].clihost, ch1, sizeof(clinfo[cln].clihost));
      break;
    case DEL:
      clinfo[cln].ex = 0;
      memset(clinfo[cln].clihost, 0, sizeof(clinfo[cln].clihost));
      break;
    case ID:
      clinfo[cln].ex = 1 + id;
      break;
    }

    if(upf){
      sock = infcon(ebhostname);
      ret = eb_set(sock, INF_SET_CLIHOST, (char *)clinfo,
		   sizeof(clinfo));
      close(sock);
      if(!ret){
	printf("Run is now running, can't change parameters\n");
      }

      sock = infcon(ebhostname);
      eb_get(sock, INF_GET_CLIHOSTS, (char *)clinfo);
      close(sock);
    }

    show_clinfo();
  }else{
    synopsis(this);
  }


  return 1;
}


int settcpclinfo(char *arg){
  char this[] = "settcpclinfo";
  int argc;
  char *argv[10];
  //int cln, fl;
  //char ch1[80];
  //int sock, ret;
  //int com, upf, id;
  //enum comlist {ADD, DEL};

  argc = striparg(arg, argv);

  if(argc != 3){
    synopsis(this);
    return 0;
  }

  printf("not implemented yet\n");

  /*
  fl = 1;
  cln = 0;
  ret = 0;
  upf = 1;
  id = 0;

  cln = strtol(argv[0], NULL, 0);
  if(cln < 0 || cln >= MAXCLI){
    printf("0 < CLIN < %d\n", MAXCLI);
    return 0;
  }

  if((com = chkcom(argv[1], comsetclinfo)) < 0){
    printf("Unknown command : %s \n", argv[1]);
    synopsis(this);
    return 0;
  }

  switch(com){
  case ADD:
    if(argc != 3){
      fl = 0;
    }else{
      memset(ch1, 0, sizeof(ch1));
      strncpy(ch1, argv[2], sizeof(ch1));
    }
    break;
  case ID:
    if(argc != 3){
      fl = 0;
    }else{
      id = strtol(argv[2], NULL, 0);
      if(id < 0 || id >= MAXANPT){
	printf("Error : 0 <= ID < %d\n\n", MAXANPT);
	fl = 0;
      }
    }
    break;
  }
    
  if(fl){
    sock = infcon(ebhostname);
    eb_get(sock, INF_GET_CLIHOSTS, (char *)clinfo);
    close(sock);
    
    switch(com){
    case ADD:
      clinfo[cln].ex = 1;
      strncpy(clinfo[cln].clihost, ch1,
		sizeof(clinfo[cln].clihost));
      break;
    case DEL:
      clinfo[cln].ex = 0;
      memset(clinfo[cln].clihost, 0, sizeof(clinfo[cln].clihost));
      break;
    case ID:
      clinfo[cln].ex = 1 + id;
      break;
    }

    if(upf){
      sock = infcon(ebhostname);
      ret = eb_set(sock, INF_SET_CLIHOST, (char *)clinfo,
		   sizeof(clinfo));
      close(sock);
      if(!ret){
	printf("Run is now running, can't change parameters\n");
      }

      sock = infcon(ebhostname);
      eb_get(sock, INF_GET_CLIHOSTS, (char *)clinfo);
      close(sock);
    }

    show_clinfo();
  }else{
    synopsis(this);
  }
  */

  return 1;
}

int delscr(char *arg){
  int id, tdx = -1;
  int sock, len;
  char *line, pr[128];


  if(!strlen(arg)){
    printf("delscr SCRID\n");
    return 0;
  }

  id = strtol(arg, NULL, 0);
  if(id < 0){
    printf("0 < scrid\n");
    return 0;
  }

  getscrlist(NULL);

  if((tdx = fndscrid(id)) < 0){
    printf("Not exist such scaler=%d\n", id);
    return 0;
  }

  sprintf(pr, "Delete scrid=%d [y/n] ? : ", id);
  line = readline(pr);
  if(line[0] == 'y'){
    if(!(sock = infcon(ebhostname))) return 0;
    len = sizeof(id);
    eb_set(sock, INF_DEL_SCR, (char *)&id, len);
    printf("Deleted\n");
  }else{
    printf("Canceled\n");
  }
  free(line);

  return 1;
}

int gettgig(char *arg){
  int tgig;
  int sock;

  sock = ebcon(ebhostname);
  eb_get(sock, EB_GET_TGIG, (char *)&tgig);
  close(sock);

  if(tgig == 0){
    printf(" trim mode = off\n\n");
  }else{
    printf(" trim mode = %d GB\n\n", tgig);
  }

  return 1;
}

int settgig(char *arg){
  char *line, str[80];
  char *argv[10];
  int tgig, argc, sock, ret;

  DB(printf("babicon: settgig %s\n", arg));

  argc = striparg(arg, argv);
  if(argc == 1){
    strcpy(str, argv[0]);
  }else{
    line = readline("Set tgig XGB : ");
    strcpy(str, line);
    free(line);
  }

  tgig = strtol(str, NULL, 0);

  if(tgig < 0 || tgig > 10){
    printf("settgig 0-10 GB\n");
    return 0;
  }
    
  sock = ebcon(ebhostname);
  ret = eb_set(sock, EB_SET_TGIG, (char *)&tgig, sizeof(tgig));
  close(sock);

  if(!ret){
    printf("Run is now running, can't change parameter of tgig\n");
    return 0;
  }

  gettgig(NULL);

  return 1;
}
