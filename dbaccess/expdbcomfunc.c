// functions for expdbcom

#include <stdio.h>
#include <string.h>

int subsqlstart(char *sql){
  return sprintf(sql, "(");
}

int subsqlend(char *sql){
  return sprintf(sql, ")");
}

int addcond(char *sql, int *where){
  int ret;
  if(*where == 0){
    ret = sprintf(sql, " WHERE ");
    (*where)++;
  }else{
    ret = sprintf(sql, " AND ");
    (*where)++;
  }

  return ret;
}

int addset(char *sql, char *col, char *val, int *set){
  int ret;
  if(strlen(val)){
    if(*set == 0){
      ret = sprintf(sql, " \"%s\" = '%s'", col, val);
      (*set)++;
    }else{
      ret  = sprintf(sql, ", \"%s\" = '%s'", col, val);
      (*set)++;
    }
    
    return ret;
  }else{
    return 0;
  }
}

int addlist(char *sql, char *val){
  return sprintf(sql, " \"%s\",", val);
}

int addval(char *sql, char *val){
  return sprintf(sql, " '%s',", val);
}

int sqlvalues(char *sql, char *cols, char *vals){
  return sprintf(sql, " (%s) VALUES (%s)", cols, vals);
}

int sqlupdate(char *sql, char *table){
  return sprintf(sql, "UPDATE \"%s\" SET", table);
}

int sqlinsert(char *sql, char *table){
  return sprintf(sql, "INSERT INTO \"%s\" ", table);
}


int sqlselect(char *sql, char *sval, char *table){
  return sprintf(sql, "SELECT \"%s\" from \"%s\"", sval, table);
}

int addwhere(char *sql, char *col, char *ex, char *val){
  return sprintf(sql, " \"%s\"%s'%s'", col, ex, val);
}

int addwheresub(char *sql, char *col, char *ex, char *sub){
  return sprintf(sql, " \"%s\"%s(%s)", col, ex, sub);
}


int addorder(char *sql, char *val, int desc, int limit){
  int ret;

  ret = sprintf(sql, " ORDER BY \"%s\"", val);
  if(desc){
    ret += sprintf(sql+ret, " DESC");
  }
  if(limit > 0){
    ret += sprintf(sql+ret, " LIMIT %d", limit);
  }

  return ret;
}

int addlimit(char *sql, int limit){
  if(limit > 0){
    return sprintf(sql, " LIMIT %d", limit);
  }
  return 0;
}

int addlatest(char *sql, char *val){
  return addorder(sql, val, 1, 1);
}

int sqlexplist(char *sql, char *expid, char *expname,
		char *after, char *before, int latest){
  int idx, where=0;

  idx = sprintf(sql, "SELECT \"ExpID\", \"Name\", \"StartDate\", \"StopDate\", \"Comment\"");
  idx += sprintf(sql+idx, ", \"GatedID\", \"GatedCh\", \"UngatedID\", \"UngatedCh\" from \"ExpInfo\"");

  if(strlen(expid)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "ExpID", "=", expid);
  }
  if(strlen(expname)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "Name", "=", expname);
  }
  if(strlen(after)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "StartDate", ">=", after);
  }
  if(strlen(before)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "StartDate", "<=", before);
  }
  if(latest){
    idx += addlatest(sql+idx, "ExpID");
  }else{
    idx += addorder(sql+idx, "ExpID", 0, 0);
  }

  return idx;
}

int sqlrunlist(char *sql, char *expid, char *expname, char *runname,
	       char *runnumber, char *status, char *after, char *before,
	       int latest, int showxml, int desc){
  int idx, where=0, subidx, subwhere=0;
  char subsql[1024];

  idx = sprintf(sql, "SELECT \"RunID\", \"Name\", \"Number\", \"StartDate\", \"StopDate\", \"Header\", \"Ender\", \"Status\"");
  if(showxml){
    idx += sprintf(sql+idx, " \"XML\"");
  }
  idx += sprintf(sql+idx, " from \"RunInfo\"");

  if(strlen(expid)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "ExpID", "=", expid);
  }else{
    idx += addcond(sql+idx, &where);
    memset(subsql, 0, sizeof(subsql));
    subidx = sqlselect(subsql, "ExpID", "ExpInfo");
    subidx += addcond(subsql+subidx, &subwhere);
    subidx += addwhere(subsql+subidx, "Name", "=", expname);
    subidx += addlimit(subsql+subidx, 1);
    idx += addwheresub(sql+idx, "ExpID", "=", subsql);
  }
  if(strlen(runname)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "Name", "=", runname);
  }    
  if(strlen(runnumber)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "Number", "=", runnumber);
  }    
  if(strlen(status)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "Status", "=", status);
  }    
  if(strlen(after)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "StartDate", ">=", after);
  }
  if(strlen(before)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "StartDate", "<=", before);
  }
  if(latest){
    idx += addlatest(sql+idx, "ExpID");
  }
  if(desc){
    idx += addorder(sql+idx, "RunID", 1, 0);
  }


  return idx;
}

