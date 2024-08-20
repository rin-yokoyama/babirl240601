/* bi-file.c
 * last modified : 10/10/06 20:57:25 
 * 
 * File access utils
 *
 * Hidetada Baba (RIKEN)
 * baba@rarfaxp.riken.jp
 *
 */

//#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>

#include <bi-config.h>
#include <bi-common.h>

struct fdinfost *fdfirst = NULL;
char ofname[256];


/** Check existence of directory
 *  @param path Path of directory
 *  @return Directory = 1, other = 0
 */
int isdir(char *path){
  struct stat st;

  if(stat(path, &st)){
    return 0;
  }

  if(S_ISDIR(st.st_mode)) return 1;

  return 0;
}

/** Check existence of the file
 *  @param path Path of file
 *  @return Exist = 1, other = 0
 */
int isfile(char *path){
  struct stat st;

  if(!stat(path, &st)){
    return 1;
  }

  return 0;
}


int lfprintf(FILE *fd, const char *fmt, ...){
  time_t now;
  int fno;

  if(!fd) return 0;

  time(&now);
  fprintf(fd, "%s ", ctime(&now));

  va_list argp;
  va_start(argp, fmt);
  vfprintf(fd, fmt, argp);
  va_end(argp);

  fflush(fd);
  fno = fileno(fd);
  fdatasync(fno);

  return 1;
}


/** gethdst
 *  @param st pointer for hdstatst[MAXMTPT]
 *  @return number of mount point, 0 = fail
 */
int gethdst(struct hdstatst *st){
#define FSLISTN 6

  int hdstn, i;
  struct statfs vfs;
  FILE *fd;
  char dev[80], path[80], sys[80], mod[80];
  int i1, i2;
  char *fslist[FSLISTN] = {"ext2", "ext3", "nfs", "fat", "vfat", "gfs"};
  int fslistn = FSLISTN;

  hdstn = 0;
  
  if((fd = fopen("/etc/mtab", "r")) == NULL){
    return 0;
  }

  while(fscanf(fd, "%s %s %s %s %d %d", 
	       dev, path, sys, mod, &i1, &i2) == 6){
    for(i=0;i<fslistn;i++){
      if(strcmp(sys, fslist[i]) == 0){
	statfs(path, &vfs);

	st[hdstn].ex    = 1;
	strncpy(st[hdstn].dev, dev, 80);
	strncpy(st[hdstn].path, path, 80);
	st[hdstn].tot   = vfs.f_bsize / 1024 * vfs.f_blocks;
	st[hdstn].free  = vfs.f_bsize / 1024 * vfs.f_bavail;
	st[hdstn].used  = vfs.f_bsize / 1024 * (vfs.f_blocks - vfs.f_bfree);
	
	hdstn++;
	break;
      }
      if(fslistn >= MAXMTPT) break;
    }
  }
    
  fclose(fd);

  return hdstn;
}

int addfilelist(struct fdinfost *n, int u, int s){
  char ofile[272], tname[272];
  FILE *fd;
  int fn = 0;

  DB(printf("bi-file addfilelist name=%s\n", n->name));

  snprintf(ofile, sizeof(ofile)-1, "%s.flt", n->name);

  if(u){
    fd = fopen(ofile, "w");
  }else{
    fd = fopen(ofile, "a+");
  }

  if(fd){
    while(fscanf(fd, "%d %s", &fn, ofile) != EOF){
      if(s){
	snprintf(tname, sizeof(tname)-1, "%s.ridf", n->name);
      }else{
	snprintf(tname, sizeof(tname)-1, "%s%04d.ridf", n->name, n->runnumber);
      }
      if(!strcmp(ofile, tname)){
	printf("Table for %s is exist\n", tname);
	return 0;
      }
      DB(printf("bi-file: file list %d=%s\n", fn, ofile));
    }
  }else{
    printf("Can't open output file %s\n", ofile);
    return 0;
  }

  if(u) fn = 0;

  fn++;
  n->fn = fn;
  if(s){
    fprintf(fd, "%d %s.ridf\n", n->fn, n->name);
  }else{
    fprintf(fd, "%d %s%04d.ridf\n", n->fn, n->name, n->runnumber);
  }
  DB(printf(" - add file list %d=%s%04d.ridf\n", n->fn, n->name, n->runnumber));

  fclose(fd);

  return 1;
}


