/* babinfodb.c
 * last modified : 16/12/28 17:03:37 
 * Hidetada Baba (RIKEN)
 *
 * Functions for connection to DB
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <bi-config.h>
#include <bi-common.h>

#include <bbpsql.h>
#include "babinfodb.h"

static int expid = -1;
char daqname[64] = {0};
char *runstatstr[3] =  {"IDLE\0", "START\0", "NSSTA\0"};

extern int updatebabinforcxml();

int babinfodb_connect(struct stdbcon dbcon){
  if(!bbpsql_connect(dbcon.host, dbcon.dbname, dbcon.user, dbcon.passwd)){
    return 0;
  }

  return 1;
}


int babinfodb_chkconnect(struct stdbcon dbcon){
  if(!babinfodb_connect(dbcon)){
    return 0;
  }
  bbpsql_end();

  return 1;
}


int babinfodb_runstart(struct stdbcon dbcon, struct stdaqinfo *daqinfo,
		       struct struninfo *runinfo){
  char sql[1024]={0};

  if(!babinfodb_connect(dbcon)){
    return 0;
  }

  sql_insert_run(daqinfo, runinfo, sql);

  DB(printf("Run Start SQL = %s\n", sql));
  bbpsql_sql_noresult(sql);
  bbpsql_end();

  
  return 1;
}

int babinfodb_runstop(struct stdbcon dbcon, struct stdaqinfo *daqinfo,
		      struct struninfo *runinfo){


  char sql[1024]={0};

  if(!babinfodb_connect(dbcon)){
    return 0;
  }

  sql_update_run_stop(daqinfo, runinfo, sql);

  DB(printf("Run Stop SQL = %s\n", sql));
  bbpsql_sql_noresult(sql);
  bbpsql_end();

  return 1;
}

int babinfodb_runstop_after(struct stdbcon dbcon, struct stdaqinfo *daqinfo,
		      struct struninfo *runinfo){


  char sql[1024]={0};

  if(!babinfodb_connect(dbcon)){
    return 0;
  }

  sql_update_run_stop_after(daqinfo, runinfo, sql);

  DB(printf("Run Stop SQL = %s\n", sql));
  bbpsql_sql_noresult(sql);
  bbpsql_end();

  return 1;
}


void sql_insert_run(struct stdaqinfo *daqinfo,
		    struct struninfo *runinfo, char *sql){
  int idx = 0;
  char tdate[256]={0};
  char sheader[1024]={0};

  strncpy(sheader, runinfo->header, sizeof(sheader));
  bbpsql_safetext(sheader, strlen(sheader));

  int2date(runinfo->starttime, tdate);

  idx = sprintf(sql, "INSERT INTO \"RunInfo\"(\"ExpID\", \"Name\", ");
  idx += sprintf(sql+idx, "\"Number\", \"Status\", \"StartDate\", ");
  idx += sprintf(sql+idx, "\"Header\") VALUES(");
  idx += sprintf(sql+idx, "'%d', ", babinfodb_getexpid());
  idx += sprintf(sql+idx, "'%s', ", daqinfo->runname);
  idx += sprintf(sql+idx, "'%d', ", daqinfo->runnumber);
  idx += sprintf(sql+idx, "'%s', ", runstatstr[runinfo->runstat]);
  idx += sprintf(sql+idx, "'%s', ", tdate);
  idx += sprintf(sql+idx, "'%s')", sheader);
}

void sql_update_run_stop(struct stdaqinfo *daqinfo,
		    struct struninfo *runinfo, char *sql){
  int idx = 0;
  char tdate[256]={0};
  char sender[1024]={0};

  strncpy(sender, runinfo->ender, sizeof(sender));
  bbpsql_safetext(sender, strlen(sender));

  int2date(runinfo->stoptime, tdate);

  idx = sprintf(sql, "UPDATE \"RunInfo\" SET ");
  idx += sprintf(sql+idx, "\"Status\" = 'STOP', ");
  idx += sprintf(sql+idx, "\"StopDate\" = '%s', ", tdate);
  idx += sprintf(sql+idx, "\"Ender\" = '%s' "   , sender);
  idx += sprintf(sql+idx, "WHERE \"Number\" = '%d' ", daqinfo->runnumber);
  idx += sprintf(sql+idx, "AND \"Name\" = '%s' ", daqinfo->runname);
  idx += sprintf(sql+idx, "AND \"ExpID\" = '%d' ", babinfodb_getexpid());
  idx += sprintf(sql+idx, "AND \"Status\" != 'STOP'");
}

void sql_update_run_stop_after(struct stdaqinfo *daqinfo,
		    struct struninfo *runinfo, char *sql){
  int idx = 0;
  char tdate[256]={0};
  char sender[1024]={0};

  strncpy(sender, runinfo->ender, sizeof(sender));
  bbpsql_safetext(sender, strlen(sender));

  int2date(runinfo->stoptime, tdate);

  idx = sprintf(sql, "UPDATE \"RunInfo\" SET ");
  idx += sprintf(sql+idx, "\"Status\" = 'STOP', ");
  idx += sprintf(sql+idx, "\"StopDate\" = '%s', ", tdate);
  idx += sprintf(sql+idx, "\"Ender\" = '%s' "   , sender);
  idx += sprintf(sql+idx, "WHERE \"Number\" = '%d' ", daqinfo->runnumber);
  idx += sprintf(sql+idx, "AND \"Name\" = '%s' ", daqinfo->runname);
  idx += sprintf(sql+idx, "AND \"ExpID\" = '%d' ", babinfodb_getexpid());
  idx += sprintf(sql+idx, "AND \"Status\" = 'STOP'");
}

void int2date(int t, char *tdate){
  struct tm *date;
  time_t tt;

  tt = t;

  date = localtime(&tt);
  sprintf(tdate, "%04d-%02d-%02d %02d:%02d:%02d",
	  date->tm_year+1900, date->tm_mon+1, date->tm_mday,
	  date->tm_hour, date->tm_min, date->tm_sec);
}

int babinfodb_chkexpid(struct stdbcon dbcon, int id){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;

  memset(text, 0, sizeof(text));

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  sprintf(sql, "SELECT \"ExpID\" FROM \"ExpInfo\" WHERE \"ExpID\" = '%d'",
	  id);
  DB(printf("Chk Exp id = %s\n", sql));
  bbpsql_sql(sql, &res);
  if(!bbpsql_get_oneresult(res, text)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  bbpsql_clear_result(res);
  bbpsql_end();

  return strtol(text, NULL, 0);
}

int babinfodb_setdaqname(struct stdbcon dbcon, int expid, 
			 char *name, char *server){
  char sql[1024]={0}, txt[1024]={0};
  int idx;
  PGresult *res = NULL;

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  DB(printf("setdaqname expid=%d / name=%s / server=%s\n", expid, name, server));

  sprintf(sql, "SELECT \"Name\" from \"DAQInfo\" WHERE \"Name\" = '%s'",
	  name);
  bbpsql_sql(sql, &res);
  if(!bbpsql_get_oneresult(res, txt)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return 0;
  }
  bbpsql_clear_result(res);
  bbpsql_end();

  idx = sprintf(sql, "UPDATE \"DAQInfo\" SET \"ExpID\" = %d", expid);
  if(strlen(server)){
    idx += sprintf(sql+idx, " ,\"Server\" = '%s'", server);
  }
  idx += sprintf(sql+idx, " WHERE \"Name\" = '%s'", name);
  strncpy(daqname, name, sizeof(daqname)-1);

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }
  bbpsql_sql_noresult(sql);
  bbpsql_end();

  updatebabinforcxml();

  return 1;
}

int babinfodb_getdaqname(char *name){
  if(strlen(daqname)){
    strcpy(name, daqname);
    return 1;
  }else{
    return 0;
  }
}

/** Set ExpID for babinfo
 *  @param dbcon Structure of DB Connection
 *  @param id ExpID
 *  @return ExpID, negative value is faild to set ExpID
 */
