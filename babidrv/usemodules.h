// CAMAC modules
/// Nativ CAMAC
#ifdef CAMAC
#include "babirldrvcamac.c"
#endif
/// CCNET
#ifdef CCNET
#include "ccnetevt.c"
#endif

// VME modules
/// Universe
#ifdef UNIV
#include "univfunc.c"
#endif
/// CAEN V775/785/792, V1190/1290
#ifdef USE_CAEN
#include "babirldrvcaen.c"
#endif

#ifdef USE_RPV130
#include "rpv130.c"
#endif

#ifdef USE_MADC32
#include "madc32.c"
#endif

#ifdef USE_SIS3300
#include "sis3300.c"
#endif
