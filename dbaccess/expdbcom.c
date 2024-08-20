/* dbaccess/expdbcom.c
 * last modified : 17/11/14 10:10:17 
 * H. Baba (RIKEN)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include <bbpsql.h>
#include "expdbcomfunc.h"

// for list
#define DEFUSER "exp\0"
#define DEFPASS "exp\0"
//#define DEFHOST "ribfdb\0"
#define DEFHOST "b01\0"
#define DEFDB   "EXPDB\0"

// for update and insert
#define DEFUUSER "bigrips\0"

#define SZOPT 64


int getrunid(char *expid, char *expname, char *runname, char *runnumber);
int getscalerinfoid(char *expid, char *expname, char *scalerid, char *scalername);
int getscalertype(char *expid, char *expname, char *scalerid, char *scalername);

struct option long_options[] = {
  {"help"         , no_argument      , NULL, 'h'},
  {"List"         , no_argument      , NULL, 'L'},
  {"Update"       , no_argument      , NULL, 'U'},
  {"Insert"       , no_argument      , NULL, 'I'},
  {"Exp"          , no_argument      , NULL, 'E'},
  {"Run"          , no_argument      , NULL, 'R'},
  {"Scaler"       , no_argument      , NULL, 'S'},
  {"Channel"      , no_argument      , NULL, 'C'},
  {"Data"         , no_argument      , NULL, 'D'},
  {"expid"        , required_argument, NULL, 'i'},
  {"expname"      , required_argument, NULL, 'e'},
  {"runnumber"    , required_argument, NULL, 'n'},
  {"runname"      , required_argument, NULL, 'r'},
  {"scalerid"     , required_argument, NULL, 'd'},
  {"scalername"   , required_argument, NULL, 'm'},
  {"channelnumber", required_argument, NULL, 'g'},
  {"channelname"  , required_argument, NULL, 'f'},
  {"gatedid"      , required_argument, NULL, 'j'},
  {"gatedch"      , required_argument, NULL, 'k'},
  {"ungatedid"    , required_argument, NULL, 'w'},
  {"ungatedch"    , required_argument, NULL, 'v'},
  {"startdate"    , required_argument, NULL, 'q'},
  {"stopdate"     , required_argument, NULL, 'y'},
  {"rate"         , required_argument, NULL, '0'},
  {"ratech"       , required_argument, NULL, '1'},
  {"comment"      , required_argument, NULL, 'o'},
  {"status"       , required_argument, NULL, 't'},
  {"after"        , required_argument, NULL, 'a'},
  {"before"       , required_argument, NULL, 'b'},
  {"latest"       , no_argument      , NULL, 'l'},
  {"showcol"      , no_argument      , NULL, 'c'},
  {"showxml"      , no_argument      , NULL, 'x'},
  {"csv"          , no_argument      , NULL, 'z'},
  {"overflow"     , no_argument      , NULL, '3'},
  {"desc"         , no_argument      , NULL, '2'},
  {"user"         , required_argument, NULL, 'u'},
  {"password"     , required_argument, NULL, 'p'},
  {0, 0, 0, 0}
};

void quit(){
  bbpsql_end();
  exit(0);
}

void help(){
  printf("expdbcom OPTIONS...\n");
  printf("=== Action\n");
  printf("--help         / -h : this help\n");
  printf("--List         / -L : show the list of Exp/Run/Scaler/Channel list\n");
  printf("--Update       / -U : update the list of Exp/Channel list (password is required)\n");
  printf("--Insert       / -I : insert new list of Exp/Channel (password is required)\n");
  printf("=== Mode\n");
  printf("--Exp          / -E : for explist\n");
  printf("--Run          / -R : for runlist (must specify ExpID or experiment Name\n");
  printf("--Scaler       / -S : for scaler list (must specify ExpID or experiment Name)\n");
  printf("--Channel      / -C : for scaler channel (must specify ExpID/Name and ScalerID/Name\n");
  printf("--Data         / -D : for scaler data (must specify ExpID/Name and ScalerID/Name and RunName and RunNumber\n");
  printf("\n");
  printf("*** Options (display)\n");
  printf("--latest       / -l : show the latest value (having latest ID)\n");
  printf("--showcol      / -c : show the title of columns\n");
  printf("--showxml      / -x : show the XML comments for RunInfo\n");
  printf("--desc         / -2 : show in descending order\n");
  printf("--csv          / -z : show as CSV\n");
  printf("--overflow     / -3 : calculate overflow\n");
  printf("\n");
  printf("*** Options (restriction or update/insert value)\n");
  printf("--expid        / -i : specify ExpID\n");
  printf("--expname      / -e : specify/enter experiment Name\n");
  printf("--runnumber    / -n : specify the run number\n");
  printf("--runname      / -r : specify the run name\n");
  printf("--scalerid     / -d : specify the scaler ID\n");
  printf("--scalername   / -m : specify/enter the scaler name\n");
  printf("--channelnumber/ -g : specify the channel number\n");
  printf("--channelname  / -f : enter the channel name\n");
  printf("--gatedid      / -j : enter the GatedId\n");
  printf("--gatedch      / -k : enter the GatedCh\n");
  printf("--ungatedid    / -w : enter the UngatedId\n");
  printf("--ungatedch    / -v : enter the UngatedCh\n");
  printf("--startdate    / -q : enter the StartDate\n");
  printf("--stopdate     / -y : enter the StopDate\n");
  printf("--rate         / -0 : enter the Rate\n");
  printf("--ratech       / -1 : enter the RateCh\n");
  printf("--comment      / -o : enter the comment\n");
  printf("--status       / -t : specify the status (RUNNING or STOP)\n");
  printf("--after        / -a : specify the StartDate after (e.g. \"2013-01-01 10:00:00\")\n");
  printf("--before       / -b : specify the StartDate before (e.g. \"2013-01-01 10:00:00\")\n");
  printf("\n");
  printf("*** Options (authorization)\n");
  printf("--user         / -u : specify the user name of DB\n");
  printf("--password     / -p : specify the password of DB\n");
}

void optcpy(char *dest, char *srv){
  strncpy(dest, srv, SZOPT-1);
}

int main(int argc, char *argv[]){
  const unsigned long long int ov24 = 16777216ULL;
  const unsigned long long int ov32 = 4294967296ULL;
  PGresult *res = NULL;
  char sql[1024*300]={0};
  int opt, i, option_index, mode = 0, latest = 0, idx, act = 0;
  char opts[256]={0}, user[64]={0}, host[64]={0}, dbname[64]={0}, pass[64]={0};
  char after[SZOPT]={0}, before[SZOPT]={0}, expname[SZOPT]={0};
  char expid[SZOPT]={0}, runnumber[SZOPT]={0}, runname[SZOPT]={0};
  char status[SZOPT]={0}, scalername[SZOPT]={0}, scalerid[SZOPT]={0};
  char username[SZOPT]={0}, password[SZOPT]={0};
  char comment[SZOPT]={0}, channelname[SZOPT]={0}, channel[SZOPT]={0};
  char gatedid[SZOPT]={0}, gatedch[SZOPT]={0}, ungatedid[SZOPT]={0};
  char ungatedch[SZOPT]={0}, startdate[SZOPT]={0}, stopdate[SZOPT]={0};
  char rate[SZOPT]={0}, ratech[SZOPT]={0};
  int showcol = 0, showxml = 0, desc = 0, csv = 0, ovf = 0;
  int runid = 0, scalerinfoid = 0, type = 13;
  unsigned long long int ovval = ov32;
  int cm[64]={0};


  sprintf(user, "%s", DEFUSER);
  sprintf(pass, "%s", DEFPASS);
  sprintf(host, "%s", DEFHOST);
  sprintf(dbname, "%s", DEFDB);

  i = 0;
  idx = 0;
  while(long_options[i].name){
    opts[idx] = (char )long_options[i].val;
    idx++;
    if(long_options[i].has_arg == required_argument){
      opts[idx] = ':';
      idx++;
    }
    i++;
  }

  idx = 0; // idx for CSV to be showed

  while((opt = getopt_long(argc, argv, opts, long_options, &option_index))
	!= -1){
    switch(opt){
    case 'h':
      help();
      quit();
      break;
    case 'L':
      //{"List"      , no_argument      , NULL, 'L'},
      act = opt;
      break;
    case 'U':
      //{"Update"    , no_argument      , NULL, 'U'},
      act = opt;
      break;
    case 'I':
      //{"Insert"    , no_argument      , NULL, 'I'},
      act = opt;
      break;
    case 'E':
      mode = opt;
      break;
    case 'R':
      mode = opt;
      break;
    case 'S':
      //{"Scaler", no_argument      , NULL, 'S'},
      mode = opt;
      break;
    case 'C':
      //{"Channel"   , no_argument      , NULL, 'C'},
      mode = opt;
      idx = 1;  // idx1 = Channel Name
      break;
    case 'D':
      //{"Data"         , no_argument      , NULL, 'D'},
      mode = opt;
      break;
    case 'i':
      //{"expid"  , required_argument, NULL, 'i'},
      optcpy(expid, optarg);
      break;
    case 'e':
      //{"expname", required_argument, NULL, 'n'},
      optcpy(expname, optarg);
      break;
    case 'n':
      //{"runnumber", required_argument, NULL, 'n'},
      optcpy(runnumber, optarg);
      break;
    case 'r':
      //{"runname"  , required_argument, NULL, 'r'},
      optcpy(runname, optarg);
      break;
    case 'd':
      //{"scalerid"  , required_argument, NULL, 'd'},
      optcpy(scalerid, optarg);
      break;
    case 'm':
      //{"scalername", required_argument, NULL, 'm'},
      optcpy(scalername, optarg);
      break;
    case 'g':
      //{"channelnumber", required_argument, NULL, 'b'},
      optcpy(channel, optarg);
      break;
    case 'f':
      //{"channelname"  , required_argument, NULL, 'f'},
      optcpy(channelname, optarg);
      break;
    case 'j':
      //{"gatedid"      , required_argument, NULL, 'j'},
      optcpy(gatedid, optarg);
      break;
    case 'k':
      //{"gatedch"      , required_argument, NULL, 'k'},
      optcpy(gatedch, optarg);
      break;
    case 'w':
      //{"ungatedid"    , required_argument, NULL, 'u'},
      optcpy(ungatedid, optarg);
      break;
    case 'v':
      //{"ungatedch"    , required_argument, NULL, 'v'},
      optcpy(ungatedch, optarg);
      break;
    case 't':
      //{"status"   , required_argument, NULL, 't'},
      optcpy(status, optarg);
      break;
    case 'o':
      //{"comment"   , required_argument, NULL, 'o'},
      optcpy(comment, optarg);
      break;
    case 'q':
      //{"startdate"    , required_argument, NULL, 'q'},
      optcpy(startdate, optarg);
      break;
    case 'y':
      //{"stopdate"     , required_argument, NULL, 'y'},
      optcpy(stopdate, optarg);
      break;
    case '0':
      //printf("--rate         / -0 : enter the Rate\n");
      optcpy(rate, optarg);
      break;
    case '1':
      //printf("--ratech       / -1 : enter the RateCh\n");
      optcpy(ratech, optarg);
      break;
    case 'a':
      //{"after"  , required_argument, NULL, 'a'},
      optcpy(after, optarg);
      break;
    case 'b':
      //{"before" , required_argument, NULL, 'b'},
      optcpy(before, optarg);
      break;
    case 'l':
      //{"latest" , no_argument      , NULL, 'l'},
      latest = 1;
      break;
    case 'c':
      //{"showcol", no_argument      , NULL, 's'},
      showcol = 1;
      break;
    case 'x':
      //{"showxml", no_argument      , NULL, 'x'},
      showxml = 1;
      break;
    case 'z':
      //{"csv"          , no_argument      , NULL, 'z'},
      csv = 1;
      break;
    case '3':
      //printf("--overflow     / -3 : calculate overflow\n");
      ovf = 1;
      break;
    case '2':
      //{"desc", no_argument      , NULL, '2'},
      desc = 1;
      break;
    case 'u':
      //{"user"      , required_argument, NULL, 'u'},
      optcpy(username, optarg);
      break;
    case 'p':
      //{"password"  , required_argument, NULL, 'p'},
      optcpy(password, optarg);
      break;
    default:
      printf("Invalid option %c\n", opt);
      help();
      return 1;
      break;
    }
  }

  if(act == 'U' || act == 'I'){
    sprintf(user, "%s", DEFUUSER);
    if(strlen(password) == 0){
      printf("for Update or Insert, --password/-p is required\n");
      exit(0);
    }
  }
      
  if(strlen(username)){
    sprintf(user, "%s", username);
  }
  if(strlen(password)){
    sprintf(pass, "%s", password);
  }

  if(!bbpsql_connect(host, dbname, user, pass)){
    printf("Cannot connect DB host = %s, db=%s, user=%s\n",
	   host, dbname, user);
    exit(0);
  }

  if(act == 0){
    printf("No action, must have --List/-L or --Update/U or --Insert/I\n");
    quit();
  }else if(act == 'I'){
    if(mode == 0){
      printf("No mode option, option of --Exp/E or --Scaler/S is required\n");
      quit();
    }
  }else if(act == 'U'){
    if(mode == 0){
      printf("No mode option, option of --Exp/E or --Scaler/-S or --Channel/-C is required\n");
      quit();
    }
  }else if(act == 'L'){
    if(mode == 0){
      printf("No mode option, option of --Exp/E or --Run/R or --Scaler/-S or --Channel/-C or --Data/D is required\n");
      quit();
    }
  }

  if(act == 'I'){
    switch(mode){
    case 'E':
      // Explist
      sqlexpinsert(sql, expname, startdate, stopdate,
		   comment, gatedid, gatedch, ungatedid, ungatedch);
      act = 'L';
      memset(expid, 0, sizeof(expid));
      memset(expname, 0, sizeof(expname));
      memset(comment, 0, sizeof(comment));
      memset(after, 0, sizeof(after));
      memset(before, 0, sizeof(before));
      latest = 1;
      break;
    }

    bbpsql_sql(sql, &res);
    memset(sql, 0, sizeof(sql));
    bbpsql_clear_result(res);
  }

  if(act == 'U'){
    switch(mode){
    case 'E':
      // Explist
      if(strlen(expid) == 0){
	printf("Update Exp must specify expid by --expid/-i\n");
	quit();
      }
      sqlexpupdate(sql, expid, expname, startdate, stopdate,
		   comment, gatedid, gatedch, ungatedid, ungatedch);
      act = 'L';
      memset(expname, 0, sizeof(expname));
      memset(comment, 0, sizeof(comment));
      memset(after, 0, sizeof(after));
      memset(before, 0, sizeof(before));
      latest = 0;
      break;
    case 'S':
      // Scaler
      if(strlen(expid) == 0 || strlen(scalerid) == 0){
	printf("Update Scaler must specify expid (--expid) and scalerid (--scalerid)\n");
	quit();
      }
      if(!strcmp(comment, "dropthisscaler")){
	sqlscalerdelete(sql, expid, scalerid);
      }else{
	sqlscalerupdate(sql, expid, scalerid, scalername, rate, ratech);
      }
      act = 'L';
      memset(expname, 0, sizeof(expname));
      memset(scalername, 0, sizeof(scalername));
      break;
    case 'C':
      // Channel
      if(strlen(expid) == 0 || strlen(scalerid) == 0){
	printf("Update Scaler channel must specify expid (--expid) and scalerid (--scalerid)\n");
	quit();
      }
      if(strlen(channel) == 0){
	printf("Update Scaler channel must specify the channel number (--channelnumber)\n");
	quit();
      }
      sqlscalerchannelupdate(sql, expid, scalerid, channel, channelname);
      act = 'L';
      memset(expname, 0, sizeof(expname));
      memset(scalername, 0, sizeof(scalername));
      memset(channelname, 0, sizeof(channelname));
      break;
    }

    bbpsql_sql(sql, &res);
    memset(sql, 0, sizeof(sql));
    bbpsql_clear_result(res);

  }

  if(act == 'L'){
    switch(mode){
    case 'E':
      // Explist
      sqlexplist(sql, expid, expname, after, before, latest);
      break;
    case 'R':
      // Runlist
      if(strlen(expid) == 0 && strlen(expname) == 0){
	printf("Show Runlist\n");
	printf("must specify ExpID (--expid) or experiment Name (--expname)\n");
	quit();
      }
      sqlrunlist(sql, expid, expname, runname, runnumber, status,
		 after, before, latest, showxml, desc);
      cm[5] = 1;
      cm[6] = 1;
      break;
    case 'S':
      // Scalerlist
      if(strlen(expid) == 0 && strlen(expname) == 0){
	printf("Show Scalerlist\n");
	printf("must specify ExpID (--expid) or experiment Name (--expname)\n");
	quit();
      }
      sqlscalerlist(sql, expid, expname, scalerid, scalername, 0);
      break;
    case 'C':
      // Channellist
      if((strlen(expid) == 0 && strlen(expname) == 0)
	 || (strlen(scalerid) == 0 && strlen(scalername) == 0)){
	printf("Show Scaler Channel list\n");
	printf("must specify ExpID/Name (--expid/--expname) and ScalerID/Name (--scalerid/--scalername)");
	quit();
      }
      sqlscalerchannel(sql, expid, expname, scalerid, scalername);
      break;
    case 'D':
      // Data
      if((strlen(expid) == 0 && strlen(expname) ==0)
	 || strlen(runname) == 0 || strlen(runnumber) == 0
	 || (strlen(scalerid) == 0 && strlen(scalername) == 0)){
	printf("Show Scaler Data\n");
	printf("must specify ExpID/Name (--expid/--expname)");
	printf(" and RunName and RunNumber (--runname and --runnumber)");
	printf(" and ScalerID or ScalerName (--scalerid/--scalername)");
	printf("\n");
	quit();
      }
      runid = getrunid(expid, expname, runname, runnumber);
      scalerinfoid = getscalerinfoid(expid, expname, scalerid, scalername);

      scalerinfoid = getscalerinfoid(expid, expname, scalerid, scalername);
      type = getscalertype(expid, expname, scalerid, scalername);

      sqlscalerdata(sql, runid, scalerinfoid);
      break;
    }
    
    //fprintf(stderr, "%s\n", sql);  // show SQL line to STDERR
    bbpsql_sql(sql, &res);
    memset(sql, 0, sizeof(sql));

    if(csv){
      if(!ovf){
	bbpsql_get_values_csv_cm(res, sql, idx, cm);
      }else{
	if(type == 11){
	  ovval = ov24;
	}
	bbpsql_get_values_csv_cal(res, sql, 0, 1, ovval);
      }
    }else{
      if(showcol){
	bbpsql_get_fields_values_cm(res, sql, cm);
      }else{
	bbpsql_get_values_cm(res, sql, cm);
      }
    }
    printf("%s\n", sql);  // show the result
    bbpsql_clear_result(res);
  }
  quit();

  return 0;
}

int getrunid(char *expid, char *expname, char *runname, char *runnumber){
  PGresult *res = NULL;
  char sql[1024]={0};
  char null[64]={0};

  sqlrunlist(sql, expid, expname, runname, runnumber, null, null, null, 1, 0, 0);
  bbpsql_sql(sql, &res);
  bbpsql_get_oneresult(res, sql);
  bbpsql_clear_result(res);

  return strtol(sql, NULL, 0);
}

int getscalerinfoid(char *expid, char *expname, char *scalerid, char *scalername){
  PGresult *res = NULL;
  char sql[1024]={0};

  sqlscalerlist(sql, expid, expname, scalerid, scalername, 1);
  bbpsql_sql(sql, &res);
  bbpsql_get_oneresult(res, sql);
  bbpsql_clear_result(res);

  return strtol(sql, NULL, 0);
}

int getscalertype(char *expid, char *expname, char *scalerid, char *scalername){
  PGresult *res = NULL;
  char sql[1024]={0};

  sqlscalerlist(sql, expid, expname, scalerid, scalername, 1);
  bbpsql_sql(sql, &res);
  bbpsql_get_idxresult(res, sql, 3);
  bbpsql_clear_result(res);

  return strtol(sql, NULL, 0);
}
