#!/bin/bash
# job.sh ALGORITHM NUMBER_OF_INSTANCES

time=`date +%m%d%k%M%S`

usage(){
    cat<<EOF
put.sh is a command to throw job for the lucy server.

Usage:
  $(basename ${0}) <algorithm> <problem number> <thread number>
EOF

}


algorithm=$1
problem_size=$2
thread_number=$3
os_parameter=$4


if [ $# -lt 2 ]
then 
    if [ $# -gt 4 ]
    then
	echo "Usage: $0 <algorithm> <problem number> OR"
	echo "Multithreaded Algorithms"
	echo "Usage: $0 <algorithm> <problem number> <thread number>"
	echo "Job Outsourcing"
	echo "Usage: $0 <algorithm> <problem number> <thread number> <outsourcing parameter>"
	exit 0
    fi
fi

echo -n "Enter comment for the job->"
read text


SCRIPT="cd /home/jinnai/workspace/ethan ; ./job.sh $time $algorithm $problem_size $thread_number $os_parameter $text"

cd ../src
make
cd ..
cp ./src/tiles ./src/tiles$time
scp  ./src/tiles$time ./put/job.sh ./put/run.sh ./put/mail.sh ./src/instances ./src/big_first_instances ./src/small_first_instances jinnai@funlucy:/home/jinnai/workspace/ethan/
ssh -l jinnai funlucy "${SCRIPT}"

