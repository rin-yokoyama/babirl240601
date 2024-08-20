#define set_amsr vmemm_set_amsr
#define vread16  vmemm_vread16
#define vread32  vmemm_vread32
#define vwrite16 vmemm_vwrite16
#define vwrite32 vmemm_vwrite32
#ifdef VMEINT
#define get_irq  vmemm_get_irq
#endif
#define vme_define_intlevel   vmemm_define_intlevel
#define vme_enable_interrupt  vmemm_enable_interrupt
#define vme_disable_interrupt vmemm_disable_interrupt
#define vme_check_interrupt   vmemm_check_interrupt
#define vme_read_intvector    vmemm_read_intvector
