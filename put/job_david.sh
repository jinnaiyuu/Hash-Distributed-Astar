#!/bin/sh

# Input
# <time> <algorithm> <problem number> <thread number> <outsourcing parameter> <comment for the job>


cd /home/yuu/workspace/ethan

time=$1
comment=$2
problem_type=$3

domain_name=$4
algname=$5
problem_size=$6
thread_number=$7
param1=$8
param2=$9
param3=$8

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
fi


JOB_ARRAY_ID=`qsub -t 1-$problem_size -l nodes=1:ppn=8,walltime=01:20:00 -N $algname.${problem_type}.${thread_number}threads.${mem}.${paramname1}${param1}${paramname2}${param2}${paramname3}${param3}.${time} -j oe -v time=$time,domain_name=$domain_name,algname=$algname,thread_number=$thread_number,problem_type=$problem_type,param1=$param1,param2=$param2,param3=$param3 ./run.sh`

JOB_NUMBER=`echo $JOB_ARRAY_ID | awk -F "." '{print $1}'` 

echo $time $JOB_NUMBER $algname $problem_size $problem_type $thread_number $mem $param1 $param2 $comment  >> job_list.dat

# Not working. Not sure how to fix it.
#qsub -W depend:afterokarray:$JOB_ARRAY_ID[] -M ddyuudd@gmail.com -m ae ./mail.sh

