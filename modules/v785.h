/*
   V785, 32 channel Peak Sensing Converter
*/

/* Address Map */
#define V785_OUTBUFF        0x0000   /* - 0x07ff (D32) */
#define V785_FREV           0x1000
#define V785_GEO_ADDR       0x1002
#define V785_MCST_ADDR      0x1004
#define V785_BIT_SET1       0x1006
#define V785_BIT_CLE1       0x1008
#define V785_INT_REG1       0x100a
#define V785_INT_VEC1       0x100c
#define V785_STS_REG1       0x100e
#define V785_CTRL_REG1      0x1010
#define V785_ADER_H         0x1012
#define V785_ADER_L         0x1014
#define V785_SS_RST         0x1016
#define V785_MCST_CTRL      0x101a
#define V785_EVT_TRIG_REG   0x1020
#define V785_STS_REG2       0x1022
#define V785_EVT_CNT_L      0x1024
#define V785_EVT_CNT_H      0x1026
#define V785_INC_EVT        0x1028
#define V785_INC_OFF        0x102a
#define V785_LOAD_TEST_REG  0x102c
#define V785_FCLR_WINDOW    0x102e
#define V785_BIT_SET2       0x1032
#define V785_BIT_CLE2       0x1034
#define V785_WMEM_TEST      0x1036
#define V785_MEM_TEST_H     0x1038
#define V785_MEM_TEST_L     0x103a
#define V785_CRATE_SEL      0x103c
#define V785_TEST_EVT       0x103e
#define V785_EVT_CNT_RST    0x1040
#define V785_RTEST          0x1064
#define V785_SW_COMM        0x1068
#define V785_SLIDE_CONST    0x106a
#define V785_AAD            0x1070
#define V785_BAD            0x1072
#define V785_THRESH         0x1080 /* - 0x10BF */

/* Bits */
#define V785_TYPE_MASK      0x06000000
#define V785_HEADER_BIT     0x02000000
#define V785_DATA_BIT       0x00000000
#define V785_EOB_BIT        0x04000000
#define V785_ERROR_BIT      0x06000000
#define V785_TYPE_MASK_S    0x0600
#define V785_HEADER_BIT_S   0x0200
#define V785_DATA_BIT_S     0x0000
#define V785_EOB_BIT_S      0x0400
#define V785_ERROR_BIT_S    0x0600
