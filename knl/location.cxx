#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
#include <sched.h>

#include "location.h"
#include "xctopo.h"

#define MAXNODES 1024 //for the time being

int worldrank, worldsize, subrank;
int ppn, coreID, nodeID, myroot, noderoot;
int nid;  // physical cnode
int numcentroids;
char pname[64];

void formGroups() {

  int node[MAXNODES];

  xctopo_t topo;
  int rc = xctopo_get_mycoords(&topo);

  int col   = topo.col;
  int row   = topo.row;
  int cage  = topo.cage;
  int slot  = topo.slot;
  int anode = topo.anode;

  //printf("%d (%d) %d %d %d %d %d %d\n", worldrank, coreID, nid, col, row, cage, slot, anode);

  //findCrucialNodes();
 
  FILE *fp = fopen("centroid", "r");
  fscanf(fp, "%d", &numcentroids);
  for (int i=0; i<numcentroids; i++) {
     fscanf(fp, "%d", &node[i]);
  } 
  fclose(fp);
  
#if 0
  if (worldrank == 0)
  for (int i=0; i<numcentroids; i++) 
     printf("%d ", node[i]);
#endif

}

int getNodeInfo () {

  int rc;
  MPI_Comm subcomm;

  MPI_Get_processor_name(pname, &rc);
  nid = atoi(&pname[3]);
  coreID = sched_getcpu();
  MPI_Comm_split(MPI_COMM_WORLD, nid, worldrank, &subcomm); 
  MPI_Comm_rank(subcomm, &subrank);
  MPI_Comm_size(subcomm, &ppn);
  nodeID = worldrank/ppn;
  //noderoot = ppn/2; // mid core 
  noderoot = 0; // 0th process on each node
  myroot = nodeID*ppn + noderoot; //world rank of my root

  //printf("pname of %d %s : nidname %d: %d: %d (%d) of %d root %d\n", 
  //   worldrank, pname, nid, nodeID, coreID, subrank, ppn, noderoot);
   
  formGroups();
  
  return 0;

}
