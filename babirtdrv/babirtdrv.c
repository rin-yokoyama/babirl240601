/* babirl: babirtdrv.c
   last modified : 08/10/09 18:16:12 

   Hidetada Baba
   (RIKEN)
  
*/

/* #define BBRL_DEBUG */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/cons.h>
#include <asm/io.h>

#include <rtl.h>
#include <rtl_sched.h>
#include <rtl_fifo.h>
#include <asm/rt_irq.h>
#include <asm/io.h>
#include <time.h>
#include <pthread.h>
#if LINUX_VERSION_CODE >=  0x020410
  MODULE_LICENSE("GPL");
#endif

#define BBRL_FREQ 40000

#define TRUE  1
#define FALSE 0
#define STAT_RUN_IDLE     0
#define STAT_RUN_START    1
#define STAT_RUN_NSSTA    2
#define STAT_RUN_WAITSTOP 3

#include "bi-config.h"
#include "bbmodules.h"
#include "babilctrl.h"

#include "babirldrvdef.h"
#include "babirldrvridfrt.c"

#include "v513.c"
#include "v775.c"
#include "v785.c"
#include "v792.c"
#include "rpv130.c"

#include "evt.c"
#include "sca.c"
#include "startup.c"
#include "clear.c"
#include "stop.c"
#include "dfull.c"

pthread_t daqtask;
static short bl;
volatile static int dfull;
volatile static int crbuf;
char bflag = 0;


void camac_handler(void){
  clear();
#ifdef CAMACINT
  rfs_enable_interrupt();           // Enable Interrupt
#endif
#ifdef VMEINT
  vme_enable_interrupt();
#endif
}

void evt_handler(void){
  if(bl == 0){
    init_block();                   // Initialize Block
    bl = 1;
  }
  evt();

#ifdef CAMACINT
  control_mod(0,LAMN,LAMA,LAMC);          // Clear Module
  control_mod(0,LAMN,LAMA,LAMF);         // Enable LAM
#endif

  if(end_event() > MAXBUFF){         // Event END
    sca();
    end_block();                    // Block END
    //write_mod(0,OPRN,0,17,&opbuff); // Output Reg.
    //crate_seti(0);
    //crate_deli(0);
#ifdef CAMACINT
    crate_enable_lam((0x0001<<(LAMN-1)));  // Enable LAM
#endif
    bl = 2;
  }

  if(bl == 1){
    camac_handler();
  }else{
    dinit_shmem(crbuf);                   // Set Flag of Shared Memory
    if(crbuf == 0){
      change_shmem(1);
      crbuf = 1;
      bl = 0;
      if(dget_flag(0) == 0x01 && dget_flag(1) == 0x02){
	dfull = 1;
	dfullf();
      }else{
	camac_handler();
      }
    }else{
      change_shmem(0);
      crbuf = 0;
      bl = 0;
      if(dget_flag(0) == 0x01 && dget_flag(1) == 0x02){
	dfull = 1;
	dfullf();
      }else{
	camac_handler();
      }
    }
  }
}

void *daq_handler(void *t){
  while(1){
    pthread_wait_np();
    
    if(dget_flag(0) == 0x01 && dget_flag(1) == 0x02){
      /* no operation (dfull) */
    }else{
      if(dfull == 1){
	camac_handler();
	dfullcl();
	dfull = 0;
      }else{
#ifdef CAMACINT
	if(check_lam() != 0)
#endif
#ifdef VMEINT
	if(vme_check_interrupt() != 0)
#endif
	  {
	    evt_handler();
	  }else{
	  /* no operation */
	}
      }
    }
  }
}

int init_module(void){
  struct sched_param p;
  hrtime_t now = gethrtime();

  crbuf = 0;
  dfull = 0;

  init_driver();        // Initialize Driver
  change_shmem(crbuf);
  bl = 0;

  init_evtn();

  startup();                        // Startup Function

#ifdef CAMACINT
  rfs_enable_interrupt();           // Enable Interrupt
#endif

  ssm_start();                     // Set start flag

  pthread_create(&daqtask,NULL,daq_handler,(void *)1);
  pthread_make_periodic_np(daqtask,now+100000,BBRL_FREQ);
  p.sched_priority = 1;
  pthread_setschedparam(daqtask,SCHED_FIFO,&p);

  return 0;
}

void cleanup_module(void){
  int chkint = 0;

  pthread_delete_np(daqtask);

#ifdef CAMACINT
  if(check_lam() != 0){
#endif
#ifdef VMEINT
  if(vme_check_interrupt() != 0){
#endif
    chkint = 1;
  }

  if(dfull == 1 && chkint == 1){
    crbuf = 2;
    change_shmem(crbuf);
    bl = 1;
    init_block();                   // Initialize Block
  }else if(chkint == 1){
    bl = 1;
    init_block();                   // Initialize Block
  }
  if(chkint == 1 && bl == 1){
    evt();
    clear();
#ifdef CAMACINT
    control_mod(0,LAMN,LAMA,LAMC);          // Clear Module
    control_mod(0,LAMN,LAMA,LAMF);         // Enable LAM
#endif
    end_event();
  }

  if(bl == 1){
    sca();
    end_block();                    // Block END
    dinit_shmem(crbuf);                   // Set Flag of Shared Memory
  }

  stop();

  store_evtn();

  ssm_stop();                     // Set stop flag
  end_driver();                     // END of Driver

#ifdef CAMACINT
  rfs_disable_interrupt();          // Disable Interrupt
#endif
#ifdef VMEINT
  vme_disable_interrupt();
#endif
}
