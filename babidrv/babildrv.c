/* babirl/babidrv/babildrv.c
 * last modified : 11/01/14 18:15:39 
 *
 * DAQ driver for normal Linux with babirl
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
*/

#include <linux/version.h>
#if LINUX_VERSION_CODE >=  0x020600
#if defined(USE_MODVERSIONS) && USE_MODVERSIONS
#  define MODVERSIONS
#  include <config/modversions.h>
#endif
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#if LINUX_VERSION_CODE >=  0x020600
#include <linux/init.h>
#include <linux/interrupt.h>
#endif
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/sched.h>
#if LINUX_VERSION_CODE <  0x020600
#include <linux/config.h>
#endif
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "bbmodules.h"
#include "babilctrl.h"
#include "babildrv.h"
#include "babilio.h"

#if LINUX_VERSION_CODE >=  0x020410
MODULE_LICENSE("GPL");
#endif
#if LINUX_VERSION_CODE >=  0x020600
MODULE_AUTHOR("Hidetada Baba");
#ifndef MOD_IN_USE
#define MOD_IN_USE module_refcount(THIS_MODULE)
#endif
#ifndef MOD_INC_USE_COUNT
#define MOD_INC_USE_COUNT
#endif
#ifndef MOD_DEC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif
#endif

#include "babirldrvdef.h"
#include "babirldrvridf.c"

#include "usemodules.h"

#include "segidlist.h"

#include "startup.c"
#include "evt.c"
#include "sca.c"
#include "stop.c"
#include "clear.c"

static int babildrv_major = BABILDRV_MAJOR;
static const char *babildrv_name = "babildrv";
#if LINUX_VERSION_CODE >= 0x020600
static int dev_id;
#else
static char *irq_name = "irq_babil";
#endif

static volatile int startflag = 0;

ssize_t babildrv_read(struct file *file,char *buff,size_t count,loff_t *pos);
static int babildrv_open(struct inode* inode, struct file* filep);
static int babildrv_release(struct inode* inode, struct file* filep);
static int babildrv_ioctl(struct inode *inode, struct file *filep,
			 unsigned int cmd, unsigned long arg);
static unsigned int babildrv_poll(struct file *file,poll_table *wait);
#if LINUX_VERSION_CODE >= 0x020600
static irqreturn_t babildrv_interrupt(int irq, void* dev_id, struct pt_regs* regs);
#else
static void babildrv_interrupt(int irq,void *dev_id,struct pt_regs* regs);
#endif

static int girq,chkbuff,chkblk;
#if LINUX_VERSION_CODE >=  0x020400
wait_queue_head_t babil_wait_queue;
#else
struct wait_queue *babil_wait_queue = NULL;
#endif

#if LINUX_VERSION_CODE >=  0x020600
static struct file_operations babildrv_fops = {
  .read = babildrv_read,
  .poll = babildrv_poll,
  .ioctl = babildrv_ioctl,
  .open = babildrv_open,
  .release = babildrv_release,
};
#else
#if LINUX_VERSION_CODE >=  0x020400
static struct file_operations babildrv_fops = {
  read: babildrv_read,
  poll: babildrv_poll,
  ioctl: babildrv_ioctl,
  open: babildrv_open,
  release: babildrv_release,
};
#else
static struct file_operations babildrv_fops = {
  NULL,             // loff_t  llseek
  babildrv_read,     // ssize_t read
  NULL,             // ssize_t write
  NULL,             // int     readdir
  babildrv_poll,     // uint    poll
  babildrv_ioctl,    // int     ioctl
  NULL,             // int     mmap
  babildrv_open,     // int     open
  NULL,             // int     flush
  babildrv_release,  // int     release
};
#endif
#endif

#ifdef DBUFF
volatile static int stopflag = 0;
volatile int drn = 0;
#endif

#if LINUX_VERSION_CODE >= 0x020600
spinlock_t lock = SPIN_LOCK_UNLOCKED;
#define BCLILOCK  spin_lock_irqsave(&lock, flags);
#define BCLIUNLOCK spin_unlock_irqrestore(&lock, flags);
#else
#if LINUX_VERSION_CODE >= 0x020400
#define BCLILOCK cli();
#define BCLIUNLOCK sti();
#else

#endif
#endif


