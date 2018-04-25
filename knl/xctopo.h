#ifndef __xctopo__
#define __xctopo__

typedef struct xctopo_s
{
    int col;
    int row;
    int cage;
    int slot;
    int anode;
}
xctopo_t;

int xctopo_get_mycoords(xctopo_t *);

#endif
