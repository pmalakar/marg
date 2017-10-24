#ifndef __route__
#define __route__

#include <stdio.h>
#include <stdint.h>
#include <mpi.h>
#ifdef BGQ
#include "hwi/include/bqc/nd_500_dcr.h"
#endif

int getRoutingOrder(int *);

#endif
