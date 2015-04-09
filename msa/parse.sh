#!/bin/bash

astar=$1
multi=$2

# walltime speedup expanded searchoverhead

awk 'NR==FNR&&$1=="wall_time:"{aw=$2; next} NR==FNR&&$1=="expanded:"{aexpd=$2; next}\
$1=="wall_time:"{w=$2} $1=="expanded:"{expd=$2}\
END{printf("%.1f %.3f %d %.3f\n", w, aw/w, expd, expd/aexpd)}' $astar $multi
