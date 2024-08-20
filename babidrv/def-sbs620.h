#define set_amsr sbs620_set_amsr
#define vread16  sbs620_vread16
#define vread32  sbs620_vread32
#define vwrite16 sbs620_vwrite16
#define vwrite32 sbs620_vwrite32
#ifdef VMEINT
#define get_irq  sbs620_get_irq
#endif
#define vme_define_intlevel   sbs620_define_intlevel
#define vme_enable_interrupt  sbs620_enable_interrupt
#define vme_disable_interrupt sbs620_disable_interrupt
#define vme_check_interrupt   sbs620_check_interrupt
#define vme_read_intvector    sbs620_read_intvector

#define set_amsr_i sbs620_set_amsr_i
#define vread16_i  sbs620_vread16_i
#define vread32_i  sbs620_vread32_i
#define vwrite16_i sbs620_vwrite16_i
#define vwrite32_i sbs620_vwrite32_i

#define vme_dma_vread32_start sbs620_dma_vread32_start
#define vme_dma_vread32_store sbs620_dma_vread32_store
#define vme_dma_vread32_start_i sbs620_dma_vread32_start_i
#define vme_dma_vread32_store_i sbs620_dma_vread32_store_i
