/*
   V775, 32 channel TDC
*/

/* Address Map */
#define V775_OUTBUFF        0x0000   /* - 0x07ff (D32) */
#define V775_FREV           0x1000
#define V775_GEO_ADDR       0x1002
#define V775_MCST_ADDR      0x1004
#define V775_BIT_SET1       0x1006
#define V775_BIT_CLE1       0x1008
#define V775_INT_REG1       0x100a
#define V775_INT_VEC1       0x100c
#define V775_STS_REG1       0x100e
#define V775_CTRL_REG1      0x1010
#define V775_ADER_H         0x1012
#define V775_ADER_L         0x1014
#define V775_SS_RST         0x1016
#define V775_MCST_CTRL      0x101a
#define V775_EVT_TRIG_REG   0x1020
#define V775_STS_REG2       0x1022
#define V775_EVT_CNT_L      0x1024
#define V775_EVT_CNT_H      0x1026
#define V775_INC_EVT        0x1028
#define V775_INC_OFF        0x102a
#define V775_LOAD_TEST_REG  0x102c
#define V775_FCLR_WINDOW    0x102e
#define V775_BIT_SET2       0x1032
#define V775_BIT_CLE2       0x1034
#define V775_WMEM_TEST      0x1036
#define V775_MEM_TEST_H     0x1038
#define V775_MEM_TEST_L     0x103a
#define V775_CRATE_SEL      0x103c
#define V775_TEST_EVT       0x103e
#define V775_EVT_CNT_RST    0x1040
#define V775_RTEST          0x1064
#define V775_SW_COMM        0x1068
#define V775_SLIDE_CONST    0x106a
#define V775_AAD            0x1070
#define V775_BAD            0x1072
#define V775_THRESH         0x1080 /* - 0x10BF */

/* Bits */
#define V775_TYPE_MASK      0x06000000
#define V775_HEADER_BIT     0x02000000
#define V775_DATA_BIT       0x00000000
#define V775_EOB_BIT        0x04000000
#define V775_ERROR_BIT      0x06000000
#define V775_TYPE_MASK_S    0x0600
#define V775_HEADER_BIT_S   0x0200
#define V775_DATA_BIT_S     0x0000
#define V775_EOB_BIT_S      0x0400
#define V775_ERROR_BIT_S    0x0600
