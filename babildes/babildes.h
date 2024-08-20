/* babirl: babirl/babirl.h
 * last modified : 11/09/22 18:14:29 
 * Hidetada Baba (RIKEN)
 * 
 * Header for babild
 *
 */

/* Constants */
const int estout = 3;


/* Globals */
int vm    = 0; // Flag verbose mode
int comfd = 0; // Communication port
int ebdfd = 0; // Event build port
int ebffd = 0; // FIFO fd
int esfd  = 0; // babildes fd
int semid;     // Semaphore for fifo
int efnum = 0; // Event flag number
unsigned int blkn;
int babildes = 0;
int argefn = 0;
int efsock;
int fstart = 0;
int fchkerhost = 0;
unsigned long long int totsize;
char commentbuff[EB_BUFF_SIZE];

char thisname[64] = "babildes\0";
char runstartstat[1024];
char runstopstat[1024];
char myhost[512],mydom[512],myip[512];

// for time stamp reset counter
long long int tsrst = 0;

struct stefrc efrc;
struct stefrc tefrc;
char esret[EB_MESS_SIZE];

int inhd[2];

fd_set fdset;
struct sembuf semb;
union semun semunion;

struct stdaqinfo daqinfo;
struct struninfo runinfo;
struct stssminfo ssminfo;
struct ststatcom statcom;

/* Pthread */
pthread_t ebthre, slthre, tcpslthre;
pthread_t efrthre[MAXEF];
static pthread_mutex_t ebfmutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t memmutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t nmemmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xmlmutex = PTHREAD_MUTEX_INITIALIZER;
struct sched_param ebpar, slpar, efpar, tcpslpar;

int ebffdt[MAXEF];
int efrun[MAXEF];
int slfd, tcpslfd;

static int gebn, gsock;

static int aliasnamen = 0;
char aliasnames[10][128];

struct stebbuf{
  char *data;
  int   pt;
}ebbuf;

/* Prototype */
int ebcom();
int efrmain();
int mkcomment();

/* strings */
char *ofstr[3] = {"off\0", "on\0", "scr\0"};


#define BABILD_COMMENT_START 1
#define BABILD_COMMENT_STOP  2

