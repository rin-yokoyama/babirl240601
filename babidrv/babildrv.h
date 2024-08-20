/* Macros */
#ifdef CC7700
#define CNAFGEN(c,n,a,f) (0x00073fff & (c<<16 | (n-1)<<9 | a <<5 | f))
#define NAFGEN(n,a,f) ( 0x3fff & ((n-1)<<9 | a <<5 | f) )
#endif

#ifdef K2915
#define CNAFGEN(c,n,a,f) (0x00073fff & (c<<16 | n<<9 | a <<5 | f))
#define NAFGEN(n,a,f) ( 0x3fff & (n<<9 | a <<5 | f) )
#endif

#ifdef CCNET
#define CNAFGEN(c,n,a,f) ((((n<<8) | a) << 8) | f)
#define NAFGEN(n,a,f)    ((((n<<8) | a) << 8) | f)
#endif

#define QSTOP            0x00000002  /* Q-Stop Block Transfer Mode */
#define QIGNORE          0x00000004  /* Q-Ignore Block Transfer Mode */
#define QREPEAT          0x00000006  /* Q-Repeat Block Transfer Mode */
#define QSCAN            0x00000008  /* Q-Scan Block Transfer Mode */
