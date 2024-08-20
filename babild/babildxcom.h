int xgeteflist(BBXMLEL *, BBXMLEL *);
int xseteflist(BBXMLEL *, BBXMLEL *);
int xgetdaqinfo(BBXMLEL *, BBXMLEL *);
int xgetruninfo(BBXMLEL *, BBXMLEL *);
int xgethdlist(BBXMLEL *, BBXMLEL *);
int xsethdlist(BBXMLEL *, BBXMLEL *);
int xsetruninfo(BBXMLEL *, BBXMLEL *);
int xsetaliasname(BBXMLEL *, BBXMLEL *);
int xgetinitialize(char *ret);
int xsetchkerhost(BBXMLEL *, BBXMLEL *);
int xsettgig(BBXMLEL *, BBXMLEL *);
int xnssta(BBXMLEL *, BBXMLEL *);
int xstart(BBXMLEL *, BBXMLEL *);
int xstop(BBXMLEL *, BBXMLEL *);
int xwth(BBXMLEL *, BBXMLEL *);

BBXMLCOM xcom[]={
  {"geteflist",    xgeteflist},
  {"seteflist",    xseteflist},
  {"getdaqinfo",   xgetdaqinfo},
  {"getruninfo",   xgetruninfo},
  {"gethdlist",    xgethdlist},
  {"sethdlist",    xsethdlist},
  {"setruninfo",   xsetruninfo},
  {"setaliasname", xsetaliasname},
  {"setchkerhost", xsetchkerhost},
  {"settgig",      xsettgig},
  {"nssta",        xnssta},
  {"start",        xstart},
  {"stop",         xstop},
  {"wth",          xwth},
  {(char *)NULL, NULL}
};

/*
<babild>
 <babildxcom>
  <aliasname>aliasname</aliasname>
  <seterhost>0</seterhost> // 0=off, 1=on
  <geteflist/>
  <getdaqinfo/>
  <gethdlist/>
  <seteflist>
   <eflist>
    <efn>22</efn>
    <host>ccnet02</host>
    <name>ccnet02</host>
    <of>0</of>
    <ex>1</ex>
   </eflist>
  </seteflist>
  <sethdlist>
   <hdlist>
     <hdn>0</hdn>
     <ex>1</ex>
     <of>1</of>
     <path>/hoge</path>
   </hdlist>
 </sethdlist>
 <setruninfo>
  <runname>RUNNAME</runname>
  <runnumber>0</runnumber> // Next run = 1
 </setruninfo>
 <wth>Header</wth>
 <nssta/>
 <start/>
 <start>Header</start>
 <stop>Ender</stop>
 </babildxcom>
</babild>

 ex : 0=delete, 1=append
 of : 0=off, 1=on, 2=scr

*/

