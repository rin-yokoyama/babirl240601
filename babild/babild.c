/* Babild/babild.c
 * last modified : 17/01/13 14:08:52
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 * Event Builder
 *
 */

/* Version */
#define VERSION "1.4.1 May 15, 2024"

// #define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <pthread.h>

#include <bi-config.h>
#include <bi-common.h>
#include <ridf.h>
#include <bbxml.h>
#include "babild.h"
#include "ebstrcom.h"

#include "kafka.h"

int chkopt(int, char *[]);
void quit(void);
int daq_close();
int store_daqinfo();
int snd_ssminfo();
int efr_connect();
int esdisconnect();
int daq_start();
int daq_nssta();
int daq_stop();
int ssm_start();
int ssm_stop();
int daq_ender();
int cntefon(struct steflist *eflist);
int escommand_force(int com);
int escommand_pas(int com);

int babildxcom(char *buff, char *ret);
int xgetinitialize(char *ret);

unsigned int gloevtn;

char ssmerror[4096] = {0};
int crashefr[MAXEF] = {0};

int evtnbuff[1024];
int esidx = 1;

rd_kafka_t *rk = NULL;
rd_kafka_topic_t *topic = NULL;

/* Check option */
int chkopt(int argc, char *argv[])
{
  int val;
  while ((val = getopt(argc, argv, "hlv")) != -1)
  {
    switch (val)
    {
    case 'h':
      printf("Usage: e-builder [option] [EFN]\n");
      printf("\n");
      printf("Options\n");
      printf("  -h                  this message\n");
      printf("  -l                  create log file\n");
      printf("  -v                  version information\n");
      return 0;
      break;
    case 'l':
      vm = 1;
      printf("babild: log verbose mode\n");
      break;
    case 'v':
      printf("babild version : %s\n", VERSION);
      printf("produced by Hidetada Baba\n");
      return 0;
      break;
    default:
      printf("Invalid option!!\n");
      return 0;
      break;
    }
  }
  return 1;
}

void lfz(char *e)
{
  int i;

  i = 0;
  while (*(e + i))
  {
    if (*(e + i) == '\n')
    {
      *(e + i) = 0;
      break;
    }
    i++;
  }
}

int eb_run_start(int com, char *err)
{
  int ret;

  if (cntefon(daqinfo.eflist))
  {
    if (!runinfo.runstat && !daqinfo.babildes)
    {
      if (com == EB_RUN_START)
      {
        if (daq_start())
        {
          if (ssminfo.ex && ssminfo.of)
            ssm_start();
          ret = 1;
        }
        else
        {
          ret = 0;
        }
      }
      else
      {
        if (daq_nssta())
        {
          if (ssminfo.ex && ssminfo.of)
            ssm_start();
          ret = 1;
        }
        else
        {
          ret = 0;
        }
      }
    }
    else
    {
      if (vm)
        lfprintf(lfd, "babild: already starting or babildes mode\n");
      sprintf(err, "already starting or babildes mode\n");
      ret = 0;
    }
  }
  else
  {
    if (vm)
      lfprintf(lfd, "babild: number of eflist.on = 0\n");
    sprintf(err, "number of eflist.on = 0\n");
    ret = 0;
  }
  if (ret)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

int eb_run_stop(char *err)
{
  int ret;
  if (runinfo.runstat && !daqinfo.babildes)
  {
    if (ssminfo.ex && ssminfo.of)
      ssm_stop();
    daq_stop();
    ret = 1;
  }
  else
  {
    if (vm)
      lfprintf(lfd, "babild: not starting or babildes mode\n");
    sprintf(err, "babild: not starting or babildes mode\n");
    ret = 0;
  }

  return ret;
}

int eb_run_close()
{
  int ret;
  if (runinfo.runstat == STAT_RUN_WAITSTOP)
  {
    daq_ender();
    ret = 1;
  }
  else
  {
    if (vm)
      lfprintf(lfd, "babild: not waiting stop\n");
    ret = 0;
  }
  return ret;
}

int eb_set_header(char *hd)
{
  if (!runinfo.runstat)
  {
    strncpy(runinfo.header, hd, sizeof(runinfo.header) - 1);
    return 1;
  }
  else
  {
    return 0;
  }
}

int eb_set_ender(char *ed)
{
  if (runinfo.runstat == STAT_RUN_WAITSTOP)
  {
    strncpy(runinfo.ender, ed, sizeof(runinfo.ender) - 1);
    return 1;
  }
  else
  {
    return 0;
  }
}

/* Quit sequence */
void quit(void)
{

  if (vm)
    lfprintf(lfd, "babild: -- quit function --\n");

  if (runinfo.runstat != STAT_RUN_IDLE)
  {
    if (vm)
      lfprintf(lfd, "babild: force quit during run\n");
    escommand_force(ES_RUN_STOP);
    escommand_pas(ES_RUN_STOP);
  }

  esdisconnect();

  if (slfd)
    close(slfd);
  if (tcpslfd)
    close(tcpslfd);

  if (ebffd)
  {
    if (vm)
      lfprintf(lfd, "babild: Close ebffd %d\n", ebffd);
    close(ebffd);
  }
  if (vm)
    lfprintf(lfd, "babild: Delete FIFO %s\n", EBFIFO);
  unlink(EBFIFO);

  if (comfd)
  {
    if (vm)
      lfprintf(lfd, "babild: Close comfd %d\n", comfd);
    close(comfd);
  }
  if (esfd)
  {
    if (vm)
      lfprintf(lfd, "babild: Close esfd %d\n", esfd);
    close(esfd);
  }
  if (ebdfd)
  {
    if (vm)
      lfprintf(lfd, "babild: Close ebdfd %d\n", ebdfd);
    close(ebdfd);
  }
  if (infd)
  {
    if (vm)
      lfprintf(lfd, "babild: Close infd %d\n", infd);
    close(infd);
  }

  if (ebbuf.data)
    free(ebbuf.data);

  // if(vm) printf("babild: Semaphore for FIFO delete\n");
  // semctl(semid, 0, IPC_RMID, semunion);

  rmpid("babild");

  if (vm)
    lfprintf(lfd, "babild: Exit\n");

  if (lfd)
    fclose(lfd);

  exit(0);
}

int addaliasname(char *aliasname)
{
  if (aliasnamen < 10)
  {
    DB(printf("add aliasnames[%d] %s\n", aliasnamen, aliasname));
    strncpy(aliasnames[aliasnamen], aliasname, sizeof(aliasnames[aliasnamen]));
    aliasnamen++;

    return 1;
  }

  return 0;
}

/* malloc event builded buffer */
int ebbufmalloc(int size)
{

  /* if(ebbuf.data) free(ebbuf.data); */

  /* malloc event builded buffer */
  if (!(ebbuf.data = malloc(size)))
  {
    if (vm)
      lfprintf(lfd, "ebbufmalloc: Can't malloc ebbuf size=%d\n", size);
    return 0;
  }
  if (vm)
    lfprintf(lfd, "babild: ebbuf malloc add=%p, size=%d\n", ebbuf.data, size);

  return 1;
}

/* ssm series */
int ssmcon(void)
{
  int ssmsock;

  if (!(ssmsock = mktcpsend_tout(ssminfo.host, SSMCOMPORT, 2)))
  {
    DB(printf("babild: Can't connect to babissm.\n"));
    return 0;
  }

  return ssmsock;
}

int ssm_start(void)
{
  int sock, com, len, ret;

  snd_ssminfo();

  if (!(sock = ssmcon()))
  {
    if (vm)
      lfprintf(lfd, "babild: ssm_start: Can't connect ssm\n");
    return 0;
  }

  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  com = SSM_START;
  send(sock, (char *)&com, sizeof(com), 0);
  recv(sock, (char *)&ret, sizeof(ret), MSG_WAITALL);
  close(sock);

  return 1;
}

int ssm_stop(void)
{
  int sock, com, len, ret;

  snd_ssminfo();

  if (!(sock = ssmcon()))
  {
    return 0;
  }

  len = sizeof(com);
  send(sock, (char *)&len, sizeof(len), 0);
  com = SSM_STOP;
  send(sock, (char *)&com, sizeof(com), 0);
  recv(sock, (char *)&ret, sizeof(ret), MSG_WAITALL);
  if (ret > 4)
  {
    DB(printf("Receive error message size=%d\n", ret));
    recv(sock, ssmerror, ret, MSG_WAITALL);
  }
  close(sock);

  usleep(100000);

  return 1;
}

/* Count eflist.on */
int cntefon(struct steflist *eflist)
{
  int i, cnt = 0;

  for (i = 0; i < MAXEF; i++)
  {
    if (eflist[i].ex)
    {
      if (eflist[i].of == EB_EFLIST_ON)
      {
        cnt++;
      }
    }
  }

  return cnt;
}

int xmlcomment(int mode)
{
  FILE *fd;
  time_t now;
  char buff[EB_BUFF_SIZE];
  char *pcom, *txt;
  RIDFHDCOM comhd;
  int i, len, wlen, flag = 1;
  int sz, bsz, idx;
  RIDFHDEOB eobhd;
  RIDFHD ghd;
  BBXMLEL *xml;
  long long int trst;
  int tinhd[2];

  memset(xmlcommentbuff, 0, sizeof(xmlcommentbuff));

  DB(printf("xmlcomment: mode=%d\n", mode));
  if (mode == BABILD_COMMENT_START)
  {
    pcom = statcom.start;
  }
  else
  {
    pcom = statcom.stop;
  }

  // for XML Status
  if (statcom.of)
  {
    memset(buff, 0, sizeof(buff));
    len = 1;
    if ((fd = popen(pcom, "r")) == NULL)
    {
      if (vm)
        lfprintf(lfd, "Can't open statcom %s\n", pcom);
      flag = 0;
      DB(printf("xmlcomment: Can't open %s\n", pcom));
    }
    else
    {
      fread(buff, 1, sizeof(buff) - 1, fd);
      len = strlen(buff) + 1;
      DB(printf("xmlcomment: opend %s len=%d\n", pcom, len));
      if (pclose(fd) < 0)
      {
        if (vm)
          lfprintf(lfd, "Can't open statcom %s\n", pcom);
        flag = 0;
        DB(printf("xmlcomment: error in close\n"));
      }
    }

    DB(printf("xmlcomment: flag=%d\n", flag));
    // user status command should have the length more than 4
    if (!flag || len < 4)
    {
      sprintf(buff, "<babild><runstatus><error>Can't open statcom command %s</error></runstatus></babild>", pcom);
      len = strlen(buff) + 1; // +1 is to include last \0 charcter
    }
    else
    {
      // check for parameters
      // <timestamp>
      //   <resetcount>12</resetcount>
      // </timestamp>
      // timestamp-resetcount is additional bits of timestamp

      xml = NULL;
      xml = bbxml_parsebuff(buff, len);
      txt = NULL;
      if (xml)
      {
        txt = bbxml_getTextByTagName(xml, "resetcount\0");
      }
      if (txt)
      {
        trst = strtoll(txt, NULL, 0);
        trst = trst & 0x0000ffff; // mask lower 16bits
        if (vm)
          lfprintf(lfd, "timestamp resetount = %d\n", trst);
        tsrst = trst << 48;
      }
      bbxml_free(xml);
    }

    // writting wordsize have to be even number
    wlen = (len + 1) / WORDSIZE;

    time(&now);
    if (BABILD_COMMENT_START)
    {
      comhd = ridf_mkhd_com(RIDF_LY1, RIDF_STATUS,
                            wlen + sizeof(comhd) / WORDSIZE,
                            daqinfo.efn, (unsigned int)now,
                            RIDF_STATUS_START_XML);
    }
    else
    {
      comhd = ridf_mkhd_com(RIDF_LY1, RIDF_STATUS,
                            wlen + sizeof(comhd) / WORDSIZE,
                            daqinfo.efn, (unsigned int)now,
                            RIDF_STATUS_STOP_XML);
    }

    sz = sizeof(eobhd) / WORDSIZE;
    bsz = wlen + sizeof(comhd) / WORDSIZE + sizeof(ghd) / WORDSIZE + sz;
    eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sz, daqinfo.efn, bsz);

    /* Make global header */
    ghd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, bsz, daqinfo.efn);

    idx = 0;
    memcpy(xmlcommentbuff, (char *)&ghd, sizeof(ghd));
    idx += sizeof(ghd);
    memcpy(xmlcommentbuff + idx, (char *)&comhd, sizeof(comhd));
    idx += sizeof(comhd);
    memcpy(xmlcommentbuff + idx, buff, wlen * 2);
    idx += wlen * 2;
    memcpy(xmlcommentbuff + idx, (char *)&eobhd, sizeof(eobhd));
    idx += sizeof(eobhd);

    for (i = 0; i < MAXHD; i++)
    {
      if (hdfd[i])
      {
        // flock(fileno(hdfd[i]), LOCK_EX);
        fwrite(&ghd, 1, sizeof(ghd), hdfd[i]);
        fwrite(&comhd, 1, sizeof(comhd), hdfd[i]);
        fwrite(buff, 2, wlen, hdfd[i]);
        fwrite(&eobhd, 2, sz, hdfd[i]);
        // flock(fileno(hdfd[i]), LOCK_UN);
      }
    }

    tinhd[0] = EARECV_DATA;
    tinhd[1] = idx;
    send(infd, (char *)tinhd, sizeof(tinhd), 0);
    send(infd, xmlcommentbuff, tinhd[1], 0);
  }

  return 1;
}

