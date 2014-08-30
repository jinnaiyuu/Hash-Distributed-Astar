#!/bin/sh

cd /home/jinnai/workspace/ethan
NAME=pastar_all

qsub -l nodes=1:ppn=8,walltime=04:00:00,mem=14gb -M ddyuudd@gmail.com -m ae -N $NAME -j oe -v arg1=$1  ./allrun.sh
