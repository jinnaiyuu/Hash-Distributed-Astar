#!/bin/sh

cd /home/jinnai/workspace/ethan

i=1

while [ $i -le 100 ]
do
    NAME=pastar$i
    qsub -l nodes=1:ppn=2,walltime=00:15:00 -M ddyuudd@gmail.com -m ae -N $NAME -j oe -v arg1=$i  ./run.sh
    i=`expr $i + 1`
done