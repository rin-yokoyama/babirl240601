/* babirl: babirl/babirl.h
 * last modified : 11/10/24 15:29:05 
 * Hidetada Baba (RIKEN)
 * 
 * Header for babild
 *
 */
/* Constants */
const int estout = 3;


/* Globals */
int vm    = 0; // Flag verbose mode
int tst   = 0; // Time stamp table 
int comfd = 0; // Communication port
int ebdfd = 0; // Event build port
int ebffd = 0; // FIFO fd
int semid;     // Semaphore for fifo
int efnum = 0; // Event flag number
int inffd = 0; // babinfo port
unsigned int blkn;
FILE *hdfd[MAXEF][MAXHD];   // FILE for event builted data
FILE *tsfd[MAXEF][MAXHD];   // FILE for timestamp table
FILE *mtfd[MAXMT];   // FILE for event builted data
FILE *lfd = NULL;     // Log file
int argefn = 0;
unsigned long long int totsize;
unsigned long long int tsfp[MAXEF];

char thisname[64] = "babiau\0";
char runstartstat[1024];
char runstopstat[1024];

char myhost[512],mydom[512],myip[512];
char esret[EB_MESS_SIZE];


// for time stamp reset counter
long long int tsrst = 0;

int fchkerhost = 0;

static int aliasnamen = 0;
char aliasnames[10][128];


struct stefrc efrc;
struct stefrc tefrc;

fd_set fdset;
struct sembuf semb;
union semun semunion;

struct stdaqinfo daqinfo;
struct struninfo runinfo;
struct stssminfo ssminfo;
struct ststatcom statcom;

/* Pthread */
pthread_t ebthre;
pthread_t efrthre[MAXEF];
static pthread_mutex_t ebfmutex  = PTHREAD_MUTEX_INITIALIZER;
struct sched_param ebpar, efpar;
char combuff[EB_BUFF_SIZE];

struct sockaddr_in cliaddr[MAXCLI];
int udpsock;
struct stclinfo clinfo[MAXCLI];

int ebffdt[MAXEF];
int efrun[MAXEF];
int auclinfo[MAXEF];

/* Prototype */
int ebcom();
int efrmain();
int mkcomment();
int store_auclinfo();
int esdisconnect(void);

/* strings */
char *ofstr[3] = {"off\0", "on\0", "scr\0"};


#define BABILD_COMMENT_START 1
#define BABILD_COMMENT_STOP  2

