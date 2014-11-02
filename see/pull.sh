#!/bin/sh

# Get data files for 15 Puzzle from server.
# $1 <algorithm> 
# $2 <Job id> (optional)

if [ $# -ne 2 ]
then
    echo "Usage: $0 <algorithm> <job id>"
    echo "Designed for see.sh"
    exit 1
fi
scp  yuu@david:/home/yuu/workspace/ethan/$1*.o$2-* ./
scp  yuu@david:/home/yuu/workspace/ethan/job_list.dat ./
ssh david<<EOF
   cd /home/yuu/workspace/ethan 
   mv $1*.o$2-* $1
EOF
