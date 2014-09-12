#!/bin/sh

cd /home/jinnai/workspace/ethan

i=1
max=`expr $2 - 1`
rm $1*.o*


t=`expr $3`
#echo $t

#while [ $i -le $max ]
#do
#    NAME=$(printf %s%02d  $1 $i)
#    qsub -l nodes=1:ppn=8,walltime=00:25:00 -N $NAME -j oe -v arg1=$1,arg2=$i,arg3=$t  ./run.sh
#    i=`expr $i + 1`
#done
#NAME=$(printf %s%02d  $1 $2)
#qsub -l nodes=1:ppn=8,walltime=00:25:00 -N $NAME -j oe -v arg1=$1,arg2=$2,arg3=$t  ./run.sh

# Array job
JOB_ARRAY_ID=`qsub -t 1-$2 -l nodes=1:ppn=8,walltime=00:25:00 -N $1 -j oe -v arg1=$1,arg2=$t  ./run.sh`
qsub -W depend:afteranyarray:$JOB_ARRAY_ID[] ./mail.sh
