#!/bin/bash

date=`date +%m%d%H%M`
#cd ../src

../src/pstrips.out hdastar-1 ../pddl/blocks/domain.pddl ../pddl/blocks/probBLOCKS-9-0.pddl h-3
gprof ../src/pstrips.out gmon.out > ../results/prof_${date}.txt
