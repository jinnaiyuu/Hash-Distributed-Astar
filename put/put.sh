#!/bin/sh
# job.sh ALGORITHM NUMBER_OF_INSTANCES

t=$3
if [ $# -ne 3 ]
then
    t=1
fi

SCRIPT="cd /home/jinnai/workspace/ethan ; ./job.sh $1 $2 $t"



if [ $# -ne 2 ]
then 
    if [ $# -ne 3 ]
    then
	echo "Usage: $0 <algorithm> <problem number> OR"
	echo "Usage: $0 <algorithm> <problem number> <thread number>"
	exit 0
    fi
fi

cd ../src
make
cd ..
scp  ./src/tiles ./put/job.sh ./put/run.sh ./put/mail.sh ./src/instances jinnai@funlucy:/home/jinnai/workspace/ethan/
ssh -l jinnai funlucy "${SCRIPT}"