int sqlscalerlist(char *sql, char *expid, char *expname, char *scalerid,
		  char *scalername, int withid){
  int idx, where=0;
  char subsql[1024];
  int subidx, subwhere=0;

  idx = sprintf(sql, "SELECT "); 
  if(withid){
    idx += sprintf(sql+idx, " \"ScalerInfo\".\"ScalerInfoID\", ");
  }
  idx += sprintf(sql+idx, " \"ScalerInfo\".\"ScalerID\", \"ScalerInfo\".\"Name\",");
  idx += sprintf(sql+idx, " \"ScalerInfo\".\"Type\", \"ScalerType\".\"Name\" as \"ScalerType\",");
  idx += sprintf(sql+idx, " \"ScalerInfo\".\"Rate\", \"ScalerInfo\".\"RateCh\"");
  idx += sprintf(sql+idx, " from \"ScalerInfo\" INNER JOIN \"ScalerType\"");
  idx += sprintf(sql+idx, " ON (\"ScalerInfo\".\"Type\" = \"ScalerType\".\"Type\")");

  if(strlen(expid)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "ScalerInfo\".\"ExpID", "=", expid);
  }else{
    idx += addcond(sql+idx, &where);
    memset(subsql, 0, sizeof(subsql));
    subidx = sqlselect(subsql, "ExpID", "ExpInfo");
    subidx += addcond(subsql+subidx, &subwhere);
    subidx += addwhere(subsql+subidx, "Name", "=", expname);
    subidx += addlimit(subsql+subidx, 1);
    idx += addwheresub(sql+idx, "ExpID", "=", subsql);
  }
  if(strlen(scalerid)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "ScalerInfo\".\"ScalerID", "=", scalerid);
  }    
  if(strlen(scalername)){
    idx += addcond(sql+idx, &where);
    idx += addwhere(sql+idx, "ScalerInfo\".\"Name", "=", scalername);
  }    
  idx += addorder(sql+idx, "ScalerInfo\".\"ScalerID", 0, 0);

  return idx;
}

/// Show Scaler Channel list
int sqlscalerchannel(char *sql, char *expid, char *expname, char *scalerid,
		     char *scalername){
  int idx, where=0;
  char subsql[1024];
  int subidx, subwhere=0;

  idx = sprintf(sql, "SELECT \"Channel\", \"Name\" FROM \"ScalerChannel\"");
  idx += sprintf(sql+idx, " WHERE \"ScalerInfoID\" = (");
  idx += sqlselect(sql+idx, "ScalerInfoID", "ScalerInfo");
  idx += addcond(sql+idx, &where);

  if(strlen(expid)){
    idx += addwhere(sql+idx, "ExpID", "=", expid);
  }else{
    memset(subsql, 0, sizeof(subsql));
    subwhere = 0;
    subidx = sqlselect(subsql, "ExpID", "ExpInfo");
    subidx += addcond(subsql+subidx, &subwhere);
    subidx += addwhere(subsql+subidx, "Name", "=", expname);
    subidx += addlimit(subsql+subidx, 1);
    idx += addwheresub(sql+idx, "ExpID", "=", subsql);
  }

  idx += addcond(sql+idx, &where);
  if(strlen(scalerid)){
    idx += addwhere(sql+idx, "ScalerID", "=", scalerid);
  }else{
    idx += addwhere(sql+idx, "Name", "=", scalername);
  }
  idx += sprintf(sql+idx, " LIMIT 1)");
  idx += addorder(sql+idx, "Channel", 0, 0);

  return idx;
}



/// Exp Update
int sqlexpupdate(char *sql, char *expid, char *expname,
		 char *startdate, char *stopdate, char *comment,
		 char *gatedid, char *gatedch, char *ungatedid, char *ungatedch){
  int idx, where=0, set=0;

  idx = sqlupdate(sql, "ExpInfo");
  if(strlen(expname)){
    idx += addset(sql+idx, "Name", expname, &set);
  }
  if(strlen(startdate)){
    idx += addset(sql+idx, "StartDate", startdate, &set);
  }
  if(strlen(stopdate)){
    idx += addset(sql+idx, "StopDate", stopdate, &set);
  }
  if(strlen(comment)){
    idx += addset(sql+idx, "Comment", comment, &set);
  }
  if(strlen(gatedid)){
    idx += addset(sql+idx, "GatedID", gatedid, &set);
  }
  if(strlen(gatedch)){
    idx += addset(sql+idx, "GatedCh", gatedch, &set);
  }
  if(strlen(ungatedid)){
    idx += addset(sql+idx, "UngatedID", ungatedid, &set);
  }
  if(strlen(ungatedch)){
    idx += addset(sql+idx, "UngatedCh", ungatedch, &set);
  }

  idx += addcond(sql+idx, &where);
  idx += addwhere(sql+idx, "ExpID", "=", expid);

  return idx;
}

