/*
 *
 * Author: Preeti Malakar
 * Affiliation: Argonne National Laboratory
 * Email: pmalakar@anl.gov
 *
 */

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include <queue>
#include <unistd.h>
#include <inttypes.h>

#include <omp.h>

#include "coalesce.h"
#include "knl/location.h"

using namespace std;

/* * * * Variables * * * */

const int MAX_BUFSIZE=33554432;
int numnodes;

MPI_Comm MPI_COMM_core, MPI_COMM_node;

/* * * * *  End Variables * * * */

/* * * Functions * * */

int modifyWrite (MPI_File fh, MPI_Offset offset, const void *buf,
              int count, MPI_Datatype datatype, MPI_Status *status, MPIO_Request *request, int blocking)
{

  int ret=0, result, idx;
  MPI_Status stat; 
  MPI_Request req[ppn], reqdata[ppn];

  int *senderinfo = new int[2];
  senderinfo[0] = offset;  senderinfo[1] = count; 

  // only relevant at the receiver (i.e. noderoot)
  int info[ppn*2] ; //][2];

  // sum of all counts 
  int countAll = 0;

  int index=0;

  //if (datatype == MPI_DOUBLE) //FIXME based on datatype
  double *aggNodeData; //[ppn][MAX_BUFSIZE]; 
 // double *aggNodeData[ppn]; //[ppn][MAX_BUFSIZE]; 
  //aggNodeData = (double **) malloc (ppn * sizeof(double *));
//#pragma omp parallel for
  //for (int i=0; i<ppn; i++) { 
  //  aggNodeData[i] = (double *) malloc (MAX_BUFSIZE * sizeof(double));
  //  assert(aggNodeData[i]);
  //}

#ifdef DEBUG
  //if (coreID != noderoot) 
  //  printf("\n%d (%d, %d) will send (%u %d) to %d (%d)\n", worldrank, subrank, nodeID, senderinfo[0], senderinfo[1], myroot, noderoot);
#endif

#ifdef DEBUG
double start = MPI_Wtime();
#endif

  //gather at myroot : //MPI_Gather is slightly slower than send recv
//  MPI_Gather (senderinfo, 2, MPI_INT, info, 2, MPI_INT, noderoot, MPI_COMM_node);

  // send offset count and data
  if (coreID != noderoot) {
    MPI_Send (senderinfo, 2, MPI_INT, noderoot, subrank, MPI_COMM_node) ; //, &sendreq);
    MPI_Send (buf, count, datatype, noderoot, subrank, MPI_COMM_node) ; //, &sendreqdata);
  }
  else {

    //populate own info
    info[coreID*2] = offset;  info[coreID*2+1] = count;

    int iter=-1;
    for (int i=0; i<ppn ; i++) { 
      if (i == noderoot) continue;
      iter++;
      //receive offset and count at the node root
      MPI_Irecv (info+i*2, 2, MPI_INT, i, i, MPI_COMM_node, &req[iter]); 
    }
    MPI_Waitall (ppn-1, req, MPI_STATUSES_IGNORE);

    // sum of counts from all cores
    for (int i=0; i<ppn; i++)  
      countAll += info[i*2+1];

    //printf("\nAllocating %d bytes\n", countAll * sizeof(double));
    aggNodeData = (double *) malloc (countAll * sizeof(double));
    assert(aggNodeData);

/*
    for (int i=0; i<ppn; i++) { 
      aggNodeData[i] = (double *) malloc (info[i*2+1] * sizeof(double));
      assert(aggNodeData[i]);
    }
*/
// copy own data
//    aggNodeData[coreID] = (double *)buf;
//    aggNodeData+coreID = buf;

    iter=-1;
    for (int i=0; i<ppn ; i++) { 
      if (i == noderoot) {
         //printf("%d Recv for (root) i=%d %d elements at %d\n", worldrank, i, info[i*2+1], index);
         for (int j=0; j<count; j++) *(aggNodeData+index+j) = ((double *)buf)[j]; 
        // for (int j=0; j<count; j++) printf ("%lf ", *(aggNodeData+index+j));

         index += info[i*2+1]; // * sizeof(double);
			   continue;
      }
      iter++;
     // MPI_Waitany (ppn-1, req, &idx, &stat);      
     // printf("%d receiving %d bytes from %d[%d]\n", worldrank, info[idx*2+1], idx, idx*2+1);
     // MPI_Irecv (aggNodeData[idx], info[idx*2+1], datatype, idx, idx, MPI_COMM_node, &reqdata[iter]); 
     // printf("%d receiving %d bytes from %d\n", worldrank, info[i*2+1], i);
     // MPI_Irecv (aggNodeData+i, info[i*2+1], datatype, i, i, MPI_COMM_node, &reqdata[iter]); 
     // MPI_Irecv (aggNodeData[i], info[i*2+1], datatype, i, i, MPI_COMM_node, &reqdata[iter]); 

    //receive the actual data from idx proc
      MPI_Irecv (aggNodeData+index, info[i*2+1], datatype, i, i, MPI_COMM_node, &reqdata[iter]); 
      //MPI_Recv (aggNodeData+index, info[i*2+1], datatype, i, i, MPI_COMM_node, MPI_STATUS_IGNORE); //, &reqdata[iter]); 
      //printf("\n%d Recv for i=%d %d elements at %d\n", worldrank, i, info[i*2+1], index);
      index += info[i*2+1]; // * sizeof(double);
    }
    MPI_Waitall (ppn-1, reqdata, MPI_STATUSES_IGNORE);
  }

#ifdef DEBUG
double end = MPI_Wtime();
#endif

  if (coreID == noderoot) {

#ifdef DEBUG
  //printf ("%d gather time %lf\n", worldrank, end-start);
#endif

   int next = info[0];
   int size = datatype == MPI_DOUBLE ? sizeof(double) : 4;  //FIXME based on correct datatype

/*
   // check for contiguity in offsets
   for (idx=0; idx<ppn ; idx++) { 
     if (next != info[idx*2]) 
     { cout << "Error: non-contiguous data is not handled at the moment " << next << endl; break; }
     printf ("\nNext %d %d %u %u %d\n", worldrank, idx, next, info[idx*2], info[idx*2+1]);
#ifdef DEBUG
  //   printf ("\nNext %d %d %u %u %d\n", worldrank, idx, next, info[idx*2], info[idx*2+1]);
#endif
     next = info[idx*2] + info[idx*2+1]*size; 
   }
*/

  // Using 1 stream
    //printf ("Root %d has offset 0 %u total count %d\n", worldrank, info[0], countAll);

//FIXME info[0] is int, needs Offset
   if (blocking == 1)
    ret = PMPI_File_write_at(fh, offset, aggNodeData, countAll, datatype, status);
    //ret = PMPI_File_write_at(fh, (MPI_Offset)info[0], aggNodeData, countAll, datatype, status);
   else if (blocking == 0) { 
    ret = PMPI_File_iwrite_at(fh, offset, aggNodeData, countAll, datatype, request);
   }

  //if (idx<ppn) return -1;
  }

 // if (blocking == 0)  
 //    MPI_Bcast (request, 1, MPI_INT, 0, MPI_COMM_node); 
     //MPI_Bcast (request, 1, MPIO_Request, 0, MPI_COMM_node); 
  
 // printf ("\nI %d have offset %u count %d\n", worldrank, offset, count);
//  ret = PMPI_File_write_at(fh, offset, buf, count, datatype, status);

  return ret;
}

