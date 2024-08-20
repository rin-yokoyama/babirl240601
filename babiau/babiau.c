/* Babiau/babiau.c
 * last modified : 11/10/24 15:50:10 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Auxiliay event builder
 *
 */

/* Version */
#define VERSION "1.2  May 15, 2024"

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <pthread.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbxml.h>
#include "babiau.h"
#include "austrcom.h"

int chkopt(int, char*[]);
void quit(void);
int daq_close();
int store_daqinfo();
int snd_ssminfo();
int escommand_force(int com);
int escommand_pas(int com);

unsigned int gloevtn;

char ssmerror[4096] = {0};
int crashefr[MAXEF] = {0};

#define NOFLOCK

/* Check option */
int chkopt(int argc, char *argv[]){
  int val;
  while((val=getopt(argc,argv,"hlvt"))!=-1){
    switch(val){
    case 'h':
      printf("Usage: e-builder [option] [EFN]\n");
      printf("\n");
      printf("Options\n");
      printf("  -h                  this message\n");
      printf("  -l                  create log file\n");
      printf("  -t                  create time stamp table\n");
      printf("  -v                  version information\n");
      return 0;
      break;
    case 'l':
      vm = 1;
      printf("babiau: log verbose mode\n");
      break;
    case 't':
      tst = 1;
      printf("babiau: with time stamp table\n");
      break;
    case 'v':
      printf("babiau version : %s\n", VERSION);
      printf("produced by Hidetada Baba\n");
      return 0;
      break;
    default:
      printf("Invalid option!!\n");
      return 0;
      break;
    }
  }
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


/* Quit sequence */
void quit(void){

  if(vm) lfprintf(lfd, "babiau: -- quit function --\n");

  if(runinfo.runstat != STAT_RUN_IDLE){
    if(vm) lfprintf(lfd, "babiau: force quit during run\n");
    escommand_force(ES_RUN_STOP);
    escommand_pas(ES_RUN_STOP);
  }


  esdisconnect();

  if(ebffd){
    if(vm) lfprintf(lfd, "babiau: Close ebffd %d\n", ebffd);
    close(ebffd);
  }
  if(vm) lfprintf(lfd, "babiau: Delete FIFO %s\n", EBFIFO);
  unlink(EBFIFO);
  
  if(comfd){
    if(vm) lfprintf(lfd, "babiau: Close comfd %d\n",comfd);
    close(comfd);
  }

  if(ebdfd){
    if(vm) lfprintf(lfd, "babiau: Close ebdfd %d\n",ebdfd);
    close(ebdfd);
  }

  //if(vm) printf("babiau: Semaphore for FIFO delete\n");
  //semctl(semid, 0, IPC_RMID, semunion);

  rmpid("babiau");

  if(vm) lfprintf(lfd, "babiau: Exit\n");

  if(lfd) fclose(lfd);

  exit(0);
}

int addaliasname(char *aliasname){
  if(aliasnamen < 10){
    DB(printf("add aliasnames[%d] %s\n", aliasnamen, aliasname));
    strncpy(aliasnames[aliasnamen], aliasname, sizeof(aliasnames[aliasnamen]));
    aliasnamen ++;

    return 1;
  }

  return 0;
}

/* ssm series */
int ssmcon(void){
  int ssmsock;

  if(!(ssmsock = mktcpsend(ssminfo.host, SSMCOMPORT))){
    DB(printf("babiau: Can't connect to babissm.\n"));
    return 0;
  }

  return ssmsock;
}

int ssm_start(void){
  int sock, com, len;

  snd_ssminfo();

  if(!(sock = ssmcon())){
    if(vm) lfprintf(lfd, "babiau: ssm_start: Can't connect ssm\n");
    return 0;
  }

  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  com = SSM_START;
  send(sock, (char *)&com, sizeof(com), 0);
  close(sock);

  return 1;
}

int ssm_stop(void){
  int sock, com, len;

  if(!(sock = ssmcon())){
    return 0;
  }

  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  com = SSM_STOP;
  send(sock, (char *)&com, sizeof(com), 0);
  close(sock);

  usleep(100000);

  return 1;
}

/* Count eflist.on */
int cntefon(struct steflist *eflist){
  int i, cnt = 0;
  
  for(i=0;i<MAXEF;i++){
    if(eflist[i].ex){
      if(eflist[i].of == EB_EFLIST_ON){
	cnt++;
      }
    }
  }

  return cnt;
}

int xmlcomment(int mode){
  FILE *fd;
  time_t now;
  char buff[EB_BUFF_SIZE];
  char *pcom, *txt;
  RIDFHDCOM comhd;
  int i, j, len, wlen, flag = 1;
  int sz, bsz;
  RIDFHDEOB eobhd;
  RIDFHD ghd;
  BBXMLEL *xml;
  long long int trst;

  DB(printf("xmlcomment: mode=%d\n", mode));
  if(mode == BABILD_COMMENT_START){
    pcom = statcom.start;
  }else{
    pcom = statcom.stop;
  }

  // for XML Status
  if(statcom.of){
    memset(buff, 0, sizeof(buff));
    len = 1;
    if((fd = popen(pcom, "r")) == NULL){
      lfprintf(lfd, "Can't open statcom %s\n", pcom);
      flag = 0;
      DB(printf("xmlcomment: Can't open %s\n", pcom));
    }else{
      fread(buff, 1, sizeof(buff)-1, fd);
      len = strlen(buff)+1;
      DB(printf("xmlcomment: opend %s len=%d\n", pcom, len));
      if(pclose(fd) < 0){
	lfprintf(lfd, "Can't open statcom %s\n", pcom);
	flag = 0;
	DB(printf("xmlcomment: error in close\n"));
      }
    }

    DB(printf("xmlcomment: flag=%d\n", flag));
    // user status command should have the length more than 4
    if(!flag || len < 4){
      sprintf(buff, "<babild><runstatus><error>Can't open statcom command %s</error></runstatus></babild>", pcom);
      len = strlen(buff)+1;  // +1 is to include last \0 charcter
    }else{
      // check for parameters
      // <timestamp>
      //   <resetcount>12</resetcount>
      // </timestamp>
      // timestamp-resetcount is additional bits of timestamp

      xml = NULL;
      xml = bbxml_parsebuff(buff, len);
      txt = NULL;
      if(xml){
	txt = bbxml_getTextByTagName(xml, "resetcount\0");
      }
      if(txt){
	trst = strtoll(txt, NULL, 0);
	trst = trst & 0x0000ffff; // mask lower 16bits
	if(vm) lfprintf(lfd, "timestamp resetount = %d\n", trst);
	tsrst = trst << 48;
      }
      bbxml_free(xml);
    }

    // writting wordsize have to be even number
    wlen = (len+1)/WORDSIZE;
    
    time(&now);
    if(BABILD_COMMENT_START){
      comhd = ridf_mkhd_com(RIDF_LY1, RIDF_STATUS, 
			    wlen + sizeof(comhd)/WORDSIZE,
			    daqinfo.efn, (unsigned int)now,
			    RIDF_STATUS_START_XML);

    }else{
      comhd = ridf_mkhd_com(RIDF_LY1, RIDF_STATUS, 
			    wlen + sizeof(comhd)/WORDSIZE,
			    daqinfo.efn, (unsigned int)now,
			    RIDF_STATUS_STOP_XML);

    }
    
    sz =  sizeof(eobhd)/WORDSIZE;
    bsz = wlen + sizeof(comhd)/WORDSIZE + sizeof(ghd)/WORDSIZE + sz;
    eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sz, daqinfo.efn, bsz);

    /* Make global header */
    ghd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, bsz, daqinfo.efn);

    for(j=0;j<MAXEF;j++){
      for(i=0;i<MAXHD;i++){
	if(hdfd[j][i]){
	  //flock(fileno(hdfd[i]), LOCK_EX);
	  fwrite(&ghd, 1, sizeof(ghd), hdfd[j][i]);
	  fwrite(&comhd, 1, sizeof(comhd), hdfd[j][i]);
	  fwrite(buff, 2, wlen, hdfd[j][i]);
	  fwrite(&eobhd, 2, sz, hdfd[j][i]);
	  //flock(fileno(hdfd[i]), LOCK_UN);
	}
      }
    }
  }

  return 1;
}


/* Calc time stamp table */
int exttst(short *buff, int maxmp, unsigned long long int *tsbuff, unsigned long long int *tsfp){
  unsigned long long int tts, tfp, tmp;
  RIDFHD thd;
  RIDFRHD rhd;
  int mp, tssz;

  mp = 0;
  tssz = 0;
  while(mp < maxmp){
    memcpy((char *)&thd, (char *)(buff + mp), sizeof(thd));
    rhd = ridf_dechd(thd);


    if(rhd.layer == RIDF_LY0){
      mp += sizeof(thd)/WORDSIZE;
      *tsfp += sizeof(thd);
    }else{
      if(rhd.classid == RIDF_EVENT_TS){
	memcpy((char *)&tts, (char *)(buff + mp + 6), sizeof(tts));

	tmp = rhd.efn & 0xff;
	tts = (tts & 0x0000ffffffffffffLL) | tsrst;
	// with timereset into buff
	memcpy((char *)(buff + mp + 6), (char *)&tts, sizeof(tts));
	tts = (tts & 0x00ffffffffffffffLL) | (tmp << 56);
	tfp = (*tsfp & 0x0000ffffffffffffLL) | (1ULL << 48);
	
	tsbuff[tssz] = tts;
	tssz++;
	tsbuff[tssz] = tfp;
	tssz++;

      }
      mp += rhd.blksize;
      *tsfp += rhd.blksize * WORDSIZE;
    } 

  }
  
  return tssz;
}


