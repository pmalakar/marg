#common
CFLAGS=-O2 -g 

ve=$(findstring vesta, ${HOSTNAME})
ce=$(findstring cetus, ${HOSTNAME})
mi=$(findstring mira,  ${HOSTNAME})
co=$(findstring cori,  ${HOSTNAME})
th=$(findstring theta, ${HOSTNAME})

ifeq ($(ve), vesta)
  $(info $(ve))
  DEFINES += -DBGQ -DVESTA
  CC=mpixlc
  CXX=mpixlcxx
  CFLAGS+=-qsmp=omp -g -pg 
  INC += -I/projects/Performance/preeti/utils	
  LIBALGO = -L./ -lalgo
  LIBS += $(LIBALGO) 
else ifeq ($(ce), cetus)
  DEFINES += -DBGQ -DCETUS
  CC=mpixlc
  CXX=mpixlcxx
  CFLAGS+=-O2 -g -qmaxmem=-1 -qflag=w -pg
  INC += -I/projects/Performance/preeti/utils	
  LIBALGO = -L./ -lalgo
  LIBS += $(LIBALGO) 
else ifeq ($(mi), mira)
  DEFINES += -DBGQ -DCETUS
  CC=mpixlc
  CXX=mpixlcxx
  CFLAGS+=-qmaxmem=-1 
  INC += -I/projects/Performance/preeti/utils	
  LIBALGO = -L./ -lalgo
  LIBS += $(LIBALGO) 
else ifeq ($(co), cori)
  $(info $(co))
  CC=cc
  CXX=CC
  DEFINES += -DKNL -DCORI 
  LIBLINFO=-L/global/cscratch1/sd/preeti/work/systest/theta/lnet -llinfo 
  INC += -I/global/cscratch1/sd/preeti/work/systest/theta/lnet
else ifeq ($(th), theta)
  CC=cc
  CXX=CC
  DEFINES += -DKNL -DTHETA 
  LIBLINFO=-L/projects/Performance/preeti/work/systest/theta/lnet -llinfo 
  INC += -I/projects/Performance/preeti/work/systest/theta/lnet
endif 

DEFINES += -DDEBUG  

LIBHPM = -L/soft/perftools/hpctw/lib -lmpihpm 
LIBBGPM = -L/bgsys/drivers/ppcfloor/bgpm/lib -lbgpm -lrt -lstdc++ 
LIBMPITRACE =-L/soft/perftools/hpctw/lib -lmpitrace

LIBUTILS =-L/projects/Performance/preeti/utils -lbgqutils 

SRCS = contiguous.cxx	

OBJS = 	$(SRCS:.cxx=.o)

TARGET = marg

all:    $(TARGET)
		@echo Compilation done.

%.o:%.cxx
		$(CXX) $(CFLAGS) -c $< -o $@ $(INC) $(LIBS) $(DEFINES)

$(TARGET): $(OBJS) 
		$(CXX) $(CFLAGS) -o $(TARGET) $(OBJS) $(INC) $(LIBS) $(DEFINES)   

clean:
		$(RM) *.o *~ $(TARGET)

