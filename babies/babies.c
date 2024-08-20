/* babirl : babies/babies.c
 * last modified : 10/12/01 12:07:29 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Event fragment sender (EFS)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#ifdef RTLINUXDRV
#include <pthread.h>
#endif

/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include "babies.h"
#include "babirldrvdef.h"
#include "babilio.h"

#define COMFIFO "/tmp/babirldbfifo"
#define RUNSTFILE "/tmp/babiesrun"
#define RUNBKFILE "/tmp/babiesblk"

int creat_runstfile(void){
  return creat(RUNSTFILE, 0666);
}

int unlink_runstfile(void){
  return unlink(RUNSTFILE);
}

int isbkfile(void){
  if(!access(RUNBKFILE, F_OK)){
    return 1;
  }

  return 0;
}

int isdir(char *path){
  struct stat st;

  if(stat(path, &st)){
    return 0;
  }

  if(S_ISDIR(st.st_mode)) return 1;

  return 0;
}

int lfprintf(FILE *fd, const char *fmt, ...){
  time_t now;
  int fno;

  if(!fd) return 0;

  time(&now);
  fprintf(fd, "%s ", ctime(&now));

  va_list argp;
  va_start(argp, fmt);
  vfprintf(fd, fmt, argp);
  va_end(argp);

  fflush(fd);
  fno = fileno(fd);
  fdatasync(fno);

  return 1;
}


/* Check option */
int chkopt(int argc,char *argv[]){
  int val;
  while((val=getopt(argc,argv,"rlcdfsmnk"))!=-1){
    switch(val){
    case 'r':
      mode = BABIES_RTLINUX;
      printf("babies: real-time linux mode\n");
      break;
    case 'l':
      mode = BABIES_LINUX;
      printf("babies: normal linux mode\n");
      break;
    case 'd':
      mode = BABIES_DUMMY;
      printf("babies: DUMMY mode\n");
      break;
    case 'c':
    case 'm':
      mode = BABIES_SHM;
      printf("babies: Shared memory mode\n");
      break;
    case 's':
      mode = BABIES_SCALER;
      printf("babies: SCALER mode\n");
      break;
    case 'f':
      fstart = 1;
      printf("babies: force start mode\n");
      break;
    case 'n':
      vm = 0;
      printf("babies: without log\n");
      break;
    case 'k':
      ckill = 1;
      printf("babies: kill child process when daq stop\n");
      break;
    default:
      printf("Invalid option!!\n");
      return 0;
      break;
    }
  }
  return 1;
}

int chkexe(void){
  int ret = 1;
  struct stat tst;

  if(!stat(efrc.mtdir, &tst)){
    efrc.mt = 1;
  }else{
    efrc.mt = 0;
    ret = 0;
  }

  return ret;
}

int chkmod(void){
  char c[272], d[256];
  int ret = 1;
  struct stat tst;

  memset(c, 0, sizeof(c));
  memset(d, 0, sizeof(d));

  if(!is26){
    sprintf(d, "%s/%s.o", efrc.mtdir, drivername);
  }else{
    sprintf(d, "%s/%s.ko", efrc.mtdir, drivername);
  }

  if(!stat(d, &tst)){
    efrc.mt = 1;
  }else{
    efrc.mt = 0;
    ret = 0;
  }

  return ret;
}


int insmod(void){
  char c[272], d[256];
  int ret = 1;
  struct stat tst;

  memset(c, 0, sizeof(c));
  memset(d, 0, sizeof(d));

  if(!is26){
    sprintf(d, "%s/%s.o", efrc.mtdir, drivername);
  }else{
    sprintf(d, "%s/%s.ko", efrc.mtdir, drivername);
  }

  if(!stat(d, &tst)){
    sprintf(c, "/sbin/insmod %s", d);
    DB(printf("%s\n", d));
    system(c);
    efrc.mt = 1;
  }else{
    efrc.mt = 0;
    ret = 0;
  }

  return ret;
}

void rmmod(void){
  char c[160];

  sprintf(c, "/sbin/rmmod %s", drivername);
  system(c);

}

