extern void set_amsr(unsigned char am);
extern void vread16(unsigned int addr,short *data);
extern void vread32(unsigned int addr,int *data);
extern void vwrite16(unsigned int addr,short *data);
extern void vwrite32(unsigned int addr,int *data);
extern int get_irq(void);
extern void vme_enable_interrupt(void);
extern void vme_disable_interrupt(void);
extern void vme_define_intlevel(int level);
extern int vme_check_interrupt(void);
extern int vme_read_intvector(void);


#ifdef V2718
extern int dma_vread32(unsigned int addr, char *data, int size);
#endif
extern int vme_dma_vread32_start(unsigned int addr,int size);
extern int vme_dma_vread32_store(char *data, int size);

#ifdef ISERIES
extern void set_amsr_i(unsigned char am, int dn);
extern void vread16_i(unsigned int addr,short *data, int dn);
extern void vread32_i(unsigned int addr,int *data, int dn);
extern void vwrite16_i(unsigned int addr,short *data, int dn);
extern void vwrite32_i(unsigned int addr,int *data, int dn);
extern int vme_dma_vread32_start_i(unsigned int addr,int size, int dn);
extern int vme_dma_vread32_store_i(char *data, int size, int dn);
#endif
