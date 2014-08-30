#!/bin/sh

cd $PBS_O_WORKDIR



i=1

while [ $i -le 100 ]
do
    ./tiles $arg1 $i < instances
    i=`expr $i + 1`
done


