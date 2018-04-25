#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

#include "common.h"
#include "marg.h"

#ifdef KNL
#include "lustreinfo.h"
#include "coalesce.h"
#include "knl/location.h"
#else
#include "algorithm.h"
#endif

#define NAME_LENGTH 256
#define MAXCBNODESLEN 128

int oneKB=1024;

int rank, size;
int count, mode;
int collective;
int blocking;

double wFS, rFS;

MPI_File fileHandle;
MPI_Status status;
MPI_Request request;

char *fileNameION = "/dev/null";
char *fileNameSSD = "/local/scratch/dummyFile";
char *fileNameFS = "dummyFile";
char *fileNameFSBN = "dummyFileBN";
char *fileNameFSCO = "dummyFileCO";
char testFileName[NAME_LENGTH];

int SKIP = 2;
int MAXTIMES = 5;
int MAXTIMESD = 5;

//void startup_() ;
void cleanup_() ;

int writeFile(dataBlock *datum, int count) 
{
  int result, nbytes;
	MPI_Request request;
  MPI_Barrier (MPI_COMM_WORLD);  //sync up all

  if (collective == 0) {
	 if (blocking == 1) {
    //printf ("writing %d doubles\n", count);
		result = MPI_File_write_at (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &status);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_write_at error:");
	 }
	 else {
		MPIO_Request req_iw;
		MPI_Status st_iw;
		result = MPI_File_iwrite_at (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &req_iw);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_iwrite_at error:");
    if (coreID == noderoot) 
		MPI_Wait(&req_iw, &st_iw);
	 }
  }
  else if (collective == 1) {
	 if (blocking == 1) {
		result = MPI_File_write_at_all (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &status);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_write_at_all error:");
	 }
	 else {
		MPIO_Request req_iw;
		MPI_Status st_iw;
		result = MPI_File_iwrite_at_all (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &req_iw);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_iwrite_at_all error:");
		MPI_Wait(&req_iw, &st_iw);
	 }
  }

	MPI_Get_elements( &status, MPI_CHAR, &nbytes );

	return nbytes;
}

void file_write(dataBlock *datum) {

  int totalBytes = 0;
  double tIOStart, tIOEnd;

#ifdef BGQ
	/*
	 * * * * * * * * * Independent MPI-IO to IO nodes from all compute nodes - shared file * * * * * * * *
   */
/*
  MPI_File_open (MPI_COMM_WORLD, fileNameION, mode, MPI_INFO_NULL, &fileHandle);
  for (int i=1; i<=SKIP; i++)
   totalBytes += writeFile(datum, count);
  tIOStart = MPI_Wtime();
  for (int i=1; i<=MAXTIMES; i++)
   totalBytes += writeFile(datum, count);
  tIOEnd = MPI_Wtime();
  tION = (tIOEnd - tIOStart)/MAXTIMES;
  MPI_File_close (&fileHandle);
*/
#endif
#ifdef KNL
	/*
	 * * * * * * * * * Independent MPI-IO to SSD from all compute nodes - shared file * * * * * * * *
   */
  /*MPI_File_open (MPI_COMM_WORLD, fileNameSSD, mode, MPI_INFO_NULL, &fileHandle);
  for (int i=1; i<=SKIP; i++)
   totalBytes += writeFile(datum, count);
  tIOStart = MPI_Wtime();
  for (int i=1; i<=MAXTIMES; i++)
   totalBytes += writeFile(datum, count);
  tIOEnd = MPI_Wtime();
  //tION = (tIOEnd - tIOStart)/MAXTIMES;
  MPI_File_close (&fileHandle);
*/
#endif

	/*
	 * * * * * * * * * Independent MPI-IO to file system from all compute nodes - shared file * * * * * * * *
	 */
  //MPI_File_open (MPI_COMM_WORLD, fileNameFS, mode, MPI_INFO_NULL, &fileHandle);
  MPI_File_open (MPI_COMM_WORLD, testFileName, mode, MPI_INFO_NULL, &fileHandle);

  if (rank == 0) {
    MPI_Info info;
    MPI_File_get_info (fileHandle, &info);
#ifdef KNL
    modifyInfo(info); //fileHandle);
#endif
  }
#ifdef KNL
  //get_file_info(testFileName); //undefined error
#endif 

	for (int i=1; i<=SKIP; i++)
	 totalBytes += writeFile(datum, count);
	tIOStart = MPI_Wtime();
	for (int i=1; i<=MAXTIMESD; i++)
	 totalBytes += writeFile(datum, count);
	tIOEnd = MPI_Wtime();
  wFS = (tIOEnd - tIOStart)/MAXTIMESD;
	MPI_File_close (&fileHandle);

}

