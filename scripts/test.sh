#!/bin/bash

../src/strips.out astar ../pddl/blocks/domain.pddl ../pddl/blocks/probBLOCKS-4-0.pddl h-1 > tmp
../src/strips.out astar ../pddl/blocks/domain.pddl ../pddl/blocks/probBLOCKS-4-1.pddl h-1 > tmp2
../src/strips.out astar ../pddl/gripper/domain.pddl ../pddl/gripper/prob01.pddl h-1 > tmp3

d1="blocks"
i1="probBLOCKS-4-0.pddl"
d2="blocks"
i2="probBLOCKS-4-1.pddl"
d3="gripper"
i3="prob01.pddl"

d4="b"
i4="b"
d5="c"
i5="c"
d6="a"
i6="a"
d7="d"
i7="d"

cat tmp3 | awk -v dom=$d3 -v ins=$i3 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' > summary
cat tmp | awk -v dom=$d1 -v ins=$i1 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary
cat tmp2 | awk -v dom=$d2 -v ins=$i2 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary
cat tmp2 | awk -v dom=$d4 -v ins=$i4 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary
cat tmp2 | awk -v dom=$d5 -v ins=$i5 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary
cat tmp2 | awk -v dom=$d6 -v ins=$i6 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary
cat tmp2 | awk -v dom=$d7 -v ins=$i7 'BEGIN{solved=0; stage=0} \
             /analyzing balances of predicates/{stage=1} \
             /generating PDB/{stage=2} \
             /#pair  initial heuristic/{stage=3} \

             /#pair  algorithm/{alg=$3} \
             /#pair  total wall time/{wtime=$5; stage=4} \
             /#pair  total nodes expanded/{expd=$5; if (expd>0){solved=1}} \
             /failed to find a plan/{solved=0} \
             END{printf("%s %s %f %d %d %d\n", dom, ins, wtime, expd, solved, stage)}' >> summary


cat summary | ./print.sh $1
#platex summary_print.tex
#dvipdfmx summary_print.dvi
