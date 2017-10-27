mpixlcxx -g -pg -c route.cxx personality.cxx neighbour.cxx algorithm.cxx -DBGQ -DVESTA 
rm libalgo.a ; ar -cvq libalgo.a route.o personality.o neighbour.o algorithm.o
make clean -f Makefile
make -f Makefile
