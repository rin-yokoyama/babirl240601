/* ridf.c
 * 
 * last modified : 09/08/17 17:40:41 
 * 
 * RIDF access library
 *
 * Hidetada Baba (RIKEN)
 * baba@ribf.riken.jp
 *
 */

#include <stdio.h>
#include <string.h>

#include <ridf.h>

/** Extract layer from RIDFHD. 
 *  @param hd structure of RIDFHD
 *  @return layer number
 */
int ridf_ly(struct ridf_hdst hd){
  return RIDF_LY(hd.hd1);
}

/** Extract class id from RIDFHD.
 *  @param hd structure of RIDFHD
 *  @return class id
 */
int ridf_ci(struct ridf_hdst hd){
  return RIDF_CI(hd.hd1);
}

/** Extract size from RIDFHD.
 *  @param hd structure of RIDFHD
 *  @return class id
 */
int ridf_sz(struct ridf_hdst hd){
  return RIDF_SZ(hd.hd1);
}

/** Extract EF number from RIDFHD.
 *  @param hd structure of RIDFHD
 *  @return EF number
 */
int ridf_ef(struct ridf_hdst hd){
  return RIDF_EF(hd.hd2);
}

/** Decode RIDFHD to RIDFRHD.\n
 *  RIDFRHD is a usufull structure for RIDFHD
 *  @param hd structure of RIDFHD
 *  @return structure of RIDFRHD
 */
struct ridf_rhdst ridf_dechd(struct ridf_hdst hd){
  struct ridf_rhdst sthd;

  sthd.layer   = ridf_ly(hd);
  sthd.classid = ridf_ci(hd);
  sthd.blksize = ridf_sz(hd);
  sthd.efn     = ridf_ef(hd);

  return sthd;
}

/** Encode RIDFHD from parameters.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @return structure of RIDFHD
 */
struct ridf_hdst ridf_mkhd(int ly, int cid, int bs, int efn){
  struct ridf_hdst rethd;

  rethd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.hd2 = efn;

  return rethd;
}

/** Encode RIDFHDEOB from parameters.\n
 *  RIDFHDEOB is a structure for event header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param size size of this block
 *  @return structure of RIDFHDEOB
 */
struct ridf_hdst_eob ridf_mkhd_eob(int ly, int cid, int bs, int efn, int size){
  struct ridf_hdst_eob rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.size = size;

  return rethd;
}

/** Encode RIDFHDBLKN from parameters.\n
 *  RIDFHDBLKN is a structure for event header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param blkn size of this block
 *  @return structure of RIDFHDBLKN
 */
struct ridf_hdst_blkn ridf_mkhd_blkn(int ly, int cid, int bs, int efn, int blkn){
  struct ridf_hdst_blkn rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.blkn = blkn;

  return rethd;
}

/** Encode RIDFHDEVT from parameters.\n
 *  RIDFHDEVT is a structure for event header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param evtn event number
 *  @return structure of RIDFHDEVT
 */
struct ridf_hdst_evt ridf_mkhd_evt(int ly, int cid, int bs, int efn, int evtn){
  struct ridf_hdst_evt rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.evtn = evtn;

  return rethd;
}

/** Encode RIDFHDEVTTS from parameters.\n
 *  RIDFHDEVTTS is a structure for event header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param evtn event number
 *  @param ts Time stamp
 *  @return structure of RIDFHDEVTTS
 */
struct ridf_hdst_evtts ridf_mkhd_evtts(int ly, int cid, int bs, int efn, int evtn, long long int ts){
  struct ridf_hdst_evtts rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.evtn = evtn;
  rethd.tsu = (int)(ts >> 32) & 0x0000ffff;
  rethd.tsl = (int)ts;

  return rethd;
}

/** Encode RIDFHDSEG from parameters.\n
 *  RIDFHDSEG is a structure for segment header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param segid segment id
 *  @return structure of RIDFHDSEG
 */
struct ridf_hdst_seg ridf_mkhd_seg(int ly, int cid, int bs, int efn, int segid){
  struct ridf_hdst_seg rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.segid = segid;

  return rethd;
}


/** Encode RIDFHDSCR from parameters.\n
 *  RIDFHDSCR is a structure for scaler header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param scrid scaler id
 *  @return structure of RIDFHDSCR
 */
struct ridf_hdst_scr ridf_mkhd_scr(int ly, int cid, int bs, int efn,
				   int date, int scrid){
  struct ridf_hdst_scr rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.date = date;
  rethd.scrid = scrid;

  return rethd;
}

/** Encode RIDFHDCOM from parameters.\n
 *  RIDFHDCOM is a structure for comment header.
 *  @param ly layer
 *  @param cid class id
 *  @param bs size
 *  @param efn EF number
 *  @param comid comment id
 *  @return structure of RIDFHDCOM
 */
struct ridf_hdst_com ridf_mkhd_com(int ly, int cid, int bs, int efn,
				   int date, int comid){
  struct ridf_hdst_com rethd;

  rethd.chd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.chd.hd2 = efn;
  rethd.date = date;
  rethd.comid = comid;

  return rethd;
}
