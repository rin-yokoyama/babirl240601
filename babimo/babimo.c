/* last modified : 13/03/28 16:38:35 
 *
 * babimo.c : babirl process monitor
 * 
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>

#include <bi-config.h>
#include <bi-common.h>

#include "babimo.h"

//#define DEBUG

void quit(void){
  if(comfd) close(comfd);
  if(udpsock) close(udpsock);

  exit(0);
}

int udpmain(void){
  int com, sz=0;
  struct sockaddr_in caddr;
  socklen_t clen;
  unsigned short rport, mag;

  clen = sizeof(caddr);
  recvfrom(udpsock, &com, sizeof(com), 0, 
	   (struct sockaddr *)&caddr, &clen);

  memcpy((char *)&rport, (char *)&com, 2);
  memcpy((char *)&mag, (char *)&com+2, 2);
  if(mag == 0xf600){
    DB(printf("com = %08x, port=%d\n", com, rport));
    caddr.sin_family = AF_INET;
    caddr.sin_port = htons(rport);
    clen = sizeof(caddr);
    sz = sendto(udpsock, udata, sizeof(udata), 0, (struct sockaddr *)&caddr, clen);
    if(sz != 256){
      perror("error");
    }
    DB(printf("send to %d %d\n", rport, sz));
  }else{
    DB(printf("invalid com = %08x\n", com));
    return 0;
  }

  return 1;
}

int commain(void){
  struct sockaddr_in caddr;
  char buff[1024*100]; // 100kB buffer
  char pbuff[1024*100]; // 100kB buffer
  int sock, clen, len;
  int com, i, ret, sf;
  DB(int rlen;)
  char name[256];
  FILE *fp;

  clen = sizeof(caddr);
  if((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0){
    perror("Error in accept commain\n");
    return 0;
  }

  memset(buff, 0, sizeof(buff));
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  DB(printf("babimo: lne=%d\n", len));

  DB(rlen =) recv(sock, buff, len, MSG_WAITALL);
  DB(printf("rlen = %d, len=%d\n", rlen, len));

  /* babimo command */
  memcpy((char *)&com, buff, sizeof(com));

  DB(printf("babimo: com=%d\n", com));

  switch(com){
  case MON_GETHDST:                 // Get disk status
    hdstn = gethdst(hdst);
    len = sizeof(hdst);
    send(sock, (char *)&len, sizeof(len), MSG_NOSIGNAL);
    send(sock, buff, len, MSG_NOSIGNAL);
    break;
  case MON_GETHDST_STR:
    hdstn = gethdst(hdst);
    for(i=0;i<hdstn;i++){
      len = sprintf(buff, "%s %s %d %d %d\n", hdst[i].dev, hdst[i].path,
	      hdst[i].tot, hdst[i].used, hdst[i].free);
      send(sock, buff, len, MSG_NOSIGNAL);
    }
    break;
  case MON_CHKPID:
    memset(name, 0, sizeof(name));
    memcpy(name, buff+sizeof(com), len-sizeof(com));
    ret = chkpid(name);
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), MSG_NOSIGNAL);
    send(sock, (char *)&ret, len, MSG_NOSIGNAL);
    break;
  case MON_CHKPID_STR:
    memset(name, 0, sizeof(name));
    memcpy(name, buff+sizeof(com), len-sizeof(com));
    DB(printf("mon_str: name=%s\n", name));
    ret = chkpid(name);
    len = sprintf(buff, "%d\n", ret);
    send(sock, buff, len, MSG_NOSIGNAL);
    break;
  case MON_KILLPID:
    memset(name, 0, sizeof(name));
    memcpy(name, buff+sizeof(com), len-sizeof(com));
    killpid(name);
    rmpid(name);
    break;
  case MON_EXEC:
    memset(pbuff, 0, sizeof(pbuff));
    memcpy(pbuff, buff+sizeof(com), len-sizeof(com));
    strcat(pbuff, " &");

    sf = strncmp(name, "/usr/babirl/babies/babies -r", 28);
    if(sf == 0){
      /* Set assumed kernel */
      setenv("LD_ASSUME_KERNEL", "2.4", 1);
    }
    system(pbuff);
    if(sf == 0){
      unsetenv("LD_ASSUME_KERNEL");
    }
    break;
  case MON_BABIEXEC:
    memset(name, 0, sizeof(name));
    
    sprintf(name, "%s/bin/", BABIRLDIR);
    memcpy(name+strlen(name), buff+sizeof(com), len-sizeof(com));
    strcat(name, " &");
    system(name);

    DB(printf("babimo: MON_BABIEXEC %s\n", name));

    break;
  case MON_PEXEC:
    memset(pbuff, 0, sizeof(pbuff));
    memcpy(pbuff, buff+sizeof(com), len-sizeof(com));
    memset(buff, 0, sizeof(buff));
    
    if((fp = popen(pbuff, "r")) == NULL){
      strcpy(buff, "-- pexec failed--");
      len = strlen(buff) + 1;
    }else{
      fread(buff, 1, sizeof(buff)-1, fp);
      len = strlen(buff);
      if(pclose(fp) < 0){
	strcpy(buff, "No such command");
	len = strlen(buff) + 1;
      }
    }
    
    send(sock, (char *)&len, sizeof(len), MSG_NOSIGNAL);
    send(sock, (char *)&buff, len, MSG_NOSIGNAL);
    break;
  case MON_COPYFILE:
    memset(name, 0, sizeof(name));
    strncpy(name, buff+sizeof(com), sizeof(name)-1);
