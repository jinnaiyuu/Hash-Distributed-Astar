#!/bin/sh

# 

cd /home/jinnai/workspace/ethan

i=1
max=`expr $2 - 1`

t=1
if [ $# -ge 3 ]
then
    t=`expr $3`
fi

# Array job
JOB_ARRAY_ID=`qsub -t 1-$2 -l nodes=1:ppn=8,walltime=02:00:00 -N $1$t -j oe -v arg1=$1,arg2=$t  ./run.sh`

JOB_NUMBER=`echo $JOB_ARRAY_ID | awk -F "." '{print $1}'` 

echo $JOB_NUMBER $1 $t $4 >> job_list.dat

# Not working. Not sure how to fix it.
#qsub -W depend:afterokarray:$JOB_ARRAY_ID[] -M ddyuudd@gmail.com -m ae ./mail.sh

