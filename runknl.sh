#!/bin/bash -evx
####COBALT -q flat-quad
#COBALT -t 03:00:00
####COBALT -A Performance ###EarlyPerf_theta 
#####COBALT -M pmalakar@anl.gov

#SBATCH -p debug
#SBATCH -t 00:20:00
#SBATCH -J my_job
#SBATCH -S 2
#SBATCH -C knl,quad,cache    ####haswell
##SBATCH -o my_job.o%j

EXE1=marg-v1
EXE2=marg-orig
EXE=marg

#module unload darshan perftools

#confirm using hugepages
export HUGETLB_VERBOSE=3

#export LD_PRELOAD=${DARSHAN_PRELOAD}

#io tracing
export DXT_ENABLE_IO_TRACE=4

#mpich_abort_on_rw_wrror 
export ATP_ENABLED=1

export MPICH_MPIIO_HINTS_DISPLAY=1
export MPICH_MPIIO_AGGREGATOR_PLACEMENT_DISPLAY=1 
export MPICH_MPIIO_STATS=1
export MPICH_MPIIO_XSTATS=1
export MPICH_MPIIO_TIMERS=1
export MPICH_MPIIO_ABORT_ON_RW_ERROR=enable
export MPICH_MEMORY_REPORT=1

export MPICH_STATS_DISPLAY=1
export MPICH_CPUMASK_DISPLAY=1
export MPICH_RANK_REORDER_DISPLAY=1
export MPICH_RANK_REORDER_DISPLAY=true
export MPICH_CPUMASK_DISPLAY=true
export KMP_AFFINITY=verbose
export MPICH_VERSION_DISPLAY=1
export MPICH_NEMESIS_ASYNC_PROGRESS=1
export MPICH_ENV_DISPLAY=1


if [[ "$HOST" == *"theta"* ]]; then
  echo "theta"
  echo "Running $COBALT_JOBID on $COBALT_PARTNAME."
  nodes=$COBALT_PARTSIZE
  jobid=${COBALT_JOBID}
  locfile=loc_${nodes}_${jobid}.txt
  echo ${COBALT_PARTNAME} > $locfile 
  jobmapfile=jobmap_${nodes}_${jobid}.txt
  python parsejobnodes.py theta $locfile > $jobmapfile
elif [[ "$HOST" == *"cori"* ]]; then
  echo "cori"
  echo "Running $SLURM_JOBID on $SLURM_JOB_NODELIST"
  nodes=$SLURM_JOB_NUM_NODES
  jobid=${SLURM_JOBID}
  locfile=loc_${nodes}_${jobid}.txt
  echo ${SLURM_NODELIST} > $locfile 
fi

if [[ "$HOST" == *"theta"* ]]; then
  aprun -n 1 -N 1 -d 1 -j 1 -r 1 ./location.x
elif [[ "$HOST" == *"cori"* ]]; then
  srun -n 1 -N 1 ./location.x
fi

THREADS=1
#export PAT_RT_SUMMARY=0

startiter=1 #$1
enditer=$(($startiter+0))

ENVVARS="" #"--env MPICH_MPIIO_HINTS "*:cb_nodes=4""
#--env MPICH_ENV_DISPLAY=1 --env MPICH_VERSION_DISPLAY=1 --env MPICH_SMP_SINGLE_COPY_SIZE=1024 --env MPICH_NEMESIS_ASYNC_PROGRESS=1 --env MPICH_SHARED_MEM_COLL_OPT=1 --env MPICH_GNI_ASYNC_PROGRESS_STATS=enabled -r 1"

for PROB in $EXE1 $EXE2
do
  
EXE="./"$PROB
echo $EXE