void open_drv(void){
  volatile char ssf;
  char c[128], n[128];

  switch(mode){
  case BABIES_RTLINUX:
    rtend = STAT_RUN_START;
    ssf = 0;

    DB(printf("open_drv RTLINUX\n"));

    if(insmod()){
      while(!ssf){
	usleep(1000);
	memcpy((char *)&ssf, (char *)(shmptr + EF_SHM_SSF), sizeof(ssf));
	DB(printf("ssf 0x%04x\n", ssf));
      }
    }

    break;
  case BABIES_LINUX:
    if((drvfd = open(BABILDRV_DIR, O_RDWR)) < 0){
      perror("Can't open driver\n");
      exit(1);
    }
    //evtn = -1;
    //while(evtn != 0){
    //ioctl(drvfd, BABIL_EVTN, (char *)&evtn);
    //}
    break;
  case BABIES_DUMMY:
    break;
  case BABIES_SHM:
    if(strlen(efrc.mtdir)){
      dirtoname(efrc.mtdir, n);
      DB(printf("dirname = %s, name=%s\n", efrc.mtdir, n));
      DB(printf("chkpidof = %d\n", chkpidof(n)));

      if(!chkpidof(n)){
	memset(c, 0, sizeof(c));
	sprintf(c, "%s %d &", efrc.mtdir, efrc.efid);
	DB(printf("try system = %s\n", c));
	//lfprintf(lfd, "try system = %s\n", c);
	system(c);
	usleep(100000);
      }
      //lfprintf(lfd, "chkpidof = %d\n", chkpidof(n));
    }

    *babiesrun = 1;
    ssf = 0;
    while(!ssf){
      usleep(1000);
      memcpy((char *)&ssf, shmptr + EF_SHM_SSF, sizeof(ssf));
    }
    break;
  case BABIES_SCALER:
    if(efrc.mt){
      memset(c, 0, sizeof(c));
      sprintf(c, "%s %d &", efrc.mtdir, efrc.efid);
      system(c);
    }
    break;
  }
}


/* Update efrc */
int update_efrc(void){
  FILE *fd;

  if((fd = fopen(BABIESRC, "w"))){
    DB(printf("babies: paramter write to file\n"));
    fprintf(fd, "%s\n", efrc.erhost);
    fprintf(fd, "%d\n", efrc.hd1);
    fprintf(fd, "%s\n", efrc.hd1dir);
    fprintf(fd, "%d\n", efrc.hd2);
    fprintf(fd, "%s\n", efrc.hd2dir);
    fprintf(fd, "%d\n", efrc.mt);
    fprintf(fd, "%s\n", efrc.mtdir);
    fclose(fd);
  }else{
    DB(printf("baies: Can't open rcfile\n"));
    return 0;
  }

  return 1;
}

/* Initialize babies efrc (defalut settting) */
void init_efrc(void){
  efrc.runnumber = -1;
  efrc.erport = ERRCVPORT;
  efrc.comport = ESCOMPORT + efrc.efid;
  memset(efrc.erhost, 0, sizeof(efrc.erhost));
  strcpy(efrc.erhost, "localhost");
  efrc.hd1 = FALSE;
  strcpy(efrc.hd1dir, "./\0");
  efrc.hd2 = FALSE;
  strcpy(efrc.hd2dir, "./\0");
  efrc.mt = FALSE;
  strcpy(efrc.mtdir, "./\0");
  strcpy(efrc.runname, "\0");
  efrc.connect = 0;
}

