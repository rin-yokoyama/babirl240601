/** devtool/anystore
 *  last modified : 
 *  TCP Client for babinfo
 *  Store any online data any time
 * 
 *  Hidetada Baba (RIKEN)
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

enum DMODE {DTIME, DRUN, DSINGLE};

int sfd = 0, sock = 0;
char *buff;

int mode = DRUN;
char dir[256]={0};
char pre[256]={0};
char host[256]={0};
float clean = 0.3;
int limit = 2000;
int min = 1;
int ut = 0; // unix time
int ns = 0; // no save (1=nosave, 0=save)
int fheader, fender, fruninfo = 0;
FILE *fd = NULL;
time_t startt = 0;
int frun = 0;
unsigned long long int cursize = 0; // byte unit

struct stdaqinfo daqinfo;
struct struninfo runinfo;


void quit(void);
void help(void);
void mkheader(void);
void mkender(void);
void store(void);
int deloldest(void);

struct option long_options[] = {
  {"help"         , no_argument      , NULL, 'h'},
  {"single"       , no_argument      , NULL, 's'},
  {"run"          , no_argument      , NULL, 'r'},
  {"directory"    , required_argument, NULL, 'd'},
  {"prefix"       , required_argument, NULL, 'p'},
  {"clean"        , required_argument, NULL, 'c'},
  {"limit"        , required_argument, NULL, 'l'},
  {"time"         , required_argument, NULL, 't'},
  {"unixtime"     , no_argument      , NULL, 'u'},
  {"myport"       , required_argument, NULL, 'm'},
  {"myip"         , required_argument, NULL, 'i'},
  {"nosave"       , no_argument,       NULL, 'n'},
  {0, 0, 0, 0}
};


void optcpy(char *dest, char *srv){
  strncpy(dest, srv, 255);
}


void help(void){
  printf("anystore [OPTION]... HOSTNAME\n");
  printf("=== MODE === (default mode = run)\n");
  printf(" -t / --time MINUTES    : divide file by given interval regardless run condition\n");
  printf("                        : PREFIXtime_20200413095757.ridf (2020, Apr, 09:57:57)\n");
  printf("                        : with --t / --unixtime option, PREFIXtime_1586738477.ridf\n");
  printf(" -r / --run             : follow runname and runnumber, PREFIXrunname_runnumber.ridf\n");
  printf("                        : nssta case, PREFIXnssta.ridf\n");
  printf(" -s / --single          : data store to a single file even runnumber is changed (PREFIXsingle_DATE.ridf)\n");
  printf("=== FILE Options ===\n");
  printf(" -d / --directory       : specify directory files to be stored\n");
  printf(" -p / --prefix PREFIX   : prefix of the file name (default=any_)\n");
  printf(" -l / --limit MBYTES    : limit the single file size (default is 2000MB)\n");
  printf("                        : exceeds the limit, new files are created with new date time\n");
  printf("                        : if the mode is run, a suffix will be added like _1.ridf, _2.ridf\n");
  printf("                        : if the limit is 0, the file size is unlimited\n");
  printf(" -c / --clean RATIO     : limit the single file size (default is 0.3 of the partition size)\n");
  printf("                        : when the available disk space is < 0.3, old ridf files are deleted\n");
  printf("                        : with -a option, when the disk space is < 0.3, the program will be terminated\n");
  printf(" -n / --nosave          : Do not save data to the disk (debug purpose)\n");
  printf("=== Program Options ===\n");
  printf(" -a / --autoend         : automatically terminate program due to the number of files or disk size\n");
  printf("=== CONNECTION Options ===\n");
  printf(" -m / --myport MYPORT   : specify my port number (default=17622)\n");
  printf(" -i / --myip MYIP       : specify my ip address (only if my hostname is not vaild)\n");
  printf("\n");
  printf("** if available disk spalce is less than 0.05, this program will be terminated forcibly\n");
  printf("** if the number of file exceeds 10000, the oldest file will be deleted;\n");
  printf("    to avoid deleting files, use -a option, the program will be ended if there are too many files\n");
}


void quit(void){

  if(sock) close(sock);
  if(sfd) close(sfd);

  exit(0);
}

int deloldest(void){
  struct dirent *de;
  struct stat fst;
  DIR *dd = NULL;
  time_t oldest = 0;
  char path[512]={0};
  char tpath[512]={0};

  dd = opendir(dir);
  if(dd == NULL) return 0;
  while((de = readdir(dd))){
    snprintf(tpath, sizeof(tpath), "%s/%s", dir, de->d_name);
    if(stat(tpath, &fst) == 0){
      if(S_ISREG(fst.st_mode)){
	printf("%s\n", tpath);
	if(!oldest){
	  oldest = fst.st_mtime;
	  strncpy(path, tpath, sizeof(path));
	}else{
	  if(oldest > fst.st_mtime){
	    oldest = fst.st_mtime;
	    strncpy(path, tpath, sizeof(path));
	  }
	}
      }
    }
  }

  if(dd) closedir(dd);
  
  if(unlink(path)){
    printf("Fatal error: Cannot delete %s\n", path);
    quit();
  }else{
    printf("delete=%s\n", path);
  }

  return 0;
}

void ridf_comment_start(void){
  RIDFHD ghd;
  RIDFHDCOM comhd;
  char runch[RIDF_COMMENT_RUNINFO_ASC_SIZE];
  int bsz, idx;
  time_t ttime;
  char chst[60];
  struct tm *strtime;

  memset(runch, 0, sizeof(runch));

  bsz = sizeof(ghd) + sizeof(runch);
  bsz = bsz/2;
  ghd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, bsz, daqinfo.efn);
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
  sprintf(runch+idx, "RUNNING");
  idx += 20;
  ttime = runinfo.starttime;
  strftime(chst, sizeof(chst), "%d-%b-%y", strtime);
  sprintf(runch+idx, "%s", chst);
  idx += 60;
  sprintf(runch+idx, "%s", runinfo.header);

  if(fd){
    fwrite((char *)&ghd, sizeof(ghd), 1, fd);
    fwrite(runch, sizeof(runch), 1, fd);
  }
}

void ridf_comment_stop(void){
  char runch[RIDF_COMMENT_RUNINFO_ASC_SIZE];
  RIDFHDCOM comhd;
  RIDFHD hd;
  char chst[60];
  int idx;
  time_t ttime;
  struct tm *strtime;

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

  if(fd){
    fseeko(fd, sizeof(hd), SEEK_SET);
    fwrite(runch, sizeof(runch), 1, fd);
  }
}

/*** functions ***/
void mkheader(void){
  char path[1024]={0}, sux[272]={0}, tstr[256]={0};
  time_t now;
  struct tm *t_st;
  struct stat fst;
  int fidx = 1;

  if(frun == 1){
    mkender();
  }
  
  time(&now);
  startt = now;
  if(ut){
    snprintf(tstr, sizeof(tstr), "%u", (unsigned int)now);
  }else{
    t_st = localtime(&now);
    snprintf(tstr, sizeof(tstr), "%04d%02d%02d%02d%02d%02d",
	     t_st->tm_year+1900, t_st->tm_mon+1, t_st->tm_mday,
	     t_st->tm_hour, t_st->tm_min, t_st->tm_sec);
  }

  //  printf("mkheader time = %s\n", tstr);

  // header / suffix
  switch(mode){
  case DTIME:
    runinfo.starttime = startt;
    snprintf(sux, sizeof(sux), "time_%s", tstr);
    break;
  case DSINGLE:
    runinfo.starttime = startt;
    snprintf(sux, sizeof(sux), "single_%s", tstr);
    break;
  case DRUN:
    if(runinfo.runstat == STAT_RUN_NSSTA){
      snprintf(sux, sizeof(sux), "nssta_%s", tstr);
    }else{
      if(strlen(daqinfo.runname)){
	snprintf(sux, sizeof(sux), "%s_%04d", daqinfo.runname, runinfo.runnumber);
      }else{
	snprintf(sux, sizeof(sux), "unknown_%s", tstr);
      }
    }
    break;
  }

  /* file open */
  if(fd==NULL && !ns){
    snprintf(path, sizeof(path), "%s/%s%s.ridf", dir, pre, sux);
    while(stat(path, &fst) == 0){
      snprintf(path, sizeof(path), "%s/%s%s_%d.ridf", dir, pre, sux, fidx);
      fidx ++;
    }
    printf("path=%s\n", path);
    fd = fopen(path, "w");
    if(!fd){
      printf("Cannot open file %s\n", path);
      exit(1);
    }
    ridf_comment_start();
  }else{
    fd = NULL;
  }


  fheader = 0;
  frun = 1;
}

