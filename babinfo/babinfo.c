/* babinfo/babinfo.c
 * last modified : 17/01/13 14:16:19 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Information server
 *
 */

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <pthread.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include "babinfo.h"
#include "babinfodb.h"

void quit(void);
void spipe(void);
int freescr(int);
int clearscr(void);
int fndscrid(int);
int escon(int);
int babinfoxcom(char *, char *);
int xgetinitialize(char *);
int update_scrname(int id);
int update_scrlive();

#ifdef USEDB
void scrdatatodb(void);
void scrtodb(void);
void scrfromdb(void);
#endif

int stf;
static int lorunstat;
const unsigned long long int ov24 = 16777216ULL;
const unsigned long long int ov32 = 4294967296ULL;

static int tcpclin;
static int lastender = 0;

#ifdef USEDB
void scrdatatodb(void){
  int i, j, expid, runid;
  int scrinfoid, chid;

  expid = babinfodb_getexpid();
  if(expid <= 0) return;

  runid = babinfodb_getrunid(dbcon, expid, daqinfo.runname, daqinfo.runnumber);
  if(runid <= 0) return;

  DB(printf("scrdatatodb: expid=%d  runid=%d\n", expid, runid));

  for(i=0;i<scranan;i++){
    scrinfoid = babinfodb_getscalerinfoid(dbcon, expid, scrana[i].scrid);
    if(scrinfoid <= 0){
      scrinfoid = babinfodb_setscalerinfo(dbcon, expid, scrana[i].scrid, scrana[i].idname, scrana[i].classid, scrana[i].rate, scrana[i].ratech);
    }
    for(j=0;j<scrana[i].scrn;j++){
      chid = babinfodb_getchannelid(dbcon, scrinfoid, j);
      if(chid <= 0){
	chid = babinfodb_setscalerchannel(dbcon, scrinfoid, j, scr[i][j].name);
      }
      babinfodb_setscalerdata(dbcon, runid, chid, lastscr[i][j], ovscr[i][j]);
    }
  }
}

void scrtodb(void){
  int i, j, expid;
  int scrinfoid;

  expid = babinfodb_getexpid();
  if(expid <= 0) return;

  DB(printf("scrnametodb: expid=%d \n", expid));

  for(i=0;i<scranan;i++){
    scrinfoid = babinfodb_setscalerinfo(dbcon, expid, scrana[i].scrid,
					scrana[i].idname, scrana[i].classid,
					scrana[i].rate, scrana[i].ratech);
    for(j=0;j<scrana[i].scrn;j++){
      babinfodb_setscalerchannel(dbcon, scrinfoid, j, scr[i][j].name);
    }
  }
  babinfodb_setexplive(dbcon, scrlive.gatedid, scrlive.gatedch, 
		       scrlive.ungatedid, scrlive.ungatedch);

}

void scrfromdb(void){
  int i, j, expid;
  int scrinfoid;

  expid = babinfodb_getexpid();
  if(expid <= 0) return;

  DB(printf("scrinfofromdb: expid=%d \n", expid));

  for(i=0;i<scranan;i++){
    scrinfoid = babinfodb_getscalerinfoid(dbcon, expid, scrana[i].scrid);
    babinfodb_getscalerinfo(dbcon, scrinfoid,
			    scrana[i].idname, 
			    &scrana[i].rate, &scrana[i].ratech);
    for(j=0;j<scrana[i].scrn;j++){
      memset(scr[i][j].name, 0, sizeof(scr[i][j].name));
      babinfodb_getscalerchannel(dbcon, scrinfoid, j, scr[i][j].name);
    }
    update_scrname(scrana[i].scrid);
  }

  babinfodb_getexplive(dbcon, &scrlive.gatedid, &scrlive.gatedch, 
		       &scrlive.ungatedid, &scrlive.ungatedch);
  update_scrlive();

}
#endif


int mktcpclient(int port, char *host){
  int i;

  DB(printf("mktcpclient host=%s, port=%d\n", host, port));
  // check duplication
  for(i=0;i<MAXTCPCLI;i++){
    if(tcpclinfo[i].sock){
      if(!strcmp(host, tcpclinfo[i].host)){
	if(port == tcpclinfo[i].port){
	  close(tcpclinfo[i].sock);
	  tcpclinfo[i].sock = 0;
	  memset(tcpclinfo[i].host, 0, sizeof(tcpclinfo[i].host));
	}
      }
    }
  }
  for(i=0;i<MAXTCPCLI;i++){
    if(!tcpclinfo[i].sock){
      tcpclinfo[i].sock = mktcpsend_tout(host, (unsigned short)port, 1); // timeout = 1sec
      if(!tcpclinfo[i].sock){
	return 0;
      }else{
	strncpy(tcpclinfo[i].host, host, sizeof(tcpclinfo[i].host));
	tcpclinfo[i].port = port;
	return tcpclinfo[i].sock;
      }
    }
  }

  return 0;
}

int retrytcpclient(void){
  int i;

  for(i=0;i<MAXTCPCLI;i++){
    if(tcpclinfo[i].sock == 0 && tcpclinfo[i].port != 0){
      tcpclinfo[i].sock = mktcpsend_tout(tcpclinfo[i].host, (unsigned short)tcpclinfo[i].port, 1); // timeout = 1sec
      if(!tcpclinfo[i].sock){
	DB(printf("retrytcpclient : %s (%d) is still dead\n", tcpclinfo[i].host, tcpclinfo[i].port));
	return 0;
      }else{
	DB(printf("retrytcpclient : %s (%d) is recovered\n", tcpclinfo[i].host, tcpclinfo[i].port));
	return tcpclinfo[i].sock;
      }
    }
  }
  return 1;
}



