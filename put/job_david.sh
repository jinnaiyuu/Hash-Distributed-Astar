#!/bin/sh

# Input
# <time> <algorithm> <problem number> <thread number> <outsourcing parameter> <comment for the job>


cd /home/yuu/workspace/ethan

time=$1
algname=$2
problem_size=$3
thread_number=$4
param1=$5
param2=$6
if [ $# -eq 7 ]
then
    comment=$7
else 
    param3=$7
    comment=$8
fi

mem="32gbmem"

# Array job

# TODO: Name the job with a bunch of parameters.
# Parameters to put on.
# algorithm.threadnumber.corenumber.memorysize.parameterforalgorithm.parametertoanalyze

# For HDASTAR example
if [ "$algname" = "hdastar" ]
then
    paramname1="incomebufsize"
    paramname2=".outgobufsize"
    paramname3=".abstraction"
else
    paramname1="osdiff"
    paramname2=".abstraction"
fi


JOB_ARRAY_ID=`qsub -t 1-$problem_size -l nodes=1:ppn=8,walltime=00:40:00 -N $algname.${thread_number}threads.${mem}.${paramname1}${param1}${paramname2}${param2}${paramname3}${param3}.${time} -j oe -v arg1=$time,arg2=$algname,arg3=$thread_number,arg4=$param1,arg5=$param2,arg6=$param3  ./run.sh`

JOB_NUMBER=`echo $JOB_ARRAY_ID | awk -F "." '{print $1}'` 

echo $time $JOB_NUMBER $algname $problem_size $thread_number $mem $param1 $param2 $comment  >> job_list.dat

# Not working. Not sure how to fix it.
#qsub -W depend:afterokarray:$JOB_ARRAY_ID[] -M ddyuudd@gmail.com -m ae ./mail.sh