/// Exp Insert
int sqlexpinsert(char *sql, char *expname,
		 char *startdate, char *stopdate, char *comment,
		 char *gatedid, char *gatedch, char *ungatedid, char *ungatedch){
  int idx;
  char cols[1024], vals[1024];
  int cidx=0, vidx=0;

  memset(cols, 0, sizeof(cols));
  memset(vals, 0, sizeof(vals));

  idx = sqlinsert(sql, "ExpInfo");
  if(strlen(expname)){
    cidx += addlist(cols+cidx, "Name");
    vidx += addval(vals+vidx, expname);
  }else{
    cidx += addlist(cols+cidx, "Name");
    vidx += addval(vals+vidx, "new exp");
  }
  if(strlen(startdate)){
    cidx += addlist(cols+cidx, "StartDate");
    vidx += addval(vals+vidx, startdate);
  }
  if(strlen(stopdate)){
    cidx += addlist(cols+cidx, "StopDate");
    vidx += addval(vals+vidx, stopdate);
  }
  if(strlen(comment)){
    cidx += addlist(cols+cidx, "Comment");
    vidx += addval(vals+vidx, comment);
  }
  if(strlen(gatedid)){
    cidx += addlist(cols+cidx, "GatedID");
    vidx += addval(vals+vidx, gatedid);
  }
  if(strlen(gatedch)){
    cidx += addlist(cols+cidx, "GatedCh");
    vidx += addval(vals+vidx, gatedch);
  }
  if(strlen(ungatedid)){
    cidx += addlist(cols+cidx, "UngatedID");
    vidx += addval(vals+vidx, ungatedid);
  }
  if(strlen(ungatedch)){
    cidx += addlist(cols+cidx, "UngatedCh");
    vidx += addval(vals+vidx, ungatedch);
  }

  // delete last ','
  cidx--;
  cols[cidx] = 0;
  vidx--;
  vals[vidx] = 0;

  idx += sqlvalues(sql+idx, cols, vals);

  return idx;
}


int sqlscalerupdate(char *sql, char *expid, char *scalerid, char *scalername,
		    char *rate, char *ratech){
  int idx, where=0, set=0;

  idx = sqlupdate(sql, "ScalerInfo");
  idx += addset(sql+idx, "Name", scalername, &set);
  idx += addset(sql+idx, "Rate", rate, &set);
  idx += addset(sql+idx, "RateCh", ratech, &set);
  idx += addcond(sql+idx, &where);
  idx += addwhere(sql+idx, "ExpID", "=", expid);
  idx += addcond(sql+idx, &where);
  idx += addwhere(sql+idx, "ScalerID", "=", scalerid);

  return idx;
}

int sqlscalerdelete(char *sql, char *expid, char *scalerid){
  int idx, where=0, set=0;
  char *newid = {"-1\0"};

  idx = sqlupdate(sql, "ScalerInfo");
  idx += addset(sql+idx, "ExpID", newid, &set);
  idx += addcond(sql+idx, &where);
  idx += addwhere(sql+idx, "ExpID", "=", expid);
  idx += addcond(sql+idx, &where);
  idx += addwhere(sql+idx, "ScalerID", "=", scalerid);

  return idx;
}


int sqlscalerchannelupdate(char *sql, char *expid, char *scalerid, char *channel, char *channelname){
  int idx, where=0, set=0;
  int subwhere=0;

  idx = sqlupdate(sql, "ScalerChannel");
  idx += addset(sql+idx, "Name", channelname, &set);
  idx += addcond(sql+idx, &where);
  idx += addwhere(sql+idx, "Channel", "=", channel);
  idx += addcond(sql+idx, &where);
  idx += sprintf(sql+idx, " \"ScalerInfoID\" = (");
  idx += sqlselect(sql+idx, "ScalerInfoID", "ScalerInfo");
  idx += addcond(sql+idx, &subwhere);
  idx += addwhere(sql+idx, "ScalerID", "=", scalerid);
  idx += addcond(sql+idx, &subwhere);
  idx += addwhere(sql+idx, "ExpID", "=", expid);
  idx += sprintf(sql+idx, ")");

  return idx;
}

int sqlscalerdata(char *sql, int runid, int scalerinfoid){
  int idx;

  idx = sprintf(sql, "SELECT \"Data\", \"Overflow\" FROM \"ScalerData\"");
  idx += sprintf(sql+idx, " INNER JOIN \"ScalerChannel\" ON");
  idx += sprintf(sql+idx, " \"ScalerData\".\"ChannelID\" = \"ScalerChannel\".\"ChannelID\"");
  idx += sprintf(sql+idx, " WHERE \"ScalerData\".\"ChannelID\" IN (");
  idx += sprintf(sql+idx, " SELECT \"ChannelID\" from \"ScalerChannel\" WHERE");
  idx += sprintf(sql+idx, " \"ScalerInfoID\" = %d)", scalerinfoid);
  idx += sprintf(sql+idx, " AND \"RunID\" = %d", runid);
  idx += addorder(sql+idx, "Channel", 0, 0);

  return idx;
}