/* Make comment */
int mkcomment(int mode)
{
  char runch[RIDF_COMMENT_RUNINFO_ASC_SIZE];
  char buff[EB_BUFF_SIZE], chst[64];
  int i, size, idx, bsize, sz;
  RIDFHDCOM comhd;
  RIDFHD hd;
  RIDFHDEOB eobhd;
  time_t ttime;
  struct tm *strtime;
  int tinhd[2];
  struct struninfo truninfo;

  memset(commentbuff, 0, sizeof(commentbuff));

  memcpy((char *)&truninfo, (char *)&runinfo, sizeof(truninfo));

  switch (mode)
  {
  case BABILD_COMMENT_START:
    // DB(printf("babild: mkcomment start\n"));
    memset(runch, 0, sizeof(runch));

    memset(buff, 0, sizeof(buff));
    size = sizeof(comhd);
    for (i = 0; i < MAXEF; i++)
    {
      if (daqinfo.eflist[i].ex)
      {
        size += sprintf(buff + size, "%3d %d %s %s\n",
                        i, daqinfo.eflist[i].of, daqinfo.eflist[i].name,
                        daqinfo.eflist[i].host);
      }
    }

    if (size / 2 * 2 != size)
    {
      buff[size] = 0;
      size++;
    }

    if (daqinfo.babildes)
    {
      if (!strlen(runinfo.header))
      {
        sprintf(runinfo.header, "started with babildes");
      }
    }

    comhd = ridf_mkhd_com(RIDF_LY1, RIDF_COMMENT, size / WORDSIZE,
                          daqinfo.efn, runinfo.starttime,
                          RIDF_COMMENT_EFINFO_ASC);
    memcpy(buff, (char *)&comhd, sizeof(comhd));

    comhd = ridf_mkhd_com(RIDF_LY1, RIDF_COMMENT,
                          RIDF_COMMENT_RUNINFO_ASC_SIZE / WORDSIZE,
                          daqinfo.efn, runinfo.starttime,
                          RIDF_COMMENT_RUNINFO_ASC);
    memcpy(runch, (char *)&comhd, sizeof(comhd));
    idx = sizeof(comhd);
    sprintf(runch + idx, "%s", daqinfo.runname);
    idx += 100;
    sprintf(runch + idx, "%04d", daqinfo.runnumber);
    idx += 100;
    ttime = runinfo.starttime;
    strtime = localtime(&ttime);
    strftime(chst, sizeof(chst), "START => %X", strtime);
    sprintf(runch + idx, "%s", chst);
    idx += 20;
    sprintf(runch + idx, "RUNNING");
    idx += 20;
    ttime = runinfo.starttime;
    strftime(chst, sizeof(chst), "%d-%b-%y", strtime);
    sprintf(runch + idx, "%s", chst);
    idx += 60;
    sprintf(runch + idx, "%s", runinfo.header);

    bsize = (sizeof(hd) + size + sizeof(runch) + sizeof(eobhd)) / WORDSIZE;
    hd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, bsize, daqinfo.efn);
    eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sizeof(eobhd) / WORDSIZE,
                          daqinfo.efn, bsize);

    idx = 0;
    memcpy(commentbuff, &hd, sizeof(hd));
    idx += sizeof(hd);
    memcpy(commentbuff + idx, runch, sizeof(runch));
    idx += sizeof(runch);
    memcpy(commentbuff + idx, buff, size);
    idx += size;
    memcpy(commentbuff + idx, &eobhd, sizeof(eobhd));
    idx += sizeof(eobhd);

    for (i = 0; i < MAXHD; i++)
    {
      if (hdfd[i])
      {
        // DB(printf("babild: writing hdfd=%d\n", i));
        //  Lock
        // flock(fileno(hdfd[i]), LOCK_EX);
        //  Write block header
        fwrite(commentbuff, 1, idx, hdfd[i]);
        // Unlock
        // flock(fileno(hdfd[i]), LOCK_UN);
      }
    }

    tinhd[0] = EARECV_DATA;
    tinhd[1] = idx;
    send(infd, (char *)tinhd, sizeof(tinhd), 0);
    send(infd, commentbuff, tinhd[1], 0);

    xmlcomment(mode);

    truninfo.runstat = STAT_RUN_START;

    break;
  case BABILD_COMMENT_STOP:
    // DB(printf("babild: mkcomment stop\n"));
    if (daqinfo.babildes)
    {
      if (!strlen(runinfo.ender))
      {
        sprintf(runinfo.ender, "stopped with babildes mode");
      }
    }

    memset(runch, 0, sizeof(runch));
    comhd = ridf_mkhd_com(RIDF_LY1, RIDF_COMMENT,
                          RIDF_COMMENT_RUNINFO_ASC_SIZE / WORDSIZE,
                          daqinfo.efn, runinfo.starttime,
                          RIDF_COMMENT_RUNINFO_ASC);
    memcpy(runch, (char *)&comhd, sizeof(comhd));
    idx = sizeof(comhd);
    sprintf(runch + idx, "%s", daqinfo.runname);
    idx += 100;
    sprintf(runch + idx, "%04d", daqinfo.runnumber);
    idx += 100;
    ttime = runinfo.starttime;
    strtime = localtime(&ttime);
    strftime(chst, sizeof(chst), "START => %X", strtime);
    sprintf(runch + idx, "%s", chst);
    idx += 20;
    ttime = runinfo.stoptime;
    strtime = localtime(&ttime);
    strftime(chst, sizeof(chst), "STOP => %X", strtime);
    sprintf(runch + idx, "%s", chst);
    idx += 20;
    ttime = runinfo.starttime;
    strftime(chst, sizeof(chst), "%d-%b-%y", strtime);
    sprintf(runch + idx, "%s", chst);
    idx += 60;
    sprintf(runch + idx, "%s", runinfo.header);
    idx += 100;
    sprintf(runch + idx, "%s", runinfo.ender);

    xmlcomment(mode);

    idx = sizeof(hd);
    memcpy(commentbuff + idx, runch, sizeof(runch));

    for (i = 0; i < MAXHD; i++)
    {
      if (hdfd[i])
      {
        // flock(fileno(hdfd[i]), LOCK_EX);
        fseeko(hdfd[i], sizeof(hd), SEEK_SET);
        fwrite(runch, 1, sizeof(runch), hdfd[i]);
        // flock(fileno(hdfd[i]), LOCK_UN);
      }
    }

    tinhd[0] = EARECV_DATA;
    tinhd[1] = idx;
    send(infd, (char *)tinhd, sizeof(tinhd), 0);
    send(infd, commentbuff, tinhd[1], 0);

    truninfo.runstat = STAT_RUN_IDLE;
    break;

  case BABILD_COMMENT_NSSTA:
    truninfo.runstat = STAT_RUN_NSSTA;
    truninfo.runnumber = 0;
    sprintf(truninfo.header, "nssta");
    sprintf(truninfo.ender, "  ");
    break;
  case BABILD_COMMENT_NSSTO:
    truninfo.runstat = STAT_RUN_IDLE;
    truninfo.runnumber = 0;
    sprintf(truninfo.header, "nssta");
    sprintf(truninfo.ender, "nssto");
    break;
  }

  /* send runstat to earecv */
  memset(commentbuff, 0, sizeof(commentbuff));
  idx = 0;

  sz = sizeof(hd) + sizeof(comhd) + sizeof(daqinfo) + sizeof(truninfo);
  hd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, sz / WORDSIZE, daqinfo.efn);
  comhd = ridf_mkhd_com(RIDF_LY1, RIDF_STATUS, sz / WORDSIZE,
                        daqinfo.efn, runinfo.starttime,
                        RIDF_STATUS_DAQRUNINFO);
  memcpy(commentbuff, (char *)&hd, sizeof(hd));
  idx += sizeof(hd);
  memcpy(commentbuff + idx, (char *)&comhd, sizeof(comhd));
  idx += sizeof(comhd);
  memcpy(commentbuff + idx, (char *)&daqinfo, sizeof(daqinfo));
  idx += sizeof(daqinfo);
  memcpy(commentbuff + idx, (char *)&truninfo, sizeof(truninfo));

  tinhd[0] = EARECV_DATA;
  tinhd[1] = sz;
  send(infd, (char *)tinhd, sizeof(tinhd), 0);
  send(infd, commentbuff, tinhd[1], 0);

  return 1;
}

/* Event Built Block */
int ebblock(void)
{
  RIDFHD ghd;
  int i, tinhd[2], len;
  int sz, bsz, pt;
  RIDFHDEOB eobhd;
  RIDFHDBLKN blknhd;
  unsigned long long int ngig;

  sz = sizeof(blknhd) / WORDSIZE;
  pt = sizeof(RIDFHD);
  blknhd = ridf_mkhd_blkn(RIDF_LY1, RIDF_BLOCK_NUMBER, sz, daqinfo.efn, blkn);
  memcpy(ebbuf.data + pt, (char *)&blknhd, sizeof(blknhd));

  sz = sizeof(eobhd) / WORDSIZE;
  bsz = ebbuf.pt + sz;

  eobhd = ridf_mkhd_eob(RIDF_LY1, RIDF_END_BLOCK, sz, daqinfo.efn, bsz);
  memcpy(ebbuf.data + ebbuf.pt * WORDSIZE, (char *)&eobhd, sizeof(eobhd));

  /* Make global header */
  ghd = ridf_mkhd(RIDF_LY0, RIDF_EA_BLOCK, bsz, daqinfo.efn);
  memcpy(ebbuf.data, (char *)&ghd, sizeof(ghd));

  DB(printf("babild: ebblock size=%d\n", ridf_sz(ghd)));
  DB(printf("babild: blkn = %d\n", blkn));

  blkn++;

  /* Store data to storage */
  if (runinfo.runstat == STAT_RUN_START)
  {
    for (i = 0; i < MAXHD; i++)
    {
      if (mxfd[i])
      {
        // flock(fileno(hdfd[i]), LOCK_EX);
        fwrite(ebbuf.data, 2, bsz, mxfd[i]); // Write into HD
                                             // flock(fileno(hdfd[i]), LOCK_UN);
      }
    }
    if (rk)
    {
      kafka_produce(topic, 2 * bsz, ebbuf.data);
    }
  }

  /* Count total ebsize */
  totsize += bsz;
  tgigsize += bsz;

  /* Send buff data to babinfo */
  tinhd[0] = EARECV_DATA;
  tinhd[1] = bsz * WORDSIZE;
  send(infd, (char *)tinhd, sizeof(tinhd), 0);
  send(infd, ebbuf.data, tinhd[1], 0);

  /* Send buff data to master babild */
  if (daqinfo.babildes)
  {
    ghd = ridf_mkhd(RIDF_LY0, RIDF_EAF_BLOCK, bsz, daqinfo.efn);
    memcpy(ebbuf.data, (char *)&ghd, sizeof(ghd));
    if (!efrc.connect)
    {
      efr_connect();
    }
    if (efrc.connect)
    {
      DB(printf("send data to EFR from babildes\n"));
      len = bsz * WORDSIZE;
      send(efsock, (char *)&len, sizeof(len), 0);
      send(efsock, ebbuf.data, len, 0);
    }
  }

  /* Clear pointer */
  ebbuf.pt = (sizeof(ghd) + sizeof(blknhd)) / WORDSIZE;

  // check 2gig
  if (runinfo.runstat == STAT_RUN_START)
  {
    if (tgig)
    {
      ngig = (unsigned long long int)tgig * 500000000LL;
      if (tgigsize > ngig)
      {
        reopen_file();
        tgigsize = 0;
      }
    }
  }

  return 1;
}

int chkerhost(char *erhost)
{
  int i;

  if (!fchkerhost)
    return 1;

  for (i = 0; i < 10; i++)
  {
    if (!strcmp(erhost, aliasnames[i]))
      return 1;
  }
  if (!strcmp(erhost, myhost) ||
      !strcmp(erhost, mydom) ||
      !strcmp(erhost, myip))
  {
    return 1;
  }

  return 0;
}

void esclose(int essock[MAXEF])
{
  int i;
  for (i = 0; i < MAXEF; i++)
  {
    if (essock[i])
    {
      close(essock[i]);
      essock[i] = 0;
    }
  }
}

