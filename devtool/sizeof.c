/* babirl/devtool/sizeof.c
 * print sizeof(structure *)
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 * 
 */

#include <stdio.h>
#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

struct testst{
  int a;
  long long int b;
  int c;
  int d;
  long long int e;
  short f;
  long long int g;
  float h;
  double i;
  long long int j;
  double k;
  short l;
  int m;
  long long int n;
  short o;
};

struct testst2{
  char a[5];
  short b;
  int c;
  long long int d;
};

int main(int argc, char *argv[]){
  struct testst t;
  struct testst2 k;

  printf("stdaqinfo %lu\n", (long unsigned int)sizeof(struct stdaqinfo));
  printf("struninfo %lu\n", (long unsigned int)sizeof(struct struninfo));
  printf("sthdinfo  %lu\n", (long unsigned int)sizeof(struct sthdlist));
  printf("steflist  %lu\n", (long unsigned int)sizeof(struct steflist));
  printf("stmtlist  %lu\n", (long unsigned int)sizeof(struct stmtlist));
  printf("stefrc    %lu\n", (long unsigned int)sizeof(struct stefrc));
  printf("stclinfo  %lu\n", (long unsigned int)sizeof(struct stclinfo));
  printf("stssminfo %lu\n", (long unsigned int)sizeof(struct stssminfo));
  printf("hdstatst  %lu\n", (long unsigned int)sizeof(struct hdstatst));
  printf("\n");
  printf("ridf_scr_anast  %lu\n", (long unsigned int)sizeof(struct ridf_scr_anast));
  printf("ridf_scr_contst %lu\n", (long unsigned int)sizeof(struct ridf_scr_contst));
  printf("\n");
  printf("ridfhdevtts %lu\n", (long unsigned int)sizeof(RIDFHDEVTTS));
  printf("ridfhdevt   %lu\n", (long unsigned int)sizeof(RIDFHDEVT));
  printf("\n");

  printf("testst  %lu\n", (long unsigned int)sizeof(struct testst));
  printf("t.a    %p  int\n", &t.a);
  printf("t.b    %p  long long int\n", &t.b);
  printf("t.c    %p  int\n", &t.c);
  printf("t.d    %p  int\n", &t.d);
  printf("t.e    %p  long long int\n", &t.e);
  printf("t.f    %p  short\n", &t.f);
  printf("t.g    %p  long long int\n", &t.g);
  printf("t.h    %p  float\n", &t.h);
  printf("t.i    %p  double\n", &t.i);
  printf("t.j    %p  long long int\n", &t.j);
  printf("t.k    %p  double\n", &t.k);
  printf("t.l    %p  short\n", &t.l);
  printf("t.m    %p  int\n", &t.m);
  printf("t.n    %p  long long int\n", &t.n);
  printf("t.o    %p  short\n", &t.o);
  

  printf("testst2  %lu\n", (long unsigned int)sizeof(struct testst2));
  printf("k.a    %p  char[5]\n", &k.a);
  printf("k.b    %p  short\n", &k.b);
  printf("k.c    %p  int\n", &k.c);
  printf("k.d    %p  double\n", &k.d);

  return 0;
}
