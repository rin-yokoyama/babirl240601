#define init_register          ccnet_init_register          
#define check_done             ccnet_check_done             
#define check_emp_fifo         ccnet_check_emp_fifo         
#define check_full_fifo        ccnet_check_full_fifo        
#define check_lam              ccnet_check_lam              
#define read_lam               ccnet_read_lam               
#define control                ccnet_control                
#define read16                 ccnet_read16                 
#define read24                 ccnet_read24                 
#define write16                ccnet_write16                
#define write24                ccnet_write24                
#define block_read16           ccnet_block_read16           
#define block_read24           ccnet_block_read24           
#define dma_block_read16       ccnet_dma_block_read16       
#define dma_block_read24       ccnet_dma_block_read24       
#define crate_reset            ccnet_crate_reset            
#define rfs_enable_interrupt   ccnet_rfs_enable_interrupt   
#define rfs_disable_interrupt  ccnet_rfs_disable_interrupt  
#define pci_enable_interrupt   ccnet_pci_enable_interrupt   
#define pci_clear_interrupt    ccnet_pci_clear_interrupt    
#define crate_enable_lam       ccnet_crate_enable_lam       
#define crate_disable_lam      ccnet_crate_disable_lam      
#define crate_define_lam       ccnet_crate_define_lam
#define crate_z                ccnet_crate_z                
#define crate_c                ccnet_crate_c                
#define crate_seti             ccnet_crate_seti             
#define crate_deli             ccnet_crate_deli             
#define get_csrdata            ccnet_get_csrdata            
#define get_bmcsdata           ccnet_get_bmcsdata
#define get_irq                ccnet_get_irq
#define chkq                   ccnet_chkq

extern int ccnet_exec(int *cmdbuf, int *rplybuf);
extern int ccnet_exec_dma(int *cmdbuf, int *rplybuf);
extern int ccnet_exec_pio(int *cmdbuf, int *rplybuf);
