int xgetesconfig(BBXMLEL *, BBXMLEL *);
int xsetesconfig(BBXMLEL *, BBXMLEL *);
int xreloadesdrv(BBXMLEL *, BBXMLEL *);
#ifdef USEDB
int xdbsetexp(BBXMLEL *, BBXMLEL *);
int xdbgetexp(BBXMLEL *, BBXMLEL *);
int xdbgetscr(BBXMLEL *, BBXMLEL *);
int xdbsetscr(BBXMLEL *, BBXMLEL *);
int xdbsetdaqname(BBXMLEL *, BBXMLEL *);
int xdbgetdaqname(BBXMLEL *, BBXMLEL *);
#endif
//int xeswhoareyou(BBXMLEL *, BBXMLEL *);

int xgetinitialize(char *);

BBXMLCOM xcom[]={
  {"getesconfig",    xgetesconfig},
  {"setesconfig",    xsetesconfig},
  {"reloadesdrv",    xreloadesdrv},
#ifdef USEDB
  {"dbsetexp",       xdbsetexp},
  {"dbgetexp",       xdbgetexp},
  {"dbgetscr",       xdbgetscr},
  {"dbsetscr",       xdbsetscr},
  {"dbsetdaqname",       xdbsetdaqname},
  {"dbgetdaqname",       xdbgetdaqname},
#endif
  //  {"eswhoareyou",    xeswhoareyou},
  {(char *)NULL, NULL}
};


/*
  getesconfig accept multiple efn
  setesconfig don't accept multiple efn
   you have to send 1-by-1 setesconfig for some babies
*/

/*
<babinfoxcom>
 <dbsetdaqname> // set daqname
  <name>BigRIPS</name>
  <server>d03</server>
 </dbsetdaqname>
 <dbgetdaqname> // get daqname
 <dbgetscr/> // get scaler infomation from DB
 <getesconfig>  // includes getesconfig
   <efn>22</efn>
   <efn>47</efn>
 </getesconfig>
 <setesconfig>
   <efn>22</efn>
   <host>d01</host>
   <rtdrv>/home/daqconfig/work</rtdrv>
 </setesconfig>
 <reloadesdrv>
   <efn>22</efn>
 </reloadesdrv>
 <reloadesdrv>
   <efn>23</efn>
 </reloadesdrv>
 // <eswhoareyou>  // included getesconfig
 <dbsetexp>
   <expid>1</expid>
   <name>Name</name>
 </dbsetexp>
</babinfoxcom>

*/

/*
 Return
<babinfo>
  <exp>
    <name>ExpName</name>
    <id>ExpId</id>
  </exp>
</babinfo>
*/
