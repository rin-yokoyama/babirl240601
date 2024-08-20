#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <bi-common.h>
#include <bi-config.h>
#include <ridf.h>

#define MOD      43
#define DET      59

/* Focal plane = channel number */
#define BEAM_FP   1
#define BETA_FP   2
#define CLOVER_FP 3
#define VETO_FP   4

#define MAXBUFSIZE 100000

#define CHKRIDF_NEXT_HD    0x01
#define CHKRIDF_NEXT_EVT   0x02
#define CHKRIDF_NEXT_BLK   0x04
#define CHKRIDF_NEXT_SCR   0x08
#define CHKRIDF_NEXT_COM   0x10
#define CHKRIDF_NEXT_STA   0x20

/* Not implemented yet */
#define CHKRIDF_PREV_HD   11
#define CHKRIDF_PREV_EVT  12
#define CHKRIDF_PREV_BLK  13

#define CHKRIDF_BLK_COL   FG_BLACK
#define CHKRIDF_EVT_COL   FG_BLUE
#define CHKRIDF_SEG_COL   FG_GREEN
#define CHKRIDF_SCR_COL   FG_MAGENTA
#define CHKRIDF_COM_COL   FG_YELLOW
#define CHKRIDF_STA_COL   FG_CYAN

int ridf_ly(struct ridf_hdst hd){
  return RIDF_LY(hd.hd1);
}

int ridf_ci(struct ridf_hdst hd){
  return RIDF_CI(hd.hd1);
}

int ridf_sz(struct ridf_hdst hd){
  return RIDF_SZ(hd.hd1);
}

int ridf_ef(struct ridf_hdst hd){
  return RIDF_EF(hd.hd2);
}

struct ridf_rhdst ridf_dechd(struct ridf_hdst hd){
  struct ridf_rhdst sthd;
  
  sthd.layer   = ridf_ly(hd);
  sthd.classid = ridf_ci(hd);
  sthd.blksize = ridf_sz(hd);
  sthd.efn     = ridf_ef(hd);
  
  return sthd;
}

void print_rhd(RIDFRHD rhd){
  printf("ly=%d, cid=%d, size=%d, efn=%d\n",
	 rhd.layer, rhd.classid, rhd.blksize, rhd.efn);
}

