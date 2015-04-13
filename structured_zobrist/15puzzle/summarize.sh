#!/bin/bash

# walltime, speedup(r), expand, search overhead(r), load balance, node send ratio

astar=$1
hdastar=$2
length=`wc -l $astar | awk '{print $1}'`

awk -v l=$length 'NR==FNR{awtime[$1]=$2; aexpd[$1]=$3} \
NR!=FNR{wtime+=$2; spdup+=awtime[$1]/$2; expd+=$3; so+=$3/aexpd[$1]; loadbalance+=$4; send+=$5;
} \
END{printf("%f %f %d %f %f %f\n", wtime/l, spdup/l, expd/l, so/l - 1.0, loadbalance/l, send/l);}' $astar $hdastar