void quit(void){
  int p;
  char f[128];
  FILE *fd;

  DB(printf("closing sock\n"));
  if(sock) close(sock);
  DB(printf("closing efsock\n"));
  if(efsock) close(efsock);

  if(mode != BABIES_RTLINUX){
    DB(printf("closing shmptr\n"));
    if(shmptr) delshm(shmid, shmptr);

    if(mode == BABIES_SCALER){
      sprintf(f, "%s.pid", efrc.mtdir);
      if((fd = fopen(f, "r")) == NULL){
      }else{
	fscanf(fd, "%d", &p);
	fclose(fd);
	kill(p, SIGINT);
      }
    }
    
  }else{
    DB(printf("closing mfd\n"));
    if(mfd) close(mfd);
    /* un map */
    DB(printf("closing munmap\n"));
#ifdef RTLINUXDRV
    munmap(shmptr, MAPSIZE);
#endif
    shmptr = 0;
  }

  if(mode == BABIES_SHM){
    if(strlen(efrc.mtdir)){
      dirtoname(efrc.mtdir, f);
      if(!killpid(f)){
	killpidof(f);
      }
    }
  }

  DB(printf("closing comfifo\n"));
  if(mode == BABIES_DUMMY || mode == BABIES_SHM){
    unlink(COMFIFO);
  }

  if(vm) lfprintf(lfd, "babies: Exit");
  if(lfd) fclose(lfd);
  rmpid("babies");

  if(data) free(data);

#ifdef RTLINUXDRV
  DB(printf("closing pthread\n"));
  pthread_exit(NULL);
#endif
  unlink_runstfile();

  DB(printf("exit\n"));
  exit(0);
}

// LINUX mode
void daq_stop(void){
  char flag;
  int len,i;

  DB(printf("babies: daq_stop\n"));

  sflag = STAT_RUN_IDLE;
  ioctl(drvfd, BABIL_STOPB, &flag);

  DB(printf("babies: ioctl desu\n"));

  if(!efrc.connect){
    efr_connect();
  }

  for(i=0;i<flag;i++){
    ioctl(drvfd, BABIL_LEN, (char *)&len);
    read(drvfd, (char *)data, len);
    if(efrc.connect){
      DB(printf("send data to EFR\n"));
      send(efsock, (char *)&len, sizeof(len), 0);
      send(efsock, data, len, 0);
    }
  }

  if(efrc.connect){
    DB(printf("End of run\n"));
    len = -1;
    send(efsock, (char *)&len, sizeof(len), 0);
  }

  ioctl(drvfd, BABIL_EVTN, (char *)&evtn);

  close(drvfd);
  drvfd = 0;
}

int fifomain(int com){
  char *buffat;
  int len;

  DB(printf("babies: fifomain\n"));

  if(com < 0){
    DB(printf("babies: com=%d\n", com));
    if(!efrc.connect){
      efr_connect();
    }
    if(efrc.connect){
      DB(printf("End of run\n"));
      len = -1;
      send(efsock, (char *)&len, sizeof(len), 0);
      sflag = STAT_RUN_IDLE;
    }

    return 1;
  }

  if(com >= 0){
    DB(printf("read data from fifo size=%d\n", com));

    if(com == 0){
      buffat = shmptr + EF_SHM_DATA1;
      if(shmptr[EF_SHM_FLAG1] != EF_SHM_READY1){
	printf("Invalid shm flag 1\n");
	return 0;
      }
    }else{
      buffat = shmptr + EF_SHM_DATA2;
      if(shmptr[EF_SHM_FLAG2] != EF_SHM_READY2){
	printf("Invalid shm flag 2\n");
	return 0;
      }
    }

    if(!efrc.connect){
      efr_connect();
    }
    if(efrc.connect){
      DB(printf("send data to EFR\n"));
      memcpy((char *)&len, buffat, sizeof(len));
      len = RIDF_SZ(len) * 2;
      DB(printf("shm %d : size = %d\n", com, len));
      send(efsock, (char *)&len, sizeof(len), 0);
      send(efsock, buffat, len, 0);
    }
    if(com == 0){
      *(shmptr + EF_SHM_FLAG1) = EF_SHM_FREE;
    }else{
      *(shmptr + EF_SHM_FLAG2) = EF_SHM_FREE;
    }
  }

  return 1;
}

int efr_connect(void){
  int ret;
  
  ret = 0;

  if(!efrc.connect){
    /* Make data port */
    if(!(efsock = mktcpsend(efrc.erhost, efrc.erport))){
      DB(printf("babies: Can't connect %s:%d\n", efrc.erhost, efrc.erport));
      return 0;
    }
    send(efsock, (char *)&efrc.efid, sizeof(efrc.efid), 0);
    recv(efsock, (char *)&ret, sizeof(ret), MSG_WAITALL);
    efrc.connect = 1;
    DB(printf("babies: EFR return = %d\n", ret));
  }else{
    DB(printf("babies: EFR already connected\n"));
    return 0;
  }

  return 1;
}