/* Make comment */
int mkcomment(int mode){
  char runch[RIDF_COMMENT_RUNINFO_ASC_SIZE];
  char buff[EB_BUFF_SIZE], chst[64];
  int i, j, size, idx, bsize;
  RIDFHDCOM comhd;
  RIDFHD hd;
  time_t ttime;
  struct tm *strtime;


  switch(mode){
  case BABILD_COMMENT_START:
    //DB(printf("babild: mkcomment start\n"));
    memset(runch, 0, sizeof(runch));

    memset(buff, 0, sizeof(buff));
    size = sizeof(comhd);
    for(i=0;i<MAXEF;i++){
      if(daqinfo.eflist[i].ex){
	size += sprintf(buff+size, "%3d %d %s %s\n", 
			i, daqinfo.eflist[i].of, daqinfo.eflist[i].name,
			daqinfo.eflist[i].host);
      }
    }

    if(size/2*2 != size){
      buff[size] = 0;
      size++;
    }

    comhd = ridf_mkhd_com(RIDF_LY1, RIDF_COMMENT, size/WORDSIZE,
			  daqinfo.efn, runinfo.starttime,
			  RIDF_COMMENT_EFINFO_ASC);
    memcpy(buff, (char *)&comhd, sizeof(comhd));

    comhd = ridf_mkhd_com(RIDF_LY1, RIDF_COMMENT, 
			  RIDF_COMMENT_RUNINFO_ASC_SIZE/WORDSIZE,
			  daqinfo.efn, runinfo.starttime,
			  RIDF_COMMENT_RUNINFO_ASC);
    memcpy(runch, (char *)&comhd, sizeof(comhd));

    bsize = (sizeof(hd) + size + sizeof(runch))/WORDSIZE;
    hd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, bsize, daqinfo.efn);

    for(j=0;j<MAXEF;j++){
      for(i=0;i<MAXHD;i++){
	if(hdfd[j][i]){
	  //DB(printf("babild: writing hdfd=%d\n", i));
#ifndef NOFLOCK
	  flock(fileno(hdfd[j][i]), LOCK_EX);
#endif
	  // Write block header
	  fwrite(&hd, 1, sizeof(hd), hdfd[j][i]);
	  // Write margin for header
	  fwrite(runch, 1, sizeof(runch), hdfd[j][i]);
	  // Make EF Information (ASCII)
	  fwrite(buff, 1, size, hdfd[j][i]);
#ifndef NOFLOCK
	  flock(fileno(hdfd[j][i]), LOCK_UN);
#endif
	  if(tst){
	    tsfp[j] = sizeof(hd) + sizeof(runch) + size;
	  }
	}
      }
    }

    xmlcomment(mode);

    break;
  case BABILD_COMMENT_STOP:
    //DB(printf("babild: mkcomment stop\n"));
    memset(runch, 0, sizeof(runch));
    comhd = ridf_mkhd_com(RIDF_LY1, RIDF_COMMENT, 
			  RIDF_COMMENT_RUNINFO_ASC_SIZE/WORDSIZE,
			  daqinfo.efn, runinfo.starttime,
			  RIDF_COMMENT_RUNINFO_ASC);
    memcpy(runch, (char *)&comhd, sizeof(comhd));
    idx = sizeof(comhd);
    sprintf(runch+idx, "%s", daqinfo.runname);
    idx += 100;
    sprintf(runch+idx, "%04d", daqinfo.runnumber);
    idx += 100;
    ttime = runinfo.starttime;
    strtime = localtime(&ttime);
    strftime(chst, sizeof(chst), "START => %X", strtime);
    sprintf(runch+idx, "%s", chst);
    idx += 20;
    ttime = runinfo.stoptime;
    strtime = localtime(&ttime);
    strftime(chst, sizeof(chst), "STOP => %X", strtime);
    sprintf(runch+idx, "%s", chst);
    idx += 20;
    ttime = runinfo.starttime;
    strftime(chst, sizeof(chst), "%d-%b-%y", strtime);
    sprintf(runch+idx, "%s", chst);
    idx += 60;
    sprintf(runch+idx, "%s", runinfo.header);
    idx += 100;
    sprintf(runch+idx, "%s", runinfo.ender);

    xmlcomment(mode);

    for(j=0;j<MAXEF;j++){
      for(i=0;i<MAXHD;i++){
	if(hdfd[j][i]){
#ifndef NOFLOCK
	  flock(fileno(hdfd[j][i]), LOCK_EX);
#endif
	  fseeko(hdfd[j][i], sizeof(hd), SEEK_SET);
	  fwrite(runch, 1, sizeof(runch), hdfd[j][i]);
#ifndef NOFLOCK
	  flock(fileno(hdfd[j][i]), LOCK_UN);
#endif
	}
      }
    }

    break;
  }

  return 1;
}

int chkerhost(char *erhost){
  int i;

  if(!fchkerhost) return 1;

  for(i=0;i<10;i++){
    if(!strcmp(erhost, aliasnames[i])) return 1;
  }
  if(!strcmp(erhost, myhost) ||
     !strcmp(erhost, mydom) ||
     !strcmp(erhost, myip)){
    return 1;
  }

  return 0;
}


void esclose(int essock[MAXEF]){
  int i;
  for(i=0;i<MAXEF;i++){
    if(essock[i]){
      close(essock[i]);
      essock[i] = 0;
    }
  }
}

/* DAQ start/stop */
int escommand(int com){
  int i, j, len, ret, rlen;
  int essock[MAXEF], chkes;
  int arg, tcom, tlen, chksscom = 0;
  char buff[128];

  DB(printf("babild: escommand com=%d\n", com));

  if(com == ES_RUN_START || com == ES_RUN_NSSTA || com == ES_RUN_STOP){
    chksscom = 1;
  }


  chkes = 1;
  memset((char *)&essock, 0, sizeof(essock));
  for(j=0;j<MAXEF;j++){
    if(com == ES_RUN_START || com == ES_RUN_NSSTA){
      i = MAXEF - j - 1;
    }else{
      i = j;
    }
    if(daqinfo.eflist[i].ex){
      //DB(printf("escommand: eflist[%d].ex = 1\n", i));
      if(com == ES_RUN_START || com == ES_RUN_NSSTA
	 || com == ES_RUN_STOP || com == ES_GET_EVTN){
	if(!daqinfo.eflist[i].of){
	  continue;
	}
	if(daqinfo.eflist[i].of == EB_EFLIST_PAS){
	  continue;
	}
      }
      if(!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT+i, estout))){
	if (vm) lfprintf(lfd, "babild: Can't connet to babies id=%d.\n", i);
	if(daqinfo.eflist[i].of){
	  chkes = 0;
	  snprintf(esret, sizeof(esret),
		   "can't connect to babies host=%s efn=%d",
		   daqinfo.eflist[i].host, i);
	  esclose(essock);
	  return chkes;
	}
      }else{
	/* Check erhost */
	if(chksscom){
	  tcom = ES_GET_CONFIG;
	  tlen = sizeof(tcom);
	  send(essock[i], (char *)&tlen, sizeof(tlen), 0);
	  send(essock[i], (char *)&tcom, tlen, 0);
	  recv(essock[i], (char *)&tlen, sizeof(tlen), MSG_WAITALL);
	  recv(essock[i], (char *)&tefrc, tlen, MSG_WAITALL);
	  if(!chkerhost(tefrc.erhost)){
	    snprintf(esret, sizeof(esret),
		     "wrong erhost = %s in babies (host=%s efn=%d), shold be %s",
		     tefrc.erhost, daqinfo.eflist[i].host, i, myhost);
	    esclose(essock);
	    return 0;
	  }else{
	    DB(printf("%d chkerhost ok\n", i));
	  }
	  if(essock[i]) close(essock[i]);
	  if(!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT+i, estout))){
	    if (vm) lfprintf(lfd, "babild: Can't connet to babies id=%d (2nd).\n", i);
	    if(daqinfo.eflist[i].of){
	      snprintf(esret, sizeof(esret),
		       "can't connect to babies host=%s efn=%d (2nd)",
		       daqinfo.eflist[i].host, i);
	      esclose(essock);
	      return 0;
	    }
	  }else{
	    DB(printf("%d newsocket = %d\n", i, essock[i]));
	  }
	}
      }
      //DB(printf("essock: %d = %d\n", i, essock[i]));
    }
  }
  
  for(j=0;j<MAXEF;j++){
    if(com == ES_RUN_START || com == ES_RUN_NSSTA){
      i = MAXEF - j - 1;
    }else{
      i = j;
    }
    if(essock[i]){
      if(chkes){
	len = sizeof(com);
	memcpy(buff, (char *)&com, sizeof(com));
	if(com == ES_RUN_START || com == ES_RUN_NSSTA){
	  if(daqinfo.eflist[i].of){
	    // for on and scr
	    efrun[i] = 1;
	    arg = ES_EF_ON;
	  }else{
	    // for off
	    // This option is no meaning for now
	    arg = ES_EF_OFF;
	  }
	  len += sizeof(arg);
	  memcpy(buff+sizeof(com), (char *)&arg, sizeof(arg));
	  if(com == ES_RUN_START){
	    len += sizeof(runinfo.header);
	    memcpy(buff+sizeof(com)+sizeof(arg), runinfo.header, sizeof(runinfo.header)-8);
	  }
	}
	send(essock[i], (char *)&len, sizeof(len), 0);
	send(essock[i], buff, len, 0);
	recv(essock[i], (char *)&rlen, sizeof(rlen), MSG_WAITALL);
	recv(essock[i], (char *)&ret, rlen, MSG_WAITALL);

	if(com == ES_GET_EVTN){
	  if(vm) lfprintf(lfd, "esevtn[%d] : %d\n", i, ret);
	}
	close(essock[i]);
      }else{
	close(essock[i]);
      }
    }
  }
    
  return chkes;
}


