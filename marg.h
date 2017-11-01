#ifndef __marg__
#define __marg__

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

enum finfokeys { cb_nodes, cb_buffer_list };
extern char infoval[2][MPI_MAX_INFO_VAL];

void modifyInfo(MPI_File);

#endif

