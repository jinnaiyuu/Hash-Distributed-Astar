#!/bin/sh
# job.sh ALGORITHM NUMBER_OF_INSTANCES
SCRIPT="cd /home/jinnai/workspace/ethan ; ./job.sh $1 $2 $3"



if [ $# -ne 2 ]
then 
    if [ $# -ne 3 ]
    then
	echo "Usage: $0 <algorithm> <problem number> OR"
	echo "Usage: $0 <algorithm> <problem number> <thread number>"
    fi
fi

cd ../src
make
cd ..
scp  ./src/tiles ./put/job.sh ./put/run.sh ./src/instances jinnai@funlucy:/home/jinnai/workspace/ethan/
ssh -l jinnai funlucy "${SCRIPT}"

