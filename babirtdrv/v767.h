/*
  V767, 128/64 channel Timing to Digital converter
*/

#define V767_OUTBUFF       0x0000
#define V767_RESET         0x0018

#define V767_TYPE_MASK     0x00600000
#define V767_HEADER_BIT    0x00400000
#define V767_DATA_BIT      0x00000000
#define V767_EOB_BIT       0x00200000
#define V767_ERROR_BIT     0x00600000

#define V767_TYPE_MASK_S   0x0060
#define V767_HEADER_BIT_S  0x0040
#define V767_DATA_BIT_S    0x0000
#define V767_EOB_BIT_S     0x0020
#define V767_ERROR_BIT_S   0x0060
