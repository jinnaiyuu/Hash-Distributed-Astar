#!/bin/sh

cd /home/jinnai/workspace/ethan

i=1

while [ $i -le 100 ]
do
    NAME=astar$i
    qsub -l nodes=1:ppn=1,walltime=00:15:00 -M ddyuudd@yahoo.co.jp -m ae -N $NAME -j oe -v arg1=$i  ./run.sh
    i=`expr $i + 1`
done