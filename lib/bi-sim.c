#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>


// return random number of 0 to 1
double rnd(void){
  return random() / (RAND_MAX + 1.0);
}

int ne(double t0, double t1, double td){
  if((t1 - t0) > td){
    return 1;
  }else{
    return 0;
  }
}


void init_nt(){
  time_t t;

  t = time(NULL);
  srandom((unsigned int)t);
}


// return new time p = rate
double nt(double p){
  double ret;

  ret = 0.;

  while(ret == 0.){
    ret = - log(rnd())/p;
  }
  
  return ret;
}