#ifdef DEBUG
    printf("MON_COPYFILE: file=%s / size=%d\n", name, len);
#endif

    if((fp = fopen(name, "w")) == NULL){
      ret = 0;
    }else{
      i = sizeof(com) + strlen(name) + 4;
      fwrite(buff+i, 1, len - i, fp);
      fclose(fp);
      ret = 1;
    }

    send(sock, (char *)&ret, sizeof(ret), MSG_NOSIGNAL);
    break;
  case MON_CHKBABIES:
    ret = 0;

    if(!access("/tmp/babiesrun", F_OK)){
      ret = 1;
    }
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), MSG_NOSIGNAL);
    send(sock, (char *)&ret, len, MSG_NOSIGNAL);
    break;
  default:
    break;
  }

  close(sock);

  return 1;
}
  
int main(int argc, char *argv[]){
  int sn;
  fd_set fdset;
  struct sockaddr_in usaddr;
  FILE *fd = NULL;
  char path[256] = {0};
  
  if(!(comfd = mktcpsock(BABIMOPORT))) quit();
  if(!(udpsock = mkudpsock(BABIMOPORT, &usaddr))) quit();

#ifndef DEBUG
  daemon(1, 0);
#endif

  udata[0] = 0x00;
  udata[1] = 0x00;
  udata[2] = 0x00;
  udata[3] = 0xf7;
  sprintf(path, "%s/init/biid", BABIRLDIR);
  if((fd = fopen(path, "r"))){
    fgets(udata+16, 32, fd);
    fclose(fd);
  }

  // get mac address
  getmacbysock(udpsock, udata+72);
  getipbysock(udpsock, udata+64);


#ifdef DEBUG
  printf("biid = %s\n", udata+16);
  printf("ip   = %u.%u.%u.%u\n",
	 (unsigned char)udata[64], (unsigned char)udata[65],
	 (unsigned char)udata[66], (unsigned char)udata[67]);
  printf("mac  = %02x:%02x:%02x:%02x:%02x:%02x\n", 
	 (unsigned char)udata[72],(unsigned char)udata[73],
	 (unsigned char)udata[74],(unsigned char)udata[75],
	 (unsigned char)udata[76],(unsigned char)udata[77]);
#endif

  if(comfd > udpsock){
    sn = comfd;
  }else{
    sn = udpsock;
  }
  
  while(1){
    /* Main loop */
    
    FD_ZERO(&fdset);
    FD_SET(comfd, &fdset);
    FD_SET(udpsock, &fdset);
    
    if(select(sn+1, &fdset, NULL, NULL, NULL) != 0){
      if(FD_ISSET(comfd, &fdset)){
	commain();
      }else if(FD_ISSET(udpsock, &fdset)){
	udpmain();
      }
    }
  }

  return 0;
}