int escon(int efn){
  int essock = 0;

  if(efn < 0 || efn > MAXEF){
    DB(printf("0 < EFSID < %d\n", MAXEF));
    return 0;
  }

  if(!(essock = mktcpsend_tout(daqinfo.eflist[efn].host, ESCOMPORT+efn, 2))){
    DB(printf("Can't connect to babies id=%d.\n", efn));
    return 0;
  }
  
  DB(printf("escon efn=%d, sock=%d\n", efn, essock));

  return essock;
}


int getesconfig(int efn, struct stefrc *efrc, char *myhost){
  int sock, len, com;
  int chk;
  fd_set fdset;
  struct timeval timeout;

  DB(printf("babinfo: getesconfig efn=%d / host=%s\n", efn, daqinfo.eflist[efn].host));
  strcpy(myhost, daqinfo.eflist[efn].host);

  if(!(sock = escon(efn))) return 0;

  DB(printf("sock = %d\n", sock));
  com = ES_GET_CONFIG;
  len = sizeof(com);
  DB(printf("babinfo: getesconfig: send len=%d\n", len));
  send(sock, (char *)&len, sizeof(len), 0);
  DB(printf("babinfo: getesconfig: send com=%d\n", com));
  send(sock, (char *)&com, len, 0);
  DB(printf("babinfo: getesconfig: recv len\n"));
  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  if((chk = select(sock+1, &fdset, NULL, NULL, &timeout))){
    DB(printf("babinfo: getesconfig: recv efrc\n"));
    recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
    recv(sock, (char *)efrc, len, MSG_WAITALL);
  }else{
    DB(printf("babinfo: getesconfig: timeout for recv length"));
    close(sock);
    return 0;
  }
  close(sock);

  
  DB(printf("babinfo: piyopiyo\n"));
  DB(printf("babinfo: host %s\n", daqinfo.eflist[efn].host));
  DB(printf("babinfo: efrc.host %s\n", efrc->erhost));

  return 1;
}


int setesconfig(int efn, struct stefrc *efrc){
  int sock, len, com;
  char ret[80];
  int chk;
  fd_set fdset;
  struct timeval timeout;

  DB(printf("babinfo: setesconfig\n"));

  if(!(sock = escon(efn))) return 0;
  
  com = ES_SET_CONFIG;
  len = sizeof(int) + sizeof(struct stefrc);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(int), 0);
  send(sock, (char *)efrc, sizeof(struct stefrc), 0);
  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  if((chk = select(sock+1, &fdset, NULL, NULL, &timeout))){
    recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
    recv(sock, (char *)&ret, len, MSG_WAITALL);
  }else{
    close(sock);
    return 0;
  }

  close(sock);
  
  return 1;
}


int reloadesdrv(int efn){
  int sock, len, com;

  DB(printf("babinfo: reloadesesdrv\n"));

  if(!(sock = escon(efn))) return 0;
  
  com = ES_RELOAD_DRV;
  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(com), 0);
  close(sock);
  
  return 1;
}


void lfz(char *e){
  int i;

  i = 0;
  while(*(e+i)){
    if(*(e+i) == '\n'){
      *(e+i) = 0;
      break;
    }
    i++;
  }
}


void updatebabinforcxml(){
  char initbuff[EB_BUFF_SIZE];
  FILE *fd;

  // XML Initialize
  if((fd = fopen(BABINFORCXML, "w"))){
    DB(printf("babinfo: write initial parameters\n"));
    memset(initbuff, 0, sizeof(initbuff));
    xgetinitialize(initbuff);
    fputs(initbuff, fd);
    fclose(fd);
  }else{
    DB(printf("babinfo: cannot write parameters\n"));
  }
}

void quit(void){
  updatebabinforcxml();
  if(comfd) close(comfd);
  if(earecvfd) close(earecvfd);
  if(udpsock) close(udpsock);
  if(eabuff[0]) free(*(eabuff));
  if(eabuff[1]) free(*(eabuff+1));
  if(tbuff) free(tbuff);
  if(rawbuff) free(rawbuff);

  rmpid("babinfo");

  exit(0);
}

void spipe(void){
  DB(printf("Cought SISPIPE\n"));
  quit();
}

int freescr(int n){
  int i;

  DB(printf("babinfo: freescr scranan=%d\n", scranan));
  if(n != (scranan-1)){
    if(scrana[n].scrn != scrana[scranan-1].scrn){
      DB(printf("babinfo: try to free scr %d (scrn=%d)\n", n, scrana[n].scrn));
      free(scr[n]);
      scr[n] = (struct ridf_scr_contst *)
	malloc(sizeof(struct ridf_scr_contst)*scrana[scranan-1].scrn);
    }
    for(i=0;i<scrana[scranan-1].scrn;i++){
      scr[n][i].cur = scr[scranan-1][i].cur;
      scr[n][i].tot = scr[scranan-1][i].tot;
      memcpy(scr[n][i].name, scr[scranan-1][i].name, sizeof(scr[n][i].name));
      ovscr[n][i] = ovscr[scranan-1][i];
    }
    memcpy((char *)&scrana[n], (char *)&scrana[scranan-1], sizeof(scrana[n]));
  }

  scranan--;
  free(scr[scranan]);
  memset((char *)&scrana[scranan], 0, sizeof(scrana[scranan]));

  DB(printf("babinfo: scranan = %d\n", scranan));

  return 1;
}

