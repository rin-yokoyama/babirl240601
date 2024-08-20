/* lib/bi-ridf.c
 * last modified : 06/11/14 17:07:58 
 * 
 * Hidetada Baba (RIKEN)
 * baba@ridf.riken.jp
 *
 * Libraly for RIDF
 *
 */

#include <ridf.h>

struct stridfrhd ridf_mkhd(int ly, int efn, int cid, int bs){
  struct stridfrhd rethd;

  rethd.hd1 = RIDF_MKHD1(ly, cid, bs);
  rethd.hd2 = efn;

  return rethd;
}

int ridf_ly(struct stridfrhd hd){
  return RIDF_LY(hd.hd1);
}
int ridf_ad(struct stridfrhd hd){
  return RIDF_AD(hd.hd2);
}
int ridf_ci(struct stridfrhd hd){
  return RIDF_CI(hd.hd1);
}
int ridf_sz(struct stridfrhd hd){
  return RIDF_SZ(hd.hd1);
}