int main(int argc,char *argv[]){
  RIDFHD hd;
  RIDFRHD rhd;
  int com = CHKRIDF_NEXT_HD;
  
  string path;
  string name;
  string out;
  
  int idx1,idx2,idx3;
  
  unsigned int blkn, evtn, segid;
  
  FILE *ifd;
  FILE *ofd1;
  FILE *ofd2;
  FILE *ofd3;
  FILE *ofd4;
  
  unsigned long long int tsval;
  unsigned long long int sval[2];
  
  int i,j,k;
  char *date;
  
  char ret[2];
  int  endflag,clflag;
  
  char tcom = 0;
  char tcombuff[1024];
  unsigned short buff[MAXBUFSIZE];
  int sz,segdatas,efn,scrid,scrdatas;
  
  struct stbsegid bsid;
  
  if(argc < 2){
    cout << "dtmon2tst [FILENAME]" << endl;
    return 0;
   }
  
  path = argv[1];
  
  idx1 = path.find_last_of('/');
  idx2 = path.find_last_of('.');
  idx3 = path.find_last_of('_');
  
  name = path.substr(idx3+1,idx2-idx3-1);
  
  cout << "name =" << name << endl;
  
  if((ifd = fopen(path.c_str(),"r")) == NULL){
    cout << "Can't open" << path << endl;
    return 0;
  }
  
  out = "beamdt_" + name + ".tst";
  if((ofd1 = fopen(out.c_str(),"w")) == NULL){
    cout << "Can't open" << name << endl;
    return 0;
  }
  
  out = "betadt_" + name + ".tst";
  if((ofd2 = fopen(out.c_str(),"w")) == NULL){
    cout << "Can't open" << name << endl;
    return 0;
  }
  
  out = "cloverdt_" + name + ".tst";
  if((ofd3 = fopen(out.c_str(),"w")) == NULL){
    cout << "Can't open" << name << endl;
    return 0;
  }
  
  out = "vetodt_" + name + ".tst";
  if((ofd4 = fopen(out.c_str(),"w")) == NULL){
    cout << "Can't open" << name << endl;
    return 0;
  }
  
  blkn = -1;
  
  memset(buff,0,sizeof(buff));
  
  while(fread((char *)&hd, sizeof(hd),1,ifd) == 1){ 
    //   while(feof(ifd) == 0){
    if(tcom == 'e'){
      com = CHKRIDF_NEXT_EVT;
    }else if(tcom == 'b'){
      com = CHKRIDF_NEXT_BLK;
    }else if(tcom == 's'){
      com = CHKRIDF_NEXT_SCR;
    }else if(tcom == 'c'){
      com = CHKRIDF_NEXT_COM;
    }else if(tcom == 't'){
      com = CHKRIDF_NEXT_STA;
    }else if(tcom == 'q'){
      break;
    }else{
      com = CHKRIDF_NEXT_HD;
    }
    
    rhd = ridf_dechd(hd);
    
    switch(rhd.classid){
    case RIDF_EF_BLOCK:
    case RIDF_EA_BLOCK:
    case RIDF_EAEF_BLOCK:
      blkn++;
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_BLK)){
	//	cprintf(CHKRIDF_BLK_COL, "EF Block Header / blkn=%d\n", blkn);
//	printf("EF Block Header / blkn=%d\n", blkn);
	print_rhd(rhd);
//	printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_END_BLOCK:
      fread((char *)&sz, sizeof(sz), 1, ifd);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_BLK)){
	//	cprintf(CHKRIDF_BLK_COL, "EF Block Ender / blkn=%d\n", blkn);
//	printf("EF Block Ender / blkn=%d\n", blkn);
	print_rhd(rhd);
//	printf("Size of this block = %d\n", sz);
//	printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_BLOCK_NUMBER:
      fread((char *)&sz, sizeof(sz), 1, ifd);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_BLK)){
	//	cprintf(CHKRIDF_BLK_COL, "EF Block Number / blkn=%d\n", blkn);
//	printf("EF Block Number / blkn=%d\n", blkn);
	print_rhd(rhd);
//	printf("The number of this block = %d\n", sz);
//	printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_EVENT:
    case RIDF_EVENT_TS:
      fread((char *)&evtn, sizeof(evtn), 1, ifd);
      if(rhd.classid == RIDF_EVENT_TS){
	fread((char *)&tsval, sizeof(tsval), 1, ifd);
      }	
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_EVT)){
	if(rhd.classid == RIDF_EVENT_TS){
	  //	  cprintf(CHKRIDF_EVT_COL, "Event Header with Time Stamp / blkn=%d\n", blkn);
//	  printf("Event Header with Time Stamp / blkn=%d\n", blkn);
	}else{
	  //	  cprintf(CHKRIDF_EVT_COL, "Event Header / blkn=%d\n", blkn);
//	  printf("Event Header / blkn=%d\n", blkn);
	}
	print_rhd(rhd);
//	printf("Event Number = %d\n", evtn);
	if(rhd.classid == RIDF_EVENT_TS){
	  // printf("Time Stamp = %lld\n", tsval);
	}
	//printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_SEGMENT:
      fread((char *)&segid, sizeof(segid), 1, ifd);
      segdatas = rhd.blksize-(sizeof(hd)+sizeof(segid))/WORDSIZE;
      fread((char *)buff, 2, segdatas, ifd);
      if(com & CHKRIDF_NEXT_HD){
	//	cprintf(CHKRIDF_SEG_COL, "Segment Header / blkn=%d\n", blkn);
	//printf("Segment Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	// printf("Segment ID = %d (0x%08x)\n", segid, segid);
	memcpy((char *)&bsid, (char *)&segid, sizeof(segid));
	// printf("Rev %d / Dev %d / FP  %d / Det %d / Mod %d\n",
	 //      bsid.revision, bsid.device, bsid.focal,
	 //      bsid.detector, bsid.module);
	if(bsid.focal == BEAM_FP){
	  printf("Focal Plane = BEAM \n");
	}else if(bsid.focal == BETA_FP){
	  printf("Focal Plane = BETA \n");
	}else if(bsid.focal == CLOVER_FP){
	  printf("Focal Plane = CLOVER \n");
	}else if(bsid.focal == VETO_FP){
	  printf("Focal Plane = VETO \n");
	}
	
	for(i=0;i<segdatas;i++){
	  // printf("%04x ", buff[i]);
	  if((i+1)%8 == 0){
	    // printf("\n");
	  }
	}
	for(i=0;i<segdatas/4;i++){
	  sval[0] = buff[i*4+3] * 0x1000000000000 ;
      sval[0] = sval[0] + buff[i*4+2] * 0x100000000 ;
      sval[0] = sval[0] + buff[i*4+1] * 0x10000 ;
      sval[0] = sval[0] + buff[i*4];
	  sval[1] = 0x1000000000000001;
	if(bsid.focal == BEAM_FP){
	  // printf("Focal Plane = BEAM \n");
	  fwrite(sval,sizeof(sval),1,ofd1);
	}else if(bsid.focal == BETA_FP){
	  // printf("Focal Plane = BETA \n");
	  fwrite(sval,sizeof(sval),1,ofd2);
	}else if(bsid.focal == CLOVER_FP){
	  // printf("Focal Plane = CLOVER \n");
	  fwrite(sval,sizeof(sval),1,ofd3);
	}else if(bsid.focal == VETO_FP){
	  // printf("Focal Plane = VETO \n");
	  fwrite(sval,sizeof(sval),1,ofd4);
	}
	}
	if(i%8 != 0){
	   //printf("\n");
	}
	// printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_TIMESTAMP:
      segdatas = rhd.blksize-sizeof(hd)/WORDSIZE;
      fread((char *)buff, 2, segdatas, ifd);
      if(com & CHKRIDF_NEXT_HD){
	//	cprintf(CHKRIDF_SEG_COL, "Timestamp Header / blkn=%d\n", blkn);
	// printf("Timestamp Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	for(i=0;i<segdatas;i+=4){
	  memcpy((char *)&tsval, (char *)(buff+i), sizeof(tsval));
	  efn = tsval >> 56;
	  tsval = tsval & 0x0000ffffffffffff;
	   // printf("efn[%3d] : TS=%llu\n", efn, tsval);
	}
	// printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_SCALER:
    case RIDF_CSCALER:
    case RIDF_NCSCALER32:
      fread((char *)&date, sizeof(date), 1, ifd);
      fread((char *)&scrid, sizeof(scrid), 1, ifd);
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      fread((char *)buff, 2, scrdatas, ifd);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_SCR)){
	//	cprintf(CHKRIDF_SCR_COL, "Scaler Header / blkn=%d\n", blkn);
	// printf("Scaler Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	// printf("Scaler Date = %d\n", date);
	// printf("Scaler ID = %d\n", scrid);
	for(i=0;i<scrdatas;i++){
	  // printf("%04x ", buff[i]);
	  if((i+1)%8 == 0){
	    // printf("\n");
	  }
	}
	if(i%8 != 0){
	  // printf("\n");
	}
	// printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_COMMENT:
      fread((char *)&date, sizeof(date), 1, ifd);
      fread((char *)&scrid, sizeof(scrid), 1, ifd);
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      fread((char *)buff, 2, scrdatas, ifd);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_COM)){
	//	cprintf(CHKRIDF_COM_COL, "Comment Header / blkn=%d\n", blkn);
	// printf("Comment Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	// printf("Comment Date = %d\n", date);
	//printf("Comment ID = %d\n", scrid);
	//printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    case RIDF_STATUS:
      fread((char *)&date, sizeof(date), 1, ifd);
      fread((char *)&scrid, sizeof(scrid), 1, ifd);
      scrdatas = rhd.blksize-(sizeof(hd)+sizeof(scrid)+sizeof(date))/WORDSIZE;
      fread((char *)buff, 2, scrdatas, ifd);
      if(com & (CHKRIDF_NEXT_HD | CHKRIDF_NEXT_STA)){
	//	cprintf(CHKRIDF_STA_COL, "Status Header / blkn=%d\n", blkn);
	//printf("Status Header / blkn=%d\n", blkn);
	print_rhd(rhd);
	//printf("Status Date = %d\n", date);
	//printf("Status ID = %d\n", scrid);
	//printf("\n: ");
#ifndef NSTOP
	fgets(tcombuff, sizeof(tcombuff), stdin);
	tcom = tcombuff[0];
#endif
      }
      break;
    default:
    //  printf("Unknown Header\n");
      print_rhd(rhd);
    //  printf("\n: ");
      fread((char *)buff, WORDSIZE, rhd.blksize - sizeof(hd)/WORDSIZE, ifd);
#ifndef NSTOP
      fgets(tcombuff, sizeof(tcombuff), stdin);
      tcom = tcombuff[0];
#endif
    }
  }
  
  fclose(ofd1);
  fclose(ofd2);
  fclose(ofd3);
  fclose(ofd4);
  fclose(ifd);
  
  return 0;
}
