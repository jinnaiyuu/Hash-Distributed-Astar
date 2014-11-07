#!/bin/bash
# job.sh ALGORITHM NUMBER_OF_INSTANCES


usage(){
    cat<<EOF
put.sh is a command to throw job for the lucy server.

Usage:
  $(basename ${0}) <algorithm> <problem number> <thread number>
EOF

}

time=`date +%m%d%H%M%S`
algorithm=$1
problem_size=$2
thread_number=$3
parameter1=$4 # OS threshold,   hdastar income buffer size
parameter2=$5 # OS abstraction, hdastar outgo buffer size
parameter3=$6 # OS N/A,         abstraction type

if [ $# -lt 4 ]
then
    echo "Usage: $0 <algorithm> <problem number> <thread number> <parameter>"
    exit 0
fi

echo -n "Enter comment for the job->"
read text


SCRIPT="cd /home/yuu/workspace/ethan ; ./job_david.sh $time $algorithm $problem_size $thread_number $parameter1 $parameter2 $parameter3 $text"

cd ..
git stage src/*.hpp src/main.cc put/*.sh see/*.sh
git commit -m "Experiment: $time $algorithm $thread_number $parameter1 $parameter2 $parameter3 $text"

cd src
touch main.cc
make
cd ..
cp ./src/tiles ./src/tiles$time
scp  ./src/tiles$time ./put/job_david.sh ./put/run.sh ./src/instances ./src/big_first_instances ./src/small_first_instances ./src/random_instances yuu@david:/home/yuu/workspace/ethan/
ssh -l yuu david "${SCRIPT}"