#if LINUX_VERSION_CODE >= 0x020600
int babildrv_init_module(void){
#else
int init_module(void){
#endif
  int ret;

  ret = register_chrdev(babildrv_major,babildrv_name,&babildrv_fops);
  if(ret < 0){
    printk("%s : can't regist.\n",babildrv_name);
    return ret;
  }
  if(babildrv_major == 0) {
    babildrv_major = ret;
  }

#if LINUX_VERSION_CODE >= 0x020600
  spin_lock_init(&lock);
#endif

  girq = get_irq();
  chkbuff = 0;

#if LINUX_VERSION_CODE >=  0x020400
  init_waitqueue_head(&babil_wait_queue);
#endif

#ifdef VMEINT
    vme_disable_interrupt();
#else
    rfs_disable_interrupt();
#endif

#if LINUX_VERSION_CODE >= 0x020600
#if LINUX_VERSION_CODE >= 0x020615
  ret = request_irq(girq,(irq_handler_t)babildrv_interrupt,IRQF_SHARED,babildrv_name, &dev_id);
#else
  ret = request_irq(girq,babildrv_interrupt,SA_SHIRQ,babildrv_name,&dev_id);
#endif
#else
  ret = request_irq(girq,babildrv_interrupt,SA_SHIRQ,babildrv_name,irq_name);
#endif

#ifdef VMIVME
  vme_interrupt_attach(bus, &interrupt, INTLEVEL, INTVEC,
		       VME_INTERRUPT_RESERVE, NULL);
#endif


  printk("%s : babildrv was installed (irq %d).\n",babildrv_name,girq);

  return 0;
}

#if LINUX_VERSION_CODE >= 0x020600
void babildrv_cleanup_module(void){
#else
void cleanup_module(void){
#endif

#if LINUX_VERSION_CODE >= 0x020600
  free_irq(girq,&dev_id);
#else
  free_irq(girq,irq_name);
#endif
  
  unregister_chrdev(babildrv_major,babildrv_name);
  printk("%s: babildrv was unregistered.\n", babildrv_name);
}

static int babildrv_open(struct inode* inode, struct file* filep){
  unsigned long flags;

  if(MOD_IN_USE){
    return -EBUSY;
  }

  BCLILOCK

#ifdef CCNET
  ccnet_check_lam();
#endif

  chkblk = 0;
  chkbuff = 0;
#ifdef DBUFF
  stopflag = 0;
  drn = 0;
#endif

  init_mem();

  MOD_INC_USE_COUNT;

  startflag = 1;
  init_evtn();
  startup();

#ifdef VMEINT
  vme_enable_interrupt();
#else
  rfs_enable_interrupt();
#endif
  BCLIUNLOCK

  return 0;
}

static int babildrv_release(struct inode* inode, struct file* filep){

  chkbuff = 0;

#ifdef VMEINT
  vme_disable_interrupt();
#else
  rfs_disable_interrupt();
#endif

  MOD_DEC_USE_COUNT;

  return 0;
}

ssize_t babildrv_read(struct file *file,char *buff,size_t count,loff_t *pos){
  int len;
  unsigned long x, flags;

#ifndef DBUFF
  len = mp * WORDSIZE;
  chkbuff = 0;
  x = copy_to_user(buff, data, len);
#else
  if(drn == 0){
    len = mpa * WORDSIZE;
  }else{
    len = mpb * WORDSIZE;
  }
  x = copy_to_user(buff,(char *)(databuff+EB_EFBLOCK_BUFFSIZE*drn),len);
  drn++;
  if(drn == 2) drn = 0;

  if(chkbuff == 2){
    BCLILOCK
    if(stopflag == 0){
#ifdef VMEINT
      vme_enable_interrupt();
#else
      rfs_enable_interrupt();
#endif
    }
    clear();
    if(stopflag == 0){
#ifdef K2915
      rfs_enable_interrupt();
#endif
    }
    BCLIUNLOCK
  }

  chkbuff--;
  if(chkbuff < 0){
    chkbuff = 0;
  }
#endif

  return 1;
}


static int babildrv_ioctl(struct inode* inode, struct file *filep,
			 unsigned int cmd, unsigned long arg){
  char flag;
  int len;
  unsigned long x, flags;

  BCLILOCK
  switch(cmd){
  case BABIL_STOP:
  case BABIL_STOPB:
#ifdef VMEINT
    vme_disable_interrupt();
#else
    rfs_disable_interrupt();
#endif
    startflag = 0;

#ifdef DBUFF
    stopflag = 1;
#endif
    if(chkblk == 1){
      sca();
      end_block();
#ifdef VMEINT
      /* none */
#else
      //crate_seti(0);
#endif
#ifndef DBUFF
      flag = 1;
#else
      chkbuff++;
      flag = chkbuff;
      chmem();
#endif
      x = copy_to_user((void *)arg,&flag,1);
#ifndef DBUFF
      chkbuff = 1;
#endif
      chkblk = 0;
    }else{
      flag = 0;
      x = copy_to_user((void *)arg, &flag, 1);
    }
    stop();

    break;
  case BABIL_EI:
#ifndef DBUFF
#ifdef VMEINT
    vme_define_intlevel(INTLEVEL);
    vme_enable_interrupt();
#else
    crate_define_lam(LAMN);
    rfs_enable_interrupt();
#endif
    clear();
#ifdef K2915
    rfs_enable_interrupt();
#endif
#endif
    break;
  case BABIL_LEN:
#ifndef DBUFF
    len = mp * WORDSIZE;
#else
    if(drn == 0){
      len = mpa * WORDSIZE;
    }else{
      len = mpb * WORDSIZE;
    }
#endif
    x = copy_to_user((void *)arg, (char *)&len, 4);
    break;
  case BABIL_EVTN:
    len = (int)get_evtn();
    x = copy_to_user((void *)arg, (char *)&len, 4);
    break;
  }

  BCLIUNLOCK
  return 1;
}

static unsigned int babildrv_poll(struct file *file,poll_table *wait){
  poll_wait(file, &babil_wait_queue,wait);
  if(chkbuff > 0){
    return POLLIN;
  }else{
    return 0;
  }
}

#if LINUX_VERSION_CODE >= 0x020600
static irqreturn_t babildrv_interrupt(int irq, void* dev_id, struct pt_regs* regs){
#else
static void babildrv_interrupt(int irq, void* dev_id, struct pt_regs* regs){
#endif
  unsigned long flags;
  // When driver is not opened by babilcom,
  // return IRQ_HANDLED
  BCLILOCK
  if(!startflag){
#ifdef VMEINT
    vme_disable_interrupt();
#else
    rfs_disable_interrupt();
#endif
    BCLIUNLOCK
    wake_up_interruptible(&babil_wait_queue);
#if LINUX_VERSION_CODE >= 0x020600
    return IRQ_HANDLED;
#else
    return;
#endif
  }

  if(dev_id == NULL){
    BCLIUNLOCK
#if LINUX_VERSION_CODE >= 0x020600
    return IRQ_NONE;
#else
    return;
#endif
  }

  if(chkblk == 0){
    init_block();
    chkblk = 1;
  }
  if(
#ifdef VMEINT
     vme_check_interrupt() != 0
#else
     check_lam() != 0
#endif
     ){
#ifdef VMEINT
    vme_disable_interrupt();
#else
    rfs_disable_interrupt();
#endif
    evt();
    if(end_event() > MAXBUFF){
      sca();
      end_block();
#ifdef VMEINT
      /* none */
#else
      //crate_seti(0);
#endif
      chkblk = 0;
#ifndef DBUFF
      chkbuff = 1;
#else
      chkbuff++;
      chmem();
      if(chkbuff != 2){
#ifdef VMEINT
	vme_enable_interrupt();
#else
	rfs_enable_interrupt();
#endif
	clear();
#ifdef K2915
	rfs_enable_interrupt();
#endif
      }
#endif
    }else{
#ifdef VMEINT
      vme_enable_interrupt();
#else
      rfs_enable_interrupt();
#endif
      clear();
#ifdef K2915
      rfs_enable_interrupt();
#endif
    }
  }else{
    BCLIUNLOCK
#if LINUX_VERSION_CODE >= 0x020600
    return IRQ_NONE;
#else
    return;
#endif
  }

  BCLIUNLOCK
  wake_up_interruptible(&babil_wait_queue);

#if LINUX_VERSION_CODE >= 0x020600
  return IRQ_HANDLED;
#endif
}

#if LINUX_VERSION_CODE >= 0x020600
module_init(babildrv_init_module);
module_exit(babildrv_cleanup_module);
#endif
