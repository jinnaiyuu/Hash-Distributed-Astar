#!/bin/bash

# 1: algorithm
# 2: domain file
# 3: instance file
# 4: heuristic (h-X)
# 5: abst      (abst-X)
# 6: pdb       (pdb-X)

# 7: output file directory

d=`echo $2 | awk -F / '{print $3}'`
i=`echo $3 | awk -F / '{print $4}'`

#echo "$d $i"

filename=`echo "$1$d$i$4$5$6"`

#./strips.out $1 $2 $3 $4 $5 $6 >> $7/$filename


# TODO: where did the program died?
#    1. when parsing
#    2. when analyzing the balance of predicates, key=/analyzing balances of predicates/
#    3. when generating PDB, key=/generating PDB/
#    4. when searching , key=/#pair  initial heuristic/

cat $7/$filename | awk -v dom=$d -v ins=$i 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> $7/summary

cat $7/$filename