for iter in `seq $startiter $enditer` 
do

 for ppn in 2 #4 #64 ##8 16 32 64
 do

  RANKS=$((${nodes}*$ppn))

	echo 

  if [[ "$HOST" == *"theta"* ]]; then
  APRUNPARAMS=" -n ${RANKS} -N ${ppn} -d 1 -j 1 -r 1 " #--attrs mcdram=cache:numa=quad "
  fi

  for SC in 48 #32 #52 # 32 # STRIPE COUNT 
  do 
   for SZ in 16M # 32M #32M #8M #2M  #STRIPE SIZE
   do
    for agg in 16 #64  multiplier
    do 
    for bufsize in 16777216 #67108864 
    do 
     for size in 16 # 64 #256 1024 # 64 128 256 512 1024 2048 #4096 #8192 4096 
     do
     for collective in 0  # 1 # 0 - independent
     do
     for blocking in 0 #1
     do

        if [ $collective -eq 1 ]; then
	
				aggnodes=$((${agg}*${ppn}))
        echo $aggnodes

        #rm MPICH_RANK_ORDER

       # export MPICH_MPIIO_HINTS="*:cray_cb_nodes_multiplier=${agg}"
       # export MPICH_MPIIO_HINTS="*:cb_buffer_size=${bufsize}:cray_cb_nodes_multiplier=${agg}"
        #export MPICH_MPIIO_HINTS="*:cb_nodes=${agg}:cb_buffer_size=${bufsize}:cray_cb_nodes_multiplier=2"
        export MPICH_MPIIO_HINTS="*:cb_nodes=${aggnodes}" #:cray_cb_nodes_multiplier=${agg}"

        fi

        OUTPUT=output_${PROB}_${nodes}_${RANKS}_R${ppn}_${SC}_${SZ}_${aggnodes}_${bufsize}_${size}_${collective}_${blocking}_${iter}_${jobid}
        export DARSHAN_LOGFILE=${OUTPUT}.darshan

        ARG=" $size ${collective} ${blocking}"
        #ARG=" $size 0 0 1 0"

	      echo "Starting $OUTPUT with $ARG"
        #mkdir pat_${OUTPUT}
        #export PAT_RT_EXPFILE_DIR=pat_${OUTPUT}

        FNAME=TestFile-${RANKS}
        rm -f $FNAME 2>/dev/null
        echo "Testing: echo $FNAME"
        lfs setstripe -c ${SC} -S ${SZ} $FNAME
        echo "Testing done: echo $FNAME"
        lfs getstripe $FNAME

        if [[ "$HOST" == *"theta"* ]]; then
          APRUNPARAMS=" -n ${RANKS} -N ${ppn} -d 1 -j 1 -r 1 " #--attrs mcdram=cache:numa=quad "
 		      xtnodestat > xtnodestat.start.${OUTPUT}
		      qstat -f > q.start.${OUTPUT}
			    #aprun ${ENVVARS} ${APRUNPARAMS} ${EXE} ${ARG} > ${OUTPUT}
			    aprun ${ENVVARS} ${APRUNPARAMS} -e LD_PRELOAD=${DARSHAN_PRELOAD} ${EXE} ${ARG} > ${OUTPUT}
		      qstat -f > q.end.${OUTPUT}
 			    xtnodestat > xtnodestat.end.${OUTPUT}
        elif [[ "$HOST" == *"cori"* ]]; then
		      squeue -l > q.start.${OUTPUT}
	        srun ${ENVVARS} -n ${RANKS} -N ${nodes} --cpu_bind=verbose,cores $EXE ${ARG} > ${OUTPUT}
	        #srun $ENVVARS -n ${RANKS} -N ${nodes} --cpu_bind=verbose,cores -c ${num_logical_cores} $EXE ${ARG} > $OUTPUT
		      squeue -l > q.end.${OUTPUT}
        fi

      done
      done
      done
      done
     done
    done
   done
  done

	echo 
	echo "* * * * *"
	echo 

 done
done

EXIT_STATUS=$?
echo "Job $COBALT_JOBID completed."
exit $EXIT_STATUS
