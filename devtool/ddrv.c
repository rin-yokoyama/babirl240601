/* babirl/devtool/ddrv.c
 * Sep 4, 2006
 *
 * Dummy driver
 * Data read from file, copy to shared memory
 *
 * Hidetada Baba (RIKEN)
 * baba@rarfaxp.riken.jp
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/file.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>

#define COMFIFO "/tmp/babirldbfifo"

int cfd;
FILE *fd;

void quit(void){
  int idx;

  DB(printf("ddrv: quit\n"));

  idx = -1;
  DB(printf("ddrv: put end-flag to fifo\n"));
  flock(cfd, LOCK_EX);
  write(cfd, (char *)&idx, sizeof(idx));
  flock(cfd, LOCK_UN);

  if(fd) fclose(fd);
  if(cfd) close(cfd);

  exit(0);
}

int main(int argc, char *argv[]){
  char *buff, *buffat;
  int efn, size;
  int shmid, idx;
  RIDFHD hd;
  RIDFRHD rhd;

  if(argc != 3){
    printf("ddrv EFN FILENAME\n");
    exit(0);
  }

  efn = strtol(argv[1], NULL, 0);
  if(efn < 1 || efn > 254){
    printf("0 < EFN < 255\n");
    exit(0);
  }

  if((cfd = open(COMFIFO, O_RDWR)) < 0){
    printf("Can't open COMFIFO\n");
    exit(1);
  }

  if((fd = fopen(argv[2], "r")) == NULL){
    perror("Can't open file\n");
    exit(1);
  }

  signal(SIGINT, (void *)quit);

  buff = 0;
  if((shmid = initshm(EFSHMKEY+efn, EF_SHM_SIZE, &buff)) == -1){
    printf("initshm failed  shmid = %d\n", shmid);
    exit(0);
  }
  if(!buff){
    printf("Can't allocate shared memory %p\n", buff);
    exit(0);
  }
  DB(printf("shmid = %d / addr = %p\n", shmid, buff));
  memset(buff, 0, EF_SHM_SIZE);

  idx = 0;
  while(!feof(fd)){
    if(buff[EF_SHM_FLAG1] && buff[EF_SHM_FLAG2]){
      DB(printf("Both buffer full\n"));
    }else{
      if(idx == 0 && !buff[EF_SHM_FLAG1]){
	DB(printf("idx %d : memcpy\n", idx));
	buffat = buff + EF_SHM_DATA1;
	*(buff + EF_SHM_FLAG1) = EF_SHM_READY1;
      }else if(idx == 1 && !buff[EF_SHM_FLAG2]){
	DB(printf("idx %d : memcpy\n", idx));
	buffat = buff + EF_SHM_DATA2;
	*(buff + EF_SHM_FLAG2) = EF_SHM_READY2;
      }else{
	printf("Invalid shared memory management\n");
	printf("idx   = %d\n", idx);
	printf("flag1 = %d\n", buff[EF_SHM_FLAG1]);
	printf("flag2 = %d\n", buff[EF_SHM_FLAG2]);
	break;
      }
      if(fread((char *)&hd, sizeof(hd), 1, fd) != 1){
	printf("End of file\n");
	break;
      }
      memcpy(buffat, (char *)&hd, sizeof(hd));
      rhd = ridf_dechd(hd);
      size = rhd.blksize - sizeof(hd)/WORDSIZE;
      if(fread(buffat+sizeof(hd), WORDSIZE, size, fd) != size){
	printf("fread error\n");
	break;
      }
      flock(cfd, LOCK_EX);
      write(cfd, (char *)&idx, sizeof(idx));
      flock(cfd, LOCK_UN);

      if(idx == 0){
	idx = 1;
      }else{
	idx = 0;
      }
    }
    getchar();
    //sleep(5);
    //usleep(100000);
    //usleep(100000);
  }

  fclose(fd);
  //free(buff);
  close(cfd);

  return 0;
}