void mkender(void){
  frun = 0;
  ridf_comment_stop();
  if(fd){
    printf("close file\n");
    fclose(fd);
  }
  fender = 0;
  fd = NULL;
}


int recvmain(void){
  struct sockaddr_in caddr;
  int clen, rsize, tsz, len, idx, date, stid;
  RIDFHD hd;
  RIDFRHD rhd;
  time_t ps;

  clen = sizeof(caddr);
  if((sock = accept(sfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    return 0;
  }

  while(1){
    tsz = recv(sock, (char *)&hd, sizeof(hd), MSG_WAITALL);
    if(tsz < 0){
      close(sock);
      return 0;
    }
    memcpy(buff, (char *)&hd, sizeof(hd));
    len = ridf_sz(hd) * 2;
    rsize = tsz;
    while(rsize < len){
      tsz = recv(sock, buff+rsize, len - rsize, MSG_WAITALL);
      if(tsz < 0){
	close(sock);
	return 0;
      }
      rsize += tsz;
    }
    // analysis each buffer
    memcpy((char *)&hd, buff, sizeof(hd));
    //size = ridf_sz(hd) * 2;
    idx = sizeof(hd);
    rhd = ridf_dechd(hd);

    if(rhd.classid == RIDF_EA_BLOCK ||
       rhd.classid == RIDF_EF_BLOCK ||
       rhd.classid == RIDF_EAEF_BLOCK){
      memcpy((char *)&hd, buff+idx, sizeof(hd));
      rhd = ridf_dechd(hd);
      idx += sizeof(hd);

      switch(mode){
      case DRUN:
	if(rhd.classid == RIDF_STATUS){
	  memcpy((char *)&date, buff+idx, sizeof(date));
	  idx += sizeof(date);
	  memcpy((char *)&stid, buff+idx, sizeof(stid));
	  idx += sizeof(stid);
	  if(stid == RIDF_STATUS_DAQRUNINFO){
	    memcpy((char *)&daqinfo, buff+idx, sizeof(daqinfo));
	    idx += sizeof(daqinfo);
	    memcpy((char *)&runinfo, buff+idx, sizeof(runinfo));
	    idx += sizeof(runinfo);
	    
	    switch(runinfo.runstat){
	    case STAT_RUN_START:
	      fheader = 1;
	      break;
	    case STAT_RUN_NSSTA:
	      fheader = 1;
	      break;
	    case STAT_RUN_IDLE:
	      fender = 1;
	      break;
	    default:
	      fheader = 0;
	      fender = 0;
	      break;
	    }
	  }
	}
	break;
      case DSINGLE:
	break;
      case DTIME:
	break;
      }
    }

    if(fheader) mkheader();
    if(fd){
      fwrite(buff, len, 1, fd);
    }

    // check time interval
    if(mode == DTIME){
      time(&ps);
      if((int)(ps - startt) > (min * 60)){
	mkender();
	mkheader();
      }
    }

    // check file size
    if(limit > 0){
      cursize = (unsigned long long int)ftello(fd);
      if((cursize/1024/1024) >= limit){
	mkender();
	mkheader();
      }
    }

    if(fender) mkender();
  }

  return 0;
}

int main(int argc, char *argv[]){
  struct sttcpclinfo thisinfo;
  fd_set fdset;
  int tsock, ret;
  int i, idx;
  int opt, option_index;
  char opts[256]={0};
  struct timeval timeout;

  i = 0;
  idx = 0;

  memset((char *)&thisinfo, 0, sizeof(thisinfo));
  //thisinfo.port = 17622;
  thisinfo.port = ANYREVPORT;

  while(long_options[i].name){
    opts[idx] = (char )long_options[i].val;
    idx++;
    if(long_options[i].has_arg == required_argument){
      opts[idx] = ':';
      idx++;
    }
    i++;
  }

  sprintf(dir, "./");
  sprintf(pre, "anystore_");

  idx = 0; // idx for CSV to be showed
  while((opt = getopt_long(argc, argv, opts, long_options, &option_index)) != -1){
    switch(opt){
    case 'h':
      help();
      quit();
      break;
    case 'd':
      optcpy(dir, optarg);
      break;
    case 'p':
      optcpy(pre, optarg);
      break;
    case 'm':
      thisinfo.port = strtof(optarg, NULL);
      break;
    case 'i':
      strncpy(thisinfo.host, optarg, sizeof(thisinfo.host)-1);
      break;
    case 'u':
      ut = 1;
      break;
    case 'c':
      clean = strtof(optarg, NULL);
      break;
    case 'l':
      limit = (int)strtol(optarg, NULL, 0);
      break;
    case 't':
      min = (int)strtol(optarg, NULL, 0);
      mode = DTIME;
      break;
    case 'r':
      mode = DRUN;
      break;
    case 's':
      mode = DSINGLE;
      break;
    case 'n':
      ns = 1;
      break;
    default:
      printf("Invalid option!!\n");
      return 0;
      break;
    }
  }


  switch(mode){
  case DTIME:
    printf("MODE = TIME (interval=%d)\n", min);
    break;
  case DRUN:
    printf("MODE = RUN\n");
    break;
  case DSINGLE:
    printf("MODE = SINGLE\n");
    break;
  }

  if(!argv[optind]){
    printf("*** HOSTNAME is required\n");
    printf("anystore [OPTION]... HOSTNAME\n");
    exit(0);
  }
  strncpy(host, argv[optind], sizeof(host)-1);
    
  if(!strlen(thisinfo.host)){
    gethostname(thisinfo.host, sizeof(thisinfo.host));
  }

  if(limit < 0){
    printf("limit must be >= 0 (%d is invalid)\n", limit);
    exit(0);
  }

  printf("clean ratio = %3.1f\n", clean);
  if(limit > 0){
    printf("limit       = %d MB\n", limit);
  }else{
    printf("limit       = none\n");
  }
  printf("prefix      = %s\n", pre);
  printf("directory   = %s\n", dir);
  printf("myport      = %d\n", thisinfo.port);
  printf("myip(host)  = %s\n", thisinfo.host);
  printf("server host = %s\n", host);
  printf("unixtime    = %s\n", ut?"on":"off");
  printf("nosave      = %d\n", ns);

  buff = malloc(EB_BUFF_SIZE);
  memset(buff, 0, EB_BUFF_SIZE);
  signal(SIGINT, (void *)quit);


  /* Make command and oneshot port */
  if(!(sfd = mktcpsock(thisinfo.port))){
    printf("Can't make socket on port = %d\n", thisinfo.port);
    quit();
  }

  if(!(tsock = mktcpsend(host, INFCOMPORT))){
    printf("Can't connecto to babinfo in %s\n", argv[1]);
    quit();
  }
  ret = eb_set(tsock, INF_MK_TCPCLIENT, (char *)&thisinfo, sizeof(thisinfo));
  close(tsock);

  if(!ret){
    printf("Exceed the maximum number of tcpclient\n");
    quit();
  }

  // init
  memset((char *)&daqinfo, 0, sizeof(daqinfo));
  memset((char *)&runinfo, 0, sizeof(runinfo));

  // timeout
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  
  // first time
  fheader = 0;
  fender = 0;

  mkheader();


  /* Main loop for command and one short */
  while(1){
    FD_ZERO(&fdset);
    FD_SET(sfd, &fdset);
    if(select(sfd+1, &fdset, NULL, NULL, &timeout) != 0){
      recvmain();
    }

  }


  return 0;
}
