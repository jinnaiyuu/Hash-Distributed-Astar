#!/bin/bash
time=`date +%m%d%H%M%S`

make tiles_mpi
#cp ./tiles ./tiles$time

scp  ./tiles_mpi jinnai@funlucy:/home/jinnai/workspace/mpi/src

exit 

scp  ../src/strips.out jinnai@funlucy:/home/jinnai/workspace/strips/src/
scp  ../src/run.sh jinnai@funlucy:/home/jinnai/workspace/strips/src/
scp  ../src/summarize.sh jinnai@funlucy:/home/jinnai/workspace/strips/src/
scp  ../scripts/*.sh jinnai@funlucy:/home/jinnai/workspace/strips/scripts/
