#include <mpi.h>
#include <stdlib.h>
#include "marg.h"

char infoval[2][MPI_MAX_INFO_VAL];

/*
int MPI_File_open()
{
  if (myrank < 2) printf("new open function executed\n");
  return PMPI_File_open(); 
}
*/

void modifyInfo(MPI_File fileHandle)
//void modifyInfo(MPI_Info info)
{
  int i;

  MPI_Info info;
  char *keyname[2] = { "cb_nodes", "cb_buffer_size" };
  char keyvalue[2][MPI_MAX_INFO_VAL];
  
  //sprintf (infoval[cb_nodes], "%d", num);

  int flag;

  MPI_File_get_info (fileHandle, &info);
  MPI_Info_get(info, keyname[cb_nodes], MPI_MAX_INFO_VAL, keyvalue[cb_nodes], &flag);
//  if (!flag) puts ("cb_nodes not defined"); 
//  else {
   // printf("cb value before %s\n", keyvalue[cb_nodes]);
   // MPI_Info_set (info, "cb_nodes", infoval[cb_nodes]); 
   // MPI_Info_set( info, "cb_buffer_size", "67108864");  //64 MB
//    MPI_Info_get(info, keyname[cb_nodes], MPI_MAX_INFO_VAL, keyvalue[cb_nodes], &flag);
//    MPI_Info_get(info, keyname[cb_buffer_list], MPI_MAX_INFO_VAL, keyvalue[cb_buffer_list], &flag);
    //printf("cb value after %s %s\n", keyvalue[cb_nodes], keyvalue[cb_buffer_list]);

    /* Disables ROMIO's data-sieving */
 //   MPI_Info_set(info, "romio_ds_read", "disable");
 //   MPI_Info_set(info, "romio_ds_write", "disable");
 
    /* Enable ROMIO's collective buffering */
   // MPI_Info_set(info, "romio_cb_read", "enable");
   // MPI_Info_set(info, "romio_cb_write", "enable");
//  }

  int exists, nkeys;
  char key[MPI_MAX_INFO_KEY];
  char value[MPI_MAX_INFO_VAL];
  MPI_Info_get_nkeys(info, &nkeys);
 // printf("nkeys %d\n", nkeys);

  for (i = 0; i < nkeys; i++) {
    MPI_Info_get_nthkey(info, i, key);
    MPI_Info_get(info, key, MPI_MAX_INFO_VAL, value, &exists);
    if (exists)
      printf("key = %s, value = %s\n", key, value);
  }
 
//when its not fileHandle 
  MPI_Info_free(&info);

}


