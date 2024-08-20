#ifndef BBZFILE_H
#define BBZFILE_H
#include <zlib.h>
#endif

/* Structure for bbzfile */

typedef struct bgFilest{
  int nfd;
  gzFile gfd;
}bgFile;

bgFile *bgropen(char *);
void bgclose(bgFile *);
int bgread(bgFile *, void *, int);