/* * * Intercepting Functions * * */

int MPI_File_open(MPI_Comm comm, char *filename, int amode,
                      MPI_Info info, MPI_File *fh)
{
  //printf("new open function executed\n");
  return PMPI_File_open(comm, filename, amode, info, fh);
}

int MPI_File_write_at(MPI_File fh, MPI_Offset offset, const void *buf,
                      int count, MPI_Datatype datatype, MPI_Status *status)
{
  MPIO_Request *request; // extraneous to keep func simple
  int ret = modifyWrite(fh, offset, buf, count, datatype, status, request, 1);
  return ret; 
  //return PMPI_File_write_at(fh, offset, buf, count, datatype, status);
}

int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset, const void *buf,
                      int count, MPI_Datatype datatype, MPIO_Request *request)
{
  MPI_Status *status; // extraneous to keep func simple
  //MPI_Barrier (MPI_COMM_WORLD);
  //return 0;
  int ret = modifyWrite(fh, offset, buf, count, datatype, status, request, 0);
  return ret; 
  //return PMPI_File_iwrite_at(fh, offset, buf, count, datatype, request);
}

int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                          int count, MPI_Datatype datatype, 
                          MPI_Status *status)
{
  if (worldrank < 6 && worldrank > 3) 
    cout << worldrank << " returning from " << __FUNCTION__ << "\n";

  return PMPI_File_write_at_all(fh, offset, buf, count, datatype, status);
}

int MPI_File_iwrite_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                          int count, MPI_Datatype datatype, 
                          MPI_Request *request)
{
  if (worldrank < 6 && worldrank > 3) 
    cout << worldrank << " returning from " << __FUNCTION__ << "\n";

  return PMPI_File_iwrite_at_all(fh, offset, buf, count, datatype, request);
}

void init_knl(int argc, char **argv) { 

  double tIOStart, tIOEnd;
  int required=3, provided;

  MPI_Comm_rank (MPI_COMM_WORLD, &worldrank);
  MPI_Comm_size (MPI_COMM_WORLD, &worldsize);
  
  //get coreID and ppn
  getNodeInfo();

  //form inter-communicator - mainly reqd for core 0 processes 
  MPI_Comm_split (MPI_COMM_WORLD, coreID, worldrank, &MPI_COMM_core);
  MPI_Comm_size (MPI_COMM_core, &numnodes);

  //form intra-communicator - mainly reqd for processes on a node
  MPI_Comm_split (MPI_COMM_WORLD, nodeID, worldrank, &MPI_COMM_node);

  return;
}

