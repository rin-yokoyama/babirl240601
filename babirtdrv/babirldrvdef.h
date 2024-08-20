/* Common definition file */

#define DMAMAX 4096
#define DMADELAYMAX 10000 // > 10ms

#define WORDSIZE  2
#define SIZEOFINT 4

/* for RIDF */
/* Macros */
#define RIDF_MKHD1(a,b,c) ((a&0x3)<<28|(b&0x3f)<<22|(c&0xffffff))
#define RIBF_MKHD2(d)      (d & 0xffffffff)
#define RIDF_LY(x) ((x & 0x30000000) >> 28) // Layer
#define RIDF_CI(x) ((x & 0x0fc00000) >> 22) // Class ID
#define RIDF_SZ(x)  (x & 0x003fffff)        // Block Size
#define RIDF_EF(x)  (x & 0xffffffff)        // Event Fragment Number

/* Bits */
#define RIDF_LY0               0
#define RIDF_LY1               1
#define RIDF_LY2               2
#define RIDF_LY3               3

/* Class ID */
#define RIDF_EF_BLOCK          0
#define RIDF_EA_BLOCK          1
#define RIDF_EAEF_BLOCK        2
#define RIDF_EVENT             3
#define RIDF_SEGMENT           4
#define RIDF_COMMENT           5
#define RIDF_SCALER            11
#define RIDF_CSCALER           12
#define RIDF_STATUS            21

#define RIDF_HD_SZ             4
#define RIDF_HDEVT_SZ          6
#define RIDF_HDSEG_SZ          6
#define RIDF_HDSCR_SZ          8
