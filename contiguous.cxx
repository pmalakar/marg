#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>
#include "common.h"

#include "marg.h"

#ifdef KNL
#include "lustreinfo.h"
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

double tION, tFS;

MPI_File fileHandle;
MPI_Status status;
MPI_Request request;

char *fileNameION = "/dev/null";
char *fileNameFS = "dummyFile";
char *fileNameFSBN = "dummyFileBN";
char *fileNameFSCO = "dummyFileCO";
char testFileName[NAME_LENGTH];

int SKIP = 1;
int MAXTIMES = 10;
int MAXTIMESD = 10;

//void startup_() ;
void cleanup_() ;

int writeFile(dataBlock *datum, int count) 
{
  int result, nbytes;

	MPI_Request request;

  if (collective == 0) {
	 if (blocking == 1) {
		result = MPI_File_write_at (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &status);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_write_at error:");
	 }
	 else {
		MPIO_Request req_iw;
		MPI_Status st_iw;
		result = MPI_File_iwrite_at (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &req_iw);
//		if (result != MPI_SUCCESS) 
	//		prnerror (result, "MPI_File_iwrite_at error:");
	//	MPI_Wait(&req_iw, &st_iw);
	 }
  }
  else if (collective == 1) {
	 if (blocking == 1) {
		result = MPI_File_write_at_all (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &status);
		if (result != MPI_SUCCESS) 
			prnerror (result, "MPI_File_write_at_all error:");
	 }
//test MPI version 
/*
	 else {
		MPIO_Request req_iw;
		MPI_Status st_iw;
		result = MPI_File_iwrite_at_all (fileHandle, (MPI_Offset)rank*count*sizeof(double), datum->getAlphaBuffer(), count, MPI_DOUBLE, &req_iw);
		MPI_Wait(&req_iw, &st_iw);
	 }*/
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
	/*
	 * * * * * * * * * Independent MPI-IO to file system from all compute nodes - shared file * * * * * * * *
	 */
  //MPI_File_open (MPI_COMM_WORLD, fileNameFS, mode, MPI_INFO_NULL, &fileHandle);
  MPI_File_open (MPI_COMM_WORLD, testFileName, mode, MPI_INFO_NULL, &fileHandle);

  modifyInfo(fileHandle);

#ifdef KNL
  //get_file_info(testFileName); //undefined error
#endif 

	for (int i=1; i<=SKIP; i++)
	 totalBytes += writeFile(datum, count);
	tIOStart = MPI_Wtime();
	for (int i=1; i<=MAXTIMESD; i++)
	 totalBytes += writeFile(datum, count);
	tIOEnd = MPI_Wtime();
  tFS = (tIOEnd - tIOStart)/MAXTIMESD;
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
  tFS = (tIOEnd - tIOStart)/MAXTIMESD;
	MPI_File_close (&fileHandle);

}

int main(int argc, char *argv[]) {

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  sprintf(testFileName,"TestFile-%d",size);

#ifndef KNL
  if (!collective) init_bgq(argc, argv);
#endif

  count = atoi(argv[1]) * oneKB;
  collective = atoi(argv[2]);
  blocking = atoi(argv[3]);

  MPI_Barrier(MPI_COMM_WORLD);

	/* allocate buffer */
  dataBlock *datum = new dataBlock(count);
	datum->allocElement ();

  assert (datum->getAlphaBuffer() != NULL);

  /* set file open mode */
  mode = MPI_MODE_CREATE | MPI_MODE_RDWR; //WRONLY;

  tION = 0, tFS = 0;

  file_write(datum);
//  file_read(datum);

  cleanup_();

#ifndef KNL
  if (!collective) fini_bgq();
#endif

  MPI_Finalize();

}


void cleanup_() {

  double max_tION, max_tFS;

	MPI_Reduce(&tION, &max_tION, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&tFS, &max_tFS, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  if (rank == 0)
  printf ("0: Times: %d | %6.2f KB | %d %d | %4.2lf %4.2lf\n", size, 8.0*count/1024.0, collective, blocking, max_tION, max_tFS);

}

