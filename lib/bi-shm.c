/* babirl/lib/bi-shm.c
 * 
 * last modified : 07/01/11 16:45:55 
 * 
 * Hidetada Baba
 * baba@ribf.riken.jp
 *
 * Liabrary for shared memory and semaphore
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <bi-config.h>
#include <bi-common.h>


/** Semaphore lock.
 *  @param semid semaphore ID
 *  @param semb  semaphore structure
 */
void sem_p(int semid, struct sembuf *semb){
  semb->sem_op = -1;
  semop(semid, semb, 1);
}

/** Semaphore unlock.
 *  @param semid semaphore ID
 *  @param semb  semaphore structure
 */
void sem_v(int semid, struct sembuf *semb){
  semb->sem_op = 1;
  semop(semid, semb, 1);
}

/** Initialize shared memory.
 *  Automatically make shared memory and return its ID.
 *  @param key shared memory key
 *  @param size shared memory size
 *  @param pointer of shared memory buffer
 *  @return shmid shared memory ID (0 = error)
 */
int initshm(int key, int size, char **addr){
  int shmid = 0;

  while(!shmid){
    if((shmid = shmget(key, size, IPC_CREAT | 0777)) == -1){
      perror("Can't create shared memory.\n");
      return 0;
    }

    *addr = shmat(shmid, 0, 0);

    /* to avoid shmid == 0 */
    if(!shmid){
      delshm(shmid, *addr);
    }
  }

  return shmid;
}
   
/** Initialize sempahore.
 *  Automatically make semaphore and return its ID.
 *  @param key semaphore key
 *  @semunion semaphore union
 *  @return semid semaphore ID (0 = error)
 */
int initsem(int key, union semun *semunion){
  int semid = 0;

  while(!semid){
    if((semid = semget(key, 1, IPC_CREAT | 0666)) == -1){
      perror("Can't create semaphore.\n");
      return 0;
    }

    /* to avoid semid == 0 */
    if(semid){
      semunion->val = 1;
      if(semctl(semid, 0, SETVAL, *semunion) == -1){
	perror("Can't control semaphore.\n");
	delsem(semid, semunion);
	return 0;
      }
    }else{
      delsem(semid, semunion);
    }      
  }

  return semid;
}

/** Delete shared memory.
 *  @param semid shared memory ID
 *  @return 1 (0 = error)
 */
int delshm(int shmid, char *shmp){

  shmdt(shmp);

  if(shmctl(shmid, IPC_RMID, 0) < 0){
    return 0;
  }

  return 1;
}

/** Delete semaphore.
 *  @param semid semaphore ID
 *  @return 1 (0 = error)
 */
int delsem(int semid, union semun *semunion){

  if(semctl(semid, 0, IPC_RMID, *semunion) < 0){
    return 0;
  }

  return 1;
}
