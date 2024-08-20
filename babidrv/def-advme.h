#define set_amsr advme_set_amsr
#define vread16  advme_vread16
#define vread32  advme_vread32
#define vwrite16 advme_vwrite16
#define vwrite32 advme_vwrite32
#ifdef VMEINT
#define get_irq  advme_get_irq
#endif
#define vme_define_intlevel   advme_define_intlevel
#define vme_enable_interrupt  advme_enable_interrupt
#define vme_disable_interrupt advme_disable_interrupt
#define vme_check_interrupt   advme_check_interrupt
#define vme_read_intvector    advme_read_intvector
