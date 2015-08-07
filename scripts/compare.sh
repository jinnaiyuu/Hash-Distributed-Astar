#!/bin/bash

##########################
### Run run_all.sh with all combination of parameters.
##########################
##
## This script is to compare the performance of planner 
## with different parameters. 
##
##
##

# -a hdastar-N: algorithms
# -s abst-N:    structure
# -h h-N:       heuristic
# -p pdb-N:     pdb

alg="hdastar-1"
structure="abst-0"
heuristic="h-1"
pdb="pdb-0"
runtime="5m"
dom=""

while getopts "i:t:d:a:s:h:p:" opt; do
    case $opt in
	a) # alg
	        alg="$OPTARG"
		    ;;
	s) # structured zobrist hash
	        structure="$OPTARG"
		    ;;
	h) # heuristic
	        heuristic="$OPTARG"
		    ;;
	p) # pdb
	        pdb="$OPTARG"
		    ;;
	i) # number of instances to run
	        i=$OPTARG
		    ;;
	t) # time to run each instance
	        runtime="$OPTARG"
		    ;;
	d) # domains to run
	        dom="$OPTARG"
		    ;;
	\?)
	        ;;
    esac
done

#echo $alg
for a in $alg
do
    for h in $heuristic
    do
	for s in $structure
	do
	    for p in $pdb
	    do
	    ./run_all.sh -a $a -s "$s" -h "$h" -p "$p" -i "$i" -t "$runtime" -d "$dom"
	    done
	done
    done
done
