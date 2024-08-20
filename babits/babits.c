/* babirl : babits/babits.c
 * last modified : 09/12/11 23:28:49 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Time stamp based event builder
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

/* babirl */
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include "babits.h"

void quit(void);

/* Check option */
int chkopt(int argc,char *argv[]){
  int oval;
  char *option, file[256];
  int off = 0, wid = 0, i;

  while((oval=getopt(argc, argv, "lutcdrfnpsb:a:o:e:m:v:"))!=-1){
    switch(oval){
    case 'b':
      mode |= BABITS_BUILD;
      printf("      : event build\n");
      if((bn.fd = fopen(optarg, "r")) == NULL){
	printf("Can't open Master time stamp table %s\n", optarg);
	quit();
      }else{
	memset(bn.name, 0, sizeof(bn.name));
	strncpy(bn.name, optarg, strlen(optarg)-4);
	printf("name = %s\n", bn.name);
      }
      break;
    case 'a':
    case 'v':
    case 'o':
      for(i=0;i<3;i++){
	option = strsep(&optarg, ",");
	switch(i){
	case 0:
	  strcpy(file, option);
	  printf("file  : %s\n", file);
	  break;
	case 1:
	  off = strtol(option, NULL, 0);
	  printf("off   : %d\n", off);
	  break;
	case 2:
	  wid = strtol(option, NULL, 0);
	  printf("wid   : %d\n", wid);
	  break;
	}
      }
      if((cn[coinn].fd = fopen(file, "r")) == NULL){
	printf("Can't open slave time stamp table %s\n", file);
	quit();
      }else{
	cn[coinn].off = off;
	cn[coinn].wid = wid;
	memset(cn[coinn].name, 0, sizeof(cn[coinn].name));
	strncpy(cn[coinn].name, file, strlen(file)-4);
      }
      if(oval == 'a'){
	/* AND Coincidence */
	coin |= 1 << coinn;
	printf("AND  : name = %s\n", cn[coinn].name);
      }else if(oval == 'o'){
	/* OR Coincidence */
	printf("OR   : name = %s\n", cn[coinn].name);
      }else if(oval == 'v'){
	/* OR Coincidence */
	anti |= 1 << coinn;
	printf("VETO : name = %s\n", cn[coinn].name);
      }
      coinn++;
      break;
    case 'l':
      overlap = 1;
      printf("      : allow overlap coincidence\n");
      break;
    case 'u':
      update = 1;
      printf("      : update (rewrite) existing files\n");
      break;
    case 'e':
      efn = strtol(optarg, NULL, 0);
      printf("EFN : %d\n", efn);
      break;
    case 'm':
      maxbuff = strtol(optarg, NULL, 0);
      printf("MAXBUF: %d\n", maxbuff);
      break;
    case 't':
      mode |= BABITS_TABLE;
      printf("      : make time stamp table\n");
      break;
    case 'p':
      mode |= BABITS_PRINT;
      printf("      : print time stamp table\n");
      break;
    case 'r':
      mode |= BABITS_RUNNAME;
      printf("      : for runname search\n");
      break;
    case 'd':
      mode |= BABITS_DIRNAME;
      printf("      : for directory search\n");
      break;
    case 'f':
      mode |= BABITS_FILENAME;
      printf("      : for one file\n");
      break;
    case 'n':
      mode |= BABITS_NONSTOP;
      printf("      : non stop mode\n");
      break;
    case 's':
      single = 1;
      printf("      : single time stamp table\n");
      break;
    case 'c':
      mode |= BABITS_CHECK;
      printf("      : Check time stamp sequence\n");
      break;
    default:
      printf("Invalid option!!\n");
      return 0;
      break;
    }
  }
  return 1;
}