/* DAQ start/stop */
int escommand(int com)
{
  int i, j, len, rlen, runst;
  int essock[MAXEF], chkes;
  int ret[1024];
  int arg, tcom, tlen, chksscom = 0;
  char buff[64];

  DB(printf("babild: escommand com=%d\n", com));

  if (com == ES_RUN_START || com == ES_RUN_NSSTA || com == ES_RUN_STOP)
  {
    chksscom = 1;
  }

  chkes = 1;
  memset((char *)&essock, 0, sizeof(essock));
  for (j = 0; j < MAXEF; j++)
  {
    if (com == ES_RUN_START || com == ES_RUN_NSSTA)
    {
      i = MAXEF - j - 1;
    }
    else
    {
      i = j;
    }
    if (daqinfo.eflist[i].ex)
    {
      // DB(printf("escommand: eflist[%d].ex = 1\n", i));
      if (com == ES_RUN_START || com == ES_RUN_NSSTA || com == ES_RUN_STOP || com == ES_GET_EVTN)
      {
        if (!daqinfo.eflist[i].of)
        {
          continue;
        }
        if (daqinfo.eflist[i].of == EB_EFLIST_PAS)
        {
          continue;
        }
      }
      if (!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT + i, estout)))
      {
        if (vm)
          lfprintf(lfd, "babild: Can't connet to babies id=%d.\n", i);
        if (daqinfo.eflist[i].of)
        {
          chkes = 0;
          snprintf(esret, sizeof(esret),
                   "can't connect to babies host=%s efn=%d",
                   daqinfo.eflist[i].host, i);
          esclose(essock);
          return chkes;
        }
      }
      else
      {
        /* Check erhost */
        if (chksscom)
        {
          // Check reseiver host
          tcom = ES_GET_CONFIG;
          tlen = sizeof(tcom);
          send(essock[i], (char *)&tlen, sizeof(tlen), 0);
          send(essock[i], (char *)&tcom, tlen, 0);
          recv(essock[i], (char *)&tlen, sizeof(tlen), MSG_WAITALL);
          recv(essock[i], (char *)&tefrc, tlen, MSG_WAITALL);
          if (!chkerhost(tefrc.erhost))
          {
            snprintf(esret, sizeof(esret),
                     "wrong erhost = %s in babies (host=%s efn=%d), shold be %s",
                     tefrc.erhost, daqinfo.eflist[i].host, i, myhost);
            esclose(essock);
            return 0;
          }
          else
          {
            DB(printf("%d chkerhost ok\n", i));
          }
          if (essock[i])
            close(essock[i]);
          if (!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT + i, estout)))
          {
            if (vm)
              lfprintf(lfd, "babild: Can't connet to babies id=%d (2nd).\n", i);
            if (daqinfo.eflist[i].of)
            {
              snprintf(esret, sizeof(esret),
                       "can't connect to babies host=%s efn=%d (2nd)",
                       daqinfo.eflist[i].host, i);
              esclose(essock);
              return 0;
            }
          }
          else
          {
            DB(printf("%d newsocket = %d\n", i, essock[i]));
          }

          if (fchkerhost)
          {
            // Check run status
            tcom = ES_GET_RUNSTAT;
            tlen = sizeof(tcom);
            send(essock[i], (char *)&tlen, sizeof(tlen), 0);
            send(essock[i], (char *)&tcom, tlen, 0);
            recv(essock[i], (char *)&tlen, sizeof(tlen), MSG_WAITALL);
            recv(essock[i], (char *)&runst, tlen, MSG_WAITALL);
            if (runst == -1)
            {
              snprintf(esret, sizeof(esret),
                       "babies (host=%s efn=%d) is blocked by others (e.g. vmestat)",
                       daqinfo.eflist[i].host, i);
              esclose(essock);
              return 0;
            }
            else
            {
              DB(printf("%d chkerhost ok\n", i));
            }
            if (essock[i])
              close(essock[i]);
            if (!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT + i, estout)))
            {
              if (vm)
                lfprintf(lfd, "babild: Can't connet to babies id=%d (2nd).\n", i);
              if (daqinfo.eflist[i].of)
              {
                snprintf(esret, sizeof(esret),
                         "can't connect to babies host=%s efn=%d (2nd)",
                         daqinfo.eflist[i].host, i);
                esclose(essock);
                return 0;
              }
            }
            else
            {
              DB(printf("%d newsocket = %d\n", i, essock[i]));
            }
          }
        }
      }
      // DB(printf("essock: %d = %d\n", i, essock[i]));
    }
  }

  for (j = 0; j < MAXEF; j++)
  {
    if (com == ES_RUN_START || com == ES_RUN_NSSTA)
    {
      i = MAXEF - j - 1;
    }
    else
    {
      i = j;
    }
    if (essock[i])
    {
      if (chkes)
      {
        len = sizeof(com);
        memcpy(buff, (char *)&com, sizeof(com));
        if (com == ES_RUN_START || com == ES_RUN_NSSTA)
        {
          if (daqinfo.eflist[i].of)
          {
            // for on and scr
            efrun[i] = 1;
            arg = ES_EF_ON;
          }
          else
          {
            // for off
            // This option is no meaning for now
            arg = ES_EF_OFF;
          }
          len += sizeof(arg);
          memcpy(buff + sizeof(com), (char *)&arg, sizeof(arg));
        }
        send(essock[i], (char *)&len, sizeof(len), 0);
        send(essock[i], buff, len, 0);
        recv(essock[i], (char *)&rlen, sizeof(rlen), MSG_WAITALL);
        recv(essock[i], (char *)ret, rlen, MSG_WAITALL);

        if (com == ES_GET_EVTN)
        {
          if (vm)
            lfprintf(lfd, "esevtn[%d] : %d\n", i, ret[0]);
        }
        close(essock[i]);
      }
      else
      {
        close(essock[i]);
      }
    }
  }

  return chkes;
}

/* force DAQ start/stop */
int escommand_force(int com)
{
  int i, j, len, rlen, fret = -1, plen = 0;
  int essock[MAXEF];
  int ret[1024];
  int arg;
  char buff[64];

  DB(printf("babild: escommand_force com=%d\n", com));

  memset((char *)&essock, 0, sizeof(essock));
  for (j = 0; j < MAXEF; j++)
  {
    if (com == ES_RUN_START || com == ES_RUN_NSSTA)
    {
      i = MAXEF - j - 1;
    }
    else
    {
      i = j;
    }
    if (daqinfo.eflist[i].ex)
    {
      if (com == ES_RUN_START || com == ES_RUN_NSSTA || com == ES_RUN_STOP || com == ES_GET_EVTN)
      {
        if (!daqinfo.eflist[i].of)
        {
          continue;
        }
      }
      if (daqinfo.eflist[i].of == EB_EFLIST_PAS)
      {
        continue;
      }
      if (!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT + i, estout)))
      {
        DB(printf("babild : force escommand cannot make socket to %s\n", daqinfo.eflist[i].host));
        fret = i;
      }
      else
      {
        // DB(printf("essock: %d = %d\n", i, essock[i]));
      }
    }
  }

  for (j = 0; j < MAXEF; j++)
  {
    if (com == ES_RUN_START || com == ES_RUN_NSSTA)
    {
      i = MAXEF - j - 1;
    }
    else
    {
      i = j;
    }
    if (essock[i])
    {
      len = sizeof(com);
      memcpy(buff, (char *)&com, sizeof(com));
      if (com == ES_RUN_START || com == ES_RUN_NSSTA)
      {
        if (daqinfo.eflist[i].of)
        {
          // for on and scr
          efrun[i] = 1;
          arg = ES_EF_ON;
        }
        else
        {
          // for off
          // This option is no meaning for now
          arg = ES_EF_OFF;
        }
        len += sizeof(arg);
        memcpy(buff + sizeof(com), (char *)&arg, sizeof(arg));
      }
      send(essock[i], (char *)&len, sizeof(len), 0);
      send(essock[i], buff, len, 0);
      recv(essock[i], (char *)&rlen, sizeof(rlen), MSG_WAITALL);
      recv(essock[i], (char *)ret, rlen, MSG_WAITALL);

      if (com == ES_GET_EVTN)
      {
        if (vm)
          lfprintf(lfd, "force esevtn[%d] : %d\n", i, ret[0]);
        DB(printf("escommand_force esidx=%d\n", esidx));

        if ((esidx * 4 + rlen + 8) < sizeof(evtnbuff))
        {
          if (plen > 4)
          {
            evtnbuff[esidx] = -2;
          }
          else
          {
            evtnbuff[esidx] = 0;
          }
          esidx++;
          evtnbuff[esidx] = i;
          esidx++;
          memcpy((char *)(evtnbuff + esidx), (char *)ret, rlen);
          esidx += rlen / 4;
        }
        plen = rlen;
      }

      close(essock[i]);
    }
    else
    {
      // close(essock[i]);
    }
  }

  return fret;
}

/* passive DAQ start/stop */
int escommand_pas(int com)
{
  int i, j, len, fret = -1;
  int essock[MAXEF];
  int arg;
  char *buff;

  DB(printf("babild: escommand_ps com=%d\n", com));
  buff = (char *)malloc(1024 * 100);

  if (com != ES_RUN_START &&
      com != ES_RUN_NSSTA &&
      com != ES_RUN_STOP)
  {
    return 0;
  }

  memset((char *)&essock, 0, sizeof(essock));
  for (j = 0; j < MAXEF; j++)
  {
    if (com == ES_RUN_START || com == ES_RUN_NSSTA)
    {
      i = MAXEF - j - 1;
    }
    else
    {
      i = j;
    }
    if (!daqinfo.eflist[i].ex)
    {
      continue;
    }
    if (daqinfo.eflist[i].of != EB_EFLIST_PAS)
    {
      continue;
    }

    if (!(essock[i] = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT + i, 1)))
    {
      fret = i;
    }
    else
    {
      // DB(printf("essock: %d = %d\n", i, essock[i]));
    }
  }

  for (j = 0; j < MAXEF; j++)
  {
    if (com == ES_RUN_START || com == ES_RUN_NSSTA)
    {
      i = MAXEF - j - 1;
    }
    else
    {
      i = j;
    }
    if (essock[i])
    {
      len = sizeof(com);
      memcpy(buff, (char *)&com, sizeof(com));
      arg = ES_EF_OFF;
      memcpy(buff + len, (char *)&arg, sizeof(arg));
      len += sizeof(arg);
      memcpy(buff + len, (char *)&daqinfo, sizeof(daqinfo));
      len += sizeof(daqinfo);
      memcpy(buff + len, (char *)&runinfo, sizeof(runinfo));
      len += sizeof(runinfo);

      send(essock[i], (char *)&len, sizeof(len), 0);
      send(essock[i], buff, len, 0);

      close(essock[i]);
    }
  }

  return fret;
}

/* DAQ start/stop */
int esdisconnect(void)
{
  int i, len;
  int essock;
  char buff[64];
  int com = ES_DIS_EFR;

  for (i = 0; i < MAXEF; i++)
  {
    essock = 0;
    if (daqinfo.eflist[i].ex && daqinfo.eflist[i].of)
    {
      if (!(essock = mktcpsend_tout(daqinfo.eflist[i].host, ESCOMPORT + i, 1)))
      {
        if (vm)
          lfprintf(lfd, "babild: force escommand : Can't connet to babies id=%d.\n", i);
      }
      else
      {
        len = sizeof(com);
        memcpy(buff, (char *)&com, sizeof(com));
        send(essock, (char *)&len, sizeof(len), 0);
        send(essock, buff, len, 0);
        close(essock);
      }
    }
  }

  return 1;
}

int reopen_file()
{
  char fname[256], ftime[64];
  int i;
  time_t now;
  struct tm *stm;

  tgign++;
  if (vm)
    lfprintf(lfd, "babild: reopen hdfiles number=%d\n", tgign);

  // ReOpen write file
  for (i = 0; i < MAXHD; i++)
  {
    if (hdfd[i])
    {
      if (tgigfd[i])
      {
        fclose(tgigfd[i]);
        tgigfd[i] = NULL;
      }
      sprintf(fname, "%s/%s%04d_%s%02d.ridf",
              daqinfo.hdlist[i].path, daqinfo.runname, daqinfo.runnumber,
              tgigftime, tgign);
      if (isfile(fname))
      {
        time(&now);
        stm = localtime(&now);
        strftime(ftime, sizeof(ftime), "%Y.%m.%d_%H:%M:%S", stm);
        sprintf(fname, "%s/%s%04d_%s_%s%02d.ridf",
                daqinfo.hdlist[i].path, daqinfo.runname, daqinfo.runnumber,
                ftime, tgigftime, tgign);
      }
      if (!(tgigfd[i] = fopen(fname, "w")))
      {
        if (vm)
          lfprintf(lfd, "babild: Can't Re open %s (reopen_file)\n", fname);
      }
      else
      {
        mxfd[i] = tgigfd[i];
        if (vm)
          lfprintf(lfd, "babild: Re Open HD[%d]=%s\n", i, fname);
      }
    }
  }

  return 1;
}

