#ifndef __algorithm__
#define __algorithm__

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

//EXTERNC 
#ifdef BGQ
void init_bgq(int, char **);

void fini_bgq ();
#endif

#endif