int readFile(dataBlock *datum, int count) 
{
  int result, nbytes;

	MPI_Request request;

  if (collective == 0) {
	 if (blocking == 1) {
		result = MPI_File_read_at (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &status);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_read_at error:");
	 }
	 else {
		MPIO_Request req_iw;
		MPI_Status st_iw;
		result = MPI_File_iread_at (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &req_iw);
		MPI_Wait(&req_iw, &st_iw);
	 }
  }
  else if (collective == 1) {
	 if (blocking == 1) {
		result = MPI_File_read_at_all (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &status);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_read_at_all error:");
	 }
	 else {
		MPIO_Request req_iw;
		MPI_Status st_iw;
		result = MPI_File_iread_at_all (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &req_iw);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_iread_at_all error:");
		MPI_Wait(&req_iw, &st_iw);
	 }
  }

	MPI_Get_elements( &status, MPI_CHAR, &nbytes );

	return nbytes;
}

void file_read(dataBlock *datum) {

  int totalBytes = 0;
  double tIOStart, tIOEnd;

#ifdef BGQ
   // something
  puts(" ");
#endif

  MPI_File_open (MPI_COMM_WORLD, testFileName, mode, MPI_INFO_NULL, &fileHandle);

#ifdef KNL
  //get_file_info(testFileName);
#endif

	for (int i=1; i<=SKIP; i++)
	 totalBytes += readFile(datum, count);
	tIOStart = MPI_Wtime();
	for (int i=1; i<=MAXTIMESD; i++)
	 totalBytes += readFile(datum, count);
	tIOEnd = MPI_Wtime();
  rFS = (tIOEnd - tIOStart)/MAXTIMESD;
	MPI_File_close (&fileHandle);

}

int main(int argc, char *argv[]) {

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  sprintf(testFileName,"TestFile-%d",size);

#ifdef BGQ
  if (!collective) init_bgq(argc, argv);
#endif

#ifdef KNL
  //if (!collective) 
  init_knl(argc, argv);
#endif

  count = atoi(argv[1]) * oneKB;
  collective = atoi(argv[2]);
  blocking = atoi(argv[3]);

  //MPI_Barrier(MPI_COMM_WORLD);

	/* allocate buffer */
  dataBlock *datum = new dataBlock(count);
	datum->allocElement ();

  assert (datum->getAlphaBuffer() != NULL);

  /* set file open mode */
  mode = MPI_MODE_CREATE | MPI_MODE_RDWR; //WRONLY;

  rFS = 0, wFS = 0;

  file_write(datum);
  file_read(datum);

  cleanup_();

#ifdef BGQ
  if (!collective) fini_bgq();
#endif

  MPI_Finalize();

}

void cleanup_() {

  double max_rFS, max_wFS;

	MPI_Reduce(&rFS, &max_rFS, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&wFS, &max_wFS, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  if (rank == 0)
  printf ("0: Times: %d | %6.2f KB | %d %d | %6.4lf %6.4lf\n", 
     size, 8.0*count/1024.0, collective, blocking, max_rFS, max_wFS);

}

