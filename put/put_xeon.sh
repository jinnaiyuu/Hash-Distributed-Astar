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
computer=$5
if [ $computer == "" ]
then
    computer="xeon8"
fi

if [ $# -ne 4 ]
then
    echo "Usage: $0 <algorithm> <problem number> <thread number> <parameter>"
    exit 0
fi

echo -n "Enter comment for the job->"
read text


SCRIPT="cd /home/jinnai/workspace/ethan ; ./job_xeon.sh $time $algorithm $problem_size $thread_number $os_parameter $text"

cd ../src
make
cd ..
cp ./src/tiles ./src/tiles$time
scp  ./src/tiles$time ./put/job.sh ./put/run.sh ./put/mail.sh ./src/instances ./src/big_first_instances ./src/small_first_instances jinnai@${computer}:/home/jinnai/workspace/ethan/
ssh -l jinnai ${computer} "${SCRIPT}"

