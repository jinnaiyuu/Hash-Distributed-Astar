#!/bin/sh
# USAGE
# job.sh ALGORITHM NUMBER_OF_INSTANCES
SCRIPT="cd /home/jinnai/workspace/ethan ; ./job.sh $1 $2"

if [ $# -ne 2 ]
then 
    echo "Usage: $0 <Algorithm> <Number>"
fi


cd src
make
cd ..
scp  ./src/tiles job.sh run.sh ./src/instances jinnai@funlucy:/home/jinnai/workspace/ethan/

ssh -l jinnai funlucy "${SCRIPT}"