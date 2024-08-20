#define set_amsr v2718_set_amsr
#define vread16  v2718_vread16
#define vread32  v2718_vread32
#define vwrite16 v2718_vwrite16
#define vwrite32 v2718_vwrite32
#ifdef VMEINT
#define get_irq  v2718_get_irq
#endif
#define vme_define_intlevel   v2718_define_intlevel
#define vme_enable_interrupt  v2718_enable_interrupt
#define vme_disable_interrupt v2718_disable_interrupt
#define vme_check_interrupt   v2718_check_interrupt
#define vme_read_intvector    v2718_read_intvector

#define dma_vread32 v2718_dma_vread32
#define vme_dma_vread32_start v2718_dma_vread32_start
#define vme_dma_vread32_store v2718_dma_vread32_store

extern void v2718_set_input_conf(int ch);
extern void v2718_set_output_conf(int ch);
extern void v2718_set_output_reg(int mask);
extern void v2718_clear_output_reg(int mask); 
