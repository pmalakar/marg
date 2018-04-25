#ifndef __coalesce__
#define __coalesce__

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

extern int numnodes;

void init_knl(int, char **) ; 

#endif
