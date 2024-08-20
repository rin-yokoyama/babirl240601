/* readout for HINP */

const unsigned int SRAMA = 0x000000;
const unsigned int SRAMB = 0x200000;
const unsigned int REQ_A = 0x00000001;
const unsigned int REQ_B = 0x00000002;
const unsigned int REQ_X = 0x00010000;
const unsigned int FPGABase = 0x400000; //  Base address of FPGA `register' set.
const unsigned int DSP = 0x600000; //  Base address of DSP interface.
const unsigned int InterfaceBase= 0x800000;
const unsigned int BusRequest = 0x800000; // Register for submitting bus requests.
const unsigned int Interrupt  =0x800004; // Interrupt/reset register.
const unsigned int FPGABootSrc=0x800008; // Select boot source for FPGA.
const unsigned int ForceOffBus=0x80000c; // Register to force FPGA/DSP off bus.
const unsigned int BUSAOwner  =0x810000; // Shows who has bus A (SRAM A)
const unsigned int BUSBOwner  =0x810004; // Shows who has bus B (SRAM B)
const unsigned int BUSXOwner  =0x810008; // Shows who has bus X (FPGA).
const unsigned int IRQSerial  =0x820048; // Write for IRQ id reads serial number.
const unsigned int POLLISR    =0x820824; // 'mailbox' betweenFPGA and DSP.

const unsigned int InterruptResetFPGA    =0x00000001;
const unsigned int InterruptResetDSP     =0x00000002;
const unsigned int InterruptInterruptFPGA=0x00010000;
const unsigned int InterruptInterruptDSP =0x00020000;

const unsigned int BootSrcRom0 =0x00000000;	// Load from sector 0 of PROM.
const unsigned int BootSrcRom1 =0x00000001;	// Load from sector 1 of PROM.
const unsigned int BootSrcRom2 =0x00000002; // Load from sector 2 of PROM.
const unsigned int BootSrcRom3 =0x00000003; // Load from sector 3 of PROM.
const unsigned int BootSrcSRAMA=0x00010000;	// Load from SRAM A image.

const unsigned int ForceOffBusForce=0x00000001; // Force all but VME off bus.
const unsigned int BusOwnerNone=0;	// Bus not owned.
const unsigned int BusOwnerVME =1;	// VME host owns bus.
const unsigned int BusOwnerFPGA=2;	// Bus owned by FPGA.
const unsigned int BusOwnerDSP =3;	// Bus owned by DSP.


#ifdef UNIV

void accessBus(unsigned int accessPattern, int n){
  univ_map_write32(BusRequest, &accessPattern, n);
}

void initialize(int n){
  unsigned int fpga, srama;
  unsigned short test1;
  unsigned int test2, lval;

  accessBus(REQ_X, n);
  univ_map_read32(BUSXOwner, &test2, n);
  fpga = FPGABase;           // establish base address of module's FPGA
  univ_map_read32(0x820048, &test2, n);
  lval = 0;
  univ_map_write32(fpga+FPGA_ABus*4, &lval, n); // turn off glbl_enbl
  lval = forcereset;
  univ_map_write32(fpga+FPGA_ABus*4, &lval, n); // reset the chips
  lval = 1;
  univ_map_write32(fpga+FPGA_enblA*4, &lval, n); // turn on ext enbl
  lval = glbl_enable;
  univ_map_write32(fpga+FPGA_ABus*4, &lval, n); // turn on glbl_enbl
  accessBus(0, n); // release bus

}

// mapsize = 0xa00000
// srama = DMA mapn of srama SRAMA = 0x000000;
// sramb = DMA mapn of sramb SRAMB = 0x200000;
int hinp_segdata(int sraman, int srambn, int n){
  //unsigned int busDelayTicks = static_cast<unsigned int>(busDelay);
  unsigned int fpga = FPGABase, lval, cnt;

  accessBus(REQ_A | REQ_B | REQ_X, n);
  delay_us(); delay_us();  // delay 2su
  lval = 0;
  univ_map_write32(fpga+FPGA_ABus*4, &lval); // turn off glbl_enbl
  // read data A
  univ_map_read32(SRAMA, &cnt, n);
  cnt = cnt & 0xfff;
  univ_dma_read((char *)(data + mp), cnt*4, sraman);
  segmentsize += cnt*2;
  mp += cnt*2;
  // read data B
  univ_map_read32(SRAMA, &cnt, n); // SRAMB ???
  cnt = cnt & 0xfff;
  univ_dma_read((char *)(data + mp), cnt*4, srambn);
  segmentsize += cnt*2;
  mp += cnt*2;

  lval = forcereset;
  univ_map_write32(fpga+FPGA_ABus*4, &lval, n); // reset the chips
  lval = 1;
  univ_map_write32(fpga+FPGA_enblA*4, &lval, n); // turn on ext enbl
  lval = glbl_enable;
  univ_map_write32(fpga+FPGA_ABus*4, &lval, n); // turn on glbl_enbl
  accessBus(0, n);
  
  return segmentsize;
}

#endif