int help(void){
  printf("babits -btprdfn PATH [PATH]\n");
  printf("    -b : event build\n");
  printf("    -l : allow overlap coincidence\n");
  printf("    -t : make time stamp table\n");
  printf("    -p : print time stamp table\n");
  printf("    -r : for run search\n");
  printf("    -d : for directory search\n");
  printf("    -f : for one file\n");
  printf("    -s : make single time stamp table with -f\n");
  printf("    -n : non stop mode\n");
  printf("    -e : set efn (default = 256)\n");
  printf("    -m : set maximum buffer size (1 block size, default = 16384 short word)\n");
  printf("    -c : check the time stamp sequence\n");
  printf("    -a : slave timing table, it shoud be coincident (AND)\n");
  printf("    -o : slave timing table, if in coincidence, it will be stored (OR)\n");
  printf("\n");
  printf(" -b or -t or -p is must\n");
  printf(" for -t, -r -d -f shold be chose one\n");
  printf(" for -b, TABLE\n");
  printf("          = PATH for master timing table\n");
  printf(" for -a, TABLE,OFF,WID\n");
  printf("          = PATH for slave timing table and offset and width\n");
  printf(" for -o, TABLE,OFF,WID\n");
  printf("          = PATH for slave timing table and offset and width\n");
  printf("\n");
  printf("---------- example ------------\n");
  printf("for single run time stamp table (overwrite old file)\n");
  printf("-- babits -tfus RIDFFILE\n");
  printf("   \n");
  printf("for auto run increment time stamp table (overwirte old file)\n");
  printf("-- babits -trusn RIDFFILE\n");
  printf("   \n");
  printf("for time stamp event build\n");
  printf("-- babits -b TABLE1 -a TABLE2,OFF,WIDTH OUTPUT\n");
  printf("   TABLE1=master timing, TABLE2=slave timing, OUTPUT=output file\n");
  printf("   \n");
  printf("for non-stop time stamp event build for 1 run number\n");
  printf("-- babits -n -b TABLE1 -a TABLE2,OFF,WIDTH OUTPUT\n");
  printf("   TABLE1=master timing, TABLE2=slave timing, OUTPUT=output file\n");
  printf("   \n");
  printf("for check the time stamp sequence\n");
  printf("-- babits -c hoge.tst\n\n");
  printf("-------------------------------\n");


  return 0;
}

void quit(void){
  int i;

  printf("Exit babits\n");

  if(binfd.fd) fclose(binfd.fd);
  for(i=0;i<MAXCN;i++){
    if(infd[i].fd) fclose(infd[i].fd);
    if(cn[i].fd) fclose(cn[i].fd);
  }

  cleanfd();

  exit(0);
}


int readts(struct fdinfost *n){
  int rsz, evtn;
  unsigned long long int ts, fp, tmp;
  RIDFHD thd;
  RIDFRHD rhd;
  off_t tfp, nfp;

  if(lowspeed) usleep(1000);
  rsz = read(n->ifd, &thd, sizeof(thd));
  rhd = ridf_dechd(thd);

  if(rsz <= 0){
    return BABITS_EOF;
  }else if(rsz < sizeof(thd)){
    lseek(n->ifd, (off_t)-rsz, SEEK_CUR);
    return BABITS_EOF;
  }

  if(rhd.layer == RIDF_LY0){
    return BABITS_READ;
  }else{
    if(rhd.classid == RIDF_EVENT_TS){
      fp = lseek(n->ifd, 0, SEEK_CUR) - sizeof(thd);
      rsz = read(n->ifd, (char *)&evtn, sizeof(evtn));
      if(rsz < sizeof(evtn)){
	lseek(n->ifd, (off_t)-rsz, SEEK_CUR);
	return BABITS_EOF;
      }
      rsz = read(n->ifd, (char *)&ts, sizeof(ts));
      if(rsz < sizeof(ts)){
	lseek(n->ifd, (off_t)-rsz, SEEK_CUR);
	return BABITS_EOF;
      }

      if(ts != 0){
	tmp = rhd.efn & 0xff;
	ts = (ts & 0x0000ffffffffffffLL) | (tmp << 56);
	tmp = n->fn & 0xff;
	fp = (fp & 0x0000ffffffffffffLL) | (tmp << 48);
	
	write(n->ofd, (char *)&ts, sizeof(ts));
	write(n->ofd, (char *)&fp, sizeof(fp));
      }else{
	printf("ts = 0, skipped\n");
      }

    }else{
      if(lowspeed) usleep(1000);
      tfp = lseek(n->ifd, 0, SEEK_CUR);
      nfp = lseek(n->ifd, rhd.blksize*WORDSIZE - sizeof(thd), SEEK_CUR);
      if(nfp != (tfp + rhd.blksize*WORDSIZE - sizeof(thd))){
	lseek(n->ifd, tfp, SEEK_SET);
	DB(printf("lseek faild fp %lld/%lld\n", (unsigned long long int)tfp, (unsigned long long int)nfp)); 
      }
    }      
  }

  return BABITS_READ;
}