int babinfodb_setexpid(struct stdbcon dbcon, int id){
  int tid;
  tid = babinfodb_chkexpid(dbcon, id);

  if(tid > 0){
    expid = tid;
    updatebabinforcxml();
  }

  return tid;
}

/** Get selected ExpID in babinfo
 *  return ExpID in babinfo
 */
int babinfodb_getexpid(void){
  return expid;
}

int babinfodb_getexpname(struct stdbcon dbcon, char *name){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;
  int idx;

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  idx = sprintf(sql, "SELECT \"Name\" FROM \"ExpInfo\"");
  idx += sprintf(sql+idx, " WHERE \"ExpID\" = '%d'", expid);
  bbpsql_sql(sql, &res);
  if(!bbpsql_get_oneresult(res, text)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  bbpsql_clear_result(res);
  bbpsql_end();
  strcpy(name, text);

  return 1;
}


/** Get ScalerInfoName by ScalerInfoID */
int babinfodb_getscalerinfo(struct stdbcon dbcon, int scrinfoid,
			    char *name, int *rate, int *ratech){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;
  int idx;

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  idx = sprintf(sql, "SELECT \"Name\", \"Rate\", \"RateCh\" FROM \"ScalerInfo\"");
  idx += sprintf(sql+idx, " WHERE \"ScalerInfoID\" = %d", scrinfoid);
  DB(printf("Get ScalerInfo = %s\n", sql));
  bbpsql_sql(sql, &res);
  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 0)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  sprintf(name, "%s", text);

  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 1)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  *rate = strtol(text, NULL, 0);

  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 2)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  *ratech = strtol(text, NULL, 0);

  bbpsql_clear_result(res);
  bbpsql_end();

  return 1;
}

