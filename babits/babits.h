/* babirl : babits/babits.h
 * last modified : 09/12/11 23:06:46 
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Header file for babits
 *
 */

/* Constants */
#define BABITS_TABLE    0x00000001
#define BABITS_PRINT    0x00000002
#define BABITS_BUILD    0x00000004
#define BABITS_CHECK    0x00000008
#define BABITS_DIRNAME  0x00000010
#define BABITS_RUNNAME  0x00000020
#define BABITS_FILENAME 0x00000040
#define BABITS_SINGLE   0x00000100
#define BABITS_NONSTOP  0x00001000

/* Flags */
#define BABITS_EOF      0x0001
#define BABITS_READ     0x0002

#define MAXFD 256
#define MAXCN 32

#define SIZEOFTS        8

/* Globals */
int mode;
int maxfd;
int fdn;
int lowspeed = 0;
int overlap = 0;
FILE *bfd = NULL;
int efn = 256, update = 0, single = 0;
int maxbuff = 16384;
char runname[256], curfile[512], nextfile[512];
int runnumber;

short buff[EB_EFBLOCK_BUFFSIZE];
FILE *fd = NULL;
static int tn[MAXCN];
static int blkn = 0;
static int mp, coinn, coin, anti;
static unsigned int tsevtn = 0;
static int trdn=0, brdn=0;


/* Prototype */
int chkopt(int, char **);


struct fdinfost *fdfirst;

struct cnlistst{
  int id;
  int fn;
  int off;
  int wid;
  FILE *fd;
  char name[256];
  unsigned long long int ts;
  unsigned long long int fp;
}cn[MAXCN];

struct infdst{
  int fn;
  FILE *fd;
};

struct bnlisttst{
  int id;
  int fn;
  FILE *fd;
  char name[256];
  unsigned long long int ts;
  unsigned long long int fp;
}bn;

struct infdst binfd;
struct infdst infd[MAXCN];
