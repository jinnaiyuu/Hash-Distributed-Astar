#!/bin/bash

../src/strips.out astar ../pddl/blocks/domain.pddl ../pddl/blocks/probBLOCKS-4-0.pddl h-1 > tmp
../src/strips.out astar ../pddl/blocks/domain.pddl ../pddl/blocks/probBLOCKS-4-1.pddl h-1 > tmp2

d="blocks"
i="probBLOCKS-4-0.pddl"

cat tmp | awk -v dom=$d -v ins=$i 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' > summary
cat tmp2 | awk -v dom=$d -v ins=$i 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary


cat summary | ./print.sh > summary_print
