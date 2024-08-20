/* babirl/lib/rdf.c
 *
 * last modified : 08/07/01 12:48:46 
 *
 * Header file for RDF
 *
 * Hidetada Baba
 * baba@ribf.riken.jp
 *
 */

//! Maximum event number for 1 block
#define RDF_MAXEVENT 2000
//! Maximum segment number for 1 event
#define RDF_MAXSEGID 30

/*! Structure for rdf information */
struct rdf_blkinfost{
  //! Event number
  unsigned int evtn;
  //! Pointer for each event
  unsigned short *evtptr[RDF_MAXEVENT];
  //! Size of each event
  int evtsize[RDF_MAXEVENT];
  //! Pointer for each event and segment
  unsigned short *segptr[RDF_MAXEVENT][RDF_MAXSEGID];
  //! Size of each segment of each event
  int segsize[RDF_MAXEVENT][RDF_MAXSEGID];
  //! SegmentID of each segment of each event
  int segid[RDF_MAXEVENT][RDF_MAXSEGID];
};


//! Macro for extracting event size
#define RDF_EVTSIZE(x)    (x & 0x7fff)

//! Mask for event header
#define RDF_EVTHEAD_MASK  0x8000


// Prototype
int rdf_scanblk(unsigned short *idata, struct rdf_blkinfost *info,
		int blksize);
