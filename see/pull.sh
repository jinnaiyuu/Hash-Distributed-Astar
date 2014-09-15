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
scp  jinnai@funlucy:/home/jinnai/workspace/ethan/$1*.o$2-* ./
ssh funlucy<<EOF
   cd /home/jinnai/workspace/ethan 
   mv $1*.o$2-* $1
EOF
