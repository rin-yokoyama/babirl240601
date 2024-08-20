#include <stdio.h>
#include <stdarg.h>

int bbcpri_code[20] = 
{1,4,5,7,30,31,32,33,34,35,36,37,40,41,42,43,44,45,46,47};


void cl_screen(void){
  printf("\x1b[2J");
}

void cl_line(void){
  printf("\x1b[2K");
}

void cl_cur_end(void){
  printf("\x1b[0J");
}

void nx_line(void){
  printf("\x1b[1E");
}

void mv_cur(int x,int y){
  printf("\x1b[%d;%dH",y,x);
}

void line_cur(int n){
  printf("\x1b[%dG",n);
}

void up_cur(int n){
  printf("\x1b[%dA",n);
}

void down_cur(int n){
  printf("\x1b[%dB",n);
}

void right_cur(int n){
  printf("\x1b[%dC",n);
}

void left_cur(int n){
  printf("\x1b[%dD",n);
}

void save_cur(void){
  printf("\x1b[s");
}

void restore_cur(void){
  printf("\x1b[u");
}

void cprintf(int att,char *fmt,...){
  int bbcpri_i;

  va_list argp;
  va_start(argp,fmt);
  for(bbcpri_i=0;bbcpri_i<30;bbcpri_i++){
    if((att & 0x01)){
      printf("\x1b[%dm",bbcpri_code[bbcpri_i]);
    }
    att = att>>1;
  }
  vprintf(fmt,argp);
  va_end(argp);
  printf("\x1b[00m");
}

void csprintf(char *str, int att,char *fmt,...){
  int bbcpri_i,cnt=0;

  va_list argp;
  va_start(argp,fmt);
  for(bbcpri_i=0;bbcpri_i<30;bbcpri_i++){
    if((att & 0x01)){
      cnt += sprintf(str+cnt,"\x1b[%dm",bbcpri_code[bbcpri_i]);
    }
    att = att>>1;
  }
  cnt += vsprintf(str+cnt,fmt,argp);
  va_end(argp);
  sprintf(str+cnt,"\x1b[00m");
}
