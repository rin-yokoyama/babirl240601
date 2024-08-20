/* 
 * example to use libbabies with pas mode
 * H.B. RIKEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libbabies.h"
#include "segidlist.h"

extern struct stdaqinfo daqinfo;
extern struct struninfo runinfo;

int efn = 0;
int usl = 10000;

void quit(void){
  printf("Exit\n");
}

void start(void){
  printf("Start\n");
}

void stop(void){
  printf("Stop\n");
}

void reload(void){
  printf("Reload\n");
}

// thread
void evtloop(void){
  int status;
  volatile int ssflag = 0;

  while((status = babies_status()) != -1){
    switch(status){
    case STAT_RUN_IDLE:
      /* noop */
      usleep(usl);
      break;
    case STAT_RUN_START:
    case STAT_RUN_NSSTA:
      if(ssflag == 0){
	if(status == STAT_RUN_START){
	  printf("START\n");
	}else{
	  printf("NSSTA\n");
	}
	printf("header %s\n", runinfo.header);
	printf("runname %s\n", daqinfo.runname);
	printf("number %d\n", daqinfo.runnumber);
	printf("start %d\n", runinfo.starttime);
	printf("stop %d\n", runinfo.stoptime);
	ssflag = 1;
      }
      usleep(usl); // wait event
      break;
    case STAT_RUN_WAITSTOP:
      pas_ending();
      ssflag = 0;
      break;
    default:
      break;
    }
  }

  // write codes to quit safely

}

int main(int argc, char *argv[]){

  if(argc < 2){
    printf("csample EFN [uleep]\n");
    exit(0);
  }else{
    efn = strtol(argv[1], NULL, 0);
  }

  if(argc == 3){
    usl = strtol(argv[2], NULL, 0);
  }

  babies_quit(quit);
  babies_start(start);
  babies_stop(stop);
  babies_reload(reload);
  babies_evtloop(evtloop);
  babies_name("passample");

  babies_init(efn);
  babies_check();

  babies_pasmain();

  return 0;
}
