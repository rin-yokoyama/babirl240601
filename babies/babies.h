/* babirl : babies/babies.h
 * last modified : 09/11/11 15:30:10 
 * 
 * Hidetada Baba (RIKEN)
 * baba@rarfaxp.riken.jp
 * Header for Event fragment sender
 *
 */

/* prototype */
void quit(void);
int reset(void);
int sethd1(boolean t);
int sethd1dir(char *name);
int sethd2(boolean t);
int sethd2dir(char *name);
int setmt(boolean t);
int setmtdir(char *name);
int seterhost(char *name);
int setefid(int id);
int setexpname(char *name);
int setrunnumber(int n);
int efr_connect(void);
void rtthread(void);

/* common variavle */
int comfd, fifofd, mfd;
struct stefrc efrc;
//char data[EB_EFBLOCK_BUFFSIZE];
char *data;
int sock=0, sn;
int fifocom, shmid;
int efsock=0;
char *shmptr;
int mode, fstart;
int drvfd;
unsigned int evtn;
char drivername[128];
int is26, isexec = 0;
int ckill = 0;

char thisname[64] = "babies\0";

/* static values */
volatile int sflag=STAT_RUN_IDLE;
volatile int rtend=STAT_RUN_IDLE;
char cle = 0;
pthread_t rtthread_t;


/* File */
#define BABIESFILE "babies.txt"

/* MODE */
#define BABIES_RTLINUX 1
#define BABIES_LINUX   2
#define BABIES_DUMMY   3
#define BABIES_CCNET   4
#define BABIES_SHM     4
#define BABIES_SCALER  5

volatile char *babiesrun;
char babies_mode[6][10];
int vm = 1; // default = log on
FILE *lfd = NULL;