int daq_start()
{
  char fname[256], ftime[64];
  int i;
  time_t now;
  struct tm *stm;

  tgign = 0;
  daqinfo.runnumber++;
  runinfo.runnumber = daqinfo.runnumber;
  store_daqinfo();

  if (vm)
    lfprintf(lfd, "babild: daq_start runnumber=%d\n", daqinfo.runnumber);
  if (daqinfo.babildes && vm)
  {
    lfprintf(lfd, "babild: with babildes mode\n");
  }

  /* Start time */
  time(&now);
  runinfo.starttime = (int)now;
  gloevtn = 0;
  totsize = 0;
  tgigsize = 0;
  blkn = 0;

  memset(tgigftime, 0, sizeof(tgigftime));

  // Open write file
  for (i = 0; i < MAXHD; i++)
  {
    if (daqinfo.hdlist[i].ex && daqinfo.hdlist[i].of)
    {
      if (daqinfo.hdlist[i].ex == 2)
      {
        if (!rk)
          delete_kafka(rk, topic);
        char efn[4];
        char topicName[128];
        sprintf(efn, "%d", daqinfo.efn);
        sprintf(topicName, "ridf-%d", daqinfo.efn);
        init_kafka_producer(rk, topic, efn, daqinfo.hdlist[i].path, topicName);
        mxfd[i] = NULL;
      }
      else
      {
        sprintf(fname, "%s/%s%04d.ridf",
                daqinfo.hdlist[i].path, daqinfo.runname, daqinfo.runnumber);
        if (isfile(fname))
        {
          time(&now);
          stm = localtime(&now);
          strftime(ftime, sizeof(ftime), "%Y.%m.%d_%H:%M:%S", stm);
          sprintf(fname, "%s/%s%04d_%s.ridf",
                  daqinfo.hdlist[i].path, daqinfo.runname, daqinfo.runnumber,
                  ftime);
          sprintf(tgigftime, "%s_", ftime);
        }
        if (!(hdfd[i] = fopen(fname, "w")))
        {
          if (vm)
            lfprintf(lfd, "babild: Can't open %s (daq_start)\n", fname);
          mxfd[i] = NULL;
          return 0;
        }
        else
        {
          mxfd[i] = hdfd[i];
          if (vm)
            lfprintf(lfd, "babild: Open HD[%d]=%s\n", i, fname);
        }
      }
    }
  }

  // MT is not implemented, yet

  mkcomment(BABILD_COMMENT_START);

  if (escommand(ES_RUN_START))
  {
    runinfo.runstat = STAT_RUN_START;
    inhd[0] = EARECV_RUNINFO;
    inhd[1] = sizeof(runinfo);
    send(infd, (char *)&inhd, sizeof(inhd), 0);
    send(infd, (char *)&runinfo, sizeof(runinfo), 0);

    escommand_pas(ES_RUN_START);
  }
  else
  {
    return 0;
  }

  return 1;
}

int daq_nssta(void)
{
  time_t now;

  mkcomment(BABILD_COMMENT_NSSTA);
  if (escommand(ES_RUN_NSSTA))
  {
    /* Start time */
    time(&now);
    runinfo.starttime = (int)now;
    gloevtn = 0;
    totsize = 0;
    tgigsize = 0;
    blkn = 0;

    runinfo.runstat = STAT_RUN_NSSTA;
    inhd[0] = EARECV_RUNINFO;
    inhd[1] = sizeof(runinfo);
    send(infd, (char *)&inhd, sizeof(inhd), 0);
    send(infd, (char *)&runinfo, sizeof(runinfo), 0);

    escommand_pas(ES_RUN_NSSTA);
    if (vm)
      lfprintf(lfd, "babild: daq_nssta\n");
  }
  else
  {
    return 0;
  }

  return 1;
}

int daq_stop(void)
{
  int ret, len, ebn, fd;
  char lomes[1024] = {0};

  if (vm)
    lfprintf(lfd, "babild: daq_stop run=%d\n", runinfo.runnumber);

  ret = escommand(ES_RUN_STOP);
  escommand_pas(ES_RUN_STOP);

  if (ret != 1)
  {
    DB(printf("babild: error during ES_RUN_STOP\n"));
    ret = escommand_force(ES_RUN_STOP);
    DB(printf("ret = %d\n", ret));
    if (ret >= 0)
    {
      DB(printf(" daq_stop: # EFN %d is not responding \n", ret));
      sprintf(lomes, " # EFN %d is not responding ", ret);
      strcat(ssmerror, lomes);
    }

    for (ebn = 0; ebn < MAXEF; ebn++)
    {
      if (crashefr[ebn])
      {
        DB(printf("daq_stop: Force End of Run EFN=%d\n", ebn));
        if (vm)
          lfprintf(lfd, "daq_stop: Force End of Run EFN=%d\n", ebn);
        len = (ebn * -1) - 1;

        if ((fd = open(EBFIFO, O_RDWR)) == -1)
        {
          DB(printf("daq_stop: Can't open %s\n", EBFIFO));
        }
        else
        {
          pthread_mutex_lock(&ebfmutex);
          write(fd, (char *)&len, sizeof(len));
          pthread_mutex_unlock(&ebfmutex);
          close(fd);
        }
      }
    }
  }

  return 1;
}

int daq_close(void)
{
  int i;

  DB(printf("babild : daq_close \n"));

  // Close rawdata files
  for (i = 0; i < MAXHD; i++)
  {
    if (hdfd[i])
    {
      fclose(hdfd[i]);
      hdfd[i] = NULL;
      mxfd[i] = NULL;
    }
    if (tgigfd[i])
    {
      fclose(tgigfd[i]);
      tgigfd[i] = NULL;
    }
  }
  if (rk)
  {
    // Flush final messages
    rd_kafka_flush(rk, 10 * 1000); // Wait for max 10 seconds
    delete_kafka(rk, topic);
  }

  inhd[0] = EARECV_RUNINFO;
  inhd[1] = sizeof(runinfo);
  send(infd, (char *)&inhd, sizeof(inhd), 0);
  send(infd, (char *)&runinfo, sizeof(runinfo), 0);

  if (vm)
    lfprintf(lfd, "babild: daq stopped\n");

  escommand(ES_GET_EVTN); // for logging

  memset((char *)crashefr, 0, sizeof(crashefr));

  return 1;
}

int daq_ender(void)
{
  if (runinfo.runstat == STAT_RUN_WAITSTOP)
  {
    mkcomment(BABILD_COMMENT_STOP);
  }
  else
  {
    return 0;
  }

  runinfo.runstat = STAT_RUN_IDLE;
  daq_close();

  return 1;
}

/* Update babildes port */
int update_esport(int nefn)
{
  DB(printf("babild: update_esport %d\n", nefn));
  if (daqinfo.babildes)
  {
    if (nefn != efrc.efid)
    {
      if (esfd)
        close(esfd);
      efrc.efid = nefn;
      daqinfo.efn = efrc.efid;

      /* Make babildes port */
      if ((esfd = mktcpsock(ESCOMPORT + efrc.efid)) == -1)
        quit();
      if (vm)
        lfprintf(lfd, "babild: esfd number = %d\n", esfd);
      DB(printf("babild: new esport fd = %d\n", esfd));
    }
  }

  return efrc.efid;
}

/* Store daqinfo to babies.rc */
int store_daqinfo(void)
{
  FILE *fd;
  char initbuff[EB_BUFF_SIZE];

  if (!(fd = fopen(BABILDRC, "w")))
  {
    if (vm)
      lfprintf(lfd, "Can't open %s\n", BABILDRC);
    return 0;
  }
  fprintf(fd, "%s\n", daqinfo.runname);
  fprintf(fd, "%d\n", daqinfo.runnumber);
  fprintf(fd, "%d\n", daqinfo.ebsize);
  fprintf(fd, "%d\n", daqinfo.efn);
  fprintf(fd, "%d\n", daqinfo.babildes);
  if (vm)
    lfprintf(lfd, "babild: store_daqinfo: babild.rc updated\n");
  fclose(fd);

  inhd[0] = EARECV_DAQINFO;
  inhd[1] = sizeof(daqinfo);
  send(infd, (char *)&inhd, sizeof(inhd), 0);
  send(infd, (char *)&daqinfo, sizeof(daqinfo), 0);
  DB(printf("send daqinfo to ea\n"));

  if ((fd = fopen(BABILDRCXML, "w")))
  {
    DB(printf("babild: write xml initial parameters\n"));
    DB(printf("   fchkerhost %d\n", fchkerhost));
    memset(initbuff, 0, sizeof(initbuff));
    xgetinitialize(initbuff);
    fputs(initbuff, fd);
    fclose(fd);
  }
  else
  {
    DB(printf("babinfo: cannot write parameters\n"));
  }

  return 1;
}

/* Store mtlist */
int store_mtlist(void)
{
  FILE *fd;
  int i;

  if (!(fd = fopen(MTLIST, "w")))
  {
    printf("Can't open %s\n", MTLIST);
    return 0;
  }
  for (i = 0; i < MAXMT; i++)
  {
    if (daqinfo.mtlist[i].ex)
    {
      fprintf(fd, "%d %d %s %Lu\n", i, daqinfo.mtlist[i].of,
              daqinfo.mtlist[i].path, daqinfo.mtlist[i].maxsize);
    }
  }
  fclose(fd);

  return 1;
}

/* check disk size */
int update_disksize(void)
{
  int i;
  struct statvfs vfs = {0};

  for (i = 0; i < MAXHD; i++)
  {
    if (daqinfo.hdlist[i].ex == 1)
    {
      if (statvfs(daqinfo.hdlist[i].path, &vfs) >= 0)
      {
        daqinfo.hdlist[i].full = vfs.f_blocks * vfs.f_bsize;
        daqinfo.hdlist[i].free = vfs.f_bavail * vfs.f_bsize;
      }
    }
  }

  return 1;
}

/* Store hdlist */
int store_hdlist(void)
{
  FILE *fd;
  int i;

  if (!(fd = fopen(HDLIST, "w")))
  {
    printf("Can't open %s\n", HDLIST);
    return 0;
  }
  for (i = 0; i < MAXHD; i++)
  {
    if (daqinfo.hdlist[i].ex)
    {
      fprintf(fd, "%d %d %s %Lu\n", i, daqinfo.hdlist[i].of,
              daqinfo.hdlist[i].path, daqinfo.hdlist[i].maxsize);
    }
  }
  fclose(fd);

  update_disksize();

  return 1;
}

/* store statcom */
int store_statcom(void)
{
  FILE *fd;

  if (!(fd = fopen(STATCOMRC, "w")))
  {
    if (vm)
      lfprintf(lfd, "babild: can't open statcomrc = %d\n", STATCOMRC);
    return 0;
  }

  fprintf(fd, "%d\n", statcom.id);
  fprintf(fd, "%d\n", statcom.of);
  fprintf(fd, "%s\n", statcom.start);
  fprintf(fd, "%s\n", statcom.stop);
  fclose(fd);

  return 1;
}

/* Store eflist */
int store_eflist(void)
{
  FILE *fd;
  int i;

  DB(printf("babild: store_eflist\n"));
  if (!(fd = fopen(EFLIST, "w")))
  {
    printf("Can't open %s\n", EFLIST);
    return 0;
  }
  for (i = 0; i < MAXEF; i++)
  {
    if (daqinfo.eflist[i].ex)
    {
      fprintf(fd, "%d %d %s %s\n", i, daqinfo.eflist[i].of,
              daqinfo.eflist[i].name, daqinfo.eflist[i].host);
    }
  }
  fclose(fd);

  return 1;
}

/* Send ssminfo */
int snd_ssminfo(void)
{
  int sock, len, com, ret;

  if (ssminfo.ex)
  {
    if (!(sock = ssmcon()))
    {
      return 0;
    }
    len = sizeof(com) + sizeof(ssminfo);
    send(sock, (char *)&len, sizeof(len), 0);
    com = SSM_SET_SSMINFO;
    send(sock, (char *)&com, sizeof(com), 0);
    send(sock, (char *)&ssminfo, sizeof(ssminfo), 0);
    recv(sock, (char *)&ret, sizeof(ret), MSG_WAITALL);
    close(sock);
  }

  return 1;
}

/* Store ssminfo */
int store_ssminfo(void)
{
  FILE *fd;

  DB(printf("store_ssminfo\n"));
  if (!(fd = fopen(SSMINFO, "w")))
  {
    printf("Can't open %s\n", SSMINFO);
    return 0;
  }

  fprintf(fd, "%d %d %s\n", ssminfo.ex, ssminfo.of, ssminfo.host);
  DB(printf("ssminfo %d %d %s\n", ssminfo.ex, ssminfo.of, ssminfo.host));
  fprintf(fd, "%s\n", ssminfo.start);
  fprintf(fd, "%s\n", ssminfo.stop);
  fclose(fd);

  snd_ssminfo();

  return 1;
}

