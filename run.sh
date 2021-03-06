#!/bin/bash -x
#COBALT --disable_preboot
 
#export L1P_POLICY=std
#export BG_THREADLAYOUT=1   # 1 - default next core first; 2 - my core first

#Free bootable blocks
boot-block --reboot
 
NODES=$1
startiter=$2

EXE1=indep
EXE2=marg.tau

TAUVARS=" tau_exec -T ompt,pdt,mpi,papi -ebs "

for PROG in ${EXE2} # ${EXE1}
do
for iter in $startiter 
do
for THRD in 4 #2 4 8 16
do
for ppn in 1  
do
for MSG in 16 # 1024 2048 # 4096 8192
do 
 for collective in 0 #1 # for type in 0 1 2 
 do
 for blocking in 1 #1 
 do

  #rm -f dummy*

	RANKS=`echo "$NODES*$ppn"|bc`
	OUTPUT=${PROG}_N${NODES}_R${ppn}_${iter}_${MSG}_${collective}_${blocking} #_${type}_${streams}

	#rm -f ${OUTPUT}.cobaltlog ${OUTPUT}.output ${OUTPUT}.error

	echo 
	echo "* * * * *"
	echo 
	echo "Starting $OUTPUT with numthreads=$THRD ppn=$ppn args=${MSG} ${collective} ${blocking} " # ${type} ${streams}"

	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs PAMID_ASYNC_PROGRESS=1 --envs "OMP_MAX_NUM_THREADS=${THRD}" : ${PROG} ${MSG} ${collective} ${blocking} > ${OUTPUT}


  runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs PAMID_ASYNC_PROGRESS=1 --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs PAMID_RZV_LOCAL=4M --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" --envs PAMI_MEMORY_OPTIMIZED=1 --envs PAMID_COLLECTIVES=1 --envs PAMID_CONTEXT_POST=1 --envs TAU_TRACK_MEMORY_FOOTPRINT=1 --envs TAU_TRACK_UNIFIED_MEMORY=1 --envs TAU_CALLPATH=1 --envs TAU_THROTTLE=0 --envs TAU_COMM_MATRIX=1 : ${PROG} ${MSG} ${collective} ${blocking} > ${OUTPUT}


  #runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs PAMID_ASYNC_PROGRESS=1 --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs PAMID_RZV_LOCAL=4M --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" --envs PAMI_MEMORY_OPTIMIZED=1 --envs PAMID_COLLECTIVES=1 --envs PAMID_CONTEXT_POST=1 : ${PROG} ${MSG} ${collective} ${blocking} > ${OUTPUT}


#  mv TestFile-512 TestFile-512-${OUTPUT}

	continue;


 	rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV_LOCAL=4M --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_rzvlocal_4M
 	rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "PAMID_RZV_LOCAL=4M" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_local_4M
 	rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV_LOCAL=4M MUSPI_RECFIFOSIZE=2097152 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_rec_2M_rzvlocal_4M



	continue;



 	rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV_LOCAL=8M --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_rzvlocal_8M
 	rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV_LOCAL=16M --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_rzvlocal_16M

	if [ $MSG -gt 128 ] && [ $streams -gt 0 ] 
	then
    rm -f dummy*
	  runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=8 MUSPI_NUMRECFIFOS=8 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_8
    rm -f dummy*
	  runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_RECFIFOSIZE=8388608" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_recfifo_8M
  	#rm -f dummy*
	#	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_INJFIFOSIZE=16777216" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_injfifo_16M
  	#rm -f dummy*
	#	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_RECFIFOSIZE=16777216" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_recfifo_16M
	fi

  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=4 MUSPI_NUMRECFIFOS=4 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_4
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs PAMID_RZV_LOCAL=2M MUSPI_RECFIFOSIZE=2097152 --envs PAMID_STATISTICS=1 --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_local_2M_recfifo_2M
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs PAMID_RZV_LOCAL=4M MUSPI_RECFIFOSIZE=2097152 --envs PAMID_STATISTICS=1 --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_local_4M_recfifo_2M
 	rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV_LOCAL=4M MUSPI_RECFIFOSIZE=4194304 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_rec_4M_rzvlocal_4M

	if [ $MSG -gt 64 ] 
	then
			continue;
	fi

  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_RECFIFOSIZE=2097152" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_recfifo_2M
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_INJFIFOSIZE=2097152" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_injfifo_2M

	#streams > 1
	if [ $streams -gt 1 ] 
	then
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_RECFIFOSIZE=4194304" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_recfifo_4M
	fi

	if [ $MSG -gt 4 ] 
	then
			continue;
	fi

  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_RECFIFOSIZE=2097152 MUSPI_INJFIFOSIZE=2097152 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_injrec_2M
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "PAMID_RZV=4M" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_remote_4M
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_INJFIFOSIZE=4194304" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_injfifo_4M
  rm -f dummy*
	runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "PAMID_RZV_LOCAL=4M" --envs "PAMID_RZV=4M" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_4M

  #rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "PAMID_THREAD_MULTIPLE=1" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_ptm
  #rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "MUSPI_INJFIFOSIZE=8388608" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_injfifo_8M

	#if [ $MSG -gt 32 ] 
	#then
	#		continue;
	#fi
	
  #rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_RECFIFOSIZE=4194304 MUSPI_INJFIFOSIZE=4194304 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_injrec_4M
  #rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs PAMID_RZV_LOCAL=4M PAMID_RZV=4M MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_4M_numfifos2
  #rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs "PAMID_RZV=8M" --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_rzv_remote_8M
 	#rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV=4M PAMID_RZV_LOCAL=4M MUSPI_INJFIFOSIZE=4194304 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_inj_4M_rzv_4M
 	#rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=2 MUSPI_NUMRECFIFOS=2 PAMID_RZV=8M PAMID_RZV_LOCAL=8M MUSPI_INJFIFOSIZE=4194304 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_2_inj_4M_rzv_8M

 	#rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=4 MUSPI_NUMRECFIFOS=4 PAMID_RZV=4M PAMID_RZV_LOCAL=4M MUSPI_INJFIFOSIZE=4194304 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_4_inj_4M_rzv_4M
 	#rm -f dummy*
	#runjob --np $RANKS -p $ppn --block $COBALT_PARTNAME --verbose=INFO --envs "OMP_MAX_NUM_THREADS=${THRD}" --envs MUSPI_NUMINJFIFOS=4 MUSPI_NUMRECFIFOS=4 PAMID_RZV=8M PAMID_RZV_LOCAL=8M MUSPI_INJFIFOSIZE=4194304 --envs "PAMID_STATISTICS=1" --envs "PAMID_VERBOSE=1" : ${PROG} ${MSG} ${coalesced} ${blocking} ${type} ${streams} > ${OUTPUT}_numfifos_4_inj_4M_rzv_8M


	echo 
	echo "* * * * *"
	echo


done 
done 
done 
done 
done 
done 
done

exit