int efr_disconnect(void){
  if(efrc.connect){
    close(efsock);
    efrc.connect = 0;
  }else{
    DB(printf("babies: EFR not connected\n"));
    return 0;
  }
  return 1;
}

int commain(void){
  struct sockaddr_in caddr;
  char buff[BABIRL_COM_SIZE], tdir[80]={0}, f[128]={0}, n[128]={0};
  int sock, clen, len, rcnt;
  int com, ret, arg, p;
  volatile char ssf;
  FILE *fd;

  clen = sizeof(caddr);
  if((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("Error in accept commain\n");
    return 0;
  }

  memset(buff, 0, sizeof(buff));
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);

  /* babies command */
  memcpy((char *)&com, buff, sizeof(com));

  switch(com){
  case ES_SET_CONFIG:
    DB(printf("babies: ES_SET_CONFIG\n"));
    memset(tdir, 0, sizeof(tdir));
    strcpy(tdir, efrc.mtdir);

    memcpy((char *)&efrc, buff+sizeof(com), sizeof(efrc));
    memset(buff, 0, sizeof(buff));
    ret = 1;
    memcpy(buff, (char *)&ret, sizeof(ret));
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len),0);
    send(sock, buff, len, 0);
    update_efrc();
    efr_disconnect();

    if(mode == BABIES_LINUX){
      if(strcmp(tdir, efrc.mtdir) != 0){
	rmmod();
	insmod();
      }
    }else if(mode == BABIES_RTLINUX){
      chkmod();
    }else if(mode == BABIES_SCALER){
      chkexe();
    }else if(mode == BABIES_SHM){
      if(strcmp(tdir, efrc.mtdir) != 0){
	dirtoname(tdir, f);
	killpidof(f);
	chkexe();
      }
    }
    break;
  case ES_GET_CONFIG:
    DB(printf("babies: ES_GET_CONFIG\n"));
    memset(buff, 0, sizeof(buff));
    memcpy(buff, (char *)&efrc, sizeof(efrc));
    len = sizeof(efrc);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, buff, len, 0);
    break;
  case ES_RELOAD_DRV:
    if(sflag == STAT_RUN_IDLE){
      if(mode == BABIES_LINUX){
	rmmod();
	insmod();
      }
    }
    break;
  case ES_RUN_START:
    DB(printf("babies: ES_RUN_START\n"));
    memcpy((char *)&arg, buff+sizeof(com), sizeof(arg));
    if(arg == ES_EF_OFF && !fstart){
      DB(printf("babies: Run started, but not joining event build\n"));
      break;
    }
    /* open driver */
    open_drv();

    ret = 1;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    sflag = STAT_RUN_START;
    creat_runstfile();
    break;
  case ES_RUN_NSSTA:
    DB(printf("babies: ES_NSRUN_START\n"));
    memcpy((char *)&arg, buff+sizeof(com), sizeof(arg));
    if(arg == ES_EF_OFF && !fstart){
      DB(printf("babies: Run started, but not joining event build\n"));
      break;
    }
    /* open driver */
    open_drv();

    ret = 1;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    sflag = STAT_RUN_NSSTA;
    creat_runstfile();

    break;
  case ES_RUN_STOP:
    DB(printf("babies: ES_RUN_STOP\n"));

    ret = 1;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    
    if(sflag){
      if(drvfd && mode == BABIES_LINUX){
	daq_stop();
	unlink_runstfile();
      }else if(mode == BABIES_SHM){
	*babiesrun = 0; 
	usleep(10000);
	ssf = 1;
	while(ssf){
	  if(ckill){  //kill child process
	    dirtoname(efrc.mtdir, n);
	    if(!killpid(n)){ //kill via /var/run/...
	      killpidof(n);  //kill by pidof
	    }
	  }

	  memcpy((char *)&ssf, shmptr + EF_SHM_SSF, sizeof(ssf));
	  usleep(1000);
	  if(ssf){
	    flock(fifofd, LOCK_EX);
	    rcnt = read(fifofd, (char *)&fifocom, sizeof(fifocom));
	    DB(printf("babies: fifocom = %d\n", fifocom));
	    if(rcnt > 0){
	      fifomain(fifocom);
	    }
	    flock(fifofd, LOCK_UN);
	  }
	}
	memcpy((char *)&evtn, shmptr + EF_SHM_EVTN, sizeof(evtn));
	unlink_runstfile();
      }else if(mode == BABIES_RTLINUX){
	rmmod();
	while(rtend != STAT_RUN_IDLE){
	  DB(printf("babies: wainting rtend\n"));
	  usleep(100);
	}
	memcpy((char *)&evtn, (char *)(shmptr + EF_SHM_EVTN), sizeof(evtn));
	DB(printf("babies: evtn=%d\n", evtn));
	if(efrc.connect){
	  DB(printf("End of run\n"));
	  len = -1;
	  send(efsock, (char *)&len, sizeof(len), 0);
	}
	sflag = STAT_RUN_IDLE;
	unlink_runstfile();
      }else if(mode == BABIES_SCALER){
	sprintf(f, "%s.pid", efrc.mtdir);
	if((fd = fopen(f, "r")) == NULL){
	  if(!efrc.connect){
	    efr_connect();
	  }
	  if(efrc.connect){
	    DB(printf("End of run\n"));
	    len = -1;
	    send(efsock, (char *)&len, sizeof(len), 0);
	    sflag = STAT_RUN_IDLE;
	    unlink_runstfile();
	  }
	}else{
	  fscanf(fd, "%d", &p);
	  fclose(fd);
	  kill(p, SIGINT);
	  unlink_runstfile();
	}
      }

    }
    break;
  case ES_GET_EVTN:
    ret = evtn;
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    break;
  case WHOAREYOU:
    DB(printf("babies : WHOAREYOU\n"));
    memset(f, 0, sizeof(f));
    len = sprintf(f, "%s %s", thisname, babies_mode[mode]);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, thisname, len, 0);
    break;
  case ES_GET_RUNSTAT:
    if(isbkfile()){
      ret = -1;
    }else{
      ret = sflag;
    }
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);
    break;
  case ES_QUIT:
    DB(printf("babies: ES_QUIT\n"));
    if(!sflag) quit();
    break;
  case ES_CON_EFR:
    DB(printf("babies: ES_CON_EFR\n"));
    if(!efr_connect()){
      DB(printf("babies: EFR connection faild\n"));
    }
    break;
  case ES_DIS_EFR:
    DB(printf("babies: ES_DIS_EFR\n"));
    if(!efr_disconnect()){
      DB(printf("babies: EFR disconnection faild\n"));
    }
  }

  close(sock);
  return 1;
}


