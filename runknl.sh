#!/bin/bash -evx
####COBALT -q flat-quad
#COBALT -t 30
#COBALT -A Performance ###EarlyPerf_theta 

#SBATCH -p debug
#SBATCH -t 00:20:00
#SBATCH -J my_job
#SBATCH -S 2
#SBATCH -C knl,quad,cache    ####haswell
##SBATCH -o my_job.o%j

EXE=./marg


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

export MPICH_CPUMASK_DISPLAY=1
export MPICH_RANK_REORDER_DISPLAY=1
export MPICH_RANK_REORDER_DISPLAY=true
export MPICH_CPUMASK_DISPLAY=true
export KMP_AFFINITY=verbose
export MPICH_VERSION_DISPLAY=1
export MPICH_NEMESIS_ASYNC_PROGRESS=1
export MPICH_ENV_DISPLAY=1
export MPICH_MPIIO_HINTS="*:cb_nodes=2"


if [[ "$HOST" == *"theta"* ]]; then
  echo "theta"
  echo "Running $COBALT_JOBID on $COBALT_PARTNAME."
  nodes=$COBALT_PARTSIZE
  jobid=${COBALT_JOBID}
  locfile=loc_${nodes}_${jobid}.txt
  echo ${COBALT_PARTNAME} > $locfile 
  jobmapfile=jobmap_${nodes}_${jobid}.txt
  #python parsejobnodes.py theta.computenodes $locfile > $jobmapfile
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

startiter=$1
enditer=$(($startiter+1))

ENVVARS="" #"--env MPICH_MPIIO_HINTS "*:cb_nodes=4""
#--env MPICH_ENV_DISPLAY=1 --env MPICH_VERSION_DISPLAY=1 --env MPICH_SMP_SINGLE_COPY_SIZE=1024 --env MPICH_NEMESIS_ASYNC_PROGRESS=1 --env MPICH_SHARED_MEM_COLL_OPT=1 --env MPICH_GNI_ASYNC_PROGRESS_STATS=enabled -r 1"

for iter in `seq $startiter $enditer` 
do
 for ppn in 64
 do

  RANKS=$((${nodes}*$ppn))

	echo 

  if [[ "$HOST" == *"theta"* ]]; then
  APRUNPARAMS=" -n ${RANKS} -N ${ppn} -d 1 -j 1 -r 1 " #--attrs mcdram=cache:numa=quad "
  fi

  for STRIPECNT in 16 
  do 
   for STRIPESZ in 8M #2M  
   do
    for size in 32 1024 4096
    do
     for collective in 0 1
     do
      for blocking in 1 #0
      do

        OUTPUT=output_${nodes}_${RANKS}_R${ppn}_${STRIPECNT}_${STRIPESZ}_${size}_${collective}_${iter}_${jobid}

        ARG=" $size ${collective} ${blocking}"
        #ARG=" $size 0 0 1 0"

	      echo "Starting $OUTPUT with $ARG"
        #mkdir pat_${OUTPUT}
        #export PAT_RT_EXPFILE_DIR=pat_${OUTPUT}

        FNAME=TestFile-${RANKS}
        rm -f $FNAME 2>/dev/null
        echo "Testing: echo $FNAME"
        lfs setstripe -c ${STRIPECNT} -S ${STRIPESZ} $FNAME
        echo "Testing done: echo $FNAME"
        lfs getstripe $FNAME

        if [[ "$HOST" == *"theta"* ]]; then
          APRUNPARAMS=" -n ${RANKS} -N ${ppn} -d 1 -j 1 -r 1 " #--attrs mcdram=cache:numa=quad "
 		      xtnodestat > xtnodestat.start.${OUTPUT}
		      qstat -f > q.start.${OUTPUT}
			    aprun ${ENVVARS} ${APRUNPARAMS} ${EXE} ${ARG} > ${OUTPUT}
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

	echo 
	echo "* * * * *"
	echo 

 done
done

EXIT_STATUS=$?
echo "Job $COBALT_JOBID completed."
exit $EXIT_STATUS