/* force DAQ start/stop */
int escommand_force(int com){
  int i, j, len, ret, rlen, fret=-1;
  int essock[MAXEF];
  int arg;
  char buff[64];

  DB(printf("babild: escommand_force com=%d\n", com));

  memset((char *)&essock, 0, sizeof(essock));
  for(j=0;j<MAXEF;j++){
    if(com == ES_RUN_START || com == ES_RUN_NSSTA){
      i = MAXEF - j - 1;
    }else{
      i = j;
    }
    if(daqinfo.eflist[i].ex){
      if(com == ES_RUN_START || com == ES_RUN_NSSTA
	 || com == ES_RUN_STOP || com == ES_GET_EVTN){
	if(!daqinfo.eflist[i].of){
	  continue;
	}
      }
      if(daqinfo.eflist[i].of == EB_EFLIST_PAS){
	continue;
      }
      if(!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT+i, estout))){
	DB(printf("babild : force escommand cannot make socket to %s\n", daqinfo.eflist[i].host));
	fret = i;
      }else{
      //DB(printf("essock: %d = %d\n", i, essock[i]));
      }
    }
  }
  
  for(j=0;j<MAXEF;j++){
    if(com == ES_RUN_START || com == ES_RUN_NSSTA){
      i = MAXEF - j - 1;
    }else{
      i = j;
    }
    if(essock[i]){
      len = sizeof(com);
      memcpy(buff, (char *)&com, sizeof(com));
      if(com == ES_RUN_START || com == ES_RUN_NSSTA){
	if(daqinfo.eflist[i].of){
	  // for on and scr
	  efrun[i] = 1;
	  arg = ES_EF_ON;
	}else{
	  // for off
	  // This option is no meaning for now
	  arg = ES_EF_OFF;
	}
	len += sizeof(arg);
	memcpy(buff+sizeof(com), (char *)&arg, sizeof(arg));
      }
      send(essock[i], (char *)&len, sizeof(len), 0);
      send(essock[i], buff, len, 0);
      recv(essock[i], (char *)&rlen, sizeof(rlen), MSG_WAITALL);
      recv(essock[i], (char *)&ret, rlen, MSG_WAITALL);
      
      if(com == ES_GET_EVTN){
	if(vm) lfprintf(lfd, "force esevtn[%d] : %d\n", i, ret);
      }
      close(essock[i]);
    }else{
      //close(essock[i]);
    }
  }
    
  return fret;
}


/* passive DAQ start/stop */
int escommand_pas(int com){
  int i, j, len, fret=-1;
  int essock[MAXEF];
  int arg;
  char *buff;

  DB(printf("babild: escommand_ps com=%d\n", com));
  buff = (char *)malloc(1024*100);

  if(com != ES_RUN_START &&
     com != ES_RUN_NSSTA &&
     com != ES_RUN_STOP){
    return 0;
  }

  memset((char *)&essock, 0, sizeof(essock));
  for(j=0;j<MAXEF;j++){
    if(com == ES_RUN_START || com == ES_RUN_NSSTA){
      i = MAXEF - j - 1;
    }else{
      i = j;
    }
    if(!daqinfo.eflist[i].ex){
      continue;
    }
    if(daqinfo.eflist[i].of != EB_EFLIST_PAS){
      continue;
    }

    if(!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT+i, 1))){
      fret = i;
    }else{
      //DB(printf("essock: %d = %d\n", i, essock[i]));
    }
  }
  
  for(j=0;j<MAXEF;j++){
    if(com == ES_RUN_START || com == ES_RUN_NSSTA){
      i = MAXEF - j - 1;
    }else{
      i = j;
    }
    if(essock[i]){
      len = sizeof(com);
      memcpy(buff, (char *)&com, sizeof(com));
      arg = ES_EF_OFF;
      memcpy(buff+len, (char *)&arg, sizeof(arg));
      len += sizeof(arg);
      memcpy(buff+len, (char *)&daqinfo, sizeof(daqinfo));
      len += sizeof(daqinfo);
      memcpy(buff+len, (char *)&runinfo, sizeof(runinfo));
      len += sizeof(runinfo);

      send(essock[i], (char *)&len, sizeof(len), 0);
      send(essock[i], buff, len, 0);
      
      close(essock[i]);
    }
  }
    
  return fret;
}



/* DAQ start/stop */
int esdisconnect(void){
  int i, len;
  int essock;
  char buff[64];
  int com = ES_DIS_EFR;

  for(i=0;i<MAXEF;i++){
    essock = 0;
    if(daqinfo.eflist[i].ex && daqinfo.eflist[i].of){
      if(!(essock = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT+i, 1))){
	if (vm) lfprintf(lfd, "babild: force escommand : Can't connet to babies id=%d.\n", i);
      }else{
	len = sizeof(com);
	memcpy(buff, (char *)&com, sizeof(com));
	send(essock, (char *)&len, sizeof(len), 0);
	send(essock, buff, len, 0);
	close(essock);
      }
    }
  }
    
  return 1;
}



int daq_start(){
  char fname[128];
  int i, j;
  time_t now;
  FILE *tfd;

  daqinfo.runnumber ++;
  runinfo.runnumber = daqinfo.runnumber;
  store_daqinfo();

  if(vm) lfprintf(lfd, "babiau: daq_start runnumber=%d\n", daqinfo.runnumber);

  /* Start time */
  time(&now);
  runinfo.starttime = (int)now;
  gloevtn = 0;
  totsize = 0;
  blkn = 0;

  // Open write file
  for(j=0;j<MAXEF;j++){
    if(daqinfo.eflist[j].ex && daqinfo.eflist[j].of == EB_EFLIST_ON){
      for(i=0;i<MAXHD;i++){
	if(daqinfo.hdlist[i].ex && daqinfo.hdlist[i].of){
	  sprintf(fname, "%s/%s_%s%04d.ridf",
		  daqinfo.hdlist[i].path, daqinfo.eflist[j].name, 
		  daqinfo.runname, daqinfo.runnumber);
	  if(!(hdfd[j][i] = fopen(fname, "w"))){
	    if(vm) lfprintf(lfd, "babiau: Can't open %s (daq_start)\n", fname);
	    return 0;
	  }else{
	    if(vm) lfprintf(lfd, "babiau: Open HD[%d][%d]=%s\n", j, i, fname);
	  }

	  /* Time stamp table */
	  if(tst){
	    sprintf(fname, "%s/%s_%s%04d.flt",
		    daqinfo.hdlist[i].path, daqinfo.eflist[j].name, 
		    daqinfo.runname, daqinfo.runnumber);
	    if(!(tfd = fopen(fname, "w"))){
	      if(vm) lfprintf(lfd, "babiau: Can't open %s (daq_start)\n", fname);
	      return 0;
	    }else{
	      if(vm) lfprintf(lfd, "babiau: Create flt file %s\n", fname);
	      fprintf(tfd, "1 %s_%s%04d.ridf\n", daqinfo.eflist[j].name,
		      daqinfo.runname, daqinfo.runnumber);
	      fclose(tfd);
	    }

	    sprintf(fname, "%s/%s_%s%04d.tst",
		    daqinfo.hdlist[i].path, daqinfo.eflist[j].name, 
		    daqinfo.runname, daqinfo.runnumber);
	    if(!(tsfd[j][i] = fopen(fname, "w"))){
	      if(vm) lfprintf(lfd, "babiau: Can't open %s (daq_start)\n", fname);
	      return 0;
	    }else{
	      if(vm) lfprintf(lfd, "babiau: Create tst file %s\n", fname);
	      tsfp[j] = 0;
	    }
	  }
	}
      }
    }
  }
    
  // MT is not implemented, yet

  mkcomment(BABILD_COMMENT_START);

  if(escommand(ES_RUN_START)){
    runinfo.runstat = STAT_RUN_START;

    escommand_pas(ES_RUN_START);
  }else{
    return 0;
  }

  return 1;
}

