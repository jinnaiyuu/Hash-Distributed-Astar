#!/bin/bash
# job.sh ALGORITHM NUMBER_OF_INSTANCES

usage(){
    cat<<EOF
put.sh is a command to throw job for the lucy server.

Usage:
  $(basename ${0}) <algorithm> <problem number> <thread number>
EOF

}

t=$3
if [ $# -ne 3 ]
then
    t=1
fi




if [ $# -ne 2 ]
then 
    if [ $# -ne 3 ]
    then
	echo "Usage: $0 <algorithm> <problem number> OR"
	echo "Usage: $0 <algorithm> <problem number> <thread number>"
	exit 0
    fi
fi

echo -n "Enter comment for the job->"
read text


SCRIPT="cd /home/jinnai/workspace/ethan ; ./job.sh $1 $2 $t $text"

cd ../src
make
cd ..
scp  ./src/tiles ./put/job.sh ./put/run.sh ./put/mail.sh ./src/instances jinnai@funlucy:/home/jinnai/workspace/ethan/
ssh -l jinnai funlucy "${SCRIPT}"

