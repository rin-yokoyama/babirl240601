/* babirl/lib/rdf.c
 *
 * last modified : 06/12/26 17:32:54 
 *
 * RDF access library
 *
 * Hidetada Baba
 * baba@ribf.riken.jp
 *
 */

#include <stdio.h>
#include <string.h>

#include <rdf.h>

/** Scan location of events and segments
 * @param *idata Pointer of buffer including 1 block data
 * @param *info  Pointer of rdf_blkinfost
 * @param blksize Size of *idata (unit = short)
 * @return Event number
 */
int rdf_scanblk(unsigned short *idata, struct rdf_blkinfost *info,
		int blksize){
  int idx, segn;
  int tevts, evtend, evtmsk;

  for(idx=0;idx<4;idx++){
    if(idata[idx] != 0) return 0;            // Non event block
  }

  idx = 4;
  info->evtn = 0;

  while(idx < blksize){
    segn = 0;
    if(idata[idx] == 0xffff) break;
    evtmsk = idata[idx] & RDF_EVTHEAD_MASK;  // Check for event header
    tevts =  RDF_EVTSIZE(idata[idx]);        // Event size
    if(!tevts || !evtmsk) break;

    info->evtsize[info->evtn] = tevts;       // Event size storing
    info->evtptr[info->evtn] = idata+idx;    // Event pointer
    evtend = idx + tevts;
    idx += 3;

    while(idx < evtend){
      info->segsize[info->evtn][segn] = idata[idx];  // Segment size
      info->segptr[info->evtn][segn] = idata+idx;    // Segment pointer
      info->segid[info->evtn][segn] = idata[idx+1];  // Segment ID
      idx += info->segsize[info->evtn][segn];        // Jump next segment
      segn ++;
      if(segn >= RDF_MAXSEGID){
	printf("segn = %d\n", segn);
	printf("Plese change RDF_MAXSEGID size\n");
	break;
      }
    }

    info->evtn ++;
    if(info->evtn >= RDF_MAXEVENT){
      printf("evtn = %d\n", info->evtn);
      printf("Plese change RDF_MAXEVENT size\n");
      break;
    }
  }

  return info->evtn;
}

