#!/bin/sh

cd /home/jinnai/workspace/ethan

i=1
max=`expr $2 - 1`
rm $1*.o*


t=`expr $3`
echo $t

while [ $i -le $max ]
do
    NAME=$(printf %s%02d  $1 $i)
    qsub -l nodes=1:ppn=$t,walltime=00:25:00,mem=14gb -M ddyuudd@gmail.com -N $NAME -j oe -v arg1=$1,arg2=$i,arg3=$t  ./run.sh
    i=`expr $i + 1`
done
    NAME=$(printf %s%02d  $1 $2)
    qsub -l nodes=1:ppn=$t,walltime=00:25:00,mem=14gb -M ddyuudd@gmail.com -m ae -N $NAME -j oe -v arg1=$1,arg2=$2,arg3=$t  ./run.sh
