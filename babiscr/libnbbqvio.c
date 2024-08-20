/* cmdvme
   use /dev/nbbqvio
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include "nbbqvio.h"

/* #define DEBUG */

static int nbbqvio_fd;
unsigned int nbbqvio_val[2];
unsigned short sval;

int init_nbbqvio(void){
  int try = 0;
  
  while((nbbqvio_fd = open(NBBQVIO_DRV,O_RDWR)) < 0){
    usleep(100000);
    try ++;
    if(try > 100000){
      printf("Can't open %s\n",NBBQVIO_DRV);
      return -1;
    }
  }

  return 0;
}

int release_nbbqvio(void){
  if(nbbqvio_fd < 0){
    return -1;
  }else{
    close(nbbqvio_fd);
  }

  return 0;
}

unsigned short vme_read16(unsigned int addr){
  nbbqvio_val[0] = addr;
  ioctl(nbbqvio_fd,NBBQVIO_READ16,nbbqvio_val);
  sval = nbbqvio_val[1] & 0x0000ffff;
  return sval;
}

unsigned int vme_read32(unsigned int addr){
  nbbqvio_val[0] = addr;
  ioctl(nbbqvio_fd,NBBQVIO_READ32B,nbbqvio_val);

  return nbbqvio_val[1];
}

int vme_read(unsigned int addr, unsigned short *val){
  *val = vme_read16(addr);
  return 1;
}

int vme_write16(unsigned int addr, unsigned short val){
  nbbqvio_val[0] = addr;
  nbbqvio_val[1] = val & 0x0000ffff;

  return ioctl(nbbqvio_fd,NBBQVIO_WRITE16,nbbqvio_val);
}

int vme_write32(unsigned int addr, unsigned int val){
  nbbqvio_val[0] = addr;
  nbbqvio_val[1] = val;
  return ioctl(nbbqvio_fd,NBBQVIO_WRITE32,nbbqvio_val);
}

int vme_write(unsigned int addr, unsigned short *val){
  return vme_write16(addr,*val);
}

int vme_amsr(unsigned int val){
  return ioctl(nbbqvio_fd,NBBQVIO_AMSR,&val);
}


int vread32(unsigned int addr, int *val){
  *val = vme_read32(addr);

  return *val;
}

int vwrite32(unsigned int addr, int *val){
  vme_write32(addr, (unsigned int)*val);

  return 1;
}


int vread16(unsigned int addr, short *val){
  *val = vme_read16(addr);

  return *val;
}

int vwrite16(unsigned int addr, short *val){
  vme_write16(addr, (unsigned short)*val);

  return 1;
}