int daq_nssta(void){
  time_t now;

  DB(printf("daq nssta start\n"));
  if(escommand(ES_RUN_NSSTA)){
    /* Start time */
    time(&now);
    runinfo.starttime = (int)now;
    gloevtn = 0;
    totsize = 0;
    blkn = 0;

    runinfo.runstat = STAT_RUN_NSSTA;
    escommand_pas(ES_RUN_NSSTA);

    if(vm) lfprintf(lfd, "babiau: daq_nssta\n");
  }else{
    return 0;
  }
  
  return 1;
}

int daq_stop(void){
  int ret, len, ebn, fd;
  char lomes[1024] = {0};

  if(vm) lfprintf(lfd, "babild: daq_stop run=%d\n", runinfo.runnumber);

  ret = escommand(ES_RUN_STOP);
  escommand_pas(ES_RUN_STOP);

  if(ret != 1){
    DB(printf("babild: error during ES_RUN_STOP\n"));
    ret = escommand_force(ES_RUN_STOP);
    DB(printf("ret = %d\n", ret));
    if(ret >= 0){
      DB(printf(" daq_stop: # EFN %d is not responding \n", ret));
      sprintf(lomes, " # EFN %d is not responding ", ret);
      strcat(ssmerror, lomes);
    }

    for(ebn=0;ebn<MAXEF;ebn++){
      if(crashefr[ebn]){
	DB(printf("daq_stop: Force End of Run EFN=%d\n", ebn));
	if(vm) lfprintf(lfd, "daq_stop: Force End of Run EFN=%d\n", ebn);
	len = (ebn * -1) - 1;

	if((fd = open(EBFIFO, O_RDWR)) == -1){
	  DB(printf("daq_stop: Can't open %s\n", EBFIFO));
	}else{
	  pthread_mutex_lock(&ebfmutex);
	  write(fd, (char *)&len, sizeof(len));
	  pthread_mutex_unlock(&ebfmutex);
	  close(fd);
	}
      }
    }
  }


  return 1;
}

int daq_close(void){
  int i, j;

  // Close rawdata files
  for(j=0;j<MAXEF;j++){
    for(i=0;i<MAXHD;i++){
      if(hdfd[j][i]){
	fclose(hdfd[j][i]);
	hdfd[j][i] = NULL;
	if(tst){
	  fclose(tsfd[j][i]);
	  tsfd[j][i] = NULL;
	}
      }
    }
  }

  if(vm) lfprintf(lfd, "babiau: daq stopped\n");

  memset((char *)crashefr, 0, sizeof(crashefr));

  return 1;
}

int daq_ender(void){
  //int sock, tcom, tlen;
  //int i;
  //char tbuff[4096];

  if(runinfo.runstat == STAT_RUN_WAITSTOP){
    mkcomment(BABILD_COMMENT_STOP);
  }else{
    return 0;
  }
  
  runinfo.runstat = STAT_RUN_IDLE;
  daq_close();

  /*
  for(i=0;i<MAXEF;i++){
    sock = 0;
    if(daqinfo.eflist[i].ex && daqinfo.eflist[i].of){
      if(!(sock = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT+i, estout))){
      }else{
	tcom = WHOAREYOU;
	tlen = sizeof(tcom);
	send(sock, (char *)&tlen, sizeof(tlen), 0);
	send(sock, (char *)&tcom, tlen, 0);
	recv(sock, (char *)&tlen, sizeof(tlen), MSG_WAITALL);
	recv(sock, (char *)&tbuff, tlen, MSG_WAITALL);

	close(sock);
	sock = 0;

	DB(printf("ES[%d] = %s\n", i, tbuff));

	if(!strncmp(tbuff, "babildes", 8)){
	  // send ender to babildes-babinfo

	  if(!(sock = mktcpsend_tout(daqinfo.eflist[i].host, INFCOMPORT, estout))){
	  }else{
	    tcom = INF_UPDATE_DBRUNSTOP;
	    tlen = sizeof(tcom) + sizeof(runinfo.ender);;
	    send(sock, (char *)&tlen, sizeof(tlen), 0);
	    send(sock, (char *)&tcom, sizeof(tcom), 0);
	    send(sock, runinfo.ender, sizeof(runinfo.ender), 0);

	    close(sock);

	    DB(printf("ES[%d] put ender %s\n", i, runinfo.ender));
	    sock = 0;
	  }
	}
      }
    }
  }
  */

  return 1;
}


/* Store clinfo */
int store_auclinfo(void){
  FILE *fd;
  int i;

  if(!(fd = fopen(AUCLINFORC, "w"))){
    printf("Can't open %s\n", AUCLINFORC);
    return 0;
  }
  for(i=0; i<MAXEF; i++){
    if(auclinfo[i] >= 0){
      fprintf(fd, "%d %d\n", i, auclinfo[i]);
    }
  }
  fclose(fd);

  return 1;
}


/* Store daqinfo to babies.rc */
int store_daqinfo(void){
  FILE *fd;

  if(!(fd = fopen(BABIAURC, "w"))){
    printf("Can't open %s\n", BABIAURC);
    return 0;
  }
  fprintf(fd, "%s\n", daqinfo.runname);
  fprintf(fd, "%d\n", daqinfo.runnumber);
  fprintf(fd, "%d\n", daqinfo.ebsize);
  fprintf(fd, "%d\n", daqinfo.efn);
  fprintf(fd, "%d\n", daqinfo.babildes);
  if(vm) lfprintf(lfd, "babiau: store_daqinfo: babiau.rc updated\n");
  fclose(fd);


  return 1;
}

/* Store mtlist */
int store_mtlist(void){
  FILE *fd;
  int i;

  if(!(fd = fopen(MTLIST, "w"))){
    printf("Can't open %s\n", MTLIST);
    return 0;
  }
  for(i=0; i<MAXMT; i++){
    if(daqinfo.mtlist[i].ex){
      fprintf(fd, "%d %d %s %Lu\n", i, daqinfo.mtlist[i].of,
	     daqinfo.mtlist[i].path, daqinfo.mtlist[i].maxsize);
    }
  }
  fclose(fd);

  return 1;
}


/* check disk size */
int update_disksize(void){
  int i;
  struct statvfs vfs = {0};

  for(i=0; i<MAXHD; i++){
    if(daqinfo.hdlist[i].ex){
      if(statvfs(daqinfo.hdlist[i].path, &vfs) >= 0){
	daqinfo.hdlist[i].full = vfs.f_blocks * vfs.f_bsize;
	daqinfo.hdlist[i].free  = vfs.f_bavail * vfs.f_bsize;
      }
    }
  }

  return 1;
}

/* Store hdlist */
int store_hdlist(void){
  FILE *fd;
  int i;

  if(!(fd = fopen(HDLIST, "w"))){
    printf("Can't open %s\n", HDLIST);
    return 0;
  }
  for(i=0; i<MAXHD; i++){
    if(daqinfo.hdlist[i].ex){
      fprintf(fd, "%d %d %s %Lu\n", i, daqinfo.hdlist[i].of,
	     daqinfo.hdlist[i].path, daqinfo.hdlist[i].maxsize);
    }
  }
  fclose(fd);

  update_disksize();

  return 1;
}

/* store statcom */
int store_statcom(void){
  FILE *fd;
  
  if(!(fd = fopen(STATCOMRC, "w"))){
    lfprintf(lfd, "babiau: can't open statcomrc = %d\n", STATCOMRC);
    return 0;
  }

  fprintf(fd, "%d\n", statcom.id);
  fprintf(fd, "%d\n", statcom.of);
  fprintf(fd, "%s\n", statcom.start);
  fprintf(fd, "%s\n", statcom.stop);
  fclose(fd);

  return 1;
}


/* Store eflist */
int store_eflist(void){
  FILE *fd;
  int i;

  if(!(fd = fopen(EFLIST, "w"))){
    printf("Can't open %s\n", EFLIST);
    return 0;
  }
  for(i=0; i<MAXEF; i++){
    if(daqinfo.eflist[i].ex){
      fprintf(fd, "%d %d %s %s\n", i, daqinfo.eflist[i].of, 
	      daqinfo.eflist[i].name, daqinfo.eflist[i].host);
    }
  }
  fclose(fd);

  return 1;
}

/* Send ssminfo */
int snd_ssminfo(void){
  int sock, len, com;

  if(ssminfo.ex){
    if(!(sock = ssmcon())){
      return 0;
    }
    len = sizeof(com) + sizeof(ssminfo);
    send(sock, (char *)&len, sizeof(len), 0);
    com = SSM_SET_SSMINFO;
    send(sock, (char *)&com, sizeof(com), 0);
    send(sock, (char *)&ssminfo, sizeof(ssminfo), 0);
    close(sock);
  }

  return 1;
}

/* Store ssminfo */
int store_ssminfo(void){
  FILE *fd;

  if(!(fd = fopen(SSMINFO, "w"))){
    printf("Can't open %s\n", SSMINFO);
    return 0;
  }

  fprintf(fd, "%d %d %s\n", ssminfo.ex, ssminfo.of, ssminfo.host);
  fprintf(fd, "%s\n", ssminfo.start);
  fprintf(fd, "%s\n", ssminfo.stop);
  fclose(fd);

  snd_ssminfo();

  return 1;
}

