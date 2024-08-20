/* babirl: babinfo/babinfo.h
 * last modified : 13/11/01 13:45:41 
 * Hidetada Baba (RIKEN)
 * 
 * Header for babinfo
 *
 */

/* definitions */
#define BUFFN 2

/* Globals */
// Event assembly buffer
char *eabuff[BUFFN];
char *tbuff;
char *rawbuff;
char combuff[EB_BUFF_SIZE];
char sndbuff[EB_BUFF_SIZE];

char thisname[64] = "babinfo\0";

int comfd, earecvfd;
struct stdaqinfo daqinfo;
struct struninfo runinfo;
struct sockaddr_in cliaddr[MAXCLI];
int udpsock;
struct stclinfo clinfo[MAXCLI];
struct stdbcon dbcon;
struct sttcpclinfo tcpclinfo[MAXTCPCLI];

unsigned int earecvbuffnum = 0;

static int scranan = 0;
struct ridf_scr_anast scrana[MAXSCRANA];
struct ridf_scr_contst *scr[MAXSCRANA];
unsigned int lastscr[MAXSCRANA][128];
unsigned int ovscr[MAXSCRANA][128];
struct ridf_scr_livest scrlive;

/* thread */
pthread_t eathread;
pthread_t conthread[MAXBABINFO];
static int eaflag;
static pthread_mutex_t eamutex0 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t eamutex1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t nummutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t daqmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t runmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t scrmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xmlmutex = PTHREAD_MUTEX_INITIALIZER;

