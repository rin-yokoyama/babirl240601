/*
   SIS3700 VME ECL FIFO
*/

/* Address Map */
#define SIS3700_DATAFIFO       0x0000 /* read,write */
#define SIS3700_COUNTERFIFO    0x0004 /* read */
#define SIS3700_STATUS_REG     0x0008 /* read */
#define SIS3700_CTRL_REG       0x0008 /* write */
#define SIS3700_TEST_FCN_REG   0x000c /* write */_

/* Bits */
/* for Data FIFO */
#define SIS3700_FIRST_DATA     0xffff0000
#define SIS3700_SECOND_DATA    0x0000ffff
#define SIS3700_NOT_PACKED     0x0000ffff

/* for Event/Word counter FIFO(0x4) */
#define SIS3700_OVF            0x00008000
#define SIS3700_TIMEOUT        0x00004000
#define SIS3700_WORD_COUNT     0x00001fff
#define SIS3700_EVT_COUNT      0x000000ff

/* for Status register(0x8, read) */
#define SIS3700_DFIFO_EMPTY    0x00000080
#define SIS3700_FIFO_FULL      0x00000040
#define SIS3700_CFIFO_EMPTY    0x00000020
#define SIS3700_BYSY           0x00000010
#define SIS3700_INPUT_VME      0x00000008 /* bit is set 0 input = ECL */
#define SIS3700_OUTPUT_VME     0x00000004 /* bit is set 0 output = local */
#define SIS3700_PACK_MODE      0x00000002 
#define SIS3700_EN_TIMEOUT     0x00000001 /* bit is set 1 enable timeout */

/* for Control register(0x8, write) */
#define SIS3700_CLEAR           0x00000100
#define SIS3700_ENABLE_IN_ECL   0x00000080
#define SIS3700_DISABLE_OUT_VME 0x00000040
#define SIS3700_DISABLE_PACK    0x00000020
#define SIS3700_ENABLE_TIMEOUT  0x00000010
#define SIS3700_DISABLE_IN_ECL  0x00000008
#define SIS3700_ENABLE_OUT_VME  0x00000004
#define SIS3700_ENABLE_PACK     0x00000002
#define SIS3700_DIABLE_TIMEOUT  0x00000001