int main(int argc, char *argv[]){
  FILE *fd;
  fd_set fdset;
  //struct timespec pollfreq;
  struct stat fifost;
  int len, eiflag;
  struct utsname uts;
  char ftime[256], lfname[512];
  struct tm *stm;
  time_t now;
  char n[124], c[124], f[124];
  int p=-1;


  if(argc < 2){
    printf("babies -[r/l/c/d] [-n] EFID\n");
    printf(" options    r = rtlinux mode (default) not implemented yet\n");
    printf("            l = normal linux mode (with babildrv)\n");
    printf("            c = shared memory mode (with Xexecuter)\n");
    printf("            d = dummy driver mode (with ddrv)\n");
    printf("            s = Scaler mode (periodic)\n");
    printf("            n = no log mode\n");
    //printf("\n");
    //printf(" additional -e = path for external executable in shared memory mode\n");
    exit(0);
  }

  memset(babies_mode, 0, sizeof(babies_mode));
  sprintf(babies_mode[0], "NONE");
  sprintf(babies_mode[1], "RTLINUX");
  sprintf(babies_mode[2], "LINUX");
  sprintf(babies_mode[3], "DUMMY");
  sprintf(babies_mode[4], "SHM");
  sprintf(babies_mode[5], "SCALER");

  fstart = 0;
  evtn = 0;

  if(!chkopt(argc, argv)){
    exit(0);
  }

  if(argc == 2){
    printf("babies : real-time linux mode\n");
    mode = BABIES_RTLINUX;
  }


  chdir(BABIRLDIR);

  /* Get Event Fragment Number */
  if(sscanf(argv[argc - 1],"%d",&efrc.efid) != 1){
    printf("babies : babies EFID\n");
    exit(0);
  }    
  if(efrc.efid < 0 || efrc.efid > MAXEF){
    printf("babies: Error 0 < EFN < %d\n", MAXEF);
    exit(0);
  }

  DB(printf("babies: EFN = %d\n",efrc.efid));

  /* PID file */
  if(chkpid("babies")){
    printf("babies: Error, another babies may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babies\n");
    exit(0);
  }


  init_efrc();
  if((fd = fopen(BABIESRC, "r"))){
    DB(printf("babies: parameter from file\n"));
    fscanf(fd, "%s", efrc.erhost);
    fscanf(fd, "%d", &efrc.hd1);
    fscanf(fd, "%s", efrc.hd1dir);
    fscanf(fd, "%d", &efrc.hd2);
    fscanf(fd, "%s", efrc.hd2dir);
    fscanf(fd, "%d", &efrc.mt);
    fscanf(fd, "%s", efrc.mtdir);
    //fgets(efrc.mtdir, sizeof(efrc.mtdir), fd);
    fclose(fd);
  }

  /* Clean up remaining process */
  if(mode == BABIES_SCALER){
    sprintf(f, "%s.pid", efrc.mtdir);
    if((fd = fopen(f, "r")) == NULL){
    }else{
      fscanf(fd, "%d", &p);
      fclose(fd);
      printf("kill remainig process %s (%d)\n", f, p);
      kill(p, SIGINT);
      sleep(2);
    }
  }


  //char data[EB_EFBLOCK_BUFFSIZE];
  data = malloc((size_t)EB_EFBLOCK_BUFFSIZE);
  memset(data, 0, EB_EFBLOCK_BUFFSIZE);


#ifndef DEBUG
  if(!(int)getuid()) nice(-20);
  daemon(1, 0);
#endif




  chdir(BABIRLDIR);
  /* PID file */
  if(!mkpid("babies")){
    printf("babies: Error, Can't create PID file or another babinfo may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babies\n");
    exit(0);
  }

  /* Signal SIGINT -> quit() */
  signal(SIGINT,(void *)quit);

  sflag = STAT_RUN_IDLE;
  memset(data, 0, EB_EFBLOCK_BUFFSIZE);

  if(mode == BABIES_SCALER) unlink(COMFIFO);

  if(vm){
    time(&now);
    stm = localtime(&now);
    strftime(ftime, sizeof(ftime), "%Y.%m.%d_%H:%M:%S", stm);
    if(!isdir("log")){
      mkdir("log", 0775);
    }

    sprintf(lfname, "log/babies%s.log", ftime);
    if(!(lfd = fopen(lfname, "w"))){
      printf("Can't make log file %s\n", lfname);
    }
  }



  /* Make fifo */
  if(mode == BABIES_DUMMY || mode == BABIES_SHM || mode == BABIES_SCALER){
    if(stat(COMFIFO, &fifost) < 0){
      mkfifo(COMFIFO, 0666);
    }
    shmptr = 0;
    if(!(shmid = initshm(EFSHMKEY + efrc.efid, EF_SHM_SIZE, &shmptr))){
      printf("initshm faild shmid = %d\n", shmid);
      exit(0);
    }
    if(!shmptr){
      printf("Can't allocate shared mempry %p\n", shmptr);
      exit(1);
    }
    babiesrun = shmptr + EF_SHM_RUN;

    if(vm) lfprintf(lfd, "babies: shmid=%d, shmptr=%p, babiesrun=%p\n", shmid, shmptr, babiesrun);

  }
#ifdef RTLINUXDRV
  else if(mode == BABIES_RTLINUX){
    /* mbuff allocate shared memroy area */
    if(!(mfd = open("/dev/mem",O_RDWR))){
      printf("babies: Can't open /dev/mem\n");
      exit(1);
    }

    if((shmptr=(char *)mmap(0, MAPSIZE, PROT_READ|PROT_WRITE,
			    MAP_FILE|MAP_SHARED, mfd, RTSHMEM)) == NULL){
      printf("babies: Can't alloc shared memory.\n");
      exit(1);
    }
    memset((char *)shmptr, 0, MAPSIZE);
    DB(printf("shm addr = %p\n", shmptr));

    DB(printf("flag1 %d\n", shmptr[EF_SHM_FLAG1]));
    DB(printf("ssf   %d\n", *(shmptr+EF_SHM_SSF)));
    babiesrun = shmptr + EF_SHM_RUN;
  }
#endif


  /* Make command port */
  if(!(comfd = mktcpsock(ESCOMPORT + efrc.efid))) quit();
  DB(printf("babies: comfd number = %d\n", comfd));

  fifofd = 0;
  
  if(mode != BABIES_LINUX && mode != BABIES_RTLINUX){
    if((fifofd = open(COMFIFO, O_RDWR | O_NONBLOCK)) < 0){
      printf("babies: Can't open FIFO\n");
      quit();
    }
  }


  /* Driver name */
  is26 = 0;
  uname(&uts);
  if(strncmp(uts.release, "2.6", 3) == 0){
    is26 = 1;
  }
  memset(drivername, 0, sizeof(drivername));
  if(mode == BABIES_LINUX){
    strcpy(drivername, "babildrv");
    rmmod();
    insmod();
  }else if(mode == BABIES_RTLINUX){
    strcpy(drivername, RTDRIVERNAME);
    rmmod();
    chkmod();
  }else if(mode == BABIES_SCALER || mode == BABIES_SHM){
    chkexe();
  }

#ifdef RTLINUXDRV
  /* Make rtthread */
  if(mode == BABIES_RTLINUX){
    pthread_create(&rtthread_t, NULL, (void *)&rtthread, NULL);
  }
#endif

  /* Initialize drvfd */
  drvfd = 0;

  /* polling frequency */
  //pollfreq.tv_sec = 0;
  //pollfreq.tv_nsec = 1000;

  if(mode == BABIES_SHM){
    if(strlen(efrc.mtdir)){
      dirtoname(efrc.mtdir, n);
      DB(printf("dirname = %s, name=%s\n", efrc.mtdir, n));
      DB(printf("chkpidof = %d\n", chkpidof(n)));
      
      if(!chkpidof(n)){
	memset(c, 0, sizeof(c));
	sprintf(c, "%s %d &", efrc.mtdir, efrc.efid);
	DB(printf("try system = %s\n", c));
	if(vm) lfprintf(lfd, "try system = %s\n", c);
	system(c);
	usleep(10000);
      }
      //lfprintf(lfd, "chkpidof = %d\n", chkpidof(n));
    }
  }

  unlink_runstfile();

  if(vm) lfprintf(lfd, "babies: Start with mode = %d\n", mode);
  while(1){
    /* Main loop */
    /* prepaire fd set for select() */
    DB(printf("babies: Main\n"));

    sn = comfd;
    if(fifofd > sn) sn = fifofd;
    if(drvfd > sn) sn = drvfd;

    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    if(fifofd) FD_SET(fifofd, &fdset);
    if(drvfd) FD_SET(drvfd, &fdset);
    
    if(select(sn+1, &fdset, NULL, NULL, NULL) != 0){
      DB(printf("kitayo****\n"));
      if(FD_ISSET(comfd, &fdset)){
	DB(printf("select : commain\n"));
	commain();
      }else if(fifofd){
	if(FD_ISSET(fifofd, &fdset) && sflag){
	  flock(fifofd, LOCK_EX);
	  read(fifofd, (char *)&fifocom, sizeof(fifocom));
	  DB(printf("babies: fifocom = %d\n", fifocom));
	  fifomain(fifocom);
	  flock(fifofd, LOCK_UN);
	}else{
	  DB(printf("valid fifo fd but invalid isset, sflag=%d\n", sflag));
	  usleep(100000);
	}
      }else if(drvfd){
	if(FD_ISSET(drvfd, &fdset) && sflag){
	  DB(printf("babies: data reading\n"));
	  //ioctl(drvfd, BABIL_DBUFF, (char *)&dn);

	  //for(i=0;i<dn;i++){
	  ioctl(drvfd, BABIL_LEN, (char *)&len);
	  DB(printf("babies: data size = %d\n", len));
	  DB(printf("babies: dataxsize = %x\n", len));
	  read(drvfd, (char *)data, len);
	  ioctl(drvfd, BABIL_EI, &eiflag);
	  if(!efrc.connect){
	    efr_connect();
	  }
	  if(efrc.connect){
	    DB(printf("send data to EFR\n"));
	    send(efsock, (char *)&len, sizeof(len), 0);
	    send(efsock, data, len, 0);
	  }
	  //}
	}else{
	  DB(printf("valid drv fd but invalid isset, sflag=%d\n", sflag));	}	  usleep(1);
      }
    }
  }


  return 0;
}