/** Get ScalerChannelName by ScalerInfoID, channel */
int babinfodb_getscalerchannel(struct stdbcon dbcon, int scrinfoid,
			       int ch, char *name){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;
  int idx;

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  idx = sprintf(sql, "SELECT \"Name\" FROM \"ScalerChannel\"");
  idx += sprintf(sql+idx, " WHERE \"ScalerInfoID\" = %d", scrinfoid);
  idx += sprintf(sql+idx, " AND \"Channel\" = %d", ch);
  DB(printf("Get ScalerChannel = %s\n", sql));
  bbpsql_sql(sql, &res);
  memset(text, 0, sizeof(text));
  if(!bbpsql_get_oneresult(res, text)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  //sprintf(name, "%s", text);
  strcpy(name, text);
  bbpsql_clear_result(res);
  bbpsql_end();

  return 1;
}


/** Get ScalerInfoID by ScalerID and ExpID.
 *  @param dbcon Structure of DB connection
 *  @param scrid ScalerID
 *  @param expid ExpID
 *  @return ScalerInfoID, -1 = no ScalerInfo
 */
int babinfodb_getscalerinfoid(struct stdbcon dbcon, int expid, int scrid){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;
  int idx;
  memset(text, 0, sizeof(text));

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  idx = sprintf(sql, "SELECT \"ScalerInfoID\" FROM \"ScalerInfo\"");
  idx += sprintf(sql+idx, " WHERE \"ExpID\" = '%d' AND \"ScalerID\" = %d",
		 expid, scrid);
  DB(printf("Get Scaler InfoId = %s\n", sql));
  bbpsql_sql(sql, &res);
  if(!bbpsql_get_oneresult(res, text)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  bbpsql_clear_result(res);
  bbpsql_end();

  return strtol(text, NULL, 0);
}

/** Get ChannelID by ScalerInfoID and Channel Number.
 *  @param dbcon Structure of DB connection
 *  @param scrinfoid ScalerInfoID
 *  @param channel Channel Number
 *  @return ChannelID, -1 = no ScalerInfo
 */
int babinfodb_getchannelid(struct stdbcon dbcon, int scrinfoid, int channel){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;
  int idx;
  memset(text, 0, sizeof(text));

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  idx = sprintf(sql, "SELECT \"ChannelID\" FROM \"ScalerChannel\"");
  idx += sprintf(sql+idx, " WHERE \"ScalerInfoID\" = '%d' AND \"Channel\" = %d",
		 scrinfoid, channel);
  DB(printf("Get ChannelID = %s\n", sql));
  bbpsql_sql(sql, &res);
  if(!bbpsql_get_oneresult(res, text)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  bbpsql_clear_result(res);
  bbpsql_end();

  return strtol(text, NULL, 0);
}


/** Get RunID by ExpID and Run Name and Run Number.
 *  @param dbcon Structure of DB connection
 *  @param expid ExpID
 *  @param name Run Name
 *  @param number Run Number
 *  @return RunID, -1 = no RunID
 */
int babinfodb_getrunid(struct stdbcon dbcon, int expid,
		       char *name, int number){
  char sql[1024]={0}, text[256]={0};
  PGresult *res = NULL;
  int idx;
  memset(text, 0, sizeof(text));

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  idx = sprintf(sql, "SELECT \"RunID\" FROM \"RunInfo\"");
  idx += sprintf(sql+idx,
       " WHERE \"ExpID\" = '%d' AND \"Name\" = '%s' AND \"Number\" = '%d'",
		 expid, name, number);
  DB(printf("Get RunID = %s\n", sql));
  bbpsql_sql(sql, &res);
  if(!bbpsql_get_oneresult(res, text)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  bbpsql_clear_result(res);
  bbpsql_end();

  return strtol(text, NULL, 0);
}


/** SQL text to insert ScalerData.
 *  @param runid RunID
 *  @param chid  ChannelID
 *  @param val   Value of scale
 *  @param ovf   Count of overflow
 *  @param sql   Char buffer for SQL text
 */
void sql_insert_scalerdata(int runid, int chid, 
			   unsigned int val, unsigned int ovf, char *sql){
  int idx;

  idx = sprintf(sql, "INSERT INTO \"ScalerData\"(\"RunID\", \"ChannelID\", ");
  idx += sprintf(sql+idx, "\"Data\", \"Overflow\") ");
  idx += sprintf(sql+idx, " VALUES(");
  idx += sprintf(sql+idx, "'%d', ", runid);
  idx += sprintf(sql+idx, "'%d', ", chid);
  idx += sprintf(sql+idx, "'%d', ", val);
  idx += sprintf(sql+idx, "'%d') ", ovf);
}

/** SQL text to insert ScalerChannel.
 *  @param scrinfoid ScalerInfoID
 *  @param ch    Channel number
 *  @param name  Name of this channel
 */
void sql_insert_scalerchannel(int scrinfoid, int ch, char *name, char *sql){
  int idx;
  char sname[1024]={0};
  sprintf(sname, "%s", name);
  bbpsql_safetext(sname, strlen(sname));

  idx = sprintf(sql, "INSERT INTO \"ScalerChannel\"(\"ScalerInfoID\", ");
  idx += sprintf(sql+idx, "\"Name\", \"Channel\") ");
  idx += sprintf(sql+idx, " VALUES(");
  idx += sprintf(sql+idx, "'%d', ", scrinfoid);
  idx += sprintf(sql+idx, "'%s', ", sname);
  idx += sprintf(sql+idx, "'%d') ", ch);
}

/** SQL text to update ScalerChannel.
 *  @param chid  ScalerChannelID
 *  @param name  Name of this channel
 */
void sql_update_scalerchannel(int chid, char *name, char *sql){
  int idx;
  char sname[1024]={0};
  sprintf(sname, "%s", name);
  bbpsql_safetext(sname, strlen(sname));

  idx = sprintf(sql, "UPDATE \"ScalerChannel\" SET ");
  idx += sprintf(sql+idx, "\"Name\" = '%s' ", sname);
  idx += sprintf(sql+idx, "WHERE \"ChannelID\" = '%d'", chid);
}


/** SQL text to insert ScalerInfo.
 *  @param expid  ExpID
 *  @param scrid  ScalerID
 *  @param name   Name of this scaler
 *  @param type   Type of this scaler (NCSCALER24=11, NCSCALER32 = 13)
 *  @param sql    Char buffer for SQL text
 */
void sql_insert_scalerinfo(int expid, int scrid, char *name,
			   int type, int rate, int ratech, char *sql){
  int idx;
  char sname[1024]={0};
  sprintf(sname, "%s", name);
  bbpsql_safetext(sname, strlen(sname));

  idx = sprintf(sql, "INSERT INTO \"ScalerInfo\"(\"ExpID\", \"ScalerID\", ");
  idx += sprintf(sql+idx, "\"Name\", \"Type\", \"Rate\", \"RateCh\") ");
  idx += sprintf(sql+idx, " VALUES(");
  idx += sprintf(sql+idx, "'%d', ", expid);
  idx += sprintf(sql+idx, "'%d', ", scrid);
  idx += sprintf(sql+idx, "'%s', ", sname);
  idx += sprintf(sql+idx, "'%d', ", type);
  idx += sprintf(sql+idx, "'%d', ", rate);
  idx += sprintf(sql+idx, "'%d') ", ratech);
}

/** SQL text to update ScalerInfo.
 *  @param scrinfoid ScalerInfoID
 *  @param name   Name of this scaler
 *  @param type   Type of this scaler (NCSCALER24=11, NCSCALER32 = 13)
 *  @param sql    Char buffer for SQL text
 */
void sql_update_scalerinfo(int scrinfoid, char *name,
			   int type, int rate, int ratech, char *sql){
  int idx;
  char sname[1024] = {0};
  sprintf(sname, "%s", name);
  bbpsql_safetext(sname, strlen(sname));

  idx = sprintf(sql, "UPDATE \"ScalerInfo\" SET ");
  idx += sprintf(sql+idx, "\"Name\" = '%s', ", sname);
  idx += sprintf(sql+idx, "\"Type\" = '%d', ", type);
  idx += sprintf(sql+idx, "\"Rate\" = '%d', ", rate);
  idx += sprintf(sql+idx, "\"RateCh\" = '%d' ", ratech);
  idx += sprintf(sql+idx, " WHERE \"ScalerInfoID\" = '%d'", scrinfoid);
}


/** Insert scaler data.
 *  @param dbcon Structure of DB connection
 *  @param runid Run ID
 *  @param chid  Channel ID
 *  @param val   Scaler data to be registered
 *  @param ovf   Overflow count to be registered
 */
int babinfodb_setscalerdata(struct stdbcon dbcon, int runid,
			     int chid, unsigned int val, int ovf){
  char sql[1024]={0};

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  sql_insert_scalerdata(runid, chid, val, ovf, sql);
  bbpsql_sql_noresult(sql);
  bbpsql_end();

  return 1;
}

/** Insert new ScalerChannel.
 *  @param scrinfoid ScalerInfoID
 *  @param ch    Channel number
 *  @param name  Name of this channel
 */
int babinfodb_setscalerchannel(struct stdbcon dbcon, 
				 int scrinfoid, int ch, char *name){

  char sql[1024]={0};
  int tid;

  tid = babinfodb_getchannelid(dbcon, scrinfoid, ch);

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  if(tid > 0){
    sql_update_scalerchannel(tid, name, sql);
    bbpsql_sql_noresult(sql);
    bbpsql_end();
    return tid;
  }

  sql_insert_scalerchannel(scrinfoid, ch, name, sql);
  bbpsql_sql_noresult(sql);
  bbpsql_end();
  
  return babinfodb_getchannelid(dbcon, scrinfoid, ch);
}

/** Insert new Scaler Info
 *  @param dbcon Structure for DB connection
 *  @param expid ExpID
 *  @param scrid Scaler ID
 *  @param name  Name of this channel
 *  @param type  Type of scaler
 *  @param rate  Rate(Hz) of reference clock channel
 *  @param ratech Channel number of reference clock channel
 *  @return ScalerInfoID
 */
int babinfodb_setscalerinfo(struct stdbcon dbcon, int expid, int scrid,
			    char *name, int type, int rate, int ratech){
  int tid;
  char sql[1024]={0};


  tid = babinfodb_getscalerinfoid(dbcon, expid, scrid);

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }

  if(tid > 0){
    sql_update_scalerinfo(tid, name, type, rate, ratech, sql);
    bbpsql_sql_noresult(sql);
    bbpsql_end();
    return tid;
  }

  sql_insert_scalerinfo(expid, scrid, name, type, rate, ratech, sql);
  DB(printf("insert scaler info = %s\n", sql));
  bbpsql_sql_noresult(sql);
  bbpsql_end();
  
  return babinfodb_getscalerinfoid(dbcon, expid, scrid);
}


/** Update scaler channel for live time caluclation in ExpInfo
 *  @param dbcon Structure for DB connection
 *  @param gid   GatedID
 *  @param gch   GatedChannel
 *  @param uid   UnGatedID
 *  @param uch   UnGatedChannnel
 *  @return 1 1=OK, others=NG
 */
int babinfodb_setexplive(struct stdbcon dbcon, int gid, int gch, int uid, int uch){
  char sql[1024]={0};
  int idx;

  if(expid <= 0){
    return NOEXPID;
  }

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }
  
  idx = sprintf(sql, "UPDATE \"ExpInfo\" SET ");
  idx += sprintf(sql+idx, "\"GatedID\" = '%d', ", gid);
  idx += sprintf(sql+idx, "\"GatedCh\" = '%d', ", gch);
  idx += sprintf(sql+idx, "\"UngatedID\" = '%d', ", uid);
  idx += sprintf(sql+idx, "\"UngatedCh\" = '%d' ", uch);
  idx += sprintf(sql+idx, "WHERE \"ExpID\" = '%d' ", expid);

  bbpsql_sql_noresult(sql);
  bbpsql_end();

  return 1;
}

int babinfodb_getexplive(struct stdbcon dbcon, int *gid, int *gch,
			 int *uid, int *uch){
  char sql[1024]={0}, text[1024]={0};
  int idx;
  PGresult *res = NULL;

  if(expid <= 0){
    return NOEXPID;
  }

  if(!babinfodb_connect(dbcon)){
    return DBCONNECTIONERROR;
  }
  
  idx = sprintf(sql, "SELECT \"GatedID\", \"GatedCh\", \"UngatedID\"");
  idx += sprintf(sql+idx, " \"UngatedCh\" FROM \"ExpInfo\"");
  idx += sprintf(sql+idx, " WHERE \"ExpID\" = %d", expid);
  DB(printf("Get explive = %s\n", sql));
  bbpsql_sql(sql, &res);
  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 0)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  *gid = strtol(text, NULL, 0);

  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 1)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  *gch = strtol(text, NULL, 0);

  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 2)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  *uid = strtol(text, NULL, 0);

  memset(text, 0, sizeof(text));
  if(!bbpsql_get_idxresult(res, text, 3)){
    bbpsql_clear_result(res);
    bbpsql_end();
    return -1;
  }
  *uch = strtol(text, NULL, 0);
  
  bbpsql_clear_result(res);
  bbpsql_end();


  return 1;
}

