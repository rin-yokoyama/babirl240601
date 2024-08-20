/* babirl: babirldrvdefrt.h
 * last modified : 08/03/17 22:39:41 
 *
 * Hidetada Baba (RIKEN)
 * baba@rarfaxp.riken.jp
 * 
 * Configuration file
 *
 */


/* RTLinux installed */
#define RTLINUXDRV
#define BABIESRL_SHM "babiesrt_shm"
#define RTDRIVERNAME "babiesrt"

/* Configuration parameers */
/* Event Fragment */
#define EBDEFSIZE  16384     ///< Default Event Block Size
#define MAXEF      256       ///< Maximum number of Event fragment
#define MAXEVTMEM  1<<30     ///< Maximum number of event memory

#define EB_BUFF_SIZE    81920      ///< Buffer size of babirl tcp commands
#define EB_EFBLOCK_SIZE 0x20000   ///< Usual max size of block data = 128kB
#define EB_EFBLOCK_BUFFSIZE EB_EFBLOCK_SIZE * 2  ///< Usual size of block data buffer
#define EB_EFBLOCK_MAXSIZE 0x400000 ///< Real max size of block data = 8MB

#define WORDSIZE      2
#define EF_SHM_SIZE   EB_EFBLOCK_BUFFSIZE * 2 + 12
#define EF_SHM_ADSIZE EB_EFBLOCK_BUFFSIZE + 2
#define EF_SHM_DATA1  0
#define EF_SHM_DATA2  EB_EFBLOCK_BUFFSIZE
#define EF_SHM_DATA3  EF_SHM_SIZE
#define EF_SHM_FLAG1  EB_EFBLOCK_BUFFSIZE * 2
#define EF_SHM_FLAG2  EB_EFBLOCK_BUFFSIZE * 2 + 2
#define EF_SHM_FLAG3  EF_SHM_SIZE + EF_SHM_ADSIZE - 2
#define EF_SHM_EVTN   EB_EFBLOCK_BUFFSIZE * 2 + 4
#define EF_SHM_SSF    EB_EFBLOCK_BUFFSIZE * 2 + 8
#define EF_SHM_FREE   0x00
#define EF_SHM_READY1 0x01
#define EF_SHM_READY2 0x02

/* Maximum values */
#define MAXHD      10
#define HDMAXSIZE  2000000000
#define MAXMT      1
#define MTMAXSIZE  4000000000
#define MAXCLI     16
#define MAXSCRANA  32      ///< Maximum number of sclaer ananlysis

/* nomally, not change following */
/* Keys */
#define EFSHMKEY    523400   ///< First number of Shared memory
#define EFSEMKEY    523700   ///< First number of Semaphore key
//#define FSEMKEY     523600   // Semaphore for FIFO

#define ANSHMKEY    561000   ///< Shared memory key for babian
#define ANSEMKEY    561001   ///< Semaphore key for babian shm

/* Ports */
/** 17511-17519 : for event builder */
#define EBCOMPORT   17511
/** 17550       : for eb data */
/** 17601       : for receiver */
#define ERRCVPORT   17601
/** 17651-17000 : for babies communication port */
#define ESCOMPORT   17651
#define ANRCVPORT   17502   ///< babian receive port
#define SLRCVPORT   17512   ///< Slow data receive port (UDP)
#define INFEAPORT   17515   ///< babinfo EA receive port
#define INFCOMPORT  17516   ///< babinfo communication port
#define SSMCOMPORT  17517   ///< babissm communication port

/* babinfo setting*/
#define MAXBABINFO  256     ///< Maximum number of thread for babinfo

/* Files */
#define EBFIFO      "/tmp/ebfifo"          ///< Path of EBFIFO
#define RCVPID      "/tmp/bi-rcvpid"       ///< 
#define BABIRLDIR   "/home/rips/babirl-work/"    ///< Install directory
#define EFLIST      "init/eflist.rc"       ///< Path of EF list file
#define HDLIST      "init/hdlist.rc"       ///< Path of HD list file
#define MTLIST      "init/mtlist.rc"       ///< Path of MT list file
#define BABIESRC    "init/babies.rc"       ///< Path of rc file for babies
#define BABILDRC    "init/babild.rc"       ///< Path of rc file for babild
#define CLIHOSTSRC  "init/clihosts.rc"     ///< Path of clihost list file
#define SSMINFO     "init/ssminfo.rc"      ///< Path of ssm information

#define PROMPT      "BABICON>"             ///< Prompt of babicon
