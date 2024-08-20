/* babinfodb.h
 * last modified : 11/10/09 16:54:07 
 * Hidetada Baba (RIKEN)
 *
 * Header : Functions for connection to DB
 *
 */


int babinfodb_connect(struct stdbcon);
int babinfodb_chkconnect(struct stdbcon);
int babinfodb_runstart(struct stdbcon, struct stdaqinfo*, struct struninfo*);
int babinfodb_runstop(struct stdbcon, struct stdaqinfo*, struct struninfo*);
int babinfodb_runstop_after(struct stdbcon dbcon, struct stdaqinfo *daqinfo, struct struninfo *runinfo);
void sql_insert_run(struct stdaqinfo*, struct struninfo*, char*);
void sql_update_run_stop(struct stdaqinfo*, struct struninfo*, char*);
void sql_update_run_stop_after(struct stdaqinfo*, struct struninfo*, char*);
void int2date(int , char *);
int babinfodb_chkexpid(struct stdbcon dbcon, int id);
int babinfodb_setexpid(struct stdbcon dbcon, int id);
int babinfodb_getexpid(void);
int babinfodb_getscalerinfoid(struct stdbcon dbcon, int expid, int scrid);
int babinfodb_getchannelid(struct stdbcon dbcon, int scrinfoid, int channel);
int babinfodb_getrunid(struct stdbcon dbcon, int expid, char *name, int number);
void sql_insert_scalerdata(int runid, int chid, unsigned int val, unsigned int ovf, char *sql);
void sql_insert_scalerchannel(int scrinfoid, int ch, char *name, char *sql);
void sql_update_scalerchannel(int chid, char *name, char *sql);
void sql_insert_scalerinfo(int expid, int scrid, char *name,
			   int type, int rate, int ratech, char *sql);
void sql_update_scalerinfo(int scrinfoid, char *name,
			   int type, int rate, int ratech, char *sql);
int babinfodb_setscalerdata(struct stdbcon dbcon, int runid,
			    int chid, unsigned int val, int ovf);
int babinfodb_setscalerchannel(struct stdbcon dbcon, 
			       int scrinfoid, int ch, char *name);
int babinfodb_setscalerinfo(struct stdbcon dbcon, int expid, int scrid,
			    char *name, int type, int rate, int ratech);
int babinfodb_setexplive(struct stdbcon dbcon, int gid, int gch, int uid, int uch);
int babinfodb_getscalerinfo(struct stdbcon dbcon, int scrinfoid,
			    char *name, int *rate, int *ratech);
int babinfodb_getexplive(struct stdbcon dbcon, int *gid, int *gch,
			 int *uid, int *uch);
int babinfodb_getscalerchannel(struct stdbcon dbcon, int scrinfoid,
			       int ch, char *name);
int babinfodb_setdaqname(struct stdbcon dbcon, int expid, 
			 char *name, char *server);
int babinfodb_getdaqname(char *name);
int babinfodb_getexpname(struct stdbcon dbcon, char *name);

#define DBCONNECTIONERROR -10
#define NOEXPID           -11

