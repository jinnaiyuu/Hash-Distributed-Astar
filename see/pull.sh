#!/bin/sh

#scp  jinnai@funlucy:/home/jinnai/workspace/ethan/astar/*.o* ./astar
#scp  jinnai@funlucy:/home/jinnai/workspace/ethan/pastar/*.o* ./pastar

scp  jinnai@funlucy:/home/jinnai/workspace/ethan/$1*.o* ./
ssh funlucy<<EOF
   cd /home/jinnai/workspace/ethan 
   mv $1*.o* $1
EOF