int eb_connect(void){
  struct sockaddr_in caddr;
  int tsock, clen;
  int ebn, ret;
  int opt[2];

  ret = 0;

  clen = sizeof(caddr);
  if((tsock = accept(ebdfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("Error in accept eb_connect\n");
    return 0;
  }
  recv(tsock, (char *)&ebn, sizeof(ebn), MSG_WAITALL);
  ebn = LEFN(ebn);
  if(ebn > 0 && ebn < MAXEF){
    //DB(printf("babiau: eb_connect ebn %d\n", ebn));
    ret = 1;
  }else{
    DB(printf("babiau: eb_connect invalid ebn %d\n", ebn));
    ret = 0;
  }
  send(tsock, (char *)&ret, sizeof(ret), 0);


  if(ret){
    DB(printf("Make EFR thread tsock=%d, ebn=%d\n", tsock, ebn));
    opt[0] = ebn;
    opt[1] = tsock;

    /* EFR thread */
    pthread_create(&efrthre[ebn], NULL, (void *)efrmain, opt);
    pthread_setschedparam(efrthre[ebn], SCHED_RR, &efpar);
    pthread_detach(efrthre[ebn]);
    usleep(1000);

  }else{
    close(tsock);
  }
  

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

/* EB command, get = return values */
void com_get(int sock, char *src, int len){
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, src, len, 0);
}

/* EB command, set = receive values and return values*/
void com_set(int sock, char *src, char *dest, int len){
  int ret;

  memcpy(dest, src+sizeof(int), len-sizeof(int));
  len = sizeof(ret);
  ret = 1;
  com_get(sock, (char *)&ret, len);
}

/* EB command, rejected */
void com_rej(int sock){
  int ret = 0, len;

  len = sizeof(ret);
  com_get(sock, (char *)&ret, len);
}

/* Communication port */
int commain(void){
  struct sockaddr_in caddr;
  char buff[EB_BUFF_SIZE], sret[EB_BUFF_SIZE];
  int sock, clen, len;
  int com, ret, tebsize;
  int isasc = 0, slen, asccom = 0;
  char *ascret = NULL;

  clen = sizeof(caddr);
  if((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("Error in accept commain\n");
    return 0;
  }

  recv(sock, (char *)&len, sizeof(int), MSG_WAITALL);
  if(len > EB_BUFF_SIZE){
    isasc = 1;
  }

  if(!isasc){
    recv(sock, buff, len, MSG_WAITALL);
  }else{
    slen = recv(sock, buff, EBDEFSIZE, 0);
    buff[slen] = '\0';
    if(runinfo.runstat && chksetcom(buff)){
      com = 0;
      sprintf(buff, "error\nNow running, can't change parameters.");
      ascret = buff;
    }else{
      ascret = parsetextcom(buff, &asccom);
    }
  }

  /* babiau command */
  if(!isasc) memcpy((char *)&com, buff, sizeof(com));
  switch(com){
  case EB_GET_DAQINFO:
    //DB(printf("babiau: EB_GET_ALL\n"));
    update_disksize();
    len = sizeof(daqinfo);
    com_get(sock, (char *)&daqinfo, len);
    break;
  case EB_SET_DAQINFO:
    //DB(printf("babiau: EB_SET_ALL\n"));
    if(!runinfo.runstat){
      tebsize = daqinfo.ebsize;
      com_set(sock, buff, (char *)&daqinfo, len);
      if(tebsize != daqinfo.ebsize){
	if(daqinfo.ebsize > EB_EFBLOCK_MAXSIZE){
	  daqinfo.ebsize = EB_EFBLOCK_MAXSIZE;
	}
	if(vm) lfprintf(lfd, "babiau: ebsize changed %d\n", daqinfo.ebsize);
      }
      runinfo.runnumber = daqinfo.runnumber;
      store_eflist();
      store_mtlist();
      store_hdlist();
      store_daqinfo();
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change daqinfo\n");
      com_rej(sock);
    }
    break;
  case EB_GET_RUNINFO:
    //DB(printf("babiau: EB_GET_RUNINFO\n"));
    len = sizeof(runinfo);
    com_get(sock, (char *)&runinfo, len);
    break;
  case EB_SET_RUNINFO:
    //DB(printf("babiau: EB_SET_RUNINFO\n"));
    if(!runinfo.runstat){
      com_set(sock, buff, (char *)&runinfo, len);
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change runinfo\n");
      com_rej(sock);
    }
    break;
  case EB_SET_HEADER:
    //DB(printf("babiau: EB_SET_HEADER\n"));
    if(!runinfo.runstat){
      com_set(sock, buff, runinfo.header, len);
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change runinfo\n");
      com_rej(sock);
    }
    break;
  case EB_SET_ENDER:
    //DB(printf("babiau: EB_SET_ENDER\n"));
    if(runinfo.runstat == STAT_RUN_WAITSTOP){
      com_set(sock, buff, runinfo.ender, len);
      DB(printf("ender = %s\n", runinfo.ender));
    }else{
      if(vm) lfprintf(lfd, "babiau: now not waiting stop, can't change ender\n");
      com_rej(sock);
    }
    break;
  case EB_GET_EFNUM:
    //DB(printf("babiau: EB_GET_EFNUM %d\n", efnum));
    len = sizeof(efnum);
    com_get(sock, (char *)&efnum, len);
    break;
  case EB_GET_EFLIST:
    //DB(printf("babiau: EB_GET_EFLIST\n"));
    len = sizeof(daqinfo.eflist);
    com_get(sock, (char *)daqinfo.eflist, len);
    break;
  case EB_SET_EFLIST:
    //DB(printf("babiau: EB_SET_EFLIST\n"));
    if(!runinfo.runstat){
      len = sizeof(daqinfo.eflist);
      com_set(sock, buff, (char *)daqinfo.eflist, len);
      store_eflist();
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change eflist.\n");
      com_rej(sock);
    }
    break;
  case EB_GET_MTLIST:
    //DB(printf("babiau: EB_GET_MTLIST\n"));
    len = sizeof(daqinfo.mtlist);
    com_get(sock, (char *)daqinfo.mtlist, len);
    break;
  case EB_SET_MTLIST:
    //DB(printf("babiau: EB_SET_MTLIST\n"));
    if(!runinfo.runstat){
      len = sizeof(daqinfo.mtlist);
      com_set(sock, buff, (char *)daqinfo.mtlist, len);
      store_mtlist();
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change mtlist.\n");
      com_rej(sock);
    }
    break;
  case EB_GET_HDLIST:
    //DB(printf("babiau: EB_GET_HDLIST\n"));
    update_disksize();
    len = sizeof(daqinfo.hdlist);
    com_get(sock, (char *)daqinfo.hdlist, len);
    break;
  case EB_GET_EVTN:
    ret = gloevtn;
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    break;
  case EB_GET_TOTSIZE:
    len = sizeof(totsize);
    com_get(sock, (char *)&totsize, len);
    break;
  case EB_SET_HDLIST:
    //DB(printf("babiau: EB_SET_HDLIST\n"));
    if(!runinfo.runstat){
      len = sizeof(daqinfo.hdlist);
      com_set(sock, buff, (char *)daqinfo.hdlist, len);
      store_hdlist();
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change hdlist.\n");
      com_rej(sock);
    }
    break;
  case EB_GET_SSMINFO:
    //DB(printf("babiau: EB_GET_SSMINFO\n"));
    len = sizeof(ssminfo);
    com_get(sock, (char *)&ssminfo, len);
    break;
  case EB_SET_SSMINFO:
    //DB(printf("babiau: EB_SET_SSMINFO\n"));
    if(!runinfo.runstat){
      len = sizeof(ssminfo);
      com_set(sock, buff, (char *)&ssminfo, len);
      store_ssminfo();
    }else{
      if(vm) lfprintf(lfd, "babiau: now running, can't change ssminfo.\n");
      com_rej(sock);
    }
    break;
  case EB_RUN_START:
  case EB_RUN_NSSTA:
    //DB(printf("babiau: Run start\n"));
    if(cntefon(daqinfo.eflist)){
      if(!runinfo.runstat && !daqinfo.babildes){
	if(com == EB_RUN_START){
	  if(daq_start()){
	    if(ssminfo.ex && ssminfo.of) ssm_start();
	    ret = 1;
	  }else{
	    ret = 0;
	  }
	}else{
	  if(daq_nssta()){
	    if(ssminfo.ex && ssminfo.of) ssm_start();
	    ret = 1;
	  }else{
	    ret = 0;
	  }
	}
      }else{
	if(vm) lfprintf(lfd, "babiau: already starting or babildes mode\n");
	if(runinfo.runstat){
	  sprintf(esret, "already starting");
	}else if(daqinfo.babildes){
	  sprintf(ssmerror, " this babiau = babildes mode ");
	}
	ret = 0;
      }
    }else{
      if(vm) lfprintf(lfd, "babiau: number of eflist.on = 0\n");
      sprintf(esret, "number of eflist.on = 0\n");
      ret = 0;
    }
    if(ret){
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }else{
      memset(sret, 0, sizeof(sret));
      len = sizeof(ret) + strlen(esret) + 1;
      memcpy(sret, (char *)&ret, sizeof(ret));
      memcpy(sret + sizeof(ret), esret, strlen(esret));
      sret[len] = 0;
      com_get(sock, sret, len);
    }
    break;
  case EB_RUN_STOP:
    //DB(printf("babiau: Run stop\n"));
    if(runinfo.runstat){
      memset(ssmerror, 0, sizeof(ssmerror));
      if(ssminfo.ex && ssminfo.of) ssm_stop();
      daq_stop();
      ret = 1;
    }else{
      if(vm) lfprintf(lfd, "babiau: not starting or babildes mode\n");

      if(!runinfo.runstat){
	sprintf(ssmerror, " runstatus=IDLE");
      }else if(daqinfo.babildes){
	sprintf(ssmerror, " this babild = babildes mode ");
      }	
      ret = 0;
    }
    memset(sret, 0, sizeof(sret));
    len = sizeof(ret);
    memcpy(sret, (char *)&ret, sizeof(ret));

    DB(printf("babild: EB_RUN_STOP ssmstrlen=%d ret=%d\n", (int)strlen(ssmerror), ret));
    if(strlen(ssmerror)){
      len += strlen(ssmerror) + 1;
      memcpy(sret + sizeof(ret), ssmerror, strlen(ssmerror));
      memcpy((char *)&ret, sret, 4);
      DB(printf("babild: ret=%d / %d\n", ret, len));
    }
    com_get(sock, (char *)&sret, len);
    break;
  case EB_RUN_CLOSE:
    //DB(printf("babild: Run close\n"));
    if(runinfo.runstat == STAT_RUN_WAITSTOP){
      daq_ender();
      ret = 1;
    }else{
      if(vm) lfprintf(lfd, "babild: not waiting stop\n");
      ret = 0;
    }
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    break;
  case EB_CHK_DIR:
    //DB(printf("babild: EB_CHK_DIR\n"));
    ret = isdir(buff+sizeof(com));
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);
    break;
  case EB_SET_RUNNUMBER:
    //DB(printf("babild: EB_SET_RUNNUMBER\n"));
    if(!runinfo.runstat){
      com_set(sock, buff, (char *)&daqinfo.runnumber, len);
      runinfo.runnumber = daqinfo.runnumber;
    }else{
      if(vm) lfprintf(lfd, "babild: now running, can't change runnumber\n");
      com_rej(sock);
    }
    break;
  case EB_SET_RUNNAME:
    //DB(printf("babild: EB_SET_RUNNAME\n"));
    if(!runinfo.runstat){
      com_set(sock, buff, daqinfo.runname, len);
    }else{
      if(vm) lfprintf(lfd, "babild: now running, can't change runname\n");
      com_rej(sock);
    }
    break;
  case EB_SET_EBSIZE:
    //DB(printf("babild: EB_SET_EBSIZE\n"));
    if(!runinfo.runstat){
      com_set(sock, buff, (char *)&daqinfo.ebsize, len);
      store_daqinfo();
    }else{
      if(vm) lfprintf(lfd, "babild: now running, can't change ebsize\n");
      com_rej(sock);
    }
    break;
  case EB_SET_BABILDES:
    //DB(printf("babild: EB_SET_BABILDES\n"));
    com_rej(sock);
    if(vm) lfprintf(lfd, "babiau: babiau can't change babildes mode\n");

    break;
  case EB_SET_CHKERHOST:
    if(!runinfo.runstat){
      com_set(sock, buff, (char *)&fchkerhost, len);
      store_daqinfo();
    }else{
      if(vm) lfprintf(lfd, "babild: now running, can't change babildes mode\n");
      com_rej(sock);
    }
    break;
  case EB_GET_CHKERHOST:
    com_get(sock, (char *)&fchkerhost, sizeof(chkerhost));
    break;
  case WHOAREYOU:
    DB(printf("babiau : WHOAREYOU\n"));
    len = sizeof(thisname);
    com_get(sock, thisname, len);
    break;
  case EB_QUIT:
    DB(printf("babild: EB_QUIT\n"));
    if(!runinfo.runstat){
      ret = 1;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
      escommand(ES_QUIT);
      quit();
    }else{
      DB(printf("babild: EB_QUIT but now running\n"));
      ret = 0;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }
    break;
  case EB_SET_STAT_COMMAND:
    if(!runinfo.runstat){
      com_set(sock, buff, (char *)&statcom, len);
      store_statcom();
    }else{
      ret = 0;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
    }
    break;
  case EB_GET_STAT_COMMAND:
    len = sizeof(statcom);
    com_get(sock, (char *)&statcom, len);
    break;
  default:
    /* noop */
    break;
  }

  if(isasc){
    if(asccom & ASC_EB_SET_DAQINFO){
      if(daqinfo.ebsize > EB_EFBLOCK_MAXSIZE){
	daqinfo.ebsize = EB_EFBLOCK_MAXSIZE;
      }
      runinfo.runnumber = daqinfo.runnumber;
      store_eflist();
      store_mtlist();
      store_hdlist();
      store_daqinfo();
    }
    if(asccom & ASC_EB_SET_EFLIST){
      store_eflist();
    }
    if(asccom & ASC_EB_SET_MTLIST){
      store_mtlist();
    }
    if(asccom & ASC_EB_SET_HDLIST){
      store_hdlist();
    }
    if(asccom & ASC_EB_SET_SSMINFO){
      store_ssminfo();
    }
    if(asccom & ASC_EB_RUN_START || asccom & ASC_EB_RUN_NSSTA){
      if(cntefon(daqinfo.eflist)){
	if(!runinfo.runstat){
	  if(asccom & ASC_EB_RUN_START){
	    daq_start();
	  }else{
	    daq_nssta();
	  }
	  if(ssminfo.ex && ssminfo.of) ssm_start();
	}else{
	  if(vm) lfprintf(lfd, "babild: already starting or babildes mode\n");
	}
      }else{
	if(vm) lfprintf(lfd, "babild: number of eflist.on = 0\n");
      }
    }

    if(asccom & ASC_EB_RUN_STOP){
      if(runinfo.runstat){
	if(ssminfo.ex && ssminfo.of) ssm_stop();
	daq_stop();
      }else{
	if(vm) lfprintf(lfd, "babild: not starting or babildes mode\n");
      }
    }
    if(asccom & ASC_EB_RUN_CLOSE){
      if(runinfo.runstat == STAT_RUN_WAITSTOP){
	daq_ender();
      }else{
	if(vm) lfprintf(lfd, "babild: not waiting stop\n");
      }
    }
    if(asccom & ASC_EB_SET_RUNNUMBER){
      runinfo.runnumber = daqinfo.runnumber;
    }
    if(asccom & ASC_EB_SET_RUNNAME || asccom & ASC_EB_SET_EBSIZE){
      store_daqinfo();
    }
    if(asccom & ASC_EB_SET_BABILDES || asccom & ASC_EB_SET_EFN){
      store_daqinfo();
    }
    if(asccom & ASC_EB_QUIT){
      if(!runinfo.runstat){
	escommand(ES_QUIT);
	quit();
      }
    }

    len = strlen(ascret);
    //DB(printf("len=%d : ret=%s\n", len, ascret));
    send(sock, ascret, len, 0);
  }

  close(sock);
  return 1;
}


/************************* infmain ****************************/
int infmain(void){
  struct sockaddr_in caddr;
  int sock, clen, ret;
  int com, len;

  clen = sizeof(caddr);
  if((sock = accept(inffd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("babiau: Error in accept commain\n");
    return 0;
  }

  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, combuff, len, MSG_WAITALL);
  memcpy((char *)&com, combuff, sizeof(com));

  DB(printf("babiau: commain com=%d\n", com));
  
  switch(com){
  case INF_GET_DAQINFO:
    len = sizeof(daqinfo);
    com_get(sock, (char *)&daqinfo, len);
    break;
  case INF_GET_RUNINFO:
    len = sizeof(runinfo);
    com_get(sock, (char *)&runinfo, len);
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
  case AU_SET_CLINFO:
    if(!runinfo.runstat){
      com_set(sock, combuff, (char *)auclinfo, len);
      store_auclinfo();
    }else{
      com_rej(sock);
    }
    break;
  case AU_GET_CLINFO:
    len = sizeof(auclinfo);
    com_get(sock, (char *)auclinfo, len);
    break;
  default:
    com_rej(sock);

    break;
  }


  close(sock);

  return 1;
}


/*********************   Store EF to Memory    **********************/
int storeef(char *buff){
  RIDFRHD ghd;
  RIDFHD ighd;

  memcpy((char *)&ighd, buff, sizeof(ighd));
  ghd = ridf_dechd(ighd);

  if(ghd.classid != RIDF_EF_BLOCK 
     && ghd.classid != RIDF_EAF_BLOCK){
    if(vm) lfprintf(lfd, "babild: storeef: Invalid class id %d from efn %d \n",  ghd.classid, ghd.efn);
    return 0;
  }

  // Change classid to RIDF_EA_BLOCK
  ghd.classid = RIDF_EA_BLOCK;
  ighd = ridf_mkhd(ghd.layer, ghd.classid, ghd.blksize, ghd.efn);
  memcpy(buff, (char *)&ighd, sizeof(ighd));

  //DB(printf("New block ly=%d, ci=%d, sz=%d, en=%d\n",
  //    ghd.layer, ghd.classid, ghd.blksize, ghd.efn));

  return ghd.blksize;
}

/*********************  Event Reseiver Thread  **********************/
int efrmain(void *opt){
  int len, er, sz;
  int ebn, sock, i, tssz = 0;
  unsigned long long int tsbuff[16384];
  char *buff = NULL;
  int ebffdt = 0;

  memcpy((char *)&ebn, opt, 4);
  memcpy((char *)&sock, opt+4, 4);

  buff = malloc(EB_EFBLOCK_BUFFSIZE);
  if(buff == NULL){
    if(vm) lfprintf(lfd, "efrmain[%d] Cannot malloc \n", ebn);
    return 0;
  }


  /* Open FIFO FD */
  if((ebffdt = open(EBFIFO, O_RDWR)) == -1){
    printf("efrmain[%d]: Can't open %s\n", ebn, EBFIFO);
    quit();
  }
  if(vm) lfprintf(lfd, "efrmain[%d]: ebffd number = %d\n", ebn, ebffdt);
  DB(printf("efrmain[%d]: buffat=%p sock=%d\n", ebn, buff, sock));

  memset(buff, 0, EB_EFBLOCK_BUFFSIZE);

  while(1){
    DB(printf("efrmain[%d] wait recv\n", ebn));
    if((er = recv(sock, (char *)&len, sizeof(len), MSG_WAITALL)) < 1){
      if(vm) lfprintf(lfd, "efrmain[%d] : Error in recv er=%d\n", ebn, er);
      DB(printf("efrmain[%d] : EF%d closed\n", ebn, ebn));
      break;
    }
      /*
	if(er == 0){
	lfprintf(lfd, "continue while loop\n");
	continue;
	}else{
	if(vm) lfprintf(lfd, "efrmain[%d] : put dummy end of Run EFN=%d\n", ebn, ebn);
	len = (ebn * -1) - 1;
	
	pthread_mutex_lock(&ebfmutex);
	write(ebffdt, (char *)&len, sizeof(len));
	pthread_mutex_unlock(&ebfmutex);
	
	if(vm) lfprintf(lfd, "efrmain[%d] : This thread exit\n");
	close(ebffdt);
	close(sock);
	pthread_exit(NULL);
	break;
	}
	}
      */
    DB(printf("efrmain[%d] len=%d\n", ebn, len));
    if(len > 0){
      if((er = recv(sock, buff, len, MSG_WAITALL)) < 1){
	if(vm) lfprintf(lfd, "efrmain[%d] : Error in recv data er=%d\n", ebn, er);
      }	
      DB(printf("efrmain[%d] : try storeef\n", ebn));
      sz = storeef(buff);

      if(tst) tssz = exttst((short *)buff, sz, tsbuff, &tsfp[ebn]); 

      /* Data store */
      if(runinfo.runstat == STAT_RUN_START){
	for(i=0;i<MAXHD;i++){
	  if(hdfd[ebn][i]){
#ifndef NOFLOCK
	    flock(fileno(hdfd[ebn][i]), LOCK_EX);
#endif
	    fwrite(buff, 2, sz, hdfd[ebn][i]);  // Write into HD
#ifndef NOFLOCK
	    flock(fileno(hdfd[ebn][i]), LOCK_UN);
#endif
	    if(tst){
#ifndef NOFLOCK
	      flock(fileno(tsfd[ebn][i]), LOCK_EX);
#endif
	      fwrite(tsbuff, sizeof(long long int), tssz, tsfd[ebn][i]);  // Write into HD
#ifndef NOFLOCK
	      flock(fileno(tsfd[ebn][i]), LOCK_UN);
#endif
	    }
	  }
	}
      }
      
      if(auclinfo[ebn] != -1 && clinfo[auclinfo[ebn]].ex){
	sendto(udpsock, buff, sz*2, 0, 
		  (struct sockaddr *)&cliaddr[auclinfo[ebn]],
		  sizeof(cliaddr[auclinfo[ebn]]));
      }
      
    }else{
      DB(printf("efrmain[%d]: End of Run EFN=%d\n", ebn, ebn));
      if(vm) lfprintf(lfd, "efrmain[%d]: End of Run EFN=%d\n", ebn, ebn);
      len = (ebn * -1) - 1;

      pthread_mutex_lock(&ebfmutex);
      write(ebffdt, (char *)&len, sizeof(len));
      pthread_mutex_unlock(&ebfmutex);

    }
    usleep(1000);
  }


  if(runinfo.runstat != STAT_RUN_IDLE){
    crashefr[ebn] = 1;
  }
  
  if(ebffdt) close(ebffdt);
  if(buff) free(buff);

  return 1;
}


/*********************   Event Build Thread    *********************/
int ebmain(void){
  int n, st, i;
  fd_set ebfdset;
  time_t now;

  n = 0;

  /* Open FIFO FD */
  if((ebffd = open(EBFIFO, O_RDWR)) == -1){
    printf("ebmain: Can't open %s\n", EBFIFO);
    quit();
  }
  
  while(1){
    FD_ZERO(&ebfdset);
    FD_SET(ebffd, &ebfdset);
    DB(printf("ebmain: selecting\n"));
    if(select(ebffd+1, &ebfdset, NULL, NULL, NULL) != 0){
      DB(printf("ebmain: try lock\n"));
      pthread_mutex_lock(&ebfmutex);
      DB(printf("ebmain: try lock-in\n"));
      read(ebffd, (char *)&n, sizeof(n));
      pthread_mutex_unlock(&ebfmutex);
      DB(printf("ebmain: try lock-out n=%d\n", n));

      if(n < 0){
	n = (n * -1) - 1;
	DB(printf("ebmain: End of run from %d\n", n));

	efrun[n] = 0;
	st = 0;
	for(i=0;i<MAXEF;i++){
	  if(efrun[i]) st++;
	}
        if(vm) lfprintf(lfd, "ebmain: End of Run from=%d remain=%d\n", n, st);
	if(!st){
	  DB(printf("ebmain: All EF run stoped\n"));
          if(vm) lfprintf(lfd, "ebmain: All EF run stoped=%d\n", n);

	  time(&now);
	  runinfo.stoptime = now;

	  if(runinfo.runstat == STAT_RUN_START){
	    runinfo.runstat = STAT_RUN_WAITSTOP;
            if(vm) lfprintf(lfd, "ebmain: goto WAITSTOP=%d\n");
	  }else{
	    if(runinfo.runstat == STAT_RUN_START){
	      mkcomment(BABILD_COMMENT_STOP);
	    }
	    runinfo.runstat = STAT_RUN_IDLE;
	    daq_close();
	  }
	}
      }else{
	if(vm) lfprintf(lfd, "ebmain: Error n=%d\n", n);
      }
    }
  }
  
  if(ebffd) close(ebffd);

  return 1;
}


/*********************         Main            *********************/
/*! Mail loop of babiau */
int main(int argc,char *argv[]){
  int maxfd = 0, i, j, tef, ti, tof, pid;
  int tn, tid;
  u64 tmx;
  char tch[80], tch2[80], lfname[80];
  FILE *fd;
  int mton=0;
  time_t now;
  unsigned int ftime;

  if(sscanf(argv[argc-1], "%d", &argefn) != 1){
    argefn = 0;
  }

  /* PID file */
  if(chkpid("babiau")){
    printf("babiau: Error, another babiau may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babiau\n");
    exit(0);
  }

#ifndef DEBUG
  if(!(int)getuid()) nice(-20);
  daemon(1, 0);
#endif

  gloevtn = 0;
  memset(aliasnames, 0, sizeof(aliasnames));

  // To know my ipaddress
  getmyaddress(myhost, mydom, myip);


  /* Change working directory */
  /* BABIRLDIR = installed dir */
  if(getenv("BABIRLDIR")){
    chdir(getenv("BABIRLDIR"));
  }else{
    chdir(BABIRLDIR);
  }


  /* Initialize */
  memset((char *)&daqinfo, 0, sizeof(daqinfo));
  for(i=0;i<MAXEF;i++){
    auclinfo[i] = -1;
  }

  for(i=0;i<MAXEF;i++){
    daqinfo.eflist[i].ex = 0;
    daqinfo.eflist[i].of =  EB_EFLIST_OFF;
    ebffdt[i] = -1;
    efrun[i] = 0;
  }
  for(i=0;i<MAXHD;i++){
    for(j=0;j<MAXEF;j++){
      hdfd[j][i] = NULL;
      tsfd[j][i] = NULL;
    }
  }
  for(i=0;i<MAXMT;i++){
    mtfd[i] = NULL;
  }

  /* Check command line option */
  if(!chkopt(argc,argv)){
    exit(0);
  }

  /* PID file */
  if(!(pid = mkpid("babiau"))){
    printf("babiau: Error, another babiau may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babiau\n");
  }
  

  /* Signal SIGINT -> quit() */
  signal(SIGINT,(void *)quit);

  if(vm){
    time(&now);
    ftime = (unsigned int)now;
    if(!isdir("log")){
      mkdir("log", 0775);
    }
    sprintf(lfname, "log/babiau%d.log", ftime);
    if(!(lfd = fopen(lfname, "w"))){
      printf("Can't make log file %s\n", lfname);
      quit();
    }
  }

  if(vm) lfprintf(lfd, "babiau: Start PID=%d\n", pid);

  /* Make FIFO to communicate with babier */
  unlink(EBFIFO);
  if(mkfifo(EBFIFO, 0666) == -1){
    perror("babiau");
    printf("babiau: Can't make %s\n",EBFIFO);
    quit();
  }
  if(vm) lfprintf(lfd, "babiau: Make FIFO %s\n",EBFIFO);

  /* Make command port */
  if((comfd = mktcpsock(EBCOMPORT)) == -1) quit();
  if(vm) lfprintf(lfd, "babiau: comfd number = %d\n", comfd);
  if(comfd > maxfd) maxfd = comfd;

  /* Make EB data port */
  if((ebdfd = mktcpsock(ERRCVPORT)) == -1) quit();
  if(vm) lfprintf(lfd, "babiau: ebfd number = %d\n", ebdfd);
  if(ebdfd > maxfd) maxfd = ebdfd;

  /* Make information port */
  if((inffd = mktcpsock(INFCOMPORT)) == -1){
    printf("babiau: Can't make command and oneshot port\n");
    quit();
  }
  if(vm) lfprintf(lfd, "babiau: inffd number = %d\n", inffd);
  if(inffd > maxfd) maxfd = inffd;


  /* UDP multisender */
  udpsock = mkmultisend();

  /* clihosts */
  if((fd = fopen(CLIHOSTSRC, "r"))){
    while(fscanf(fd, "%d %s %d", &tn, tch, &tid) == 3){
      clinfo[tn].ex = tid + 1;
      strcpy(clinfo[tn].clihost, tch);
      DB(printf("babiau: clihosts %d %s %d\n", tn, clinfo[tn].clihost, tid));
    }
    fclose(fd);
  }
  update_clihosts();

  /* auclinfo */
  if((fd = fopen(AUCLINFORC, "r"))){
    while(fscanf(fd, "%d %d", &tn, &tid) == 2){
      auclinfo[tn] = tid;
    }
    fclose(fd);
  }

  /* Event fragment */
  efnum =0;
  if((fd = fopen(EFLIST, "r"))){
    while(!feof(fd)){
      fscanf(fd, "%d %d %s %s\n", &tef, &tof, tch, tch2);
      if(tef >= 0 && tef <= MAXEF){
	efnum ++;
	daqinfo.eflist[tef].ex = 1;
	daqinfo.eflist[tef].of = tof;
	strncpy(daqinfo.eflist[tef].name, tch,
		sizeof(daqinfo.eflist[tef].name));
	strncpy(daqinfo.eflist[tef].host, tch2,
		sizeof(daqinfo.eflist[tef].host));
	if(vm) lfprintf(lfd, "EF %d : %s (%s)\n", tef,
		      daqinfo.eflist[tef].name, ofstr[daqinfo.eflist[tef].of]);
      }
    }
    fclose(fd);
  }

  if((fd = fopen(MTLIST, "r"))){
    while(!feof(fd)){
      if(fscanf(fd, "%d %d %s %Lu", &ti, &tof, tch, &tmx) == 4){
	if(ti >= 0 && ti < MAXMT){
	  daqinfo.mtlist[ti].ex = 1;
	  daqinfo.mtlist[ti].of = tof;
	  if(tmx < 1) tmx = MTMAXSIZE;
	  daqinfo.mtlist[ti].maxsize = tmx;
	  strcpy(daqinfo.mtlist[ti].path, tch);
	  mton++;
	  if(vm) lfprintf(lfd, "MT%d %d %s %Lu\n", ti, tof, 
			daqinfo.mtlist[ti].path,
			daqinfo.mtlist[ti].maxsize);
	}
      }
    }
    fclose(fd);
  }
  if((fd = fopen(HDLIST, "r"))){
    while(!feof(fd)){
      if(fscanf(fd, "%d %d %s %Lu", &ti, &tof, tch, &tmx) == 4){
	if(ti >= 0 && ti < MAXHD){
	  daqinfo.hdlist[ti].ex = 1;
	  daqinfo.hdlist[ti].of = tof;
	  if(tmx < 1) tmx = HDMAXSIZE;
	  daqinfo.hdlist[ti].maxsize = tmx;
	  strcpy(daqinfo.hdlist[ti].path, tch);
	  if(vm) lfprintf(lfd, "HD%d %d %s %Lu\n", ti, tof, 
			daqinfo.hdlist[ti].path,
			daqinfo.hdlist[ti].maxsize);
	}
      }
    }
    fclose(fd);
  }else if(!mton){
    daqinfo.hdlist[0].ex = 1;
    daqinfo.hdlist[0].of = 1;
    daqinfo.hdlist[0].maxsize = HDMAXSIZE;
    strcpy(daqinfo.hdlist[0].path, "./");
    if(vm) lfprintf(lfd, "HD%d %s %Lu\n", 0, daqinfo.hdlist[0].path,
		  daqinfo.hdlist[0].maxsize);
  }    

  if((fd = fopen(BABIAURC, "r"))){
    fscanf(fd, "%s", daqinfo.runname);
    fscanf(fd, "%d", &daqinfo.runnumber);
    fscanf(fd, "%d", &daqinfo.ebsize);

    // Usual max event build buffer = 1MB, not 8MB
    if(daqinfo.ebsize > EB_EFBLOCK_MAXSIZE){
      daqinfo.ebsize = EB_EFBLOCK_MAXSIZE;
    }
    fscanf(fd, "%d", &daqinfo.efn);
    fscanf(fd, "%d", &daqinfo.babildes);
    daqinfo.babildes = 0;
    if(vm) lfprintf(lfd, "babiau.rc: name=%s, runnumber=%d, ebsize=%d\n",
		  daqinfo.runname, daqinfo.runnumber, daqinfo.ebsize);
    fclose(fd);
  }else{
    strcpy(daqinfo.runname, "data");
    daqinfo.runnumber = 0;
    daqinfo.efn = 1;
    daqinfo.ebsize = EBDEFSIZE;
    daqinfo.babildes = 0;
  }

  if((fd = fopen(STATCOMRC, "r"))){
    fscanf(fd, "%d", &statcom.id);
    fscanf(fd, "%d", &statcom.of);
    fscanf(fd, "%s", statcom.start);
    fscanf(fd, "%s", statcom.stop);
    fclose(fd);
  }else{
    memset((char *)&statcom, 0, sizeof(statcom));
  }



  if(argefn){
    daqinfo.efn = argefn;
    if(vm) lfprintf(lfd, "babiau: EFN is set %d by arg\n", daqinfo.efn);
  }

  store_daqinfo();


  /* ssm */
  memset((char *)&ssminfo, 0, sizeof(ssminfo));
  if((fd = fopen(SSMINFO, "r"))){
    fscanf(fd, "%d %d %s\n", &ssminfo.ex, &ssminfo.of, ssminfo.host);
    fgets(ssminfo.start, sizeof(ssminfo.start), fd);
    fgets(ssminfo.stop, sizeof(ssminfo.stop), fd);
    lfz(ssminfo.start);
    lfz(ssminfo.stop);
    fclose(fd);
  }
  store_ssminfo();


  runinfo.runnumber = daqinfo.runnumber;
  
  /* Event build thread */
  ebpar.sched_priority = sched_get_priority_max(SCHED_RR) - 5;
  efpar.sched_priority = sched_get_priority_max(SCHED_RR) - 6;

  pthread_mutex_init(&ebfmutex, NULL);
  pthread_create(&ebthre, NULL, (void *)ebmain, NULL);
  pthread_setschedparam(ebthre, SCHED_RR, &ebpar);
  pthread_detach(ebthre);

  while(1){
    /* Main loop */
    DB(printf("babiau: Main loop\n"));
    
    /* prepaire fd set for select() */
    FD_ZERO(&fdset);
    FD_SET(ebdfd, &fdset);
    FD_SET(comfd, &fdset);
    FD_SET(inffd, &fdset);

    if(select(maxfd+1, &fdset,NULL,NULL,NULL) != 0){
      DB(printf("kita-----------!!\n"));
      if(FD_ISSET(ebdfd, &fdset)){
	DB(printf("babiau: select from eb data port\n"));
	eb_connect();
      }else if(FD_ISSET(comfd, &fdset)){
	DB(printf("babiau: select from communication port\n"));
	commain();
      }else if(FD_ISSET(inffd, &fdset)){
	DB(printf("babiau: select from inf communication port\n"));
	infmain();
      }
    }
  }

  return 0;
}