int eb_connect(void)
{
  struct sockaddr_in caddr;
  int tsock, clen;
  int ebn, ret;
  int opt[2];
  pthread_attr_t tattr;

  ret = 0;

  clen = sizeof(caddr);
  if ((tsock = accept(ebdfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0)
  {
    perror("Error in accept eb_connect\n");
    return 0;
  }
  recv(tsock, (char *)&ebn, sizeof(ebn), MSG_WAITALL);
  ebn = LEFN(ebn);
  if (ebn > 0 && ebn < MAXEF)
  {
    // DB(printf("babild: eb_connect ebn %d\n", ebn));
    ret = 1;
  }
  else
  {
    DB(printf("babild: eb_connect invalid ebn %d\n", ebn));
    ret = 0;
  }
  send(tsock, (char *)&ret, sizeof(ret), 0);

  if (ret)
  {
    DB(printf("Make EFR thread %d %d\n", ebn, tsock));
    gebn = ebn;
    gsock = tsock;
    opt[0] = ebn;
    opt[1] = tsock;

    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

    /* EFR thread */
    pthread_create(&efrthre[ebn], &tattr, (void *)efrmain, opt);
    pthread_setschedparam(efrthre[ebn], SCHED_RR, &efpar);
    pthread_attr_destroy(&tattr);
    // pthread_detach(efrthre[ebn]);
    usleep(1000);
  }
  else
  {
    close(tsock);
  }

  return 1;
}

/* EB command, get = return values */
void com_get(int sock, char *src, int len)
{
  send(sock, (char *)&len, sizeof(len), 0);
  send(sock, src, len, 0);
}

/* EB command, set = receive values and return values*/
void com_set(int sock, char *src, char *dest, int len)
{
  int ret;

  memcpy(dest, src + sizeof(int), len - sizeof(int));
  len = sizeof(ret);
  ret = 1;
  com_get(sock, (char *)&ret, len);
}

/* EB command, rejected */
void com_rej(int sock)
{
  int ret = 0, len;

  len = sizeof(ret);
  com_get(sock, (char *)&ret, len);
}

/* Communication port */
int commain(void)
{
  struct sockaddr_in caddr;
  char buff[EB_BUFF_SIZE], sret[EB_BUFF_SIZE];
  int sock, clen, len;
  int com, ret, tebsize;
  int ihd;

  clen = sizeof(caddr);
  if ((sock = accept(comfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0)
  {
    perror("Error in accept commain\n");
    return 0;
  }

  memset(buff, 0, sizeof(buff));
  recv(sock, (char *)&len, sizeof(int), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);

  /* babild command */
  memcpy((char *)&com, buff, sizeof(com));
  /* Check XML command */
  if (!strncmp((char *)&com, "<?", 2))
  {
    memset(sret, 0, sizeof(sret));
    pthread_mutex_lock(&xmlmutex);
    babildxcom(buff, sret);
    pthread_mutex_unlock(&xmlmutex);
    len = strlen(sret) + 1;
    com_get(sock, sret, len);
  }
  else
  {
    switch (com)
    {
    case EB_GET_DAQINFO:
      // DB(printf("babild: EB_GET_ALL\n"));
      update_disksize();
      len = sizeof(daqinfo);
      com_get(sock, (char *)&daqinfo, len);
      break;
    case EB_SET_DAQINFO:
      // DB(printf("babild: EB_SET_ALL\n"));
      if (!runinfo.runstat)
      {
        tebsize = daqinfo.ebsize;
        com_set(sock, buff, (char *)&daqinfo, len);
        if (tebsize != daqinfo.ebsize)
        {
          if (daqinfo.ebsize > EB_EFBLOCK_MAXSIZE)
          {
            daqinfo.ebsize = EB_EFBLOCK_MAXSIZE;
          }
          // ebbufmalloc(daqinfo.ebsize*WORDSIZE*2);
          if (vm)
            lfprintf(lfd, "babild: ebsize changed %d\n", daqinfo.ebsize);
        }
        runinfo.runnumber = daqinfo.runnumber;
        store_eflist();
        store_mtlist();
        store_hdlist();
        store_daqinfo();
        update_esport(daqinfo.efn);
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change daqinfo\n");
        com_rej(sock);
      }
      break;
    case EB_GET_RUNINFO:
      // DB(printf("babild: EB_GET_RUNINFO\n"));
      len = sizeof(runinfo);
      com_get(sock, (char *)&runinfo, len);
      break;
    case EB_SET_RUNINFO:
      // DB(printf("babild: EB_SET_RUNINFO\n"));
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&runinfo, len);
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change runinfo\n");
        com_rej(sock);
      }
      break;
    case EB_SET_HEADER:
      // DB(printf("babild: EB_SET_HEADER\n"));
      if (!runinfo.runstat)
      {
        com_set(sock, buff, runinfo.header, len);
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change runinfo\n");
        com_rej(sock);
      }
      break;
    case EB_SET_ENDER:
      // DB(printf("babild: EB_SET_ENDER\n"));
      if (runinfo.runstat == STAT_RUN_WAITSTOP)
      {
        com_set(sock, buff, runinfo.ender, len);
        DB(printf("ender = %s\n", runinfo.ender));
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now not waiting stop, can't change ender\n");
        com_rej(sock);
      }
      break;
    case EB_GET_EFNUM:
      // DB(printf("babild: EB_GET_EFNUM %d\n", efnum));
      len = sizeof(efnum);
      com_get(sock, (char *)&efnum, len);
      break;
    case EB_GET_EFLIST:
      // DB(printf("babild: EB_GET_EFLIST\n"));
      len = sizeof(daqinfo.eflist);
      com_get(sock, (char *)daqinfo.eflist, len);
      break;
    case EB_SET_EFLIST:
      // DB(printf("babild: EB_SET_EFLIST\n"));
      if (!runinfo.runstat)
      {
        len = sizeof(daqinfo.eflist);
        com_set(sock, buff, (char *)daqinfo.eflist, len);
        store_eflist();
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change eflist.\n");
        com_rej(sock);
      }
      break;
    case EB_GET_MTLIST:
      // DB(printf("babild: EB_GET_MTLIST\n"));
      len = sizeof(daqinfo.mtlist);
      com_get(sock, (char *)daqinfo.mtlist, len);
      break;
    case EB_SET_MTLIST:
      // DB(printf("babild: EB_SET_MTLIST\n"));
      if (!runinfo.runstat)
      {
        len = sizeof(daqinfo.mtlist);
        com_set(sock, buff, (char *)daqinfo.mtlist, len);
        store_mtlist();
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change mtlist.\n");
        com_rej(sock);
      }
      break;
    case EB_GET_HDLIST:
      // DB(printf("babild: EB_GET_HDLIST\n"));
      update_disksize();
      len = sizeof(daqinfo.hdlist);
      com_get(sock, (char *)daqinfo.hdlist, len);
      break;
    case EB_GET_EVTN:
      ret = gloevtn;
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
      break;
    case EB_GET_TOTSIZE:
      len = sizeof(totsize);
      com_get(sock, (char *)&totsize, len);
      break;
    case EB_SET_HDLIST:
      // DB(printf("babild: EB_SET_HDLIST\n"));
      if (!runinfo.runstat)
      {
        len = sizeof(daqinfo.hdlist);
        com_set(sock, buff, (char *)daqinfo.hdlist, len);
        for (ihd = 0; ihd < MAXHD; ++ihd)
        {
          if (contains_colon(daqinfo.hdlist[ihd].path))
          {
            daqinfo.hdlist[ihd].ex = 2;
          }
        }
        store_hdlist();
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change hdlist.\n");
        com_rej(sock);
      }
      break;
    case EB_GET_SSMINFO:
      // DB(printf("babild: EB_GET_SSMINFO\n"));
      len = sizeof(ssminfo);
      com_get(sock, (char *)&ssminfo, len);
      break;
    case EB_SET_SSMINFO:
      // DB(printf("babild: EB_SET_SSMINFO\n"));
      if (!runinfo.runstat)
      {
        len = sizeof(ssminfo);
        com_set(sock, buff, (char *)&ssminfo, len);
        store_ssminfo();
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change ssminfo.\n");
        com_rej(sock);
      }
      break;
    case EB_RUN_START:
    case EB_RUN_NSSTA:
      // DB(printf("babild: Run start\n"));
      if (cntefon(daqinfo.eflist))
      {
        if (!runinfo.runstat && !daqinfo.babildes)
        {
          if (com == EB_RUN_START)
          {
            if (daq_start())
            {
              if (ssminfo.ex && ssminfo.of)
                ssm_start();
              ret = 1;
            }
            else
            {
              ret = 0;
            }
          }
          else
          {
            if (daq_nssta())
            {
              if (ssminfo.ex && ssminfo.of)
                ssm_start();
              ret = 1;
            }
            else
            {
              ret = 0;
            }
          }
        }
        else
        {
          if (vm)
            lfprintf(lfd, "babild: already starting or babildes mode\n");
          if (runinfo.runstat)
          {
            sprintf(esret, "already starting");
          }
          else if (daqinfo.babildes)
          {
            sprintf(ssmerror, " this babild = babildes mode ");
          }
          ret = 0;
        }
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: number of eflist.on = 0\n");
        sprintf(esret, "number of eflist.on = 0\n");
        ret = 0;
      }
      if (ret)
      {
        len = sizeof(ret);
        com_get(sock, (char *)&ret, len);
      }
      else
      {
        memset(sret, 0, sizeof(sret));
        len = sizeof(ret) + strlen(esret) + 1;
        memcpy(sret, (char *)&ret, sizeof(ret));
        memcpy(sret + sizeof(ret), esret, strlen(esret));
        sret[len] = 0;
        com_get(sock, sret, len);
      }
      break;
    case EB_RUN_STOP:
      DB(printf("babild: EB_RUN_STOP (runstat=%d)\n", runinfo.runstat));

      if (runinfo.runstat && !daqinfo.babildes)
      {
        memset(ssmerror, 0, sizeof(ssmerror));
        if (ssminfo.ex && ssminfo.of)
          ssm_stop();
        daq_stop();

        ret = 1;
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: not starting or babildes mode\n");

        if (!runinfo.runstat)
        {
          sprintf(ssmerror, " runstatus=IDLE");
        }
        else if (daqinfo.babildes)
        {
          sprintf(ssmerror, " this babild = babildes mode ");
        }
        ret = 0;
      }
      memset(sret, 0, sizeof(sret));
      len = sizeof(ret);
      memcpy(sret, (char *)&ret, sizeof(ret));

      DB(printf("babild: EB_RUN_STOP ssmstrlen=%d ret=%d\n", (int)strlen(ssmerror), ret));
      if (strlen(ssmerror))
      {

        len += strlen(ssmerror) + 1;
        memcpy(sret + sizeof(ret), ssmerror, strlen(ssmerror));
        memcpy((char *)&ret, sret, 4);
        DB(printf("babild: ret=%d / %d\n", ret, len));
      }
      com_get(sock, sret, len);
      break;
    case EB_RUN_CLOSE:
      DB(printf("babild: Run close\n"));
      if (runinfo.runstat == STAT_RUN_WAITSTOP)
      {
        daq_ender();
        ret = 1;
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: not waiting stop\n");
        ret = 0;
      }
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
      break;
    case EB_CHK_DIR:
      // DB(printf("babild: EB_CHK_DIR\n"));
      ret = isdir(buff + sizeof(com));
      len = sizeof(ret);
      com_get(sock, (char *)&ret, len);
      break;
    case EB_SET_RUNNUMBER:
      // DB(printf("babild: EB_SET_RUNNUMBER\n"));
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&daqinfo.runnumber, len);
        runinfo.runnumber = daqinfo.runnumber;
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change runnumber\n");
        com_rej(sock);
      }
      break;
    case EB_SET_RUNNAME:
      // DB(printf("babild: EB_SET_RUNNAME\n"));
      if (!runinfo.runstat)
      {
        com_set(sock, buff, daqinfo.runname, len);
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change runname\n");
        com_rej(sock);
      }
      break;
    case EB_SET_EBSIZE:
      // DB(printf("babild: EB_SET_EBSIZE\n"));
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&daqinfo.ebsize, len);
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change ebsize\n");
        com_rej(sock);
      }
      break;
    case EB_SET_BABILDES:
      // DB(printf("babild: EB_SET_BABILDES\n"));
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&daqinfo.babildes, len);
        update_esport(daqinfo.efn);
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change babildes mode\n");
        com_rej(sock);
      }
      break;
    case EB_SET_CHKERHOST:
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&fchkerhost, len);
        store_daqinfo();
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: now running, can't change babildes mode\n");
        com_rej(sock);
      }
      break;
    case EB_GET_CHKERHOST:
      com_get(sock, (char *)&fchkerhost, sizeof(fchkerhost));
      break;
    case WHOAREYOU:
      DB(printf("babild : WHOAREYOU\n"));
      if (daqinfo.babildes)
      {
        strcpy(thisname, "babildes");
      }
      else
      {
        strcpy(thisname, "babild");
      }
      len = sizeof(thisname);
      com_get(sock, thisname, len);
      break;
    case EB_QUIT:
      DB(printf("babild: EB_QUIT\n"));
      if (!runinfo.runstat)
      {
        ret = 1;
        len = sizeof(ret);
        com_get(sock, (char *)&ret, len);
        escommand(ES_QUIT);
        quit();
      }
      else
      {
        DB(printf("babild: EB_QUIT but now running\n"));
        ret = 0;
        len = sizeof(ret);
        com_get(sock, (char *)&ret, len);
      }
      break;
    case EB_SET_STAT_COMMAND:
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&statcom, len);
        store_statcom();
      }
      else
      {
        ret = 0;
        len = sizeof(ret);
        com_get(sock, (char *)&ret, len);
      }
      break;
    case EB_GET_STAT_COMMAND:
      len = sizeof(statcom);
      com_get(sock, (char *)&statcom, len);
      break;
    case EB_SET_TGIG:
      if (!runinfo.runstat)
      {
        com_set(sock, buff, (char *)&tgig, len);
        store_daqinfo();
      }
      else
      {
        ret = 0;
        len = sizeof(ret);
        com_get(sock, (char *)&ret, len);
      }
      break;
    case EB_GET_TGIG:
      len = sizeof(tgig);
      com_get(sock, (char *)&tgig, len);
      break;
    case EB_GET_EFBLKN:
      len = sizeof(efblkn);
      pthread_mutex_lock(&blknmutex);
      com_get(sock, (char *)efblkn, len);
      pthread_mutex_unlock(&blknmutex);
      break;
    default:
      com_rej(sock);

      break;
    }
  }

  close(sock);
  return 1;
}

