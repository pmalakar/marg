#ifndef __location__
#define __location__

extern int worldrank, worldsize, subrank; 
extern int ppn, coreID, nodeID, myroot, noderoot;
extern int nid;  // physical cnode
extern char pname[64];

int getNodeInfo (void);

#endif