int tfreescr(int n){
  int i;

  DB(printf("babinfo: tfreescr\n"));

  memset((char *)&scrana[n], 0, sizeof(scrana[n]));
  for(i=0;scrana[n].scrn;i++){
    free(&scr[n][i]);
  }
  scranan--;

  return 1;
}

int clearscr(void){
  int i, j;

  DB(printf("babinfo: clearscr, scranan=%d\n", scranan));

  for(j=0;j<scranan;j++){
    for(i=0;i<scrana[j].scrn;i++){
      scr[j][i].cur = 0;
      scr[j][i].tot = 0;
      ovscr[j][i] = 0;
    }
  }


  return 1;
}

int update_scrname(int id){
  FILE *fd;
  int tdx, i;
  char fname[128];

  DB(printf("babinfo: update_scrname\n"));

  if((tdx = fndscrid(id)) < 0) return 0;

  sprintf(fname, "scr/scr%d.dat", id);
  if(!(fd = fopen(fname, "w"))) return 0;
  fprintf(fd, "%d\n", scrana[tdx].classid);  // scrid
  fprintf(fd, "%d\n", scrana[tdx].scrid);  // scrid
  fprintf(fd, "%d\n", scrana[tdx].ratech);
  fprintf(fd, "%d\n", scrana[tdx].rate);
  fprintf(fd, "%d\n", scrana[tdx].scrn);   // scrn
  fprintf(fd, "%s\n", scrana[tdx].idname); // idname
  for(i=0;i<scrana[tdx].scrn;i++){
    fprintf(fd, "%s\n", scr[tdx][i].name);
  }
  fclose(fd);

  return 1;
}

int delete_scrname(int id){
  int tdx;
  char fname[128];

  DB(printf("babinfo: delete_scrname %d(%d)\n", fndscrid(id), id));

  if((tdx = fndscrid(id)) < 0) return 0;
  sprintf(fname, "scr/scr%d.dat", id);
  tdx = unlink(fname);
  DB(printf("babinfo: try to delete scr file %d\n", id));

  return 1;
}


int read_scrname(void){
  FILE *fd;
  int i;
  struct dirent *de;
  DIR *dp;
  char fname[272];

  DB(printf("babinfo: read_scrname\n"));
  if(!(dp = opendir("scr"))) return 0;

  while((de = readdir(dp))){
    printf("%s\n", de->d_name);
    if(strncmp("scr", de->d_name, 3) == 0){
      sprintf(fname, "scr/%s", de->d_name);
      if(!(fd = fopen(fname, "r"))) return 0;
      fscanf(fd, "%d\n", &scrana[scranan].classid);
      fscanf(fd, "%d\n", &scrana[scranan].scrid);
      fscanf(fd, "%d\n", &scrana[scranan].ratech);
      fscanf(fd, "%d\n", &scrana[scranan].rate);
      fscanf(fd, "%d\n", &scrana[scranan].scrn);
      fgets(scrana[scranan].idname, sizeof(scrana[scranan].idname), fd);
      lfz(scrana[scranan].idname);
      scr[scranan] = (struct ridf_scr_contst *)
	malloc(sizeof(struct ridf_scr_contst)*scrana[scranan].scrn);
      
      for(i=0;i<scrana[scranan].scrn;i++){
	fgets(scr[scranan][i].name, sizeof(scr[scranan][i].name), fd);
	lfz(scr[scranan][i].name);
	scr[scranan][i].cur = 0;
	scr[scranan][i].tot = 0;
	ovscr[scranan][i] = 0;
      }
      fclose(fd);
      scranan++;
    }
  }

  closedir(dp);
  DB(printf("scranan = %d\n", scranan));

  return 1;
}

int mallocscr(int classid, int scrid, int scrn){
  int i;

  DB(printf("babinfo: mallocscr\n"));

  scrana[scranan].classid = classid;
  scrana[scranan].scrid = scrid;
  scrana[scranan].scrn = scrn;
  scr[scranan] = (struct ridf_scr_contst *)
    malloc(sizeof(struct ridf_scr_contst)*scrana[scranan].scrn);
  for(i=0;i<scrana[scranan].scrn;i++){
    scr[scranan][i].cur = 0;
    scr[scranan][i].tot = 0;
    memset(scr[scranan][i].name, 0, sizeof(scr[scranan][i].name));
    ovscr[scranan][i] = 0;
  }

  scranan++;
  DB(printf("malloc scr scranan = %d\n", scranan));

  DB(printf("babinfo: new scrana create id=%d, sz=%d\n",
	    scrana[scranan].scrid, scrana[scranan].scrn));

  return scranan - 1;
}

int fndscrid(int id){
  int ret, i;

  //DB(printf("babinfo: fndscrid\n"));

  ret = -1;
  for(i=0;i<scranan;i++){
    if(scrana[i].scrid == id){
      ret = i;
    }
  }

  return ret;
}