int openoutfile(struct fdinfost *n, int u){
  int ret = 0;
  struct fdinfost *p, *t;
  char ofile[272];

  DB(printf("babits: openoutfile name=%s\n", n->name));

  p = fdfirst;
  while(p->next){
    if(!strcmp(n->name, p->name)){
      ret = p->ofd;
      n->ofd = ret;
      n->fn = p->fn;
      close(p->ifd);
      t = p->prev;

      DB(printf("- found opend file id[%d].ofd=%d\n", p->id, ret));
      DB(printf("- close previous file id[%d].ifd=%d\n", p->id, p->ifd));

      // p->prev = NULL -> p = fdfirst;
      if(t){
	DB(printf(" - exists prev and next fdinfo id[%d]\n", p->id));

	t->next = p->next;
      }else{
	if(p->next){
	  DB(printf(" - exists next fdinfo (it was first) id[%d]\n", p->id));
	  fdfirst = p->next;
	}else{
	  DB(printf(" - it was only one fdinfo id[%d]\n", p->id));
	  fdfirst = n;
	}
      }
      free(p);

      break;
    }
    p = p->next;
  }

  if(!ret){
    snprintf(ofile, sizeof(ofile)-1, "%s.tst", n->name);
    if(u){
      ret = open(ofile, O_WRONLY | O_CREAT, 0644);
    }else{
      ret = open(ofile, O_WRONLY | O_APPEND | O_CREAT, 0644);
    }
    DB(printf(" - open new table %s  (id[%d])\n", ofile, n->id));
    if(ret < 0) ret = 0;

  }

  return ret;
}


void splitrunname(char *iname, char *oname, int *runnumber, int s){
  int l;

  l = strlen(iname) - 9;
  sscanf(iname+l, "%04d", runnumber);
  if(s){
    l = strlen(iname) - 5;
  }
  memcpy(oname, iname, l);

}

int addfdbyfile(char *file, int fdn, int u, int s){
  int tfd;
  struct fdinfost *t, *p;

  DB(printf("bi-file: addfdbyfile %d=%s\n", fdn, file));

  //if((tfd = open(file, O_RDONLY|O_NOATIME)) < 0){
  if((tfd = open(file, O_RDONLY)) < 0){
    return 0;
  }else{
    t = (struct fdinfost *)malloc(sizeof(struct fdinfost));
    memset(t, 0, sizeof(struct fdinfost));

    t->id = fdn;

    if(!fdfirst){
      fdfirst = t;
    }else{
      p = fdfirst;
      while(p->next){
	p = p->next;
      }
      p->next = t;
      t->prev = p;
    }

    t->ifd = tfd;

    splitrunname(file, t->name, &t->runnumber, s);

    if(!addfilelist(t, u, s)){
      return 0;
    }

    if(!(t->ofd = openoutfile(t, u))){
      printf("bi-file: addfdbyfile - Can't open outfile for %s, id=%d\n", t->name, t->id);
      return 0;
    }
  }

  return tfd;
}

int getmaxfd(void){
  struct fdinfost *p;
  int ret = 0;

  p = fdfirst;

  while(p){
    if(p->ifd > ret) ret = p->ifd;
    p = p->next;
  }

  return ret;
}


void cleanfd(void){
  struct fdinfost *p, *t;

  p = fdfirst;

  while(p){
    close(p->ifd);
    close(p->ofd);
    t = p->next;
    free(p);
    p = t;
  }

  fdfirst = NULL;
}


struct fdinfost *getfdfirst(void){
  return fdfirst;
}

int getifd(struct fdinfost *r){
  int ret;

  ret = r->ifd;
  r = r->next;

  return ret;
}

int setfdset(fd_set *fdset){
  struct fdinfost *p;
  
  FD_ZERO(fdset);

  if(fdfirst){
    p = fdfirst;
  }else{
    return 0;
  }
  
  while(p){
    FD_SET(p->ifd, fdset);
    p = p->next;
  }

  return 1;
}

struct fdinfost *getisfd(fd_set *fdset){
  struct fdinfost *p, *r;

  if(fdfirst){
    p = fdfirst;
  }else{
    return NULL;
  }

  r = NULL;
  while(p){
    if(FD_ISSET(p->ifd, fdset)){
      r = p;
      break;
    }
  }
  p = NULL;

  return r;
}

int closefd(struct fdinfost *p){
  struct fdinfost *t;

  if(p == NULL) return 0;

  DB(printf("bi-file: closefd[%d] name=%s, ifd=%d, ofd=%d\n", p->id, p->name, p->ifd, p->ofd));

  close(p->ifd);
  close(p->ofd);

  t = p->prev;

  // p->prev = NULL -> p = fdfirst;
  if(t){
    DB(printf(" - exists prev and next fdinfo id[%d]\n", p->id));

    t->next = p->next;
  }else{
    if(p->next){
      DB(printf(" - exists next fdinfo (it was first) id[%d]\n", p->id));
      fdfirst = p->next;
    }else{
      DB(printf(" - it was only one fdinfo id[%d]\n", p->id));
      fdfirst = NULL;
    }
  }
  free(p);
  

  return 1;
}


char *filenamefromflt(int n, char *runname){
  FILE *fd;
  char ifname[256];
  int tn, f;

  sprintf(ifname, "%s.flt", runname);

  if((fd = fopen(ifname, "r")) == NULL){
    return 0;
  }else{
    f = 0;
    while(fscanf(fd, "%d %s", &tn, ofname) == 2){
      if(tn == n){
	f = 1;
	break;
      }
    }
    if(f){
      return ofname;
    }else{
      return 0;
    }
  }

}


