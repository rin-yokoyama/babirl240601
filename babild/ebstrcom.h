char *parsetextcom(char *, int *);
int getdaqinfo();
int setdaqinfo();
int getruninfo();
int setruninfo();
int chksetcom();
int seteflist();
int geteflist();
int sethdlist();
int setmtlist();
int getebinfo();
int setssminfo();
int setclinfo();

enum setinfo {NONE, DAQINFON, RUNINFON, EFLISTN, HDLISTN, MTLISTN,
	      CLINFON, SSMINFON};