int anascr(char *pt, int classid){
  RIDFRHD rhd;
  RIDFHDSCR scrhd;
  int i, tid;
  char *spt;
  unsigned int ts;
  unsigned long long int os;

  //DB(printf("babinfo: anascr\n"));

  memcpy((char *)&scrhd, pt, sizeof(scrhd));

  pthread_mutex_lock(&scrmutex);
  tid = fndscrid(scrhd.scrid);
  if(tid < 0){
    rhd = ridf_dechd(scrhd.chd);
    tid = mallocscr(classid, scrhd.scrid,
		    (rhd.blksize*WORDSIZE - sizeof(scrhd))
		    /sizeof(int));
  }

  scrana[tid].update = scrhd.date;
  //         header size
  spt = pt + sizeof(scrhd);

  //DB(printf("SCRID=%d\n", scrhd.scrid));

  switch(classid){
  case RIDF_SCALER:
    for(i=0;i<scrana[tid].scrn;i++){
      memcpy((char *)&ts, spt+i*sizeof(int), sizeof(int));

      os = ts + ovscr[tid][i] * ov24;
      if(os < scr[tid][i].tot){
	ovscr[tid][i]++;
      }
      scr[tid][i].cur = ts + ovscr[tid][i] * ov24 - scr[tid][i].tot;
      scr[tid][i].tot = ts + ovscr[tid][i] * ov24;
      lastscr[tid][i] = ts;
      
      //DB(printf("SCR:%d cur=%u / tot=%Lu\n", i, scr[tid][i].cur,
      //	scr[tid][i].tot));
    }
    break;
  case RIDF_CSCALER:
    for(i=0;i<scrana[tid].scrn;i++){
      memcpy((char *)&scr[tid][i].cur, spt+i*sizeof(int), sizeof(int));
      scr[tid][i].tot += scr[tid][i].cur;
      lastscr[tid][i] = scr[tid][i].tot;
      //DB(printf("SCR:%d cur=%u / tot=%Lu\n", i, scr[tid][i].cur,
      //	scr[tid][i].tot));
    }
    break;
  case RIDF_NCSCALER32:
    for(i=0;i<scrana[tid].scrn;i++){
      memcpy((char *)&ts, spt+i*sizeof(int), sizeof(int));

      os = ts + ovscr[tid][i] * ov32;
      if(os < scr[tid][i].tot){
	ovscr[tid][i]++;
      }
      scr[tid][i].cur = ts + ovscr[tid][i] * ov32 - scr[tid][i].tot;
      scr[tid][i].tot = ts + ovscr[tid][i] * ov32;
      lastscr[tid][i] = ts;
            
      //DB(printf("SCR:%d cur=%u / tot=%Lu\n", i, scr[tid][i].cur,
      //	scr[tid][i].tot));
    }
    break;
  }
  pthread_mutex_unlock(&scrmutex);

  return 1;
}

int update_clihosts(void){
  FILE *fd;
  int i;

  DB(printf("babinfo: update_clihosts\n"));

  for(i=0;i<MAXCLI;i++){
    if(clinfo[i].ex){
      registmultisend(ANRCVPORT+clinfo[i].ex-1, &cliaddr[i], clinfo[i].clihost);
    }
  }

  if((fd = fopen(CLIHOSTSRC, "w"))){
    for(i=0;i<MAXCLI;i++){
      if(clinfo[i].ex){
	fprintf(fd, "%d %s %d\n", i, clinfo[i].clihost, clinfo[i].ex-1);
      }
    }
    fclose(fd);
  }else{
    return 0;
  }

  return 1;
}

int update_tcpclihosts(void){
  FILE *fd;
  int i;

  DB(printf("babinfo: update_tcpclihosts\n"));

  if((fd = fopen(TCPCLIHOSTSRC, "w"))){
    for(i=0;i<MAXTCPCLI;i++){
      if(tcpclinfo[i].port){
	fprintf(fd, "%d %d %s\n", tcpclinfo[i].sock, tcpclinfo[i].port, 
		tcpclinfo[i].host);
      }
    }
    fclose(fd);
  }else{
    return 0;
  }

  return 1;
}

int update_scrlive(void){
  FILE *fd;

  DB(printf("babinfo: update_scrlive\n"));
  
  if((fd = fopen(SCRLIVERC, "w"))){
    fprintf(fd, "%d %d\n", scrlive.gatedid, scrlive.gatedch);
    fprintf(fd, "%d %d\n", scrlive.ungatedid, scrlive.ungatedch);
    fclose(fd);
  }else{
    return 0;
  }

  return 1;
}

int update_dbcon(void){
  FILE *fd;

  DB(printf("babinfo: update_dbcon\n"));
  
  if((fd = fopen(DBCONRC, "w"))){
    fprintf(fd, "%d\n", dbcon.of);
    fprintf(fd, "%d\n", dbcon.rev);
    fprintf(fd, "%s\n", dbcon.host);
    fprintf(fd, "%s\n", dbcon.dbname);
    fprintf(fd, "%s\n", dbcon.user);
    fprintf(fd, "%s\n", dbcon.passwd);
    fclose(fd);
  }else{
    return 0;
  }

  return 1;
}

void com_get(int sock, char *src, int len){
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, src, len, 0);
}

void com_set(int sock, char *src, char *dest, int len){
  int ret;

  memcpy(dest, src+sizeof(int), len-sizeof(int));
  len = sizeof(ret);
  ret = 1;
  com_get(sock, (char *)&ret, len);
}

void com_rej(int sock){
  int ret = 0, len;

  len = sizeof(ret);
  com_get(sock, (char *)&ret, len);
}