void storetseb(void){
  int sz, bsz;
  RIDFHD     hd;
  RIDFHDEOB  eobhd;
  RIDFHDBLKN blknhd;
  
  sz = sizeof(eobhd)/2;
  bsz = mp + sz;
  eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sz, efn, bsz);
  memcpy((char *)(buff+mp), (char *)&eobhd, sizeof(eobhd));
  
  hd = ridf_mkhd(RIDF_LY0, RIDF_EAF_BLOCK, bsz, efn);
  memcpy((char *)buff, (char *)&hd, sizeof(hd));
  
  fwrite(buff, 2, bsz, fd);
  
  // New buffer
  blkn ++;
  mp = sizeof(hd)/WORDSIZE;
  sz = sizeof(blknhd)/WORDSIZE;
  blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER, sz, efn, blkn);
  memcpy((char *)(buff+mp), (char *)&blknhd, sizeof(blknhd));
  mp += sizeof(blknhd)/2;
}


int tseb(void){
  RIDFHD     hd;
  int sz;
  long long int tb, tt;
  long long int val[2];
  long long int tval[MAXCN][2];
  RIDFHDEVT  evthd;
  RIDFHDEVTTS  evthdts;
  int segsize, evtsize, tmp, tmp2;
  int f, tcoin, i, tanti;

  if(lowspeed) usleep(1000);
  tb = fread(val, sizeof(long long int), 2, bn.fd);
  if(tb != 2){
    //if(!feof(bn.fd)){
    //printf("not eof beam\n");
    //fseeko(bn.fd, -tb * sizeof(long long int), SEEK_CUR);
    //} 
    return BABITS_EOF;
  }
  
  brdn ++;
  tb = val[0] & 0x0000ffffffffffffLL;
  bn.ts = val[0];
  bn.fp = val[1] & 0x0000ffffffffffffLL;
  bn.fn = (val[1] >> 48) & 0xffff;
  
  if(tb == 0){
    printf("bn.ts =0 / %llx %llx\n", val[0], val[1]);
  }
  tcoin = 0;
  tanti = 0;
  //printf("brdn %5d  tb=%llu\n", brdn, tb);
  for(i=0;i<coinn;i++){
    f = 1;
    while(f){
      if(!tn[i]){
	if(fread(tval[i], 8, 2, cn[i].fd) != 2){
	  //if(!feof(cn[i].fd)){
	  //printf("i=%d not eof \n", i);
	  //fseeko(cn[i].fd, -fb * sizeof(long long int), SEEK_CUR);
	  //}
	  //printf("fread != 2\n");
	  f = 0;
	  break;
	}
	trdn ++;
      }
      tt = tval[i][0] &  0x0000ffffffffffffLL;
      //printf("trdn %5d  tt=%llu\n", brdn, tt);
      
      if(tb + cn[i].off > tt){
	f = 1;
	tn[i] = 0;
      }else{
	if(tb + cn[i].off + cn[i].wid < tt){
	  f = 0;
	  tn[i] = 1;
	}else{
	  f = 0;
	  if(overlap == 1){
	    tn[i] = 1;
	  }else{
	    tn[i] = 0;
	  }
	  /* Coin */
	  tcoin |= 1<<i;
	  tanti |= (1<<i) & anti;
	  cn[i].ts = tval[i][0];
	  cn[i].fp = tval[i][1] & 0x0000ffffffffffffLL;
	  cn[i].fn = (tval[i][1] >> 48) & 0xffff;
	}
      }
    }
  }
  /* Store */
  if(tcoin){
    if(((coin & tcoin) == coin) && tanti == 0){
      tmp = mp;
      mp += sizeof(evthd)/2;
      evtsize = sizeof(evthd)/2;
      
      tmp2 = mp;
      segsize = sizeof(hd)/2;
      mp += sizeof(hd)/2;
      
      memcpy((char *)(buff+mp), (char *)&bn.ts, SIZEOFTS);
      mp += SIZEOFTS/2;
      segsize += SIZEOFTS/2;
      
      for(i=0;i<coinn;i++){
	if(tcoin & 1<<i){
	  /* Store ts data header*/
	  memcpy((char *)(buff+mp), (char *)&cn[i].ts, SIZEOFTS);
	  mp += SIZEOFTS/2;
	  segsize += SIZEOFTS/2;
	}
      }
      hd = ridf_mkhd(RIDF_LY2, RIDF_TIMESTAMP, segsize, 0);
      memcpy((char *)(buff+tmp2), (char *)&hd, sizeof(hd));
      evtsize += segsize;
      
      
      /* Segment data */
      if(binfd.fn != bn.fn){
	if(binfd.fn != -1) fclose(binfd.fd);
	if((binfd.fd = fopen(filenamefromflt(bn.fn, bn.name), "r")) == NULL){
	  printf("Can't open data file %s id=%d\n", bn.name, i);
	  quit();
	}else{
	  binfd.fn = bn.fn;
	  printf("Open %s (fn=%d)\n", 
		 filenamefromflt(bn.fn, bn.name), bn.fn);
	}
      }
      fseeko(binfd.fd, bn.fp, SEEK_SET);
      fread(&evthdts, sizeof(evthdts), 1, binfd.fd);
      sz = RIDF_SZ(evthdts.chd.hd1) - sizeof(evthdts)/2;

      fread(buff+mp, sizeof(short), sz, binfd.fd);
      /*
	while(rsz != sz){
	if(rsz != sz){
	}
	}
      */
      mp += sz;
      evtsize += sz;
      
      for(i=0;i<coinn;i++){
	if(tcoin & 1<<i){
	  /* Store event data */
	  /* Open ridf file */
	  if(infd[i].fn != cn[i].fn){
	    if(infd[i].fn != -1) fclose(infd[i].fd);
	    if((infd[i].fd = fopen(filenamefromflt(cn[i].fn, cn[i].name), "r")) == NULL){
	      printf("Can't open data file %s id=%d\n", cn[i].name, i);
	      quit();
	    }else{
	      infd[i].fn = cn[i].fn;
	      printf("Open %s (fn=%d)\n", 
		     filenamefromflt(cn[i].fn, cn[i].name), cn[i].fn);
	    }
	  }
	  fseeko(infd[i].fd, cn[i].fp, SEEK_SET);
	  fread(&evthdts, sizeof(evthdts), 1, infd[i].fd);
	  sz = RIDF_SZ(evthdts.chd.hd1) - sizeof(evthdts)/2;
	  fread(buff+mp, sizeof(short), sz, infd[i].fd);
	  mp += sz;
	  //if(rsz != sz){
	  //  printf("i=%d rsz=%d / sz=%d\n", i, sz, rsz);
	  //}
	  evtsize += sz;
	}
      }
      
      /* Store Event header*/
      tsevtn ++;
      evthd = ridf_mkhd_evt(RIDF_LY1, RIDF_EVENT, evtsize, efn, tsevtn);
      memcpy((char *)(buff+tmp), (char *)&evthd, sizeof(evthd));
    }
    
    if(mp > maxbuff){
      storetseb();
    }
  }
  
  return BABITS_READ;
}


