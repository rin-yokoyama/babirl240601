/* command line crient of babimo
 * default is rsh command
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <netinet/in.h>

#include <bi-config.h>
#include <bi-common.h>

int mocon(char *host);
int exec_rsh(int sock, char *arg, int sz);


int mocon(char *host){
  int mosock;

  /* Connect to babild */
  if(!(mosock = mktcpsend(host, BABIMOPORT))){
    printf("babimocli: Can't connet to babimo.\n");
    return 0;
  }
  
  return mosock;
}


int exec_rsh(int sock, char *arg, int sz){
  int com, len;
  char buff[1024*100] = {0};

  memset(buff ,0, sizeof(buff));
  com = MON_PEXEC;

  len = sizeof(com) + sz;
  send(sock,(char *)&len, sizeof(len), 0);
  send(sock, (char *)&com, sizeof(com), 0);
  send(sock, arg, sz, 0);

  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);

  close(sock);

  printf("%s\n", buff);

  return 0;
}

int main(int argc, char *argv[]){
  int sock;
  char buff[4096] = {0};
  int i, sz;

  if(argc < 3){
    printf("babimocli HOSTNAME COMMANDS...\n");
    exit(0);
  }

  sock = mocon(argv[1]);
  if(!sock) exit(0);

  for(i=2;i<argc;i++){
    if(i>2){
      strcat(buff, " ");
    }
    strcat(buff, argv[i]);
  }

  sz = strlen(buff);
  exec_rsh(sock, buff, sz);
  

  return 0;
}
