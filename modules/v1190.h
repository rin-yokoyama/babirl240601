/* Header for V1190 Multihit TDC */


/* Register */
#define V1190_OUTBUFF         0x0000
#define V1190_SOFT_CLEAR      0x1016
#define V1190_ALMOST_FULL     0x1022

/* Bit */
#define V1190_SOFT_CLEAR_BIT  1     

#define V1190_TYPE_MASK_S          0xf800
#define V1190_GLOBAL_HEADER_BIT_S  0x4000
#define V1190_TDC_HEADER_BIT_S     0x0800
#define V1190_TDC_DATA_BIT_S       0x0000
#define V1190_TDC_TRAILER_BIT_S    0x1800
#define V1190_TDC_ERROR_BIT_S      0x2000
#define V1190_GLOBAL_TRAILER_BIT_S 0x8000