/********************* EA recv thread *************************/
int earecv(void){
  struct sockaddr_in caddr;
  int sock, clen, eahd[2];
  int i, idx, rsize;
  RIDFHD hd;
  RIDFRHD rhd;
  char tender[80]={0};

  clen = sizeof(caddr);
  if((sock = accept(earecvfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("babinfo: Error in accept earecvfd\n");
    return 0;
  }
  lastender = 0;

  while(1){
    rsize = recv(sock, (char *)eahd, sizeof(eahd), MSG_WAITALL);
    if(rsize != sizeof(eahd)){
      printf("Error in receiving data from earecv\n");
      quit();
      break;
    }
    recv(sock, tbuff, eahd[1], MSG_WAITALL);
    switch(eahd[0]){
    case EARECV_DATA:
      if(eaflag == -1 || eaflag == 1){
	DB(printf("EARECV DATA 1\n"));
	pthread_mutex_lock(&eamutex0);
	memcpy(*(eabuff+0), tbuff, eahd[1]);
	pthread_mutex_unlock(&eamutex0);
	eaflag = 0;
      }else{
	DB(printf("EARECV DATA 2\n"));
	pthread_mutex_lock(&eamutex1);
	memcpy(*(eabuff+1), tbuff, eahd[1]);
	pthread_mutex_unlock(&eamutex1);
	eaflag = 1;
      }
      pthread_mutex_lock(&nummutex);
      earecvbuffnum ++;
      pthread_mutex_unlock(&nummutex);
      /* Send UDP data */
      for(i=0;i<MAXCLI;i++){
	if(clinfo[i].ex){
	  sendto(udpsock, *(eabuff+eaflag), eahd[1], 0, 
		 (struct sockaddr *)&cliaddr[i], sizeof(cliaddr[i]));
	}
      }
      /*  Send TCP data */
      for(i=0;i<MAXTCPCLI;i++){
	if(tcpclinfo[i].sock){
	  if(send(tcpclinfo[i].sock, *(eabuff+eaflag), eahd[1], MSG_NOSIGNAL) < 0){
	    close(tcpclinfo[i].sock);
	    tcpclinfo[i].sock = 0;
	    //printf("send tcp data -------------------------\n");
	  }else{
	    //printf("send tcp data *************************\n");
	  }
	}
      }
      /* Analysis scaler */
      if(stf){
	DB(printf("babinfo: stf=1\n"));
	clearscr();
	//DB(printf("babinf: end scrclear\n"));
	stf = 0;
      }
      idx = 0;
      while(idx < eahd[1]){
	//DB(printf("babinf: memcpy ebfuff eaflag=%d idx=%d eahd[1]=%d ptr=%p\n", eaflag, idx, eahd[1], *(eabuff+eaflag)+idx));
	memcpy((char *)&hd, *(eabuff+eaflag)+idx, sizeof(hd));
	rhd = ridf_dechd(hd);
	switch(rhd.classid){
	case RIDF_EF_BLOCK:
	case RIDF_EA_BLOCK:
	case RIDF_EAEF_BLOCK:
	  idx += sizeof(hd);
	  break;
	case RIDF_SCALER:
	case RIDF_CSCALER:
	case RIDF_NCSCALER32:
	  anascr(*(eabuff+eaflag)+idx, rhd.classid);
	  idx += rhd.blksize*WORDSIZE;
	  break;
	default:
	  idx += rhd.blksize*WORDSIZE;
	  break;
	}
      }
      break;
    case EARECV_DAQINFO:
      DB(printf("EARECV DAQINFO\n"));
      pthread_mutex_lock(&daqmutex);
      memcpy((char *)&daqinfo, tbuff, sizeof(daqinfo));
      pthread_mutex_unlock(&daqmutex);

      break;
    case EARECV_RUNINFO:
      DB(printf("EARECV RUNINFO\n"));


      DB(printf("EARECV RUNINFO mutex lock\n"));
      pthread_mutex_lock(&runmutex);
      memcpy(tender, runinfo.ender, sizeof(runinfo.ender));
      memcpy((char *)&runinfo, tbuff, sizeof(runinfo));

      if(runinfo.runstat == STAT_RUN_START){
	lastender = 0;
      }else{
	DB(printf("the last ender flag = %d (%s)\n", lastender, tender));
	if(lastender){
	  memcpy(runinfo.ender, tender, sizeof(tender));
	  DB(printf("memcpy last ender\n"));
	  lastender = 0;
	}
      }
      pthread_mutex_unlock(&runmutex);
      DB(printf("EARECV RUNINFO mutex unlock\n"));


      if(runinfo.runstat){
	stf = 1;
      }

#ifdef USEDB
      if(dbcon.of){
	if(runinfo.runstat == STAT_RUN_START){
	  lorunstat = STAT_RUN_START;
	  babinfodb_runstart(dbcon, &daqinfo, &runinfo);
	}
	if(runinfo.runstat == STAT_RUN_IDLE
	   && lorunstat == STAT_RUN_START){
	  // Run stop with store mode
	  lorunstat = STAT_RUN_IDLE;
	  DB(printf("EARECV RUNINFO mutex lock for db update\n"));
	  pthread_mutex_lock(&runmutex);
	  babinfodb_runstop(dbcon, &daqinfo, &runinfo);
	  scrdatatodb();
	  pthread_mutex_unlock(&runmutex);
	  DB(printf("EARECV RUNINFO mutex unlock for db update\n"));
	}
      }
#endif
      
      // try to connect tcp client
      retrytcpclient();

      break;
    }
  }

  return 0;
}



/************************* commain ****************************/
int commain(void){
  struct sockaddr_in caddr;
  int sock, clen, ret;
  int com, len, tch, trate;
  int id, tid, i, tn, tdx, tcid;
  unsigned int num;
  char tname[80], sret[EB_BUFF_SIZE];
  RIDFHD hd;
  struct sttcpclinfo ttcpclinfo;

  clen = sizeof(caddr);
  if((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("babinfo: Error in accept commain\n");
    return 0;
  }

  memset(combuff, 0, sizeof(combuff));
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, combuff, len, MSG_WAITALL);
  memcpy((char *)&com, combuff, sizeof(com));


  DB(printf("babinfo: commain com=%d\n", com));
  /* Check XML command */
  if(!strncmp((char *)&com, "<?", 2)){
    memset(sret, 0, sizeof(sret));
    pthread_mutex_lock(&xmlmutex);
    babinfoxcom(combuff, sret);
    pthread_mutex_unlock(&xmlmutex);
    len = strlen(sret) + 1;
    com_get(sock, sret, len);
  }else{
  switch(com){
  case INF_GET_DAQINFO:
    len = sizeof(daqinfo);
    com_get(sock, (char *)&daqinfo, len);
    break;
  case INF_GET_RUNINFO:
    len = sizeof(runinfo);
    com_get(sock, (char *)&runinfo, len);
    break;
  case INF_GET_TCPCLIHOSTS:
    len = sizeof(tcpclinfo);
    com_get(sock, (char *)&tcpclinfo, len);
    break;
  case INF_GET_CLIHOSTS:
    len = sizeof(clinfo);
    com_get(sock, (char *)&clinfo, len);
    break;
  case INF_SET_CLIHOST:
    if(!runinfo.runstat){
      com_set(sock, combuff, (char *)clinfo, len);
      update_clihosts();
    }else{
      ret = 0;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }
    break;
#ifdef USEDB
  case INF_SET_SCRFROMDB:
    if(!runinfo.runstat){
      scrfromdb();
    }else{
      ret = 0;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }
    break;
#endif
  case INF_GET_COMLIST:
    break;
  case INF_GET_SCRLIVE:
    len = sizeof(scrlive);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&scrlive, sizeof(scrlive), 0);
    break;
  case INF_SET_SCRLIVE:
    com_set(sock, combuff, (char *)&scrlive, len);
    update_scrlive();
    break;
  case INF_GET_SCRLIST:
    len = sizeof(scrana) + sizeof(scranan);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&scranan, sizeof(scranan), 0);
    send(sock, (char *)scrana, sizeof(scrana), 0);
    break;
  case INF_SET_SCRNAME:
    // 4 byte   : classid
    // 4 byte   : scrid
    // 4 byte   : scrn
    // 4 byte   : ratech
    // 4 byte   : rate
    // 80 bytes : scridname
    // 80*scrn  : scrname[i]
    memcpy((char *)&tcid, combuff+sizeof(int), sizeof(int));
    memcpy((char *)&tid, combuff+sizeof(int)*2, sizeof(int));
    memcpy((char *)&tn, combuff+sizeof(int)*3, sizeof(int));
    memcpy((char *)&tch, combuff+sizeof(int)*4, sizeof(int));
    memcpy((char *)&trate, combuff+sizeof(int)*5, sizeof(int));

    pthread_mutex_lock(&scrmutex);
    if((tdx = fndscrid(tid)) < 0){
      tdx = mallocscr(tcid, tid, tn);
      scrana[tdx].classid = tcid;
      scrana[tdx].scrid = tid;
      scrana[tdx].scrn = tn;
    }
    if(tn > scrana[tdx].scrn){
      tfreescr(tdx);
      tdx = mallocscr(tcid, tid, tn);
      scrana[tdx].classid = tcid;
      scrana[tdx].scrid = tid;
      scrana[tdx].scrn = tn;
    }
    scrana[tdx].ratech = tch;
    scrana[tdx].rate = trate;
    memcpy(tname, combuff+sizeof(int)*6, sizeof(tname));
    strncpy(scrana[tdx].idname, tname, sizeof(scrana[tdx].idname));
    for(i=0;i<tn;i++){
      memcpy(tname, combuff+sizeof(int)*6+sizeof(tname)*(i+1), sizeof(tname));
      strncpy(scr[tdx][i].name, tname, sizeof(scr[tdx][i].name));
    }
    update_scrname(tid);
    pthread_mutex_unlock(&scrmutex);

    ret = 1;
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    break;
  case INF_GET_SCRDATA:
    // (recv    : scrid)
    // 4 byte   : idx
    // 4 byte   : classid
    // 4 byte   : scrid
    // 4 byte   : scrn
    // 4 byte   : ratech
    // 4 byte   : rate
    // 80 bytes : scridname
    // 80*scrn  : scrname[i]

    memcpy((char *)&id, combuff+sizeof(com), sizeof(id));
    if((tid = fndscrid(id)) < 0){
      ret = -1;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }else{
      memset(sndbuff, 0, sizeof(sndbuff));
      memcpy(sndbuff, (char *)&tid, sizeof(tid));
      len = sizeof(tid);

      memcpy(sndbuff+len, (char *)&scrana[tid].classid, sizeof(scrana[tid].classid));
      len += sizeof(scrana[tid].classid);
      memcpy(sndbuff+len, (char *)&scrana[tid].scrid, sizeof(scrana[tid].scrid));
      len += sizeof(scrana[tid].scrid);
      memcpy(sndbuff+len, (char *)&scrana[tid].scrn, sizeof(scrana[tid].scrn));
      len += sizeof(scrana[tid].scrn);
      memcpy(sndbuff+len, (char *)&scrana[tid].ratech, sizeof(scrana[tid].ratech));
      len += sizeof(scrana[tid].ratech);
      memcpy(sndbuff+len, (char *)&scrana[tid].rate, sizeof(scrana[tid].rate));
      len += sizeof(scrana[tid].rate);
      memcpy(sndbuff+len, scrana[tid].idname, sizeof(scrana[tid].idname));
      len += sizeof(scrana[tid].idname);
      for(i=0;i<scrana[tid].scrn;i++){
	memcpy(sndbuff+len, (char *)&scr[tid][i], sizeof(scr[tid][id]));
	len += sizeof(scr[tid][id]);
      }	
      com_get(sock, sndbuff, len);
    }
    break;
  case INF_DEL_SCR:
    memcpy((char *)&id, combuff+sizeof(com), sizeof(id));
    if((tid = fndscrid(id)) < 0){
      ret = -1;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }else{
      pthread_mutex_lock(&scrmutex);
      delete_scrname(id);
      freescr(tid);
      pthread_mutex_unlock(&scrmutex);
    }

    ret = 1;
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    break;
  case INF_GET_STATLIST:
    break;
  case INF_GET_RAWDATA:
    /* return current buffer of rawdata */
    if(eaflag != -1){
      if(eaflag == 0){
	pthread_mutex_lock(&eamutex0);
	memcpy((char *)&hd, *(eabuff+0), sizeof(hd));
	len = ridf_sz(hd)*WORDSIZE;
	memcpy(rawbuff, *(eabuff+0), len);
	pthread_mutex_unlock(&eamutex0);
      }else{
	pthread_mutex_lock(&eamutex1);
	memcpy((char *)&hd, *(eabuff+1), sizeof(hd));
	len = ridf_sz(hd)*WORDSIZE;
	memcpy(rawbuff, *(eabuff+1), len);
	pthread_mutex_unlock(&eamutex1);
      }
    }else{
      len = sizeof(hd);
    }
    DB(printf("INF_GET_RAWDATA len=%d\n", len));
    com_get(sock, rawbuff, len);
    break;
  case INF_GET_BLOCKNUM:
    pthread_mutex_lock(&nummutex);
    num = earecvbuffnum;
    pthread_mutex_unlock(&nummutex);
    com_get(sock, (char *)&num, sizeof(num));
    break;
  case INF_SET_DBCON:
    com_set(sock, combuff, (char *)&dbcon, len);
    update_dbcon();
    break;
  case INF_GET_DBCON:
    len = sizeof(dbcon);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&dbcon, sizeof(dbcon), 0);
    break;
  case INF_CHK_DBCON:
#ifdef USEDB
    if(babinfodb_chkconnect(dbcon)){
      ret = 1;
    }else{
      ret = 0;
    }
#else
    ret = 0;
#endif
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    break;
  case INF_UPDATE_DBRUNSTOP:
    strncpy(runinfo.ender, combuff+sizeof(com), sizeof(runinfo.ender)-1);
    DB(printf("INF_UPDATE_DBRUNSTOP ender=%s\n", runinfo.ender));
#ifdef USEDB
    DB(printf("dbrunstop lock\n"));
    pthread_mutex_lock(&runmutex);
    lastender = 1;
    babinfodb_runstop_after(dbcon, &daqinfo, &runinfo);
    pthread_mutex_unlock(&runmutex);
    DB(printf("dbrunstop unlock\n"));
#endif
    break;
  case INF_MK_TCPCLIENT:
    DB(printf("babinfo_mk_tcpclient\n"));
    memcpy((char *)&ttcpclinfo, combuff + sizeof(com), sizeof(ttcpclinfo));
    ret = mktcpclient(ttcpclinfo.port, ttcpclinfo.host);
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    if(ret){
      update_tcpclihosts();
    }

    break;
  case WHOAREYOU:
    DB(printf("babinfo : WHOAREYOU\n"));
    len = sizeof(thisname);
    com_get(sock, thisname, len);
    break;
  case INF_QUIT:
    ret = 1;
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    quit();
    break;
  default:
    com_rej(sock);

    break;
  }
  }

  close(sock);

  return 1;
}