int main(int argc, char *argv[]){
  int t, o, j, evtn;
  fd_set fdset;
  struct fdinfost *n;
  int flag;
  int sz;
  struct stat stt;
  RIDFHD hd;
  RIDFHDBLKN blknhd;
  unsigned long long int val[2], tts, tfp, nts, nfp;

  fdfirst = NULL;
  memset((char *)tn, 0, sizeof(tn));
  tsevtn = 0;
  
  printf("babits\n");

  for(j=0;j<MAXCN;j++){
    infd[j].fn = -1;
    infd[j].fd = NULL;
  }

  binfd.fn = -1;
  binfd.fd = NULL;

  memset((char *)cn, 0, sizeof(cn));

  coin = 0;
  anti = 0;
  coinn = 0;

  if(!chkopt(argc, argv)){
    help();
    quit();
  }

  /*
    if(!lowspeed){
    printf("--- babits high-speed mode for offline ---\n");
    }
  */

  /* Check main option */
  o = mode & 0x000f;
  if(o){
    switch(o){
    case BABITS_TABLE:
      break;
    case BABITS_PRINT:
    case BABITS_CHECK:
      if((fd = fopen(argv[argc-1], "r")) == NULL){
	printf("Can't open time stamp table %s\n", argv[argc-1]);
	quit();
      }
      evtn=0;
      break;
    case BABITS_BUILD:
      if((fd = fopen(argv[argc-1], "w")) == NULL){
	printf("Can't open output file %s\n", argv[argc-1]);
	quit();
      }else{
	mp = sizeof(hd)/WORDSIZE;
	sz = sizeof(blknhd)/2;
	blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER, sz, efn, blkn);
	memcpy((char *)(buff+mp), (char *)&blknhd, sizeof(blknhd));
	mp += sizeof(blknhd)/2;
      }
      break;
    default:
      printf("Invalid option for operation mode\n");
      help();
      quit();
    }
  }else{
    printf("Invalid option for operation mode\n");
    help();
    quit();
  }

  /* Check file mode */
  if(o == BABITS_TABLE){
    if((t = mode & 0x00f0)){
      if(t != BABITS_DIRNAME && t != BABITS_RUNNAME && t != BABITS_FILENAME){
	printf("Invalid option for path option\n");
	help();
	quit();
      }
    }else{
      printf("No option for path option\n");
      
      help();
      quit();
    }
  }

  /* Signal SIGINT -> quit() */
  signal(SIGINT, (void *)quit);

  fdn = 0;
  maxfd = 0;

  t = mode & 0x00f0;
  if(t == BABITS_FILENAME){
    fdn++;

    if(!addfdbyfile(argv[argc-1], fdn, update, single)){
      printf("Error for %s\n", argv[argc-1]);
      quit();
    }else{
      strcpy(curfile, argv[argc-1]);
      maxfd = getmaxfd();
    }
  }else if(t == BABITS_RUNNAME){
    splitrunname(argv[argc-1], runname, &runnumber, 0);
    sprintf(curfile,  "%s%04d.ridf", runname, runnumber);
    sprintf(nextfile, "%s%04d.ridf", runname, runnumber+1);
    if(!addfdbyfile(curfile, fdn, update, single)){
      printf("Error for %s\n", curfile);
      quit();
    }else{
      maxfd = getmaxfd();
    }
  }


  switch(o){
  case BABITS_TABLE:
    printf("Current file = %s\n", curfile);
    while(1){
      setfdset(&fdset);
      
      if(select(maxfd+1, &fdset, NULL, NULL, NULL) != 0){
	n = getisfd(&fdset);
	flag = readts(n);
	
	if(flag == BABITS_EOF && !(mode & BABITS_NONSTOP) ){
	  closefd(n);
	}

	if(flag == BABITS_EOF && (mode & BABITS_NONSTOP) ){
	  if(!stat(nextfile, &stt)){
	    closefd(n);
	  }else{
	    sleep(1);
	    continue;
	  }
	}

	if(flag == BABITS_EOF && (mode & BABITS_RUNNAME)){
	  if(!stat(nextfile, &stt)){
	    strcpy(curfile, nextfile);
	    runnumber++;
	    sprintf(nextfile, "%s%04d.ridf", runname, runnumber+1);
	    addfdbyfile(curfile, fdn, update, single);
	    maxfd = getmaxfd();
	    printf("Current file = %s\n", curfile);
	  }else{
	    printf("No more file %s\n", nextfile);
	  }
	}


	if(!getfdfirst()) break;
      }
    }
    break;
  case BABITS_PRINT:
    while(!feof(fd)){
      if(lowspeed) usleep(1000);
      fread(val, sizeof(long long int), 2, fd);
      printf("%016llx %016llx\n", val[0], val[1]);
      printf(" EFN[%3d] : TS=%llu\n",
	     (int)(val[0] >> 56) & 0xff , val[0] & 0x0000ffffffffffffLL);
      printf("File[%3d] : FP=%llu\n\n",
	     (int)(val[1] >> 48) & 0xff , val[1] & 0x0000ffffffffffffLL);
      if(!(mode & BABITS_NONSTOP)){
	getchar();
      }
    }
    fclose(fd);
    fd = 0;
    break;
  case BABITS_CHECK:
    tts = 0;
    tfp = 0;
    while(!feof(fd)){
      evtn++;
      if(lowspeed) usleep(1000);
      fread(val, sizeof(long long int), 2, fd);
      nts =  val[0] & 0x0000ffffffffffffLL;
      nfp =  val[1] & 0x0000ffffffffffffLL;
      if(nts < tts){
	printf("Time stamp is incorrect %d :\n", evtn);
	printf("  previous TS=%llu / FP=%llu\n", tts, tfp);
	printf("  current  TS=%llu / FP=%llu\n", nts, nfp);
      }
      tts = nts;
      tfp = nfp;
    }
    fclose(fd);
    fd = 0;
    break;
  case BABITS_BUILD:
    while(1){
      flag = tseb();
      
      if(flag == BABITS_EOF && !(mode & BABITS_NONSTOP) ){
	fclose(bn.fd);
	break;
      }

      if(flag == BABITS_EOF && (mode & BABITS_NONSTOP) ){
	sleep(1);
	continue;
      }
    }
    break;
  }
  
  
  if(o == BABITS_BUILD){
    if(mp > 16){
      storetseb();
    }
  }

  if(fd) fclose(fd);
  quit();

  return 0;
}

