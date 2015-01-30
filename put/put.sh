#!/bin/bash
# job.sh ALGORITHM NUMBER_OF_INSTANCES


usage(){
    cat<<EOF
put.sh is a command to throw job for the lucy server.

Usage:
  $(basename ${0}) <instances name> <algorithm> <problem number> <thread number>
EOF

}

time=`date +%m%d%H%M%S`
#time="0129235248"

#instance_name=$1
#domain_name=$2
#algorithm=$3
#problem_size=$4
#thread_number=$5
#parameter1=$6 # OS threshold,   hdastar income buffer size
#parameter2=$7 # OS abstraction, hdastar outgo buffer size
#parameter3=$8 # OS N/A,         abstraction type


if [ $# -lt 4 ]
then
    echo "Usage: $0 <algorithm> <problem number> <thread number> <parameter>"
    exit 0
fi

echo -n "Enter comment for the job->"
read comment


#SCRIPT="cd /home/yuu/workspace/ethan ; ./job_david.sh $time $algorithm $problem_size $problem_type $thread_number $parameter1 $parameter2 $parameter3 $text"
SCRIPT="cd /home/yuu/workspace/ethan ; ./job_david.sh $time $comment $@"

cd ..
git stage src/*.hpp src/main.cc put/*.sh see/*.sh
#git commit -m "Experiment: $time $algorithm $problem_type $thread_number $parameter1 $parameter2 $parameter3 $text"
git commit -m "Experiment: $time $1 $2 $3 $4 $5 $6 $7 $8 $9 $text"

cd src
#touch main.cc
make
cd ..
cp ./src/tiles ./src/tiles$time
scp  ./src/tiles$time ./put/job_david.sh ./put/run.sh ./src/*instances yuu@david.rm:/home/yuu/workspace/ethan/
ssh -l yuu david.rm "${SCRIPT}"

