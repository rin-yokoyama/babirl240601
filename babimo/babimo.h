/* last modified : 08/05/23 14:01:34 
 *
 * babimo/babimo.h
 *
 * Header file for babimo.c
 *
 */

struct hdstatst hdst[MAXMTPT];
int hdstn;

int comfd = 0;
int udpsock = 0;

char udata[256] = {0};
