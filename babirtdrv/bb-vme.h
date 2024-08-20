extern void set_amsr(unsigned char am);
extern void vread16(unsigned long addr,short *data);
extern void vread32(unsigned long addr,long *data);
extern void vwrite16(unsigned long addr,short *data);
extern void vwrite32(unsigned long addr,long *data);
#ifdef VMEINT
extern int get_irq(void);
#endif
extern void vme_enable_interrupt(void);
extern void vme_disable_interrupt(void);
extern void vme_define_intlevel(int level);
extern int vme_check_interrupt(void);
extern int vme_read_intvector(void);

extern int vme_dma_vread32_start(unsigned long addr,int size);
extern int vme_dma_vread32_store(char *data, int size);
