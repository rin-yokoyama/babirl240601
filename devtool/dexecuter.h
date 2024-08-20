/* babirl : devtool/dexecuter.h
 * last modified : 09/08/19 13:32:20 
 * 
 * Hidetada Baba (RIKEN)
 * baba@rarfaxp.riken.jp
 * Header for dexecuter
 *
 */

/* prototype */
void quit(void);

/* static values */
volatile int sflag=STAT_RUN_IDLE;
volatile int rtend=STAT_RUN_IDLE;
char cle = 0;
pthread_t rtthread_t;

/* MODE */
#define BABIES_RTLINUX 1
#define BABIES_LINUX   2
#define BABIES_DUMMY   3
#define BABIES_CCNET   4
