#!/bin/sh

# Input
# <time> <algorithm> <problem number> <thread number> <outsourcing parameter> <comment for the job>


cd /home/yuu/workspace/ethan

time=$1
algname=$2
problem_size=$3
thread_number=1
if [ $# -ge 3 ]
then
    thread_number=$4
fi
osparam=$5
comment=$6

# Array job

# TODO: Name the job with a bunch of parameters.
# Parameters to put on.
# algorithm.threadnumber.corenumber.memorysize.parameterforalgorithm.parametertoanalyze

# For HDASTAR example
if [ "$algname" = "hdastar" ]
then
    paramname="bufmaxsize"
else
    paramname="osdiff"
fi


JOB_ARRAY_ID=`qsub -t 1-$problem_size -l nodes=1:ppn=8,walltime=00:40:00 -N $algname.${thread_number}threads.16gbmem.${paramname}${osparam} -j oe -v arg1=$time,arg2=$algname,arg3=$thread_number,arg4=$osparam  ./run.sh`

JOB_NUMBER=`echo $JOB_ARRAY_ID | awk -F "." '{print $1}'` 

echo $JOB_NUMBER $algname $problem_size $thread_number "16gb" $osparam $comment  >> job_list.dat

# Not working. Not sure how to fix it.
#qsub -W depend:afterokarray:$JOB_ARRAY_ID[] -M ddyuudd@gmail.com -m ae ./mail.sh

