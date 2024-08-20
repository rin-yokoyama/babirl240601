/*
  originaly from Ron Fox NSCL
*/

// register layout of HINP XLM
/* define bit masks for output from motherboard register */
#define acqall       0x10000

#define glbl_enable  0x40000
#define serin        0x80000
#define serclk       0x100000
#define tokenin      0x200000            // active low, inverted in FPGA
#define forcereset   0x400000
#define dacstb       0x800000
#define dacsign      0x1000000
#define selextbus    0x2000000
#define ld_dacs      0x4000000
#define vetoreset    0x8000000
#define force_track  0x10000000
#define XLMout       0x80000000

/************* Inputs  ***************/

#define ser_busy      0x80000000  // for XLM XXV or dual-port XLM 80M
// #define ser_busy      0x2000  // for old XLM 80M
#define token_out   0x4000
#define serout      0x8000
#define acqack      0x10000
#define orout       0x20000

/********* register addresses in XLM FPGA *********/
// these are word addresses
#define FPGA_reset       0x00       // write to this reg resets XLM
#define FPGA_acq_a       0x01      // write starts acq cycle on A bus
#define FPGA_acq_b       0x02       // write starts acq cycle on B bus
#define FPGA_set_delay   0x03       // set delay timings
#define FPGA_set_timeout 0x04       // set timeout counters
#define FPGA_ABus        0x05       // read or write to A bus bits
#define FPGA_Bbus        0x06       // read or write to B bus bits
#define FPGA_enblA       0x07       // External Enable for Bus A
#define FPGA_enblB       0x08       // External Enable for Bus B
#define FPGA_clear_veto  0x09       // Strobes glbl_enbl veto clear
#define FPGA_trig_delay  0x0a       // Sets the trigger delay
#define FPGA_coin_window 0x0b       // Sets width of coincidence window
#define FPGA_force_A     0x0c       // Force Readout for Bank A
#define FPGA_force_B     0x0d        // Force Read for Bank B
#define FT_DELAY         0x0e        // Force Track Delay Register
#define AA_DELAY         0x0f        // ACQ_ALL DELAY REGISTER
#define GD_DELAY         0x10        // GLOBAL DISABLE DELAY REGISTER
#define FAST_SERA        0x11        // fast serial A (16 bits at a time)
#define FAST_SERB        0x12        // fast serial B (16 bits at a time)


