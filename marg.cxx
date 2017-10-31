
#include <mpi.h>
#include "marg.h"

void modifyInfo(MPI_File fileHandle)
{

  MPI_Info info;
 // char *value = new char[MAXCBNODESLEN];
  char key[MPI_MAX_INFO_KEY];
  
  char keycb[] = "cb_nodes";
  char value[MPI_MAX_INFO_VAL];
  
  int flag;

  MPI_File_get_info (fileHandle, &info);
  MPI_Info_get(info, keycb, MPI_MAX_INFO_VAL, value, &flag);
  if (!flag) puts ("cb_nodes not defined"); 
  else printf("cb value %s\n", value);

/*
  int exists, nkeys;
  MPI_Info_get_nkeys(info, &nkeys);
  printf("%3d %d\n", rank, nkeys);

  for (int i = 0; i < nkeys; i++) {
    MPI_Info_get_nthkey(info, i, key);
    MPI_Info_get(info, key, MPI_MAX_INFO_VAL, value, &exists);
    if (exists)
      printf("%3d: key = %s, value = %s\n", rank, key, value);
  }*/
  
  MPI_Info_free(&info);

}