/*************************** Main *****************************/
int main(int argc, char *argv[]){
  fd_set fdset;
  FILE *fd;
  char tch[80];
  int tn, tid;
  int i, buffsize;
  RIDFHD hd;
  char initbuff[EB_BUFF_SIZE], sret[EB_BUFF_SIZE];

  /* Change working directory */
  //if(getenv("BABIRLDIR")){
  //chdir(getenv("BABIRLDIR"));
  //}else{
  //}

  /* PID file */
  if(chkpid("babinfo")){
    printf("babinfo: Error, another babinfo may be running.\n");
    printf(" If process is not exist, please delete PID file %s/babinfo\n", PIDDIR);
    exit(0);
  }

#ifndef DEBUG
  if(!(int)getuid()) nice(-15);
#ifndef NODAEMON
  daemon(1, 0);
#endif
#endif

  chdir(BABIRLDIR);

  buffsize = EB_EBBUFF_SIZE * 2;
  // The real maimum buffer size is 8MB
  if(buffsize > 8 * 1024 * 1024){
    buffsize = 8 * 1024 * 1024;
  }

  for(i=0;i<BUFFN;i++){
    eabuff[i] = malloc(buffsize);
    memset(eabuff[i], 0, buffsize);
  }
  tbuff = malloc(buffsize);
  memset(tbuff, 0, buffsize);
  rawbuff = malloc(buffsize);
  memset(rawbuff, 0, buffsize);

  DB(printf("eabuff[0] = %p\n", eabuff[0]));
  DB(printf("eabuff[1] = %p\n", eabuff[1]));
  DB(printf("tbuff = %p\n", tbuff));
  DB(printf("rawbuff = %p\n", rawbuff));

  
  if(!eabuff[0] || !eabuff[1] || !tbuff || !rawbuff){
    printf("Cannot malloc buffers\n");
    return 0;
  }
  

  memset((char *)clinfo, 0, sizeof(clinfo));
  memset((char *)tcpclinfo, 0, sizeof(tcpclinfo));

  memset(initbuff, 0, sizeof(initbuff));
  memset(sret, 0, sizeof(sret));

  scranan = 0;
  memset((char *)scrana, 0, sizeof(scrana));
  memset((char *)ovscr, 0, sizeof(ovscr));
  
  signal(SIGINT, (void *)quit);
  signal(SIGPIPE, (void *)spipe);

  if(!mkpid("babinfo")){
    printf("babinfo: Error, another babinfo may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babinfo\n");
    exit(0);
  }

  /* Make command and oneshot port */
  if((comfd = mktcpsock(INFCOMPORT)) == -1){
    printf("Can't make command and oneshot port\n");
    quit();
  }

  /* Make earecv socket */
  if((earecvfd = mktcpsock(INFEAPORT)) == -1){
    printf("Can't make earecv\n");
    quit();
  }


  /* UDP multisender */
  udpsock = mkmultisend();

  /* clihosts */
  if((fd = fopen(CLIHOSTSRC, "r"))){
    while(fscanf(fd, "%d %s %d", &tn, tch, &tid) == 3){
      clinfo[tn].ex = tid + 1;
      strcpy(clinfo[tn].clihost, tch);
      DB(printf("babinfo: clihosts %d %s %d\n", tn, clinfo[tn].clihost, tid));
    }
    fclose(fd);
  }
  update_clihosts();

  /* tcpclihosts */
  i = 0;
  tcpclin = 0;
  if((fd = fopen(TCPCLIHOSTSRC, "r"))){
    while(fscanf(fd, "%d %d %s", &tn, &tid, tch) == 3){
      if(tid){
	mktcpclient(tid, tch);
      }
      DB(printf("babinfo: tcpclihosts %d %s %d\n", tn, clinfo[tn].clihost, tid));
      i++;
      if(i == MAXTCPCLI) break;
    }
    fclose(fd);
  }
  update_tcpclihosts();

  /* scrlive */
  if((fd = fopen(SCRLIVERC, "r"))){
    //for(i=0;i<2;i++){
      fscanf(fd, "%d %d", &scrlive.gatedid, &scrlive.gatedch);
      fscanf(fd, "%d %d", &scrlive.ungatedid, &scrlive.ungatedch);
      //}
    fclose(fd);
  }

  /* dbcon */
  if((fd = fopen(DBCONRC, "r"))){
    fscanf(fd, "%d\n", &dbcon.of);
    fscanf(fd, "%d\n", &dbcon.rev);
    fscanf(fd, "%s\n", dbcon.host);
    fscanf(fd, "%s\n", dbcon.dbname);
    fscanf(fd, "%s\n", dbcon.user);
    fscanf(fd, "%s\n", dbcon.passwd);
    fclose(fd);
  }else{
    dbcon.of = 0;
    dbcon.rev = 0;
    memset(dbcon.host, 0, sizeof(dbcon.host));
    memset(dbcon.dbname, 0, sizeof(dbcon.dbname));
    memset(dbcon.user, 0, sizeof(dbcon.user));
    memset(dbcon.passwd, 0, sizeof(dbcon.passwd));
  }

  read_scrname();


  // XML Initialize
  if((fd = fopen(BABINFORCXML, "r"))){
    DB(printf("babinfo: read initial parameters\n"));
    memset(initbuff, 0, sizeof(initbuff));
    fread(initbuff, 1, sizeof(initbuff), fd);
    babinfoxcom(initbuff, sret);
    fclose(fd);
  }else{
    DB(printf("babinfo: no initial parameters\n"));
  }
  



  /* Initialize rawbuff for INF_GET_RAWDATA */
  hd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, sizeof(hd)/WORDSIZE, 0);
  memcpy(rawbuff, (char *)&hd, sizeof(hd));

  /* Initialize mutex */
  pthread_mutex_init(&eamutex0, NULL);
  pthread_mutex_init(&eamutex1, NULL);
  pthread_mutex_init(&nummutex, NULL);
  pthread_mutex_init(&daqmutex, NULL);
  pthread_mutex_init(&runmutex, NULL);
  pthread_mutex_init(&scrmutex, NULL);
  pthread_mutex_init(&xmlmutex, NULL);

  eaflag = -1;

  /* Create thread */
  pthread_create(&eathread, NULL, (void *)earecv, NULL);
  pthread_detach(eathread);


  // memory runstat
  lorunstat = STAT_RUN_IDLE;

  /* Main loop for command and one short */
  while(1){
    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    if(select(comfd+1, &fdset,NULL,NULL,NULL) != 0){
      commain();
    }
  }

  return 0;
}