/*********************   Store EF to Memory    **********************/
int storeef(char *buff)
{
  int pt, strsz, flag;
  RIDFRHD ghd, thd;
  RIDFHD ighd, ihd;
  unsigned int loevtn = 0;
  long long int lots;
  int tpt, tsz;
  int vchkts;
  volatile int loop = 0;

  memcpy((char *)&ighd, buff, sizeof(ighd));
  ghd = ridf_dechd(ighd);

  DB(printf("New block ly=%d, ci=%d, sz=%d, en=%d\n",
            ghd.layer, ghd.classid, ghd.blksize, ghd.efn));
  if (ghd.classid != RIDF_EF_BLOCK && ghd.classid != RIDF_EAF_BLOCK)
  {
    if (vm)
      lfprintf(lfd, "babild: storeef: Invalid class id %d from efn %d \n",
               ghd.classid, ghd.efn);
    return 0;
  }

  strsz = 0;
  pt = sizeof(ighd) / WORDSIZE;
  while (pt < ghd.blksize)
  {
    if (loevtn && (loevtn > (gloevtn + 2000)))
    {
      // printf("too large loevtn %d / %d\n", loevtn, gloevtn);
      usleep(100);
      loop++;
      if (loop < 10)
      {
        continue;
      }
      else
      {
        loop = 0;
      }
    }

    memcpy((char *)&ihd, buff + (pt * WORDSIZE), sizeof(ihd));
    thd = ridf_dechd(ihd);

    if (thd.blksize <= 2)
    {
      if (vm)
      {
        lfprintf(lfd, "storeef: Invalid classid from efn=%d\n", ghd.efn);
        lfprintf(lfd, " ly=%d, ci=%d, sz=%d, en=%d\n",
                 thd.layer, thd.classid, thd.blksize, thd.efn);
      }
      break;
    }

    if (thd.classid == RIDF_EVENT || thd.classid == RIDF_EVENT_TS)
    {
      /* Store event data to evtmem */
      if (daqinfo.eflist[LEFN(thd.efn)].of == EB_EFLIST_ON)
      {
        memcpy((char *)&loevtn, buff + ((pt + sizeof(RIDFHD) / WORDSIZE) * WORDSIZE),
               sizeof(loevtn));
        if (thd.classid == RIDF_EVENT_TS)
        {
          /* for with time stamp case */
          tpt = pt * WORDSIZE + sizeof(RIDFHDEVTTS);
          tsz = thd.blksize * WORDSIZE - sizeof(RIDFHDEVTTS);
          memcpy((char *)&lots, buff + pt * WORDSIZE + sizeof(RIDFHDEVT),
                 sizeof(lots));
          lots &= 0x0000ffffffffffffLL;
          // DB(printf("storeef EVENT_TS efn=%d tsz=%d lots=%llu\n", thd.efn, tsz, lots));
        }
        else
        {
          tpt = pt * WORDSIZE + sizeof(RIDFHDEVT);
          tsz = thd.blksize * WORDSIZE - sizeof(RIDFHDEVT);
          lots = -1;
          DB(printf("storeef EVENT efn=%d tsz=%d\n", thd.efn, tsz));
        }

        pthread_mutex_lock(&memmutex);
        while (!newevt(buff + tpt, tsz, LEFN(thd.efn), loevtn, &flag))
        {
          if (vm)
          {
            lfprintf(lfd, "storeef: Waiting newevt evtn = %d\n", loevtn);
            lfprintf(lfd, "         efn = %d, size = %d, error flag = %d\n",
                     thd.efn,
                     thd.blksize * WORDSIZE - sizeof(RIDFHD) - sizeof(loevtn),
                     flag);
          }

          // skip evtn jump (evtn < first->evtn case)
          if (flag == MEM_NMALLOC_EVTN_JUMP)
          {
            if (vm)
              lfprintf(lfd, "storeef: newevt warning1 flag = %d, evtn = %d first=%d last=%d\n",
                       flag, loevtn, getfirstevtn(), getlastevtn());
            break;
          }

          pthread_mutex_unlock(&memmutex);
          usleep(50000);
          pthread_mutex_lock(&memmutex);
        }

        if (flag)
        {
          if (vm)
            lfprintf(lfd, "storeef: newevt warning2 flag = %d, evtn = %d first=%d last=%d\n",
                     flag, loevtn, getfirstevtn(), getlastevtn());
        }

        // DB((printf("evtn: efn(%d) = %d  (chkts=%d)\n", thd.efn, loevtn, chkts(loevtn))));
        if (thd.classid == RIDF_EVENT_TS && flag == 0)
        {
          /* for with time stamp case */
          vchkts = chkts(loevtn, thd.efn);
          if (vchkts == -1)
          {
            newts(loevtn, lots, thd.efn);
          }
          else if (vchkts == -2)
          {
            DB((printf("updatets evtn=%d, efn=%d\n", loevtn, thd.efn)));
            updatets(loevtn, lots, thd.efn);
          }
          else
          {
            // DB((printf("chkts failed \n")));
          }
        }
        pthread_mutex_unlock(&memmutex);
        strsz++;
      }
      else
      {
        if (vm)
          lfprintf(lfd, "event data get from non EB member=%d\n", thd.efn);
      }
      // DB(printf("efn=%d, evtn=%u\n", thd.efn, loevtn));
    }
    else if (thd.classid == RIDF_END_BLOCK)
    {
      /* noop */
    }
    else if (thd.classid == RIDF_BLOCK_NUMBER)
    {
      /* noop */
    }
    else
    {
      /* Store non event data to nevtmem */
      DB(printf("Non Event data id=%d, efn=%d\n", thd.classid, thd.efn));
      DB(printf(" ly=%d, ci=%d, sz=%d, en=%d\n",
                thd.layer, thd.classid, thd.blksize, thd.efn));
      pthread_mutex_lock(&nmemmutex);
      if (!newnevt(buff + pt * WORDSIZE, thd.blksize * WORDSIZE))
      {
        if (vm)
          lfprintf(lfd, "storeef: Fatal!! nevt can't sotre\n");
      }
      pthread_mutex_unlock(&nmemmutex);
      strsz++;
    }
    pt += thd.blksize;
  }
  return strsz;
}

/*******************  Slow Data Reseiver UDP Thread  ********************/
int slrmain(void)
{
  int sock, clen;
  DB(int len;)
  char buff[EB_UDP_BUFFSIZE];
  struct sockaddr_in saddr, caddr;
  int sln;

  clen = sizeof(caddr);
  sln = MAXEF + 100;

  if (!(sock = mkudpsock(SLRCVPORT, &saddr)))
  {
    printf("slrmain: Can't make slow data reseive port");
    quit();
  }

  /* Open FIFO FD */
  if ((slfd = open(EBFIFO, O_RDWR)) == -1)
  {
    printf("slrmain: Can't open %s\n", EBFIFO);
    quit();
  }

  while (1)
  {
    DB(len =)
    recvfrom(sock, buff, sizeof(buff), 0,
             (struct sockaddr *)&caddr, (socklen_t *)&clen);
    DB(printf("slrmain: recv slow data len=%d\n", len));
    if (runinfo.runstat)
    {
      if (storeef(buff))
      {
        pthread_mutex_lock(&ebfmutex);
        write(slfd, (char *)&sln, sizeof(sln));
        pthread_mutex_unlock(&ebfmutex);
      }
    }
  }

  return 0;
}