#ifdef RTLINUXDRV
void rtthread(void){
  int len, chkf, chkfc, datap;
  int ff = 0;

  /* main loop */
  while(shmptr){
    if(sflag == STAT_RUN_IDLE){
      usleep(1000);
    }else{
      if(!efrc.connect){
	efr_connect();
      }
      DB(printf("ssm %x\n", *(shmptr + EF_SHM_SSF)));
      /* check buffer FULL */
      if(shmptr[EF_SHM_FLAG1] == 0x00 && shmptr[EF_SHM_FLAG2] == 0x00 &&
	 shmptr[EF_SHM_FLAG3] == 0x00 && shmptr[EF_SHM_SSF] != STAT_RUN_IDLE){
	usleep(100);
	DB(printf("idling %x\n", *(shmptr + EF_SHM_SSF)));
      }else{
	if(shmptr[EF_SHM_FLAG1] == 0x01 || shmptr[EF_SHM_FLAG2] == 0x02){
	  if(ff == 0){
	    chkf  = EF_SHM_FLAG1;
	    chkfc = 0x01;
	    datap = EF_SHM_DATA1;
	    ff = 1;
	  }else{
	    chkf  = EF_SHM_FLAG2;
	    chkfc = 0x02;
	    datap = EF_SHM_DATA2;
	    ff = 0;
	  }
	  memcpy((char *)&len, (char *)(shmptr+datap), sizeof(len));
	  len = RIDF_SZ(len) * WORDSIZE;
	  if(efrc.connect){
	    DB(printf("send data to EFR\n"));
	    send(efsock, (char *)&len, sizeof(len), 0);
	    send(efsock, (char *)(shmptr+datap), len, 0);
	  }
	  
	  /* clear buffer-1 FULL */
	  memcpy((char *)(shmptr+chkf),(char *)&cle,2);

	}else if(shmptr[EF_SHM_SSF] == STAT_RUN_IDLE){
	  if(shmptr[EF_SHM_FLAG3] == 0x03){
	    memcpy((char *)&len, (char *)(shmptr+EF_SHM_DATA3), sizeof(len));
	    len = RIDF_SZ(len) * WORDSIZE;
	    if(efrc.connect){
	      DB(printf("send data to EFR\n"));
	      send(efsock, (char *)&len, sizeof(len), 0);
	      send(efsock, (char *)(shmptr+EF_SHM_DATA3), len, 0);
	    }
	    
	    /* clear buffer-3 FULL */
	    memcpy((char *)(shmptr+EF_SHM_FLAG3),(char *)&cle,2);
	  }
	  DB(printf("babies: rtend\n"));
	  memcpy((char *)(shmptr+EF_SHM_FLAG1),(char *)&cle,2);
	  memcpy((char *)(shmptr+EF_SHM_FLAG2),(char *)&cle,2);
	  memcpy((char *)(shmptr+EF_SHM_FLAG3),(char *)&cle,2);
	  rtend = STAT_RUN_IDLE;
	  ff = 0;
	}else{
	  printf("babies: Invalid status\n");
	  printf("flag1 : %d\n", shmptr[EF_SHM_FLAG1]);
	  printf("flag2 : %d\n", shmptr[EF_SHM_FLAG2]);
	  printf("flag3 : %d\n", shmptr[EF_SHM_FLAG3]);
	  printf("ssf   : %d\n", shmptr[EF_SHM_SSF]);
	  usleep(1000);
	}
      }
    }
  }
}
#endif
