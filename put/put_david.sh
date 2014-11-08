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
problem_type=$3
thread_number=$4
parameter1=$5 # OS threshold,   hdastar income buffer size
parameter2=$6 # OS abstraction, hdastar outgo buffer size
parameter3=$7 # OS N/A,         abstraction type

if [ $# -lt 4 ]
then
    echo "Usage: $0 <algorithm> <problem number> <thread number> <parameter>"
    exit 0
fi

echo -n "Enter comment for the job->"
read text


SCRIPT="cd /home/yuu/workspace/ethan ; ./job_david.sh $time $algorithm $problem_size $problem_type $thread_number $parameter1 $parameter2 $parameter3 $text"

cd ..
git stage src/*.hpp src/main.cc put/*.sh see/*.sh
git commit -m "Experiment: $time $algorithm $problem_type $thread_number $parameter1 $parameter2 $parameter3 $text"

cd src
touch main.cc
make
cd ..
cp ./src/tiles ./src/tiles$time
scp  ./src/tiles$time ./put/job_david.sh ./put/run.sh ./src/instances ./src/difficult_instances ./src/easy_instances yuu@david:/home/yuu/workspace/ethan/
ssh -l yuu david "${SCRIPT}"

