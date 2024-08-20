// prototype
int addcond(char *sql, int idx, int *where);
int addorder(char *sql, char *val, int desc, int limit);
int sqlselect(char *sql, char *sval, char *table);
int addwhere(char *sql, char *col, char *ex, char *val);
int addwheresub(char *sql, char *col, char *ex, char *sub);
int addlimit(char *sql, int limit);
int addlatest(char *sql, char *val);
int sqlexplist(char *sql, char *expid, char *expname,
		char *after, char *before, int latest);
int sqlrunlist(char *sql, char *expid, char *expname, char *runname,
	       char *runnumber, char *status, char *after, char *before,
	       int latest, int showxml, int desc);
int sqlscalerlist(char *sql, char *expid, char *expname, char *scalerid,
		  char *scalername, int withidx);
int sqlexpupdate(char *sql, char *expid, char *expname,
		 char *startdate, char *stopdate, char *comment,
		 char *gatedid, char *gatedch, char *ungatedid, char *ungatedch);
int sqlexpinsert(char *sql, char *expname,
		 char *startdate, char *stopdate, char *comment,
		 char *gatedid, char *gatedch, char *ungatedid, char *ungatedch);
int sqlscalerchannel(char *sql, char *expid, char *expname, char *scalerid,
		     char *scalername);
int sqlscalerupdate(char *sql, char *expid, char *scalerid, char *scalername,
		    char *rate, char *ratech);
int sqlscalerdelete(char *sql, char *expid, char *scalerid);
int sqlscalerchannelupdate(char *sql, char *expid, char *scalerid, char *channel,
			   char *channelname);
int sqlscalerdata(char *sql, int runid, int scalerinfoid);
