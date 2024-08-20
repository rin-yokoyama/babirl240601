/* devtool/bpidof
 *
 * development file
 *
 */

#include <stdio.h>
#include <libbabirl.h>

int main(int argc, char *argv[]){
  //int pids[32], n, i;
  char name[1024];

  //n = pidof(argv[1], pids);

  //for(i=0;i<n;i++){
  //printf("%d %d\n", i, pids[i]);
  //}

  dirtoname(argv[1], name);
  printf("%s\n", name);
  return 0;
}

