/* nbbqvio.h */

#define NBBQVIO_DRV     "/dev/nbbqvio"

#define A32                  0x09
#define A24                  0x39
#define A16                  0x29
#define A32BLK               0x0b
#define A24BLK               0x3b

#define NBBQVIO_AMSR    0
#define NBBQVIO_READ16  1
//#define NBBQVIO_READ32  2
#define NBBQVIO_READ32B 9
#define NBBQVIO_WRITE16 3
#define NBBQVIO_WRITE32 4
#define NBBQVIO_DMAR16  5
#define NBBQVIO_DMAR32  6
#define NBBQVIO_DMAW16  7
#define NBBQVIO_DMAW32  8

extern int init_nbbqvio(void);
extern int release_nbbqvio(void);
extern unsigned short vme_read16(unsigned int addr);
extern unsigned int vme_read32(unsigned int addr);
extern int vme_read(unsigned int addr, unsigned short *val);
extern int vme_write(unsigned int addr,unsigned short *val);
extern int vme_write16(unsigned int addr, unsigned short val);
extern int vme_write32(unsigned int addr, unsigned int val);
extern int vme_amsr(unsigned int val);
extern int vwrite16(unsigned int addr, short *val);
extern int vwrite32(unsigned int addr, int *val);
extern int vread16(unsigned int addr, short *val);
extern int vread32(unsigned int addr, int *val);