/*******************  Slow Data Reseiver UDP Thread  ********************/
int tcpslrmain(void)
{
  int sock, fd, clen;
  int len;
  char buff[EB_UDP_BUFFSIZE];
  struct sockaddr_in caddr;
  int sln;
  fd_set fdset;

  clen = sizeof(caddr);
  sln = MAXEF + 101;

  if (!(fd = mktcpsock(TCPSLRCVPORT)))
  {
    printf("tcpslrmain: Can't make slow data reseive port");
    return 0;
  }

  /* Open FIFO FD */
  if ((tcpslfd = open(EBFIFO, O_RDWR)) == -1)
  {
    printf("tcpslrmain: Can't open %s\n", EBFIFO);
    return 0;
  }

  clen = sizeof(caddr);
  sock = 0;
  while (1)
  {
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    if (select(fd + 1, &fdset, NULL, NULL, NULL) != 0)
    {
      if ((sock = accept(fd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0)
      {
        continue;
      }
      DB(printf("tcpslport accept %d\n", sock));
      recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
      recv(sock, buff, len, MSG_WAITALL);
      DB(printf("tcpslport recv size=%d %d\n", len, runinfo.runstat));
      for (clen = 0; clen < len; clen++)
      {
        printf("%x\n", buff[clen]);
      }

      if (runinfo.runstat)
      {
        if (storeef(buff))
        {
          pthread_mutex_lock(&ebfmutex);
          write(tcpslfd, (char *)&sln, sizeof(sln));
          pthread_mutex_unlock(&ebfmutex);
        }
      }
      if (sock)
        close(sock);
      sock = 0;
    }
  }

  return 0;
}

/*********************  Event Reseiver Thread  **********************/
int efrmain(void *opt)
{
  char *buff;
  int len, er;
  int ebn, sock;
  fd_set fdset;
  int ebffdt = 0;
  unsigned int loefblkn = 0;

  // ebn = gebn;
  // sock = gsock;
  memcpy((char *)&ebn, opt, 4);
  memcpy((char *)&sock, opt + 4, 4);

  buff = malloc(EB_EFBLOCK_BUFFSIZE);
  if (buff == NULL)
  {
    if (vm)
      lfprintf(lfd, "efrmain[%d]: Cannot malloc buff size=%d\n", ebn, EB_EFBLOCK_BUFFSIZE);
    return 0;
  }

  /* Open FIFO FD */
  if ((ebffdt = open(EBFIFO, O_RDWR)) == -1)
  {
    if (vm)
      lfprintf(lfd, "efrmain[%d]: Can't open %s\n", ebn, EBFIFO);
    quit();
  }
  if (vm)
    lfprintf(lfd, "efrmain[%d]: ebffd number = %d\n", ebn, ebffdt);
  DB(printf("efrmain[%d]: buffat=%p sock=%d\n", ebn, buff, sock));

  memset(buff, 0, EB_EFBLOCK_BUFFSIZE);

  while (1)
  {
    DB(printf("efrmain[%d] wait recv sock=%d\n", ebn, sock));
    if ((er = recv(sock, (char *)&len, sizeof(len), MSG_WAITALL)) < 1)
    {
      if (vm)
        lfprintf(lfd, "efrmain[%d] : Error (or closed babies) in recv er=%d\n", ebn, er);
      DB(printf("efrmain[%d]: EF%d closed\n", ebn, ebn));
      // close(sock);
      break;
    }
    DB(printf("efrmain[%d] len=%d\n", ebn, len));
    if (len > 0)
    {
      if ((er = recv(sock, buff, len, MSG_WAITALL)) < 1)
      {
        if (vm)
          lfprintf(lfd, "efrmain[%d] : Error in recv data er=%d\n", ebn, er);
      }
      loefblkn++;
      pthread_mutex_lock(&blknmutex);
      efblkn[ebn] = loefblkn;
      pthread_mutex_unlock(&blknmutex);

      DB(printf("efrmain[%d] : try storeef\n", ebn));
      if (storeef(buff))
      {
        DB(printf("efrmain[%d] : try mutexlock\n", ebn));
        FD_ZERO(&fdset);
        FD_SET(ebffdt, &fdset);
        if (select(ebffdt + 1, NULL, &fdset, NULL, NULL))
        {
          pthread_mutex_lock(&ebfmutex);
          DB(printf("efrmain[%d] : lock-in\n", ebn));
          write(ebffdt, (char *)&ebn, sizeof(ebn));
          pthread_mutex_unlock(&ebfmutex);
        }
      }
    }
    else
    {
      DB(printf("efrmain[%d]: End of Run EFN=%d\n", ebn, ebn));
      if (vm)
        lfprintf(lfd, "efrmain[%d]: End of Run EFN=%d\n", ebn, ebn);
      len = (ebn * -1) - 1;

      loefblkn = 0;
      pthread_mutex_lock(&ebfmutex);
      write(ebffdt, (char *)&len, sizeof(len));
      pthread_mutex_unlock(&ebfmutex);
    }
    usleep(1000);
  }

  DB(printf("Closing efrmain[%d]\n", ebn));

  if (runinfo.runstat != STAT_RUN_IDLE)
  {
    crashefr[ebn] = 1;
  }

  if (ebffdt)
    close(ebffdt);
  if (buff)
    free(buff);

  pthread_cancel(efrthre[ebn]);

  return 1;
}

/*********************   Event Build Thread    *********************/
int ebmain(void)
{
  int n, st, i, off;
  int evts, loevtn, nevts;
  fd_set ebfdset;
  RIDFHD thd;
  int ln, len;
  time_t now;
  long long int lots, wts;

  n = 0;

  /* Open FIFO FD */
  if ((ebffd = open(EBFIFO, O_RDWR)) == -1)
  {
    printf("ebmain: Can't open %s\n", EBFIFO);
    quit();
  }

  /* Main loop */
  while (1)
  {
    /* Clear fdsets */
    FD_ZERO(&ebfdset);
    FD_SET(ebffd, &ebfdset);
    DB(printf("ebmain: selecting\n"));
    /* wait for command through EBFIFO */
    if (select(ebffd + 1, &ebfdset, NULL, NULL, NULL) != 0)
    {
      DB(printf("ebmain: try lock\n"));
      pthread_mutex_lock(&ebfmutex);
      DB(printf("ebmain: try lock-in\n"));
      read(ebffd, (char *)&n, sizeof(n));
      pthread_mutex_unlock(&ebfmutex);
      DB(printf("ebmain: try lock-out n=%d\n", n));

      // printf("ebmain: data ready from %d\n", n);
      if (n > 0)
      {
        ln = 0;
        // DB(printf("ebmain: event builded start\n"));
        while (1)
        {
          /* Non event memory loop */
          pthread_mutex_lock(&nmemmutex);
          nevts = storenevt(ebbuf.data + ebbuf.pt * WORDSIZE);
          pthread_mutex_unlock(&nmemmutex);
          if (!nevts)
            break;
          // DB(printf("ebmain: nevt data stored\n"));
          pthread_mutex_lock(&nmemmutex);
          ncleanfirst();
          pthread_mutex_unlock(&nmemmutex);
          ebbuf.pt += nevts / WORDSIZE;
          if (ebbuf.pt > daqinfo.ebsize)
          {
            ebblock();
          }
        }

        if (n > MAXEF)
        {
          DB(printf("ebmain: store slow data only\n"));
        }
        else
        {
          // pthread_mutex_lock(&memmutex);
          while (1)
          {
            /* Event memory loop */
            pthread_mutex_lock(&memmutex);
            evts = mkefts(ebbuf.data + ebbuf.pt * WORDSIZE + sizeof(RIDFHDEVT),
                          daqinfo.eflist, &loevtn, &lots);
            pthread_mutex_unlock(&memmutex);

            if (lots == -1)
            {
              off = sizeof(RIDFHDEVT);
              // DB(printf("evt lots == -1 / off=%d\n", off));
            }
            else
            {
              off = sizeof(RIDFHDEVTTS);
              // DB(printf("evtts lots = %llu / off=%d\n", lots, off));
            }

            if (!evts)
              break;

            if (lots == -1)
            {
              /* w/o timestamp event header */
              // DB(printf("lots == -1 and memcpy\n"));
              thd = ridf_mkhd(RIDF_LY1, RIDF_EVENT, (evts + off) / WORDSIZE,
                              daqinfo.efn);
              memcpy(ebbuf.data + ebbuf.pt * WORDSIZE, (char *)&thd, sizeof(thd));
              memcpy(ebbuf.data + ebbuf.pt * WORDSIZE + sizeof(RIDFHD),
                     (char *)&loevtn, sizeof(loevtn));
            }
            else
            {
              /* with timestamp event header */
              // DB(printf("lots != -1 and memcpy\n"));
              thd = ridf_mkhd(RIDF_LY1, RIDF_EVENT_TS, (evts + off) / WORDSIZE,
                              daqinfo.efn);
              /* copy Event-header to Buff */
              memcpy(ebbuf.data + ebbuf.pt * WORDSIZE, (char *)&thd, sizeof(thd));
              /* copy Local-event-number to Buff */
              memcpy(ebbuf.data + ebbuf.pt * WORDSIZE + sizeof(RIDFHD),
                     (char *)&loevtn, sizeof(loevtn));
              /* include Reset counter of Time stamp */
              // wts = lots | tsrst; //not inmplement ts rst for the safety
              wts = lots;
              /* copy Local-time-stamp to Buff */
              memcpy(ebbuf.data + ebbuf.pt * WORDSIZE + sizeof(RIDFHDEVT),
                     (char *)&wts, sizeof(wts));
            }

            ebbuf.pt += (evts + sizeof(RIDFHDEVT)) / WORDSIZE;
            if (lots != -1)
            {
              ebbuf.pt += sizeof(lots) / WORDSIZE;
            }
            pthread_mutex_lock(&memmutex);
            cleanfirst(daqinfo.eflist);
            if (lots != -1)
              tscleanfirst();
            pthread_mutex_unlock(&memmutex);
            gloevtn = loevtn;
            if (ebbuf.pt > daqinfo.ebsize)
            {
              // pthread_mutex_unlock(&memmutex);
              ebblock();
              // pthread_mutex_lock(&memmutex);
            }
            ln++;
          }
          // pthread_mutex_unlock(&memmutex);

          DB(printf("ebmain: event builded evts = %d , memn=%d\n ", ln, getmemn()));
        }
      }
      else if (n < 0)
      {
        n = (n * -1) - 1;
        DB(printf("ebmain: End of run from %d\n", n));
        efrun[n] = 0;
        st = 0;
        for (i = 0; i < MAXEF; i++)
        {
          if (efrun[i])
            st++;
        }
        if (!st)
        {
          DB(printf("ebmain: All EF run stoped\n"));
          /* residual data */
          if (ebbuf.pt > sizeof(RIDFHD) / WORDSIZE)
          {
            ebblock();
          }
          pthread_mutex_lock(&memmutex);
          cleanall(daqinfo.eflist);
          tscleanall();
          pthread_mutex_unlock(&memmutex);

          time(&now);
          runinfo.stoptime = now;

          if (runinfo.runstat == STAT_RUN_START && !daqinfo.babildes)
          {
            runinfo.runstat = STAT_RUN_WAITSTOP;
          }
          else
          {
            if (runinfo.runstat == STAT_RUN_START && daqinfo.babildes)
            {
              mkcomment(BABILD_COMMENT_STOP);
            }
            else
            { // NSSTA
              mkcomment(BABILD_COMMENT_NSSTO);
            }
            runinfo.runstat = STAT_RUN_IDLE;
            daq_close();
            if (daqinfo.babildes)
            {
              len = -1;
              send(efsock, (char *)&len, sizeof(len), 0);
            }
          }
        }
      }
      else
      {
        printf("ebmain: Error n=%d\n", n);
      }
    }
  }

  close(ebffd);

  return 1;
}

/* Update efrc */
int update_efrc(void)
{
  FILE *fd;

  if ((fd = fopen(BABIESRC, "w")))
  {
    DB(printf("babies: paramter write to file\n"));
    fprintf(fd, "%s\n", efrc.erhost);
    fprintf(fd, "%d\n", efrc.hd1);
    fprintf(fd, "%s\n", efrc.hd1dir);
    fprintf(fd, "%d\n", efrc.hd2);
    fprintf(fd, "%s\n", efrc.hd2dir);
    fprintf(fd, "%d\n", efrc.mt);
    fprintf(fd, "%s\n", efrc.mtdir);
    fclose(fd);
  }
  else
  {
    DB(printf("baies: Can't open rcfile\n"));
    return 0;
  }

  return 1;
}

/* Initialize babies efrc (defalut settting) */
void init_efrc(void)
{
  efrc.runnumber = -1;
  efrc.erport = ERRCVPORT;
  efrc.comport = ESCOMPORT + efrc.efid;
  memset(efrc.erhost, 0, sizeof(efrc.erhost));
  strcpy(efrc.erhost, "localhost");
  efrc.hd1 = FALSE;
  strcpy(efrc.hd1dir, "./\0");
  efrc.hd2 = FALSE;
  strcpy(efrc.hd2dir, "./\0");
  efrc.mt = FALSE;
  strcpy(efrc.mtdir, "./\0");
  strcpy(efrc.runname, "\0");
  efrc.connect = 0;
}

int efr_connect(void)
{
  int ret;

  ret = 0;

  if (!efrc.connect)
  {
    /* Make data port */
    if (!(efsock = mktcpsend(efrc.erhost, efrc.erport)))
    {
      DB(printf("babild: Can't connect %s:%d\n", efrc.erhost, efrc.erport));
      return 0;
    }
    send(efsock, (char *)&efrc.efid, sizeof(efrc.efid), 0);
    recv(efsock, (char *)&ret, sizeof(ret), MSG_WAITALL);
    efrc.connect = 1;
    DB(printf("babild: EFR return = %d\n", ret));
  }
  else
  {
    DB(printf("babild: EFR already connected\n"));
    return 0;
  }

  return 1;
}

int efr_disconnect(void)
{
  if (efrc.connect)
  {
    close(efsock);
    efrc.connect = 0;
  }
  else
  {
    DB(printf("babies: EFR not connected\n"));
    return 0;
  }
  return 1;
}

int ermain(void)
{
  struct sockaddr_in caddr;
  char buff[BABIRL_COM_SIZE];
  int sock, clen, len;
  int com, ret, arg;

  clen = sizeof(caddr);
  if ((sock = accept(esfd, (struct sockaddr *)&caddr, (socklen_t *)&clen)) < 0)
  {
    perror("Error in accept ermain\n");
    return 0;
  }

  memset(buff, 0, sizeof(buff));
  recv(sock, (char *)&len, sizeof(len), MSG_WAITALL);
  recv(sock, buff, len, MSG_WAITALL);

  /* babies command */
  memcpy((char *)&com, buff, sizeof(com));

  switch (com)
  {
  case ES_SET_CONFIG:
    DB(printf("babild: ES_SET_CONFIG\n"));
    memcpy((char *)&efrc, buff + sizeof(com), sizeof(efrc));
    memset(buff, 0, sizeof(buff));
    ret = 1;
    memcpy(buff, (char *)&ret, sizeof(ret));
    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, buff, len, 0);
    update_efrc();
    break;
  case ES_GET_CONFIG:
    DB(printf("babild: ES_GET_CONFIG\n"));
    memset(buff, 0, sizeof(buff));
    memcpy(buff, (char *)&efrc, sizeof(efrc));
    len = sizeof(efrc);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, buff, len, 0);
    break;
  case ES_RUN_START:
    DB(printf("babild: ES_RUN_START\n"));
    memcpy((char *)&arg, buff + sizeof(com), sizeof(arg));
    if (arg == ES_EF_OFF && !fstart)
    {
      DB(printf("babies: Run started, but not joining event build\n"));
      break;
    }
    if (cntefon(daqinfo.eflist))
    {
      if (!runinfo.runstat)
      {
        eb_set_header(buff + sizeof(com) + sizeof(arg));
        daq_start();
        if (ssminfo.ex)
          ssm_start();
        ret = 1;
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: already starting\n");
        ret = 0;
      }
    }
    else
    {
      if (vm)
        lfprintf(lfd, "babild: number of eflist.on = 0\n");
      ret = 0;
    }

    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);

    break;
  case ES_RUN_NSSTA:
    DB(printf("babild: ES_NSRUN_START\n"));
    memcpy((char *)&arg, buff + sizeof(com), sizeof(arg));
    if (arg == ES_EF_OFF && !fstart)
    {
      DB(printf("babies: Run started, but not joining event build\n"));
      break;
    }

    if (cntefon(daqinfo.eflist))
    {
      if (!runinfo.runstat)
      {
        daq_nssta();
        if (ssminfo.ex && ssminfo.of)
          ssm_start();
        ret = 1;
      }
      else
      {
        if (vm)
          lfprintf(lfd, "babild: already starting\n");
        ret = 0;
      }
    }
    else
    {
      if (vm)
        lfprintf(lfd, "babild: number of eflist.on = 0\n");
      ret = 0;
    }

    len = sizeof(ret);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&ret, len, 0);

    break;
  case ES_RUN_STOP:
    DB(printf("babild: ES_RUN_STOP (runstat=%d)\n", runinfo.runstat));

    // DB(printf("babild: Run stop\n"));
    if (runinfo.runstat)
    {
      if (ssminfo.ex && ssminfo.of)
        ssm_stop();
      daq_stop();
      ret = 1;
    }
    else
    {
      if (vm)
        lfprintf(lfd, "babild: not starting\n");
      ret = 0;
    }
    len = sizeof(ret);
    com_get(sock, (char *)&ret, len);

    break;
  case ES_GET_EVTN:
    memset((char *)evtnbuff, 0, sizeof(evtnbuff));
    evtnbuff[0] = gloevtn;
    evtnbuff[1] = -1;
    esidx = 2;
    escommand_force(ES_GET_EVTN);
    len = esidx * 4;
    DB(printf("ES_GET_EVTN len=%d\n", len));
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, (char *)&evtnbuff, len, 0);
    break;
  case WHOAREYOU:
    DB(printf("babildes : WHOAREYOU\n"));
    if (daqinfo.babildes)
    {
      strcpy(thisname, "babildes");
    }
    else
    {
      strcpy(thisname, "babild");
    }
    len = sizeof(thisname);
    send(sock, (char *)&len, sizeof(len), 0);
    send(sock, thisname, len, 0);
    break;
  case ES_QUIT:
    DB(printf("babild: ES_QUIT\n"));
    // if(!sflag) quit();
    break;
  case ES_CON_EFR:
    DB(printf("babild: ES_CON_EFR\n"));
    if (!efr_connect())
    {
      DB(printf("babild: EFR connection faild\n"));
    }
    break;
  case ES_DIS_EFR:
    DB(printf("babild: ES_DIS_EFR\n"));
    if (!efr_disconnect())
    {
      DB(printf("babies: EFR disconnection faild\n"));
    }
  }

  close(sock);
  return 1;
}

