#mpixlcxx -c route.cxx personality.cxx neighbour.cxx algorithm.cxx -DBGQ -DVESTA -DDEBUG
mpixlcxx -c route.cxx personality.cxx neighbour.cxx algorithm.cxx -DBGQ -DVESTA 
rm libalgo.a ; ar -cvq libalgo.a route.o personality.o neighbour.o algorithm.o
make clean -f Makefile.bgq
make -f Makefile.bgq