/*********************         Main            *********************/
/*! Mail loop of babild */
int main(int argc, char *argv[])
{
  int maxfd = 0, i, tef, ti, tof, pid;
  u64 tmx;
  char tch[80], tch2[80], lfname[272];
  FILE *fd;
  int mton = 0;
  time_t now;
  char ftime[256], initbuff[EB_BUFF_SIZE];
  char sret[EB_BUFF_SIZE];
  struct tm *stm;

  if (sscanf(argv[argc - 1], "%d", &argefn) != 1)
  {
    argefn = 0;
  }

  /* PID file */
  if (chkpid("babild"))
  {
    printf("babild: Error, another babild may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babild\n");
    exit(0);
  }

#ifndef DEBUG
  if (!(int)getuid())
    nice(-20);
#ifndef NODAEMON
  daemon(1, 0);
#endif
#endif

  gloevtn = 0;

  /* Change working directory */
  /* BABIRLDIR = installed dir */
  // if(getenv("BABIRLDIR")){
  // chdir(getenv("BABIRLDIR"));
  // }else{
  chdir(BABIRLDIR);
  //}

  // To know my ipaddress
  getmyaddress(myhost, mydom, myip);

  /* Initialize */
  memset((char *)&daqinfo, 0, sizeof(daqinfo));
  ebbuf.data = NULL;
  ebbuf.pt = (sizeof(RIDFHD) + sizeof(RIDFHDBLKN)) / WORDSIZE;
  memset((char *)&efrc, 0, sizeof(efrc));
  memset((char *)&statcom, 0, sizeof(statcom));
  memset(aliasnames, 0, sizeof(aliasnames));

  for (i = 0; i < MAXEF; i++)
  {
    daqinfo.eflist[i].ex = 0;
    daqinfo.eflist[i].of = EB_EFLIST_OFF;
    ebffdt[i] = -1;
    efrun[i] = 0;
  }
  for (i = 0; i < MAXHD; i++)
  {
    hdfd[i] = NULL;
    mxfd[i] = NULL;
    tgigfd[i] = NULL;
  }
  for (i = 0; i < MAXMT; i++)
  {
    mtfd[i] = NULL;
  }

  /* Check command line option */
  if (!chkopt(argc, argv))
  {
    exit(0);
  }

  /* PID file */
  if (!(pid = mkpid("babild")))
  {
    printf("babild: Error, another babild may be running.\n");
    printf(" If process is not exist, please delete PID file /var/run/babild\n");
  }

  /* Signal SIGINT -> quit() */
  signal(SIGINT, (void *)quit);

  if (vm)
  {
    time(&now);
    stm = localtime(&now);
    strftime(ftime, sizeof(ftime), "%Y.%m.%d_%H:%M:%S", stm);
    if (!isdir("log"))
    {
      mkdir("log", 0775);
    }

    sprintf(lfname, "log/babild%s.log", ftime);
    if (!(lfd = fopen(lfname, "w")))
    {
      printf("Can't make log file %s\n", lfname);
      quit();
    }
  }

  if (vm)
    lfprintf(lfd, "babild: Start PID=%d\n", pid);

  /* Make FIFO to communicate with babier */
  unlink(EBFIFO);
  if (mkfifo(EBFIFO, 0666) == -1)
  {
    perror("babild");
    printf("babild: Can't make %s\n", EBFIFO);
    quit();
  }
  if (vm)
    lfprintf(lfd, "babild: Make FIFO %s\n", EBFIFO);

  /* Make command port */
  if ((comfd = mktcpsock(EBCOMPORT)) == -1)
    quit();
  if (vm)
    lfprintf(lfd, "babild: comfd number = %d\n", comfd);
  if (comfd > maxfd)
    maxfd = comfd;

  /* Make EB data port */
  if ((ebdfd = mktcpsock(ERRCVPORT)) == -1)
    quit();
  if (vm)
    lfprintf(lfd, "babild: ebfd number = %d\n", ebdfd);
  if (ebdfd > maxfd)
    maxfd = ebdfd;

  /* Make babinfo sock */
  if (!(infd = mktcpsend("localhost", INFEAPORT)))
  {
    printf("Can't connect babinfo\n");
    quit();
  }
  if (vm)
    lfprintf(lfd, "babild: infd number = %d\n", infd);

  /* Event fragment */
  efnum = 0;
  if ((fd = fopen(EFLIST, "r")))
  {
    while (!feof(fd))
    {
      fscanf(fd, "%d %d %s %s\n", &tef, &tof, tch, tch2);
      if (tef >= 0 && tef <= MAXEF)
      {
        efnum++;
        daqinfo.eflist[tef].ex = 1;
        daqinfo.eflist[tef].of = tof;
        strncpy(daqinfo.eflist[tef].name, tch,
                sizeof(daqinfo.eflist[tef].name));
        strncpy(daqinfo.eflist[tef].host, tch2,
                sizeof(daqinfo.eflist[tef].host));
        if (vm)
          lfprintf(lfd, "EF %d : %s (%s)\n", tef,
                   daqinfo.eflist[tef].name, ofstr[daqinfo.eflist[tef].of]);
      }
    }
    fclose(fd);
  }

  if ((fd = fopen(MTLIST, "r")))
  {
    while (!feof(fd))
    {
      if (fscanf(fd, "%d %d %s %Lu", &ti, &tof, tch, &tmx) == 4)
      {
        if (ti >= 0 && ti < MAXMT)
        {
          daqinfo.mtlist[ti].ex = 1;
          daqinfo.mtlist[ti].of = tof;
          if (tmx < 1)
            tmx = MTMAXSIZE;
          daqinfo.mtlist[ti].maxsize = tmx;
          strcpy(daqinfo.mtlist[ti].path, tch);
          mton++;
          if (vm)
            lfprintf(lfd, "MT%d %d %s %Lu\n", ti, tof,
                     daqinfo.mtlist[ti].path,
                     daqinfo.mtlist[ti].maxsize);
        }
      }
    }
    fclose(fd);
  }
  if ((fd = fopen(HDLIST, "r")))
  {
    while (!feof(fd))
    {
      if (fscanf(fd, "%d %d %s %Lu", &ti, &tof, tch, &tmx) == 4)
      {
        if (ti >= 0 && ti < MAXHD)
        {
          daqinfo.hdlist[ti].ex = 1;
          daqinfo.hdlist[ti].of = tof;
          if (tmx < 1)
            tmx = HDMAXSIZE;
          daqinfo.hdlist[ti].maxsize = tmx;
          strcpy(daqinfo.hdlist[ti].path, tch);
          if (vm)
            lfprintf(lfd, "HD%d %d %s %Lu\n", ti, tof,
                     daqinfo.hdlist[ti].path,
                     daqinfo.hdlist[ti].maxsize);
        }
      }
    }
    fclose(fd);
  }
  else if (!mton)
  {
    daqinfo.hdlist[0].ex = 1;
    daqinfo.hdlist[0].of = 1;
    daqinfo.hdlist[0].maxsize = HDMAXSIZE;
    strcpy(daqinfo.hdlist[0].path, "./");
    if (vm)
      lfprintf(lfd, "HD%d %s %Lu\n", 0, daqinfo.hdlist[0].path,
               daqinfo.hdlist[0].maxsize);
  }

  if ((fd = fopen(BABILDRC, "r")))
  {
    fscanf(fd, "%s", daqinfo.runname);
    fscanf(fd, "%d", &daqinfo.runnumber);
    fscanf(fd, "%d", &daqinfo.ebsize);

    // Usual max event build buffer = 1MB, not 8MB
    if (daqinfo.ebsize > EB_EFBLOCK_MAXSIZE)
    {
      daqinfo.ebsize = EB_EFBLOCK_MAXSIZE;
    }
    fscanf(fd, "%d", &daqinfo.efn);
    fscanf(fd, "%d", &daqinfo.babildes);
    if (vm)
      lfprintf(lfd, "babild.rc: name=%s, runnumber=%d, ebsize=%d\n",
               daqinfo.runname, daqinfo.runnumber, daqinfo.ebsize);
    fclose(fd);
  }
  else
  {
    if (vm)
      lfprintf(lfd, "no babild.rc\n");
    strcpy(daqinfo.runname, "data");
    daqinfo.runnumber = 0;
    daqinfo.efn = 1;
    daqinfo.ebsize = EBDEFSIZE;
    daqinfo.babildes = 0;
  }

  if ((fd = fopen(STATCOMRC, "r")))
  {
    fscanf(fd, "%d", &statcom.id);
    fscanf(fd, "%d", &statcom.of);
    fscanf(fd, "%s", statcom.start);
    fscanf(fd, "%s", statcom.stop);
    fclose(fd);
  }
  else
  {
    memset((char *)&statcom, 0, sizeof(statcom));
  }

  if (argefn)
  {
    daqinfo.efn = argefn;
    if (vm)
      lfprintf(lfd, "babild: EFN is set %d by arg\n", daqinfo.efn);
  }

  init_efrc();
  if ((fd = fopen(BABIESRC, "r")))
  {
    DB(printf("babies: parameter from file\n"));
    fscanf(fd, "%s", efrc.erhost);
    fscanf(fd, "%d", &efrc.hd1);
    fscanf(fd, "%s", efrc.hd1dir);
    fscanf(fd, "%d", &efrc.hd2);
    fscanf(fd, "%s", efrc.hd2dir);
    fscanf(fd, "%d", &efrc.mt);
    fscanf(fd, "%s", efrc.mtdir);
    fclose(fd);
  }
  update_esport(daqinfo.efn);

  // XML Initialize
  if ((fd = fopen(BABILDRCXML, "r")))
  {
    DB(printf("babildxcom: read initial parameters\n"));
    memset(initbuff, 0, sizeof(initbuff));
    fread(initbuff, 1, sizeof(initbuff), fd);
    babildxcom(initbuff, sret);
    fclose(fd);
  }

  store_daqinfo();

  /* ssm */
  memset((char *)&ssminfo, 0, sizeof(ssminfo));
  DB(printf("before ssminfo open %s\n", SSMINFO));
  if ((fd = fopen(SSMINFO, "r")))
  {
    fscanf(fd, "%d %d %s\n", &ssminfo.ex, &ssminfo.of, ssminfo.host);
    DB(printf("ssminfo %d %d %s\n", ssminfo.ex, ssminfo.of, ssminfo.host));
    fgets(ssminfo.start, sizeof(ssminfo.start), fd);
    DB(printf("ssminfo start %s\n", ssminfo.start));
    fgets(ssminfo.stop, sizeof(ssminfo.stop), fd);
    DB(printf("ssminfo stop %s\n", ssminfo.stop));
    lfz(ssminfo.start);
    DB(printf("ssminfo start %s\n", ssminfo.start));
    lfz(ssminfo.stop);
    DB(printf("ssminfo stop %s\n", ssminfo.stop));
    fclose(fd);
  }
  store_ssminfo();

  runinfo.runnumber = daqinfo.runnumber;

  // ebbufmalloc(daqinfo.ebsize*WORDSIZE*2);
  ebbufmalloc(EB_EBBUFF_SIZE * 2);

  /* Event build thread */
  tcpslpar.sched_priority = sched_get_priority_max(SCHED_RR) - 4;
  ebpar.sched_priority = sched_get_priority_max(SCHED_RR) - 5;
  slpar.sched_priority = sched_get_priority_max(SCHED_RR) - 6;
  efpar.sched_priority = sched_get_priority_max(SCHED_RR) - 7;

  pthread_mutex_init(&ebfmutex, NULL);
  pthread_mutex_init(&memmutex, NULL);
  pthread_mutex_init(&nmemmutex, NULL);
  pthread_mutex_init(&xmlmutex, NULL);
  pthread_mutex_init(&blknmutex, NULL);
  pthread_create(&ebthre, NULL, (void *)ebmain, NULL);
  pthread_setschedparam(ebthre, SCHED_RR, &ebpar);
  pthread_create(&slthre, NULL, (void *)slrmain, NULL);
  pthread_setschedparam(slthre, SCHED_RR, &slpar);
  pthread_create(&tcpslthre, NULL, (void *)tcpslrmain, NULL);
  pthread_setschedparam(tcpslthre, SCHED_RR, &tcpslpar);
  pthread_detach(ebthre);
  pthread_detach(slthre);
  pthread_detach(tcpslthre);

  while (1)
  {
    /* Main loop */
    // DB(printf("babild: Main loop\n"));

    /* prepaire fd set for select() */
    FD_ZERO(&fdset);
    FD_SET(ebdfd, &fdset);
    FD_SET(comfd, &fdset);
    if (daqinfo.babildes)
    {
      FD_SET(esfd, &fdset);
    }
    maxfd = ebdfd;
    if (comfd > maxfd)
      maxfd = comfd;
    if (esfd > maxfd)
      maxfd = esfd;

    if (select(maxfd + 1, &fdset, NULL, NULL, NULL) != 0)
    {
      DB(printf("kita-----------!!\n"));
      /*
      if(FD_ISSET(ebdfd, &fdset)){
  DB(printf("babild: select from eb data port\n"));
  eb_connect();
      }else if(FD_ISSET(comfd, &fdset)){
  DB(printf("babild: select from communication port\n"));
  commain();
      }else if(FD_ISSET(esfd, &fdset)){
  DB(printf("babild: select from er port\n"));
  ermain();
      }
      */
      if (FD_ISSET(comfd, &fdset))
      {
        DB(printf("babild: select from communication port\n"));
        commain();
      }
      else if (FD_ISSET(esfd, &fdset))
      {
        DB(printf("babild: select from er port\n"));
        ermain();
      }
      else if (FD_ISSET(ebdfd, &fdset))
      {
        DB(printf("babild: select from eb data port\n"));
        eb_connect();
      }
    }
  }

  return 0;
}
